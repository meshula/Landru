
#include "FnContext.h"
#include "VmContext.h"

namespace Landru {

	RunState FnContext::run(std::vector<Instruction>& instructions)
	{
		RunState runstate = RunState::Continue;
		if (vm->traceEnabled)
			for (auto& i : instructions) {
				i.second.exec(*this);
				if ((runstate = i.first(*this)) != RunState::Continue)
					break;
			}
		else
			for (auto& i : instructions)
				if ((runstate = i.first(*this)) != RunState::Continue)
					break;

		return runstate;
	}

	void FnContext::clearContinuations(Fiber* f, int level)
	{
		vm->clearContinuations(f, level);
	}


}
