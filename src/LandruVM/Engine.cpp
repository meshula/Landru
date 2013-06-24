
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Continuation.h"
#include "LandruVM/Engine.h"
#include "LandruVM/Fiber.h"
#include "LandruVM/MachineCache.h"
#include "LandruVM/Stack.h"
#include "LandruVM/VarObjArray.h"

#include "LandruStd/IoVarObj.h"
#include "LandruStd/TimeVarObj.h"

//#include "Simulation/Physics2d/CollisionVarObj.h"

#include <map>
#include <string>

#define MAX_STACK_DEPTH 32
#define MAX_VARS 32

namespace Landru
{
	
	class Engine::Detail
	{
	public:
        /*
		struct LibDescriptor
		{
			const char* name;
			VarObjPtr* lib;
		};*/
				
		Detail(Engine* engine)
        : currentElapsedTime(0)
		{
//			QueueRegister(messageQueue = new MessageQueue(engine),		"engine", "message");
//			QueueRegister(tickQueue = new TickQueue(engine),			"engine", "tick");
		}
		
		~Detail()
		{
		}
				
        VarPool* vars;
		float currentElapsedTime;

        std::map<std::string, VarObjPtr*> globals;
        std::vector<VarObjPtr*> globalIndices;
		
//		TickQueue*		tickQueue;
//		MessageQueue*	messageQueue;
	};
    
    Landru::VarObj::FnTable Engine::functions;
	
	Engine::Engine(const char* name)
    : VarObj(name, &functions)
    , detail(new Detail(this))
    , _self(0)
	{
        _vars = new VarPool(8192);
        _vars->allocVarObjSlot(this, this, true);
        detail->vars = _vars;
        
        // these should maybe specified externally
        _timeEngine = new TimeEngine(_vars);
        addContinuationList(_timeEngine);
        _mouseEngine = new MouseEngine(_vars);
        addContinuationList(_mouseEngine);
        _collisionEngine = 0;

        _machineCache = new MachineCache(_vars);
        AddFunction("debugBreak", debugBreak);
	}

	Engine::~Engine()
	{
        delete _collisionEngine;
        delete _timeEngine;
        delete _machineCache;
		delete detail;
	}
    
    void Engine::collisionEngine(ContinuationList* ce)
    {
        _collisionEngine = ce;
        addContinuationList(ce);
    }
    
    float Engine::elapsedTime() const
    {
        return detail->currentElapsedTime;
    }
    
	void Engine::Update(float dt)
	{
        //Lab::Base::Cycles ct;
        
        detail->currentElapsedTime += dt;
        
        UpdateQueues(this, detail->currentElapsedTime);
        
        //ct.Sample();
        //printf("Tick %d\n", ct.Total());
        //printf("--------------\n");
	}
    
    void Engine::InitFunctionTable()
    {
    }
    
#if 0
	// Event Sources

    void Engine::RegisterTick(Fiber* f, int pc)
    {
        detail->tickQueue->registerContinuation(f, 0, pc);
    }
		
    // machineType is a filter; if none, all messageItems are checked
	void Engine::SendMessageToAll(const char* machineType, const char* message)
	{
		detail->messageQueue->sendMessageToAll(detail->currentElapsedTime, machineType, message);
	}

    void Engine::RegisterOnMessage(Fiber* f, LStack* stack, int pc)
    {
        const char* messageString = f->TopString(stack); LStackPop(stack);
		detail->messageQueue->registerContinuation(f, stack, messageString, pc);
    }
#endif
	
	//---------------------------------------------
    // Fibers

		
    void Engine::ClearContinuations(Landru::VarObjPtr* vop)
    {
        ClearQueues(vop);
        _timeEngine->clearContinuation(vop);
    }

    //---------------------------------------------
    // Globals

    void Engine::AddGlobal(const char* name, const char* lib)
    { 
        if (!name || !lib)
            return;
        
        std::map<std::string, VarObjPtr*>::iterator i = detail->globals.find(name);
        if (i != detail->globals.end())
            return;

        // manufacture a global using the factory
        VarObjPtr* v = VarObj::Factory(_vars, this, detail->globalIndices.size(), lib, name);
        AddGlobal(name, v);
    }
    
    void Engine::AddGlobal(const char* name, VarObjPtr* v)
    { 
        if (!name || !v)
            return;
        
        std::map<std::string, VarObjPtr*>::iterator i = detail->globals.find(name);
        if (i != detail->globals.end())
            return;
        
        _vars->addStrongRef(v);
        detail->globals[name] = v;
        detail->globalIndices.push_back(v);
    }
    
    VarObjPtr* Engine::Global(const char* name)
    {
        std::map<std::string, VarObjPtr*>::iterator i = detail->globals.find(name);
        if (i == detail->globals.end()) {
            return 0;
        }
        return i->second;
    }
    
    int Engine::GlobalIndex(const char* name)
    {
        std::map<std::string, VarObjPtr*>::iterator i = detail->globals.find(name);
        if (i == detail->globals.end())
            return -1;
        int k = 0;
        for (std::vector<VarObjPtr*>::const_iterator j = detail->globalIndices.begin(); j != detail->globalIndices.end(); ++j, ++k)
            if ((*j) == i->second)
                return k;
        
        return -1;
    }
    
    VarObjPtr* Engine::Global(int i)
    {
        if (i >= detail->globalIndices.size())
            return 0;
        return detail->globalIndices[i];
    }
    
    //---------------------------------------------
    // Run

    
    void Engine::RunScript(const char* machineName)
    {
        const MachineCacheEntry* mce = _machineCache->findExemplar(machineName);
        if (mce)
        {
            static int uniqueId = 0;
            VarObjStrongRef vop(Fiber::Factory(_vars, this, ++uniqueId, machineName));
            Fiber* f = (Fiber*) vop.vo->vo;
            AddGlobal(machineName, vop.vo);
            f->Create(_vars, MAX_STACK_DEPTH, MAX_VARS, vop.vo, mce);
            LStack* stack = LStackGetFromPool(1024);
            VarObjArray exeStack("exeStack");
            f->Run(this, vop.vo, detail->currentElapsedTime, f->EntryPoint(), stack, 0, &exeStack, 0);
            LStackReleaseToPool(stack, (LVarPool*) _vars);
        }
    }
    
    
    // bound functions

    //static
    void Engine::debugBreak(FnParams* p)
    {
        RaiseError(p->pc, "Debug break", 0);
    }
    

    void Engine::addContinuationList(ContinuationList* cl)
    {
        QueueRegister(cl, cl->name(), "update");
    }
    
    ContinuationList* Engine::continuationList(const char* name) const
    {
        for (std::vector<ContinuationList*>::const_iterator i = continuationLists.begin(); i != continuationLists.end(); ++i)
            if (!strcmp(name, (*i)->name()))
                return *i;
                
        return 0;
    }
    
    
	
} // Landru

EXTERNC void* landruCreateEngine()
{
    Landru::Engine* e = new Landru::Engine("engine");
    return e;
}

EXTERNC void landruDestroyEngine(void* e)
{
    Landru::VarObjPtr* vop = (Landru::VarObjPtr*) e;
    Landru::Engine* engine = dynamic_cast<Landru::Engine*>(vop->vo);
    delete engine;
}

EXTERNC void landruRun(void* e, const char* machineName)
{
    if (e) {
        Landru::Engine* engine = (Landru::Engine*) e;
        engine->RunScript(machineName);
    }
}

EXTERNC void landruUpdate(void* e, float dt)
{
    if (e) {
        Landru::Engine* engine = (Landru::Engine*) e;
        engine->Update(dt);
    }
}
