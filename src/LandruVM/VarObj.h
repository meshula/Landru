
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_RUNTIME_VAROBJ_H
#define LANDRU_RUNTIME_VAROBJ_H

#ifdef __cplusplus

#include "LandruVM/RefCounted.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <memory>
#include <map>
#include <string>
#include <mutex>
#include <iostream>
#include <vector>

// put this in the class declaration in the header
#define VAROBJ_FACTORY(name, c) \
    static const char* Name() { return #name; } \
    virtual void* TypeId() const { return (void*) Name(); } \
    virtual const char* TypeName() const { return Name(); } \
    static const void* StaticTypeId() { return (void*) Name(); } \
    static std::unique_ptr<VarObj> makeFn(char const*const n) { return std::unique_ptr<c>(new c(n)); }

/*

Foo.h:

// Declare a VarObj with a couple of functions
 
class Foo : public VarObj {
public:
  VAROBJ_FACTORY(fiber, Fiber)

   virtual ~Foo();
 
 LANDRU_DECL_BINDING_BEGIN
   LANDRU_DECL_BINDING(bar)
   LANDRU_DECL_BINDING(baz)
 LANDRU_DECL_BINDING_END
};

Foo.cpp:
#include "Foo.h"
 
// Register Foo with the system

 Landru::VarObj::FactoryRegister(FooVarObj::Name(), FooVarObj::Factory);

// Each function gets a Fnparams object.
 
LANDRU_DECL_FN(Foo, bar)
{
  ...
}
 
LANDRU_DECL_FN(Foo, baz)
{
 ...
}

// Set up the function look ups
 
LANDRU_DECL_TABLE_BEGIN(Foo)
  LANDRU_DECL_ENTRY(Foo, bar)
  LANDRU_DECL_ENTRY(Foo, baz)
LANDRU_DECL_TABLE_END(Foo)


 */

class LStack;

namespace Landru {

    class Continuation;
	class Engine;
	class Fiber;
    class VarObj;
    class VarObjArray;

    struct FnParams {
        FnParams(const FnParams& rhs)
        : parentContinuation(rhs.parentContinuation)
        , pc(rhs.pc)
        , continuationPC(rhs.continuationPC)
        , vo(rhs.vo)
        , f(rhs.f)
        , stack(rhs.stack)
        , engine(rhs.engine)
        {}

        FnParams()
        : parentContinuation(0)
        , pc(0)
        , continuationPC(0)
        , f(0)
        , stack(0)
        , engine(0)
        {}
        
        Landru::Continuation* parentContinuation;
        int pc;
        int continuationPC;
        std::shared_ptr<VarObj> vo;
        std::shared_ptr<Landru::Fiber> f;
        LStack* stack;
        Engine* engine;
    };
    
}


// These go in the class to declare all the functions
#define LANDRU_DECL_BINDING_BEGIN   public: \
                                    static void InitFunctionTable(); \
                                    static void InitVarObj(Landru::VarObjFactory* vf); \
                                    static Landru::VarObj::FnTable functions;

#define LANDRU_DECL_BINDING(fn)     static void fn_##fn(FnParams*);
#define LANDRU_DECL_BINDING_END

// This is the function declaration for the implementation
#define LANDRU_DECL_FN(c, fn)       void c::fn_##fn(FnParams* p)

#define LANDRU_DECL_TABLE_BEGIN(c) void c::InitFunctionTable() {
#define LANDRU_DECL_ENTRY(c, fn) { int unique = (int) functions.functions.size(); \
                                   std::string s(#fn); functions.functions[s] = unique; \
                                   functions.functionPtrs[unique] = c::fn_##fn; }

#define LANDRU_DECL_TABLE_END(c) } \
                                 Landru::VarObj::FnTable c::functions; \
                                 void c::InitVarObj(Landru::VarObjFactory* vf) { \
                                       vf->registerFactory(Name(), makeFn); \
                                       InitFunctionTable(); }

namespace Landru {

    class VarObjArray;

    class VarObj {
    public:
		typedef void (*FuncPtr)(FnParams*);
        struct FnTable {
            std::map<std::string, int> functions;
            std::map<int, Landru::VarObj::FuncPtr> functionPtrs;
        };
        
        VarObj(const char* name, FnTable*);
		virtual ~VarObj();

	public:
        virtual void*		 TypeId() const = 0;
        virtual const char*  TypeName() const = 0;
        
		virtual std::weak_ptr<VarObj> GetVar(int index) const;
        virtual std::weak_ptr<VarObj> GetVar(const char* name) const;
        
        bool Shared() const { return lockCount >= 0; }
		void Shared(bool v) { lockCount = v ? 0 : -1; }
        
        const char* name() const { return nameStr; }

        int Func(const char* funcName);
        FuncPtr Func(int);
        void AddFunction(const char* funcName, VarObj::FuncPtr fn);
        void AddInstanceFunction(const char* funcName, VarObj::FuncPtr fn);
        
	protected:
		int lockCount;          // lockCount >= 0 means a shared variable
        const char* nameStr;    // VarObj does not own name string, assumed to be static
        
        FnTable* _functions;
        std::unique_ptr<FnTable> _instanceFunctions;
        std::unique_ptr<VarObjArray> _vars;
    };

    class VarObjFactory {
    public:
        typedef std::unique_ptr<VarObj> (*Fn)(char const*const);
        std::unique_ptr<VarObj> make(const char* type, const char* name);
        void registerFactory(char const*const type, Fn);

    private:
        std::map<std::string, Fn> _factories;
    };


} // Landru

#endif // __cplusplus

#ifndef EXTERNC
	#ifdef __cplusplus
		#define EXTERNC extern "C"
	#else
		#define EXTERNC
	#endif
#endif

// C interface


#endif
