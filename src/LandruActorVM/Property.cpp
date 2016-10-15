//
//  Property.cpp
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#include "Property.h"
#include "Library.h"
#include "VMContext.h"

namespace Landru {
    void Property::create(VMContext& vm) {
        auto factory = vm.libs->findFactory(type.c_str());
        data = factory(vm);
    }

    Property::Property(VMContext& vm, const std::string& name, const std::string& type) \
    : name(name), type(type) {
        create(vm); }
    
    Property::Property() {}
    Property::~Property() {}

    bool Property::assign(std::shared_ptr<Wires::TypedData>& td, bool mustBeCompatible) {
        if (mustBeCompatible) {
            if (!data || td->type() != data->type())
                return false;
        }
        *data = *td;
        return true;
    }

}
