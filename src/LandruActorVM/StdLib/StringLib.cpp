//
//  StringLib.cpp
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//


#include "StringLib.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Generator.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"

#include <algorithm>
#include <string>
#include <cmath>
#include <memory>
using namespace std;

namespace Landru {
	namespace Std {

		//-------------
		// String Libary \__________________________________________
		void StringLib::registerLib(Library& l) {
			auto u = unique_ptr<Library::Vtable>(new Library::Vtable("string"));
			l.registerVtable(move(u));
			l.registerFactory("string", []()->std::shared_ptr<Wires::TypedData> { return std::make_shared<Wires::Data<string>>(); });
		}


	} // Std
} // Landru
