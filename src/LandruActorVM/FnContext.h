//
//  FnContext.h
//  Landru
//
//  Created by Nick Porcino on 11/19/14.
//
//

#pragma once
#include "LandruActorVM/LandruLibForward.h"
#include "LandruActorVM/VMContext.h"
#include <vector>

namespace Landru {

	struct FnContext 
	{
        FnContext() : vm(nullptr), self(nullptr), var(nullptr), library(nullptr) {}
        FnContext(const FnContext& rh) : vm(rh.vm), self(rh.self), var(rh.var), library(rh.library) {}
        FnContext(VMContext* vm, Fiber* self, Wires::TypedData* var, Wires::TypedData* library)
        : vm(vm), self(self), var(var), library(library) {}
        
        VMContext* vm;              // virtual machine context
        Fiber* self;                // fiber being executed upon
        Wires::TypedData* var;      // variable whose function is being invoked
        Wires::TypedData* library;  // library that created the variable
        
        RunState run(std::vector<Instruction>& instructions) 
		{
			RunState runstate = RunState::Continue;
			if (vm->activateMeta)
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

		void clearContinuations(Fiber* f, int level)
		{
			vm->clearContinuations(f, level);
		}

    };
}
