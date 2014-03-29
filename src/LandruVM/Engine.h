
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

EXTERNC void* landruCreateEngine(char const*const workingDir);
EXTERNC void  landrUDestroyEngine(void* engine);
EXTERNC void  landruRun(void* engine, const char* machine);
EXTERNC void  landruUpdate(void* engine, float dt);

#ifdef __cplusplus

#include "LandruVM/VarObj.h"
#include <vector>

namespace Landru
{
    class ContinuationList;
	class Exemplar;
	class Fiber;
    class MachineCache;
    class VarObjFactory;
    class MouseEngine;
    class TimeEngine;

	class Engine : public VarObj
	{		
	public:
		Engine(const char* name);
		~Engine();
        
        VAROBJ_FACTORY(engine, Engine);
        
        virtual void InitFunctionTable();
        
		void Update(float dt);
		
        void RunScript(const char* machineName);

		//----------------------------------------------------------------------
//		void	RegisterOnMessage(Fiber* f, LStack*, int pc); // message string must be on top of stack
//		void	SendMessageToAll(const char* machineType, const char* message);
		//----------------------------------------------------------------------
//        void    RegisterRender(Fiber* f, int pc);
		//----------------------------------------------------------------------
//        void    RegisterTick(Fiber*, int pc);

        void    ClearContinuations(std::shared_ptr<Fiber> fiber);

        void       AddGlobal(const char* name, const char* lib);
        void       AddGlobal(const char* name, std::shared_ptr<VarObj>);
        std::weak_ptr<VarObj> Global(const char* name);

        void AddRequire(const char* lib, int index);
        std::shared_ptr<VarObj> Require(int index);

		///int		CountRefs(Fiber* f);
        
        MachineCache* machineCache() const { return _machineCache; }
        VarObjFactory* factory() const;
        
        void addContinuationList(ContinuationList*);
        ContinuationList* continuationList(const char* name) const;
        
        TimeEngine* timeEngine() const { return _timeEngine; }
        ContinuationList* collisionEngine() const { return _collisionEngine; }
        void collisionEngine(ContinuationList*);
        MouseEngine* mouseEngine() const { return _mouseEngine; }
        
        float elapsedTime() const;
        
        FnTable functions;
        
        std::string workingDir;

        std::recursive_mutex renderMutex;

	private:
        
		class Detail;
        
        // bound functions
        static void debugBreak(FnParams*);
        
        MachineCache* _machineCache;
        
        std::vector<ContinuationList*> continuationLists;
        
        TimeEngine* _timeEngine;
        ContinuationList* _collisionEngine;
        MouseEngine* _mouseEngine;
        
		Detail* detail;
	};



}

#endif // __cplusplus
