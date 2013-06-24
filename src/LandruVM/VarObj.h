
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

#include <map>
#include <string>

// put t&his in the class declaration in the header
#define VAROBJ_FACTORY(name, c) static const char* Name() { return #name; } \
    virtual void* TypeId() const { return (void*) Name(); } \
    virtual const char* TypeName() const { return Name(); } \
    static const void* StaticTypeId() { return (void*) Name(); } \
    static VarObjPtr* Factory(VarPool* v, void* key, int i, const char* varName) { \
        VarObj* vo = new c(varName); \
        return v->setSlot(key, i, vo, true); } \
    static VarObjPtr* Factory(VarPool* v, void* key, const char* varName) { \
        VarObj* vo = new c(varName); \
        return v->allocVarObjSlot(key, vo, true); }

/*
    class Register##Type                                                    \
    {                                                                       \
    public:                                                                 \
        Register##Type() {                                                  \
            Landru::VarObj::FactoryRegister(Type::Name(), Type::Factory);   \
        }                                                                   \
    };                                                                      \
    Register##Type register##Type;
*/

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

typedef struct LStack LStack;


namespace Landru {

    class Continuation;
	class Engine;
	class Fiber;
    class VarPool;
    class VarObj;
    class VarObjArray;
    class VarObjPtr;

    struct FnParams
    {
        FnParams(const FnParams& rhs)
        : parentContinuation(rhs.parentContinuation)
        , pc(rhs.pc)
        , continuationPC(rhs.continuationPC)
        , vop(rhs.vop)
        , vo(rhs.vo)
        , f(rhs.f)
        , self(rhs.self)
        , stack(rhs.stack)
        , engine(rhs.engine)
        {
        }

        FnParams()
        : parentContinuation(0)
        , pc(0)
        , continuationPC(0)
        , vop(0)
        , vo(0)
        , f(0)
        , self(0)
        , stack(0)
        , engine(0)
        {
        }
        
        Landru::Continuation* parentContinuation;
        int pc;
        int continuationPC;
        Landru::VarObjPtr* vop;
        Landru::VarObj* vo;
        Landru::Fiber* f;
        Landru::VarObjPtr* self;
        LStack* stack;
        Engine* engine;
    };
    
}


// These go in the class to declare all the functions
#define LANDRU_DECL_BINDING_BEGIN   public: \
                                    static void InitFunctionTable(); \
                                    static void InitVarObj(); \
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
                                 void c::InitVarObj() { InitFunctionTable(); VarObj::FactoryRegister(Name(), Factory); }

namespace Landru {
	
    class VarObj
	{
        friend class VarPool;

    public:
		typedef void (*FuncPtr)(FnParams*);
        struct FnTable {
            std::map<std::string, int> functions;
            std::map<int, Landru::VarObj::FuncPtr> functionPtrs;
        };
        
    protected:
        VarObj(const char* name, FnTable*);
		virtual ~VarObj();
        
	public:
		static VarObjPtr*	Factory(VarPool*, void* key, int index, const char* type, const char* name);
		static VarObjPtr*	Factory(VarPool*, void* key, int index, void* typeId, const char* name);
		static void			FactoryRegister(const char* type, VarObjPtr* (*)(VarPool*, void*, int, const char*));


        virtual void*		 TypeId() const = 0;
        virtual const char*  TypeName() const = 0;
        
		virtual VarObjPtr*   GetVar(int index) const;
        virtual VarObjPtr*   GetVar(const char* name) const;
        virtual int          GetVarIndex(const char* name) const;
        
        bool Shared() const { return lockCount >= 0; }
		void Shared(bool v) { lockCount = v ? 0 : -1; }
        
        const char* name() const { return nameStr; }
        
        VarPool* varPool() const { return _vars; }
        
        int Func(const char* funcName);
        FuncPtr Func(int);
        void AddFunction(const char* funcName, VarObj::FuncPtr fn);
        void AddInstanceFunction(const char* funcName, VarObj::FuncPtr fn);
        
	protected:
        friend class VarObjStrongRef;
        friend class VarObjWeakRef;
        VarPool* _vars;
		int lockCount;          // lockCount >= 0 means a shared variable
        const char* nameStr;    // VarObj does not own name string, assumed to be static
        
        FnTable* _functions;
        std::unique_ptr<FnTable> _instanceFunctions;
    };
    
    class VarObjPtr
    {
        friend class VarPool;
        
        VarObjPtr()
        : vo(0), key(0), i(0), prevFree(-1), nextFree(-1), strong(0), weak(0)
        {
        }
        
        ~VarObjPtr();
        
    public:        
        VarObj* vo;
        const void* key;
        int i;
        
        int prevFree;
        int nextFree;
        int strong;
        int weak;
    };
    
    class VarPool
    {
    public:        
        VarPool(int s)
        : poolSize(s)
        {
            pool = (VarObjPtr*) malloc(sizeof(VarObjPtr) * poolSize);
            initPool();
        }
        
        void initPool()
        {
            firstFree = 0;
            for (int i = 0; i < poolSize; ++i) {
                pool[i].vo = 0;
                pool[i].key = 0;
                pool[i].i = 0;
                pool[i].strong = 0;
                pool[i].weak = 0;
                pool[i].prevFree = i - 1;
                pool[i].nextFree = i + 1;
            }
        }
        
        int varKey(VarObjPtr* v)
        {
            int i = (((ptrdiff_t) v) - (ptrdiff_t) pool) / sizeof(VarObjPtr);
            return i;
        }
        
        VarObjPtr* fromKey(int i)
        {
            if (i < 0 || i >= poolSize || (pool[i].prevFree != pool[i].nextFree))
                RaiseError(0, "Bad key to pool", "varpool");
            
            return &pool[i];
        }
        
        void dumpPool()
        {
            printf("\n\n\n\nPool size %d\n", poolSize);
            for (int i = 0; i < poolSize; ++i) {
                if (pool[i].strong > 0)
                    printf("[%d/%d] %s\n", i, pool[i].strong, pool[i].vo->TypeName());
            }
        }
        
        VarObjPtr* allocVarObjSlot(const void* key, VarObj* vo, bool strong)
        {
            int ret = firstFree;
            if (ret == poolSize) {
                dumpPool();
                RaiseError(0, "Pool full", "varpool");
                return 0;
            }
            
            stitch(ret);

            static int uniqueId = 0;
            
            pool[ret].strong = strong ? 1 : 0;
            pool[ret].weak = strong ? 0 : 1;
            pool[ret].vo = vo;
            pool[ret].key = key;
            pool[ret].i = ++uniqueId;
            vo->_vars = this;
            
            return &pool[ret];
        }
        
        // The Landru compiler preallocates slots for fibers, this routine
        // allows for that.
        VarObjPtr* setSlot(const void* key, int i, VarObj* vo, bool strong)
        {
            VarObjPtr* vop = allocVarObjSlot(key, vo, strong);
            vop->i = i;
            return vop;
        }
                
        VarObjPtr* varObj(const void* key, const char* name) const
        {
            if (!name)
                return 0;
            
            for (int i = 0; i < poolSize; ++i) {
                if (pool[i].prevFree != pool[i].nextFree)
                    continue;
                
                if (pool[i].key != key)
                    continue;
                
                if (pool[i].vo->name() && !strcmp(name, pool[i].vo->name()))
                    return &pool[i];
            }
            
            return 0;
        }
        
        VarObjPtr* varObj(const void* key, int id) const
        {
            for (int i = 0; i < poolSize; ++i) {
                if (pool[i].prevFree != pool[i].nextFree)
                    continue;
                
                if (pool[i].key == key && pool[i].i == id)
                    return &pool[i];
            }
            
            return 0;
        }
        
        void addStrongRef(VarObjPtr* v)
        {
            if (v) {
                if (v->prevFree == v->nextFree)
                    ++v->strong;
            }
            else
                RaiseError(0, "Adding a strong reference to NULL", 0);
        }
        
        void addWeakRef(VarObjPtr* v)
        {
            if (v) {
                if (v->prevFree == v->nextFree)
                    ++v->weak;
            }
            else
                RaiseError(0, "Adding a weak reference to NULL", 0);
        }
        
        void releaseStrongRef(VarObjPtr* v)
        {
            if (!v)
                RaiseError(0, "Releasing a strong reference to NULL", 0);
            
            int i = (((ptrdiff_t) v) - (ptrdiff_t) pool) / sizeof(VarObjPtr);
            if (pool[i].prevFree == pool[i].nextFree) {
                if (pool[i].strong <= 0)
                    RaiseError(0, "unbalanced strong release", "VarPool");
                
                --pool[i].strong;
                releaseInternal(i);
            }
        }
        
        void releaseWeakRef(VarObjPtr* v)
        {
            if (!v)
                RaiseError(0, "Releasing a weak reference to NULL", 0);

            int i = (((ptrdiff_t) v) - (ptrdiff_t) pool) / sizeof(VarObjPtr);
            if (pool[i].prevFree == pool[i].nextFree) {
                if (pool[i].weak <= 0)
                    RaiseError(0, "unbalanced weak release", "VarPool");
                --pool[i].weak;
                releaseInternal(i);
            }
        }

        ~VarPool()
        {
            free(pool);
        }
        
    private:
        void stitch(int ret)
        {
            if (ret < 0 || ret >= poolSize)
                return;
            
            if (pool[ret].prevFree == pool[ret].nextFree)
                releaseWeakRef(&pool[ret]); // slot in use
            
            // restitch list
            int prev = pool[ret].prevFree;
            int next = pool[ret].nextFree;
            if (prev > -1 && prev < poolSize)
                pool[prev].nextFree = pool[ret].nextFree;
            if (next > -1 && next < poolSize)
                pool[next].prevFree = pool[ret].prevFree;
            
            pool[ret].prevFree = -1;
            pool[ret].nextFree = -1;
            
            firstFree = next;
            if (firstFree == poolSize)
                for (int i = 0; i < poolSize; ++i) {
                    if (pool[i].nextFree != pool[i].prevFree) {
                        firstFree = i;
                        break;
                    }
                }
        }
        
        void releaseInternal(int i)
        {
            if (pool[i].prevFree == pool[i].nextFree) {
                if (!pool[i].strong) {
                    delete pool[i].vo;
                    pool[i].vo = 0;
                }
                if (!pool[i].strong && !pool[i].weak) {
                    pool[i].prevFree = -1;
                    pool[i].nextFree = firstFree;
                    if (firstFree > -1 && firstFree < poolSize)
                        pool[firstFree].prevFree = i;
                    firstFree = i;
                }
            }
        }
        
        VarObjPtr* pool;
        int poolSize;
        int firstFree;
    };
    
    
    class VarObjStrongRef
    {
    public:
        VarObjStrongRef()
        : vo(0) { }
        
        VarObjStrongRef(VarObjPtr* vo)
        : vo(vo)
        {
            if (vo && vo->vo)
                vo->vo->_vars->addStrongRef(vo);
        }
        
        ~VarObjStrongRef()
        {
            if (vo && vo->vo)
                vo->vo->_vars->releaseStrongRef(vo);
        }
        
        void set(VarObjPtr* vo_)
        {
            if (vo && vo->vo)
                vo->vo->_vars->releaseStrongRef(vo);

            if (vo_ && vo_->vo) {
                vo = vo_;
                vo->vo->_vars->addStrongRef(vo);
            }
            else
                vo = 0;
        }
        
        VarObjPtr* vo;
    };
    
    class VarObjWeakRef
    {
    public:
        VarObjWeakRef(VarObjPtr* vo)
        : v(v), vo(vo)
        {
            if (vo && vo->vo)
                vo->vo->_vars->addWeakRef(vo);
        }
        
        ~VarObjWeakRef()
        {
            if (vo && vo->vo)
                vo->vo->_vars->releaseWeakRef(vo);
        }
        
        VarObjPtr* vo;
        
    private:
        VarPool* v;
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

typedef struct LVarObj LVarObj;
typedef struct LVarPool LVarPool;
EXTERNC void LVarObjIncRefCount(LVarPool*, LVarObj*);
EXTERNC void LVarObjDecRefCount(LVarPool*, LVarObj*);

#endif
