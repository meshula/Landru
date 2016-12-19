//
//  Fiber.cpp
//  Landru
//
//  Created by Nick Porcino on 11/7/14.
//
//

#include "LandruActorVM/Fiber.h"

#ifdef PLATFORM_WINDOWS
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

uint32_t getpid() {
	return GetCurrentProcessId();
}

void uuid_generate_time(unsigned char* buff) {
	// temp lame id generator
	static uint32_t i = 0;
	uint32_t * p = reinterpret_cast<uint32_t*>(buff);
	*p++ = 0;
	*p++ = 0;
	*p++ = 0;
	*p = i++;
}

void uuid_clear(unsigned char* buff) {
	// temp lame id generator
	uint32_t * p = reinterpret_cast<uint32_t*>(buff);
	*p++ = 0;
	*p++ = 0;
	*p++ = 0;
	*p++ = 0;
}

void uuid_copy(unsigned char* dst, unsigned char const* src) {
	uint32_t * d = reinterpret_cast<uint32_t *>(dst);
	uint32_t const* s = reinterpret_cast<uint32_t const*>(src);
	*d++ = *s++;
	*d++ = *s++;
	*d++ = *s++;
	*d++ = *s++;
}

int uuid_compare(unsigned char const*const ptra, unsigned char const* ptrb) {
	uint32_t const* a = reinterpret_cast<uint32_t const*>(ptra);
	uint32_t const* b = reinterpret_cast<uint32_t const*>(ptrb);
	return (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3]) ? 0 : 1;
}

#else
# include <unistd.h>
# include <uuid/uuid.h>
#endif

using namespace std;

namespace Landru {

	Fiber::Fiber(std::shared_ptr<MachineDefinition> m, VMContext& vm)
		: machineDefinition(m)
		, vm(vm)
	{
        for (auto& p : m->properties) {
            shared_ptr<Property> prop = make_shared<Property>();
            prop->name = p.second->name;
            prop->type = p.second->type;
            prop->visibility = p.second->visibility;
            prop->create(vm);
			vm.storeInstance(this, p.first, prop);
        }
        stack.push_back(vector<shared_ptr<Wires::TypedData>>());
    }
    
    Fiber::~Fiber() {
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
