//
//  Generator.h
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#pragma once

namespace Landru {
    
    class Fiber;
    struct FnContext;
    class VMContext;
    
    class Generator {
    public:
        virtual ~Generator() { }
        
        virtual void begin() = 0;
        virtual bool done() = 0;
        virtual void next() = 0;
        
        virtual void generate(Wires::TypedData*) = 0;
        virtual void finalize(FnContext&) = 0;
        
        virtual const char* typeName() const = 0;
    };
    
} // Landru

