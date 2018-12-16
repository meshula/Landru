//
//  RealLib.h
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#pragma once
#include "LandruActorVM/LandruLibForward.h"

namespace Landru {
    namespace Std {
        
        //-------------
        // Real Libary \__________________________________________
        class RealLib {
        public:
            static void registerLib(Library& l);
            static RunState add(FnContext& run);
            static RunState sub(FnContext& run);
            static RunState mul(FnContext& run);
            static RunState div(FnContext& run);
            static RunState mod(FnContext& run);
            static RunState min(FnContext& run);
            static RunState max(FnContext& run);
            static RunState range(FnContext& run);
        };
        
    } // Std
} // Landru
