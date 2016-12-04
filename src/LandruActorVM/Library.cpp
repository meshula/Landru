//
//  Library.cpp
//  Landru
//
//  Created by Nick Porcino on 11/26/14.
//
//

#include "Library.h"
#include "VMContext.h"
#include <LabText/TextScanner.hpp>

namespace Landru 
{

    std::function<std::shared_ptr<Wires::TypedData>(VMContext& vm)> Library::findFactory(const char* name) {
        std::vector<std::string> nameParts = TextScanner::Split(name, ".");
        if (nameParts.size() == 1) {
            auto f = factories.find(name);
            if (f == factories.end()) {
                VM_RAISE("Cannot construct object of type: " << std::string(name));
            }
            return f->second;
        }
        else {
            for (auto& i : libraries) {
                if (i.name == nameParts[0]) {
                    auto f = i.factories.find(nameParts[1]);
                    if (f == i.factories.end()) {
                        VM_RAISE("Cannot construct object of type: " << std::string(name));
                    }
                    return f->second;
                }
            }
        }
        VM_RAISE("Cannot construct object of type: " << std::string(name));
    }
    
    Library::Vtable const*const Library::findVtable(const char* name) {
        auto v = vtables.find(name);
        if (v != vtables.end())
            return v->second.get();
        
        for (auto& i : libraries) {
            auto v = i.vtables.find(name);
            if (v != i.vtables.end())
                return v->second.get();
        }
        VM_RAISE("Cannot find vtable for: " << std::string(name));
    }
    
} // Landru
