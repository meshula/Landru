//
//  StdLib.cpp
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#include "StdLib.h"
#include "LandruActorVM/Library.h"
#include "Landru/Landru.h"

namespace Landru {
    namespace Std {


//-------------------------------
// Initialize Standard Libraries \__________________________________________

void populateLibrary(Library& l, VMContext &vm) {
    Std::FiberLib::registerLib(l);
    IntLib::registerLib(l);
    IoLib::registerLib(l);
    RealLib::registerLib(l);
	StringLib::registerLib(l);
	create_time_plugin(vm);
}

        
    }
} // Landru::Std

extern "C"
void landruInitializeStdLib(LandruLibrary_t* library, LandruVMContext_t* vmContext)
{
    Landru::Std::populateLibrary(* reinterpret_cast<Landru::Library*>(library), * reinterpret_cast<Landru::VMContext*>(vmContext));
}
