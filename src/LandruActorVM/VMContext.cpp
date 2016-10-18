//
//  VMContext.cpp
//  Landru
//
//  Created by Nick Porcino on 11/5/14.
//
//

#include "VMContext.h"

#include "Exception.h"
#include "FnContext.h"
#include "LandruActorVM/Fiber.h"
#include "Library.h"
#include <list>
#include <map>
#include <queue>
#include <set>
#include <tuple>

using namespace std;

namespace Landru {

    class TimeOut {
    public:
    };

	typedef vector<Instruction> __VectorInstruction;
    typedef tuple< float, float, shared_ptr<Fiber>, __VectorInstruction, int> TimeoutTuple;
    enum class TimeOutTupleNames {
        Time = 0, DeltaTime, Fiber, Instructions, Recurrence
    };

    class CompareTimeout {
    public:
        bool operator()(const TimeoutTuple& t1, const TimeoutTuple& t2) {
            return get<0>(t2) < get<0>(t1);
        }
    };
    
    typedef vector<tuple< shared_ptr<Fiber>, __VectorInstruction >> MessageQueue;
    
    class VMContext::Detail {
    public:
        Detail() : now(0) {}
        
        double now;
        
        map<Id, shared_ptr<Fiber>> fibers;
        
        set<Id> pendingMessages;
        map<Id, MessageQueue> messageQueue;

        priority_queue<TimeoutTuple, deque<TimeoutTuple>, CompareTimeout> timeoutQueue;
    };
    
    VMContext::VMContext()
    : _detail(new Detail())
    , libs(new Library("std"))
    , activateMeta(false)
    , breakPoint(~0) {
    }
    
    VMContext::~VMContext() {
    }
    
    std::shared_ptr<Wires::TypedData> VMContext::getLibraryInstanceData(const std::string& name) {
        if (libs->name == name) {
            return libs->libraryInstanceData;
        }
        for (auto& i : libs->libraries) {
            if (i.name == name) {
                return i.libraryInstanceData;
            }
        }
        return std::shared_ptr<Wires::TypedData>();
    }

    bool VMContext::deferredMessagesPending() const {
        return !_detail->timeoutQueue.empty();
    }
    bool VMContext::undeferredMessagesPending() const {
        return !_detail->pendingMessages.empty();
    }

    void VMContext::instantiateLibs() {
        for (auto r : requireDefinitions)
            if (requires.find(r.first) == requires.end())
                requires[r.first] = new Property(*this, r.first, r.second);
    }
    
    void VMContext::update(double now) {
        _detail->now = now;
        
        // launch all machines that were requested
        while (!launchQueue.empty()) {
            auto rec = launchQueue.front();
            launchQueue.pop_front();
            auto m = machineDefinitions.find(rec.first);
            if (m != machineDefinitions.end()) {
                std::shared_ptr<Landru::Fiber> f = std::make_shared<Landru::Fiber>(m->second, *this);
                _detail->fibers[f->id()] = f;
                _detail->messageQueue[f->id()] = vector<tuple< shared_ptr<Fiber>, std::vector<Instruction> >>();   //// @TODO emplace queues only when needed at first access
                try {
                    FnContext fn(this, f.get(), nullptr, nullptr);
                    f->gotoState(fn, "main");
                }
                catch(const std::exception& e) {
                    cerr << e.what() << endl;
                    auto i = _detail->fibers.find(f->id());
                    if (i != _detail->fibers.end())
                        _detail->fibers.erase(i);
                }
            }
            else {
                VM_RAISE("machine " << rec.first << " not found to launch");
            }
        }
        
        // check all timeouts and allow them to fire if they've expired
        while (!_detail->timeoutQueue.empty()) {
            auto& testMe = _detail->timeoutQueue.top();
            if (get<0>(testMe) > now) {
                break;
            }
            auto i = _detail->timeoutQueue.top();
            _detail->timeoutQueue.pop();
            float t = get<0>(i);
            if (t <= now) {
                shared_ptr<Fiber> fiberPtr = get<2>(i);
                Fiber *fiber = fiberPtr.get();
                auto& statements = get<3>(i);
                FnContext fn = {this, fiber, nullptr, nullptr};
                if (activateMeta)
                    for (auto& s : statements) {
                        s.second.exec(fn);
                        s.first(fn);
                    }
                else
                    for (auto& s : statements) {
                        s.first(fn);
                    }

                int recurrence = get<4>(i);
                if (recurrence > 1 || recurrence < 0) {
                    int newRecurrence = recurrence > 1 ? recurrence - 1 : -1;
                    float delay = get<1>(i);
                    _detail->timeoutQueue.emplace(make_tuple(t + delay, delay, fiberPtr, statements, newRecurrence));
                }
            }
        }
        
        // finally, send all messages that are pending
        for (auto i : _detail->pendingMessages) {
            map<Id, MessageQueue>::iterator qIt = _detail->messageQueue.find(i);
            if (qIt == _detail->messageQueue.end())
                continue;
            MessageQueue& q = qIt->second;
            for (auto j : q) {
                
            }
            q.clear();
        }
        _detail->pendingMessages.clear();
    }

    void VMContext::onTimeout(float delay, int recurrences,
                              const Landru::Fiber &f,
                              std::vector<Instruction> instr) {
        if (delay <= TIME_QUANTA) {
            auto messageQueue = _detail->messageQueue.find(f.id());
            if (messageQueue == _detail->messageQueue.end()) {
                VM_RAISE("Runtime error, unknown machine");
            }
            recurrences = max(0, recurrences);
            auto fiberIt = _detail->fibers.find(f.id());
            if (fiberIt == _detail->fibers.end()) {
                VM_RAISE("Runtime error, unknown machine");
            }
            auto fiber = fiberIt->second;
            for (int i = 0; i < recurrences; ++i)
                messageQueue->second.emplace_back(make_tuple(fiber, instr));
            _detail->pendingMessages.insert(f.id());
        }
        else {
            auto fiberIt = _detail->fibers.find(f.id());
            if (fiberIt == _detail->fibers.end()) {
                VM_RAISE("Runtime error, unknown machine");
            }
            _detail->timeoutQueue.emplace(make_tuple((float)_detail->now + delay, delay, fiberIt->second, instr, recurrences));
        }
    }
    
}