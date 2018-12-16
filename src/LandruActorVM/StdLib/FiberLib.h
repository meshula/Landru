//
//  FiberLib.h
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#pragma once
#include "LandruActorVM/LandruLibForward.h"

namespace Landru {
    namespace Std {
        
        //--------------
        // Fiber Libary \__________________________________________
        
        class FiberLib {
        public:
            static void registerLib(Library& l);
            
            static RunState eval(FnContext& run);
            static RunState add(FnContext& run);
            static RunState sub(FnContext& run);
            static RunState mul(FnContext& run);
            static RunState div(FnContext& run);
            static RunState min(FnContext& run);
            static RunState max(FnContext& run);
            static RunState toInt(FnContext& run);
            static RunState sqrt(FnContext& run);
            static RunState toggle(FnContext& run);
            static RunState newFn(FnContext& run);
        };
        
    } // Std
} // Landru
