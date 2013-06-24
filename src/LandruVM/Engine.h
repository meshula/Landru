
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#ifndef EXTERNC
# ifdef __cplusplus
#  define EXTERNC extern "C"
# else
#  define EXTERNC
# endif
#endif

EXTERNC void* landruCreateEngine();
EXTERNC void  landrUDestroyEngine(void* engine);
EXTERNC void  landruRun(void* engine, const char* machine);
EXTERNC void  landruUpdate(void* engine, float dt);

#ifdef __cplusplus

typedef struct LStack LStack;

#include "LandruVM/VarObj.h"
#include <vector>

namespace Landru
{
    class ContinuationList;
	class Exemplar;
	class Fiber;
    class MachineCache;
    class MouseEngine;
    class TimeEngine;
    class VarPool;

	class Engine : public VarObj
	{		
	public:
		Engine(const char* name);
		~Engine();
        
        VAROBJ_FACTORY(engine, Engine);
        
        virtual void InitFunctionTable();
        
		void Update(float dt);
		
        void RunScript(const char* machineName);
        
        VarObjPtr* self() const { return _self; }
		
		//----------------------------------------------------------------------
//		void	RegisterOnMessage(Fiber* f, LStack*, int pc); // message string must be on top of stack
//		void	SendMessageToAll(const char* machineType, const char* message);
		//----------------------------------------------------------------------
//        void    RegisterRender(Fiber* f, int pc);
		//----------------------------------------------------------------------
//        void    RegisterTick(Fiber*, int pc);

        void    ClearContinuations(VarObjPtr* fiber);

        void       AddGlobal(const char* name, const char* lib);
        void       AddGlobal(const char* name, VarObjPtr*);
        VarObjPtr* Global(const char* name);
        VarObjPtr* Global(int i);
        int        GlobalIndex(const char* name);
                
		///int		CountRefs(Fiber* f);
        
        MachineCache* machineCache() const { return _machineCache; }
        
        void addContinuationList(ContinuationList*);
        ContinuationList* continuationList(const char* name) const;
        
        TimeEngine* timeEngine() const { return _timeEngine; }
        ContinuationList* collisionEngine() const { return _collisionEngine; }
        void collisionEngine(ContinuationList*);
        MouseEngine* mouseEngine() const { return _mouseEngine; }
        
        float elapsedTime() const;
        
        LVarPool* lvarPool() const { return (LVarPool*) _vars; }
        
        static FnTable functions;
        
	private:
        
		class Detail;
        
        // bound functions
        static void debugBreak(FnParams*);
        
        MachineCache* _machineCache;
        
        std::vector<ContinuationList*> continuationLists;
        
        TimeEngine* _timeEngine;
        ContinuationList* _collisionEngine;
        MouseEngine* _mouseEngine;
        
        VarObjPtr* _self;
        
		Detail* detail;
	};



}

#endif // __cplusplus
