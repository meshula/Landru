//
//  Property.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include "LandruLibForward.h"
#include "LandruActorVM/WiresTypedData.h"

#include <string>
#include <memory>


namespace Landru {

    class Fiber;
    class Library;

    class Property {
		TypeFactory _typeFactory;

    public:
        enum class Visibility { Shared, ActorLocal, Global };

        Property(const std::string& name, const std::string& type, TypeFactory);
		Property(const std::string& name, const std::string& type, std::shared_ptr<Wires::TypedData>&);
		Property(TypeFactory);
        ~Property();

        // return false if data is compatible, and compatibility required
        bool assign(std::shared_ptr<Wires::TypedData>&, bool mustBeCompatible);

		// return false if data is not compatible, and compatibility required
		// will create data if necessary
		bool copy(std::shared_ptr<Wires::TypedData>&, bool mustBeCompatible);

		void create();

        std::string name;
        std::string type;
        Visibility visibility;
        std::shared_ptr<Wires::TypedData> data;

        Fiber* owner = nullptr; // if property is on a fiber, it's referenced here; if fiber is destroyed it will null this pointer

		int assignCount = 0; // used as a flag indicating whether the current data value is default data
    };

}
