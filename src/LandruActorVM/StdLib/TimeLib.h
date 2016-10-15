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
            static void after(FnContext& run);
            static void every(FnContext& run);
            static void recur(FnContext& run);
        };
        
    } // Std
} // Landru
