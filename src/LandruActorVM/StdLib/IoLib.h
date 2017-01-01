//
//  IoLib.h
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
        // Io Libary \__________________________________________

        class IoLib {
        public:
            static void registerLib(Library& l);
            static RunState print(FnContext& run);
            static RunState resolve(FnContext& run);
        };



    } // Std
} // Landru
