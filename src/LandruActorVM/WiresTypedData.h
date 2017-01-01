//
//  WiresTypedData.h
//  Wires
//
//  Created by Nick Porcino on 7/19/14.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//
#pragma once

#include <typeindex>

namespace Wires {

class TypedData {
public:
    TypedData() : _type(typeid(TypedData)) { }
    virtual ~TypedData() { }
    const std::type_index type() const { return _type; }

    virtual void copy(const TypedData*) = 0;

protected:
    std::type_index _type;
};

template <typename T>
class Data : public TypedData {
public:
    Data() { _type = typeid(T); }
    Data(const T& data) : _data(data) { _type = typeid(T); }
    virtual ~Data() {}
    virtual const T& value() const { return _data; }
    virtual void setValue(const T& i) { _data = i; }

    virtual void copy(const TypedData* rhs) override {
        if (_type == rhs->type()) {
            const Data* rhsData = reinterpret_cast<const Data*>(rhs);
            _data = rhsData->_data;
        }
    }

private:
    T _data;
};

} // Wires
