//
//  LandruLibForward.h
//  Landru
//
//  Created by Nick Porcino on 11/19/14.
//
//

#pragma once

#include <functional>

namespace Wires {
    class TypedData;
}

namespace Landru {
    class Fiber;
	class LandruRequire;
	class Library;
    class VMContext;
    struct FnContext;
    class Meta;

	enum class RunState {
		Stop, Continue, Goto, UndefinedBehavior
	};
    
    typedef std::pair<std::function<RunState(FnContext&)>, Meta> Instruction;
	typedef std::function<RunState(FnContext&)> ActorFn;
}

