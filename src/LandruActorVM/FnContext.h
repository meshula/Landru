//
//  FnContext.h
//  Landru
//
//  Created by Nick Porcino on 11/19/14.
//
//

#pragma once
#include "LandruActorVM/LandruLibForward.h"
#include <vector>

namespace Landru {

	struct FnContext 
	{
        FnContext() : vm(nullptr), self(nullptr), var(nullptr) {}
        FnContext(const FnContext& rh) : vm(rh.vm), self(rh.self), var(rh.var) {}
        FnContext(VMContext* vm, Fiber* self, Wires::TypedData* var)
        : vm(vm), self(self), var(var) {}
        
        VMContext* vm;              // virtual machine context
        Fiber* self;                // fiber being executed upon
        Wires::TypedData* var;      // variable whose function is being invoked
        
		RunState run(std::vector<Instruction>& instructions);
		void clearContinuations(Fiber* f, int level);
    };
}
