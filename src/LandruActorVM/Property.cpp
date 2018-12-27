//
//  Property.cpp
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#include "Property.h"
#include "Library.h"
#include "Landru/VMContext.h"

namespace Landru {
    void Property::create() {
        data = _typeFactory();
    }

    Property::Property(const std::string& name, const std::string& type, TypeFactory tf)
    : name(name), type(type), assignCount(0), _typeFactory(tf)
	{
        create(); 
	}

	Property::Property(const std::string& name, const std::string& type, std::shared_ptr<Wires::TypedData>& d)
		: name(name), type(type), assignCount(0)
	{
		create();
		data->copy(d.get());
	}

    
    Property::Property(TypeFactory tf) : _typeFactory(tf) {}
    Property::~Property() {}

    bool Property::assign(std::shared_ptr<Wires::TypedData>& td, bool mustBeCompatible) 
	{
        if (mustBeCompatible) {
            if (!data || td->type() != data->type())
                return false;
        }
        data = td;
		++assignCount;
        return true;
    }

	bool Property::copy(std::shared_ptr<Wires::TypedData>& td, bool mustBeCompatible)
	{
		if (mustBeCompatible && data) {
			if (td->type() != data->type())
				return false;
		}

		if (!data)
			create();

		data->copy(td.get());
		++assignCount;
		return true;
	}

}
