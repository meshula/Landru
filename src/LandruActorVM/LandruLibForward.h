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
    class Library;
    class VMContext;
    struct FnContext;
    class Meta;
    
    typedef std::pair<std::function<void(FnContext&)>, Meta> Instruction;
}
