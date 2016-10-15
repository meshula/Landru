//
//  IntLib.h
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#pragma once
#include "LandruActorVM/LandruLibForward.h"

namespace Landru {
    namespace Std {
                
        //-----------
        // Int Libary \__________________________________________
        class IntLib {
        public:
            static void registerLib(Library& l);
            static void add(FnContext& run);
            static void sub(FnContext& run);
            static void mul(FnContext& run);
            static void div(FnContext& run);
            static void mod(FnContext& run);
            static void min(FnContext& run);
            static void max(FnContext& run);
        };
        
    } // Std
} // Landru
