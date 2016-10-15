//
//  Fiber.cpp
//  Landru
//
//  Created by Nick Porcino on 11/7/14.
//
//

#include "LandruActorVM/Fiber.h"
#include <unistd.h>
#include <uuid/uuid.h>

using namespace std;

namespace Landru {

    Fiber::Fiber(std::shared_ptr<MachineDefinition> m, VMContext& vm)
    : machineDefinition(m) {
        for (auto& p : m->properties) {
            Property* prop = new Property();
            prop->name = p.second->name;
            prop->type = p.second->type;
            prop->visibility = p.second->visibility;
            prop->create(vm);
            properties[p.first] = prop;
        }
        stack.push_back(vector<shared_ptr<Wires::TypedData>>());
    }
    
    Fiber::~Fiber() {
        for (auto& p : properties)
            delete p.second;
    }
    
    Id::Id() {
        _pid = getpid();
        uuid_generate_time(_uuid);
    }
    
    Id::Id(const Id& rh) {
        _pid = rh._pid;
        uuid_copy(_uuid, rh._uuid);
    }
    
    Id::Id(const InvalidId& rh) {
        _pid = ~0;
        uuid_clear(_uuid);
    }
    
    Id& Id::operator=(const Id& rh) {
        _pid = rh._pid;
        uuid_copy(_uuid, rh._uuid);
        return *this;
    }
    
    Id& Id::operator=(const InvalidId&) {
        _pid = ~0;
        uuid_clear(_uuid);
        return *this;
    }

    int Id::compare(const InvalidId&) const {
        return (_pid != ~0) ? 1 : 0; // invalid instances are always smaller
    }
    
    int Id::compare(const Id& other) const {
        if (this == &other) {
            return 0; // shortcut for comparing to self
        }
        if ((_pid != ~0) != (other._pid != ~0)) {
            return (_pid != ~0) ? 1 : -1; // invalid instances are always smaller
        }
        int tmp = uuid_compare(_uuid, other._uuid);
        if (tmp == 0) {
            if (_pid < other._pid) {
                return -1;
            }
            else if (_pid == other._pid) {
                return 0;
            }
            return 1;
        }
        return tmp;
    }

    
}
