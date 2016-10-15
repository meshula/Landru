//
//  StdLib.cpp
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#include "StdLib.h"
#include "LandruActorVM/Library.h"

namespace Landru {
    namespace Std {


//-------------------------------
// Initialize Standard Libraries \__________________________________________


void populateLibrary(Library& l) {
    Std::FiberLib::registerLib(l);
    IntLib::registerLib(l);
    IoLib::registerLib(l);
    RealLib::registerLib(l);
    TimeLib::registerLib(l);
}

        
    }
} // Landru::Std

