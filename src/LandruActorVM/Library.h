//
//  Library.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include "Exception.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Wires {
    class TypedData;
}

namespace Landru {
    
    class Fiber;
    class FnContext;
    class VMContext;
    typedef std::function<void(FnContext&)> ActorFn;
    
    //---------------
    //  Library      \____________________
    
    class Library {
    public:
        Library(const char* name) : name(name) {}
        class Vtable {
        public:
            Vtable(const char* name) : name(name) {}
            Vtable(const char* name, const Vtable& rh) : name(name), entries(rh.entries) {}
            
            class Entry {
            public:
                Entry(const char* name, const char* args, ActorFn fn)
                : name(name), args(args), fn(fn) {}
                Entry() {}
                Entry(const Entry& r) : name(r.name), args(r.args), fn(r.fn) {}
                const Entry& operator =(const Entry& r) {
                    name = r.name; args = r.args; fn = r.fn;
                    return *this;
                }
                std::string name;
                std::string args;
                std::string response;
                ActorFn fn;
            };
            void registerFn(const char* protocol, const char* name, const char* args, const char* response, ActorFn fn) {
                entries[name] = Entry(name, args, fn);
            }
            Entry const*const function(const char* name) const {
                auto i = entries.find(name);
                if (i == entries.end())
                    return nullptr;
                return &i->second;
            }
            
            std::string name;
            std::map<std::string, Entry> entries;
        };
        void registerVtable(std::unique_ptr<Vtable> vtable) {
            vtables[vtable->name] = move(vtable);
        }
        void registerFactory(const char* type, std::function<std::shared_ptr<Wires::TypedData>(VMContext& vm)> factory) {
            factories[type] = factory;
        }
        Vtable const*const findVtable(const char* name);
        
        std::function<std::shared_ptr<Wires::TypedData>(VMContext& vm)> findFactory(const char* name);
        std::string name;
        std::shared_ptr<Wires::TypedData> libraryInstanceData;
        std::map<std::string, std::unique_ptr<Vtable>> vtables;
        std::map<std::string, std::function<std::shared_ptr<Wires::TypedData>(VMContext& vm)>> factories;
        std::vector<Library> libraries;
    };

}
