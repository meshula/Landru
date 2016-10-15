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
            
            static void eval(FnContext& run);
            static void add(FnContext& run);
            static void sub(FnContext& run);
            static void mul(FnContext& run);
            static void div(FnContext& run);
            static void min(FnContext& run);
            static void max(FnContext& run);
            static void toInt(FnContext& run);
            static void sqrt(FnContext& run);
            static void toggle(FnContext& run);
            static void newFn(FnContext& run);
        };
        
    } // Std
} // Landru
