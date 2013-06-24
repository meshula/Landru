
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include <vector>

namespace Landru
{
    
    class Exemplar;
    class Fiber;
    class VarPool;
    class VarObjPtr;
    
    struct MachineCacheEntry
    {
        MachineCacheEntry(Exemplar* e);
        MachineCacheEntry(const MachineCacheEntry& mc);
        
        /// @TODO The program store should go here instead of
        /// on every fiber. In order for that to happen, there
        /// has to be a way to reference vars per fiber, without
        /// modifying the program store per fiber

        // Shared variables are keyed from the exemplar
        Exemplar*	exemplar;
    };

    class MachineCache
    {
    public:
        MachineCache(VarPool*);    
        ~MachineCache();        
        const MachineCacheEntry* findExemplar(const char* name);        
        void add(Exemplar* e);        
        
		VarObjPtr* createFiber(char const* name);        
        
    private:
        VarPool* varpool;
        std::vector<MachineCacheEntry> cache;
    };
    
} // Landru

