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

    typedef vector<OnEventEvaluator> MessageQueue;

    class VMContext::Detail {
    public:
        Detail() : now(0) {}

        double now;

        map<Id, shared_ptr<Fiber>> fibers;

        set<Id> pendingMessages;
        map<Id, MessageQueue> messageQueue;

		std::deque<std::pair<std::shared_ptr<Fiber>, std::string>> gotos;
	};

    VMContext::VMContext(Library* l)
    : _detail(new Detail())
    , libs(l)
    , traceEnabled(false)
    , breakPoint(~0) {
    }

    VMContext::~VMContext() {
    }

	double VMContext::now() const {
		return _detail->now;
	}

    bool VMContext::deferredMessagesPending() const
	{
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
                    FnContext fn(this, f.get(), nullptr);
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
			_detail->gotos.push_back(std::make_pair(fiber, state));
		}
	}

	void VMContext::finalizeGotos()
	{
		while (_detail->gotos.begin() != _detail->gotos.end()) {
			auto i = _detail->gotos.front();
			FnContext run = { this, i.first.get(), nullptr };
			i.first->gotoState(run, i.second.c_str(), true);
			_detail->gotos.pop_front();
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
