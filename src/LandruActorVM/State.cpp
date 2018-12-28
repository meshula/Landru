//
//  State.cpp
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#include "State.h"

#include "FnContext.h"
#include "LandruActorVM/VMContext.h"
#include <cstdint>
#include <cstdio>

namespace Landru {

    uint32_t Meta::globalAddr = 0;
        
    void Meta::exec(FnContext& run) const 
	{
        std::cout << addr << ": " << str << std::endl;
        if (run.vm->breakPoint == addr) {
            std::cout << "Break hit" << std::endl;
        }
    }
    
}
