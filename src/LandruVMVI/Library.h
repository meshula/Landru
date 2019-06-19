#pragma once

#include <functional>
#include <map>
#include <optional>
#include <variant>
#include <vector>

#ifdef _MSC_VER
#define strdup _strdup
#endif

namespace lvmvi
{
    struct StateContext;

    struct Library
    {
        explicit Library(char const*const name) 
        {
            _name = strdup(name);
        }
        ~Library()
        {
            free((void*)_name);
        }
        char const* _name = nullptr;
        char const*const name() { return _name; }
        // stack, argc is the function input
        typedef std::function<void(StateContext&, int)> Fn;
        std::map<std::string, Fn> functions;
        std::optional<Fn> get_fn(const std::string& name)
        {
            auto it = functions.find(name);
            if (it == functions.end())
                return {};

            return it->second;
        }
    };
}

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
#if 0

// put this in the class declaration in the header
#define VAROBJ_FACTORY(name, c) \
    static const char* Name() { return #name; } \
    virtual void* TypeId() const { return (void*) Name(); } \
    virtual const char* TypeName() const { return Name(); } \
    static const void* StaticTypeId() { return TypeId(); } \
    static std::unique_ptr<VarObj> makeFn(char const*const n) { return std::unique_ptr<c>(new c(n)); }


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

#endif
