//
//  Library.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include "Exception.h"
#include "LandruLibForward.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Wires {
    class TypedData;
}

namespace Landru {

    //---------------
    //  Library      \____________________

    class Library {
    public:
        Library(const char* name) : name(name) {}
        Library(Library && rhs) { *this = std::move(rhs); }
        ~Library() {}

        Library& operator=(Library && rhs)
        {
            name = rhs.name;
            vtables.clear();
            std::swap(vtables, rhs.vtables);
            factories.clear();
            std::swap(factories, rhs.factories);
            return *this;
        }

        std::string name;
        std::vector<Library> libraries;

        //---------------------
        // Vtables             \_________________________
        class Vtable {
        public:
            Vtable(const char* name) : name(name) {}
            Vtable(const char* name, const Vtable& rh) : name(name), entries(rh.entries) {}
            Vtable(const Vtable & rh) { *this = rh; }
            Vtable(Vtable && rh)
            {
                name = rh.name;
                entries.swap(rh.entries);
            }
            ~Vtable() {}

            Vtable & operator=(Vtable && rh)
            {
                name = rh.name;
                entries.clear();
                std::swap(entries, rh.entries);
            }

            Vtable & operator=(const Vtable& rh)
            {
                name = rh.name;
                entries = rh.entries;
                return *this;
            }

            class Entry {
            public:
                Entry(const char* name, const char* args, ActorFn fn)
                : name(name), args(args), fn(fn) {}
                Entry() {}
                Entry(const Entry& rh) { *this = rh; }
                ~Entry() {}

                const Entry& operator =(const Entry& r) {
                    name = r.name; args = r.args; fn = r.fn;
                    return *this;
                }

                std::string name;
                std::string args;
                std::string response;
                ActorFn fn;
            };

            void registerFn(const char* protocol, const char* name, const char* args, const char* response, ActorFn fn)
            {
                entries[name] = Entry(name, args, fn);
            }

            Entry const*const function(const char* name) const
            {
                auto i = entries.find(name);
                if (i == entries.end())
                    return nullptr;
                return &i->second;
            }

            std::string name;
            std::map<std::string, Entry> entries;
        };



/// @TODO there should be one vtable per library, and the nested libraries should own their own vtables


        void registerVtable(std::unique_ptr<Vtable> vtable) {
            vtables[vtable->name] = move(vtable);
        }
        Vtable const*const findVtable(const char* name);

        std::map<std::string, std::unique_ptr<Vtable>> vtables;

        //---------------------
        // Type factories      \_________________________
        typedef std::function<std::shared_ptr<Wires::TypedData>(VMContext&)> TypeFactory;

        void registerFactory(const char* type, TypeFactory factory) {
            factories[type] = factory;
        }

        TypeFactory findFactory(const char* name);
        std::map<std::string, TypeFactory> factories;
    };

}
