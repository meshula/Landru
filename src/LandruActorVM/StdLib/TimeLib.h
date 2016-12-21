//
//  TimeLib.h
//  Landru
//
//  Created by Nick Porcino on 11/10/14.
//
//

#pragma once
#include "LandruActorVM/LandruLibForward.h"

namespace Landru {
    namespace Std {
        
        //-------------
        // Real Libary \__________________________________________
        class TimeLib {
        public:
            static void registerLib(Library& l);
            static RunState after(FnContext& run);
            static RunState every(FnContext& run);
            static RunState recur(FnContext& run);
        };
        
    } // Std
} // Landru
