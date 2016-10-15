//
//  Property.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include "LandruActorVM/WiresTypedData.h"

#include <string>
#include <memory>


namespace Landru {
    
    class Fiber;
    class Library;
    class VMContext;
    
    class Property {
        friend class Fiber; // only Fiber can create a property
        void create(VMContext& vm);
        
    public:
        enum class Visibility { Shared, ActorLocal, Global };
        
        Property(VMContext& vm, const std::string& name, const std::string& type);
        Property();
        ~Property();
        
        // return false if data is compatible, and compatibility required
        bool assign(std::shared_ptr<Wires::TypedData>&, bool mustBeCompatible);
        
        std::string name;
        std::string type;
        Visibility visibility;
        std::shared_ptr<Wires::TypedData> data;
    };
    
}
