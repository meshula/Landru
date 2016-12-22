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

    OnEventEvaluator::OnEventEvaluator()
    : _detail(new Detail())
    {}

    OnEventEvaluator::OnEventEvaluator(std::shared_ptr<Fiber> f, std::vector<Instruction>& vi)
    : _detail(new Detail())
    {
        _detail->fiber = f;
        _detail->instructions = vi;
    }

    OnEventEvaluator::~OnEventEvaluator()
    {
		if (_detail)
	        delete _detail;
    }



	class TimeoutTuple : public OnEventEvaluator
	{
	public:
		TimeoutTuple(TimeoutTuple && rhs)
			: OnEventEvaluator(std::move(rhs))
			, timeout(rhs.timeout)
			, delay(rhs.delay)
			, recurrence(rhs.recurrence)
		{
			rhs._detail = nullptr;
		}

		TimeoutTuple(float timeout, float delay, std::shared_ptr<Fiber> fiberPtr, vector<Instruction> & statements, int recurrence)
			: OnEventEvaluator(fiberPtr, statements)
			, timeout(timeout)
			, delay(delay)
			, recurrence(recurrence)
		{}

		TimeoutTuple& operator=(TimeoutTuple && rhs)
		{
			_detail = rhs._detail;
			timeout = rhs.timeout;
			delay = rhs.delay;
			recurrence = rhs.recurrence;
			rhs._detail = nullptr;
			return *this;
		}

		float timeout;
		float delay;
		int recurrence;
	};

    class CompareTimeout {
    public:
        bool operator()(const TimeoutTuple& t1, const TimeoutTuple& t2) {
			return t2.timeout < t1.timeout;
        }
    };



    typedef vector<OnEventEvaluator> MessageQueue;

    class VMContext::Detail {
    public:
        Detail() : now(0) {}

        double now;

        map<Id, shared_ptr<Fiber>> fibers;

        set<Id> pendingMessages;
        map<Id, MessageQueue> messageQueue;

		multiset<TimeoutTuple, CompareTimeout> timeoutQueue;
    };

    VMContext::VMContext(Library* l)
    : _detail(new Detail())
    , libs(l)
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

    bool VMContext::deferredMessagesPending() const
	{
		if (!_detail->timeoutQueue.empty())
			return true;

		for (auto & p : plugins)
			if (p.pendingContinuations && p.pendingContinuations(nullptr))
				return true;

		return false;
    }
    bool VMContext::undeferredMessagesPending() const
	{
		if (!_detail->pendingMessages.empty())
			return true;

		// ask plugins for their opinion

		return false;
    }

	void VMContext::clearContinuations(Fiber* f, int level)
	{
		/// @TODO deal with level
		for (set<Id>::iterator i = _detail->pendingMessages.begin(); i != _detail->pendingMessages.end(); ++i) {
			if (*i == f->id()) {
				i = _detail->pendingMessages.erase(i);
				if (i == _detail->pendingMessages.end())
					break;
			}
		}
		for (map<Id, MessageQueue>::iterator i = _detail->messageQueue.begin(); i != _detail->messageQueue.end(); ++i) {
			if (i->first == f->id()) {
				i = _detail->messageQueue.erase(i);
				if (i == _detail->messageQueue.end())
					break;
			}
		}
		for (multiset<TimeoutTuple, CompareTimeout>::iterator i = _detail->timeoutQueue.begin(); i != _detail->timeoutQueue.end(); ++i) {
			if ((*i)._detail->fiber.get() == f) {
				i = _detail->timeoutQueue.erase(i);
				if (i == _detail->timeoutQueue.end())
					break;
			}
		}

		for (auto & p : plugins) {
			if (p.clearContinuations)
				p.clearContinuations(f, level);
		}
	}

    void VMContext::instantiateLibs()
    {
    }

    void VMContext::update(double now)
    {
        _detail->now = now;

        // launch all machines that were requested
        while (!launchQueue.empty())
		{
            auto rec = launchQueue.front();
            launchQueue.pop_front();
            auto m = machineDefinitions.find(rec.first);
            if (m != machineDefinitions.end()) {
                std::shared_ptr<Landru::Fiber> f = std::make_shared<Landru::Fiber>(m->second, *this);
                _detail->fibers[f->id()] = f;
                _detail->messageQueue[f->id()] = vector<OnEventEvaluator>();   //// @TODO emplace queues only when needed at first access
                try {
                    FnContext fn(this, f.get(), nullptr, nullptr);
					f->gotoState(fn, "__auto__", false);
                    f->gotoState(fn, "main", true);
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

		for (auto & p : plugins) {
			if (p.update)
				p.update(now, this);
		}

        // check all timeouts and allow them to fire if they've expired
        while (!_detail->timeoutQueue.empty()) {
			auto curr = _detail->timeoutQueue.begin();
			if (curr->timeout > now)
				break;

//			auto& testMe = _detail->timeoutQueue.top();
//            if (testMe.timeout > now) {
//                break;
//            }

			TimeoutTuple i = std::move(const_cast<TimeoutTuple&>(*curr));
			_detail->timeoutQueue.erase(curr);

//			TimeoutTuple i = std::move(const_cast<TimeoutTuple&>(_detail->timeoutQueue.top()));
//            _detail->timeoutQueue.pop();

            float t = i.timeout;
            if (t <= now) {
                shared_ptr<Fiber> fiberPtr = i._detail->fiber;
                Fiber *fiber = fiberPtr.get();
                auto& statements = i._detail->instructions;
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

                int recurrence = i.recurrence;
                if (recurrence > 1 || recurrence < 0) {
                    int newRecurrence = recurrence > 1 ? recurrence - 1 : -1;
                    float delay = i.delay;
                    _detail->timeoutQueue.emplace(TimeoutTuple(t + delay, delay, fiberPtr, statements, newRecurrence));
                }
            }
        }

        // send all pending messages
        for (auto i : _detail->pendingMessages) {
            map<Id, MessageQueue>::iterator qIt = _detail->messageQueue.find(i);
            if (qIt == _detail->messageQueue.end())
                continue;
            MessageQueue& q = qIt->second;
            for (auto & j : q) {
				// &&& yup
            }
            q.clear();
        }
        _detail->pendingMessages.clear();

		// execute pending gotos for all machines simultaneously
		{
			finalizeGotos();
		}
    }

	void VMContext::enqueueGoto(Fiber * f, const std::string & state)
	{
		std::shared_ptr<Fiber> fiber = fiberPtr(f);
		if (fiber && state.length()) {
			gotos.push_back(std::make_pair(fiber, state));
		}
	}

	void VMContext::finalizeGotos()
	{
		while (gotos.begin() != gotos.end()) {
			auto i = gotos.front();
			FnContext run = { this, i.first.get(), nullptr, nullptr };
			i.first->gotoState(run, i.second.c_str(), true);
			gotos.pop_front();
		}
	}


    void VMContext::onTimeout(float delay, int recurrences,
                              const Landru::Fiber &f,
                              std::vector<Instruction> instr)
    {
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
                messageQueue->second.emplace_back(OnEventEvaluator(fiber, instr));
            _detail->pendingMessages.insert(f.id());
        }
        else {
            auto fiberIt = _detail->fibers.find(f.id());
            if (fiberIt == _detail->fibers.end()) {
                VM_RAISE("Runtime error, unknown machine");
            }
            _detail->timeoutQueue.emplace(TimeoutTuple((float)_detail->now + delay, delay, fiberIt->second, instr, recurrences));
        }
    }

	std::shared_ptr<Fiber> VMContext::fiberPtr(Fiber* f)
	{
		auto fiberIt = _detail->fibers.find(f->id());
		if (fiberIt == _detail->fibers.end()) {
			VM_RAISE("Runtime error, unknown machine");
		}
		return fiberIt->second;
	}

} // Landru
