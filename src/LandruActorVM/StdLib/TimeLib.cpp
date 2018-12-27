//
//  TimeLib.cpp
//  Landru
//
//  Created by Nick Porcino on 11/10/14.
//
//

#include "TimeLib.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/State.h"
#include "Landru/VMContext.h"
#include <cmath>
#include <memory>
#include <set>
#include <vector>
using namespace std;

namespace {
	using namespace Landru;

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

    multiset<TimeoutTuple, CompareTimeout> timeoutQueue;


	void onTimeout(float now, float delay, int recurrences,
		std::shared_ptr<Fiber> f,
		std::vector<Instruction> instr)
	{
		timeoutQueue.emplace(TimeoutTuple(now + delay, delay, f, instr, recurrences));
	}

}


namespace Landru {
    namespace Std {

		//-------------
		// Real Libary \__________________________________________
		class TimeLib {
		public:
			static RunState after(FnContext& run);
			static RunState every(FnContext& run);
			static RunState recur(FnContext& run);
		};

        //-------------
        // Time Libary \__________________________________________
        RunState TimeLib::after(FnContext& run) {
            vector<Instruction> instr = run.self->back<vector<Instruction>>(-2);
            float delay = run.self->pop<float>();
            run.self->popVar(); // drop the instr
            int recurrences = 1;
            onTimeout(float(run.vm->now()), delay, recurrences, run.vm->fiberPtr(run.self), instr);
			return RunState::Continue;
        }
		RunState TimeLib::every(FnContext& run) {
            vector<Instruction> instr = run.self->back<vector<Instruction>>(-2);
            float delay = run.self->pop<float>();
            run.self->popVar(); // drop the instr
            int recurrences = -1;
            onTimeout(float(run.vm->now()), delay, recurrences, run.vm->fiberPtr(run.self), instr);
			return RunState::Continue;
		}
		RunState TimeLib::recur(FnContext& run) {
            vector<Instruction> instr = run.self->back<vector<Instruction>>(-3);
            int recurrences = run.self->pop<int>();
            float delay = run.self->pop<float>();
            run.self->popVar(); // drop the instr
            onTimeout(float(run.vm->now()), delay, recurrences, run.vm->fiberPtr(run.self), instr);
			return RunState::Continue;
		}

    } // Std
} // Landru


extern "C"
LANDRUTIME_API
void landru_time_init(Landru::Library* lib)
{
    Landru::Library time_lib("time");

    auto vtable = unique_ptr<Library::Vtable>(new Library::Vtable("time"));
    vtable->registerFn("2.0", "after", "ff", "f", Landru::Std::TimeLib::after);
    vtable->registerFn("2.0", "every", "ff", "f", Landru::Std::TimeLib::every);
    vtable->registerFn("2.0", "repeat", "ff", "f", Landru::Std::TimeLib::every);
    vtable->registerFn("2.0", "recur", "ff", "f", Landru::Std::TimeLib::recur);
    time_lib.registerVtable(move(vtable));

    lib->libraries.emplace_back(std::move(time_lib));
}

extern "C"
LANDRUTIME_API
RunState landru_time_update(double now, VMContext* vm)
{
    // check all timeouts and allow them to fire if they've expired
    while (!timeoutQueue.empty()) {
        auto curr = timeoutQueue.begin();
        if (curr->timeout > now)
            break;

//			auto& testMe = _detail->timeoutQueue.top();
//            if (testMe.timeout > now) {
//                break;
//            }
        TimeoutTuple i = std::move(const_cast<TimeoutTuple&>(*curr));
        timeoutQueue.erase(curr);

//			TimeoutTuple i = std::move(const_cast<TimeoutTuple&>(timeoutQueue.top()));
//            timeoutQueue.pop();

        float t = i.timeout;
        if (t <= now) {
            auto& statements = i.instructions();
            FnContext fn = {vm, i.fiber(), nullptr};
            if (vm->traceEnabled)
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
                timeoutQueue.emplace(TimeoutTuple(t + delay, delay, i.fiberPtr(), statements, newRecurrence));
            }
        }
    }
    return RunState::Continue;
}

extern "C"
LANDRUTIME_API
void landru_time_finish(Landru::Library*)
{
}

extern "C"
LANDRUTIME_API
void landru_time_fiberExpiring(Fiber* f)
{
//    called when Fibers are destroyed so that pending items like onWindowsClosed can be removed
}

extern "C"
LANDRUTIME_API
void landru_time_clearContinuations(Fiber* f, int level)
{
    for (multiset<TimeoutTuple, CompareTimeout>::iterator i = timeoutQueue.begin(); i != timeoutQueue.end(); ++i) {
		if (i->fiber() == f) {
            i = timeoutQueue.erase(i);
            if (i == timeoutQueue.end())
                break;
        }
    }
}

extern "C"
LANDRUTIME_API
bool landru_time_pendingContinuations(Fiber * f)
{
	return !timeoutQueue.empty();
}

void create_time_plugin(VMContext& vm)
{
	Landru::LandruRequire plugin;
	plugin.clearContinuations = landru_time_clearContinuations;
	plugin.fiberExpiring = landru_time_fiberExpiring;
	plugin.finish = landru_time_finish;
	plugin.init = landru_time_init;
	plugin.name = "time";
	plugin.pendingContinuations = landru_time_pendingContinuations;
	plugin.update = landru_time_update;
	vm.plugins.push_back(plugin);
}


