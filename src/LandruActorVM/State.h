//
//  State.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include "LandruActorVM/LandruLibForward.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace Landru {

    struct FnContext;
    
    class Meta {
    public:
        Meta() {}
        Meta(const std::string& s) : str(s), addr(globalAddr++) {}
        Meta(const char* s) : str(s), addr(globalAddr++) {}
        std::string str;
        uint32_t addr;
        
        static uint32_t globalAddr;
        
        void exec(FnContext&) const;
    };
    
    class State {
    public:
        std::string name;
        std::vector<Instruction> instructions;
    };

}
