
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

    using namespace std;
	
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

		float currentElapsedTime;

        map<string, shared_ptr<VarObj>> globals;
        vector<shared_ptr<VarObj>> requires;

        VarObjFactory factory;
		
//		TickQueue*		tickQueue;
//		MessageQueue*	messageQueue;
	};
    
    //    Landru::VarObj::FnTable Engine::functions;
	
	Engine::Engine(const char* name)
    : VarObj(name, &functions)
    , detail(new Detail(this))
	{
        // these should maybe specified externally
        _timeEngine = new TimeEngine();
        addContinuationList(_timeEngine);
        _mouseEngine = new MouseEngine();
        addContinuationList(_mouseEngine);
        _collisionEngine = 0;

        _machineCache = new MachineCache();
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
        const char* messageString = f->TopString(stack);
        stack->pop();
		detail->messageQueue->registerContinuation(f, stack, messageString, pc);
    }
#endif
	
	//---------------------------------------------
    // Fibers

		
    void Engine::ClearContinuations(std::shared_ptr<Fiber> vop)
    {
        ClearQueues(vop);
        _timeEngine->clearContinuation(vop);
    }

    //---------------------------------------------
    // Globals

    VarObjFactory* Engine::factory() const {
        return &detail->factory;
    }

    void Engine::AddGlobal(const char* name, const char* lib)
    { 
        if (!name || !lib)
            return;
        
        if (detail->globals.find(name) != detail->globals.end())
            return;

        // manufacture a global using the factory
        std::unique_ptr<VarObj> v = detail->factory.make(lib, name);
        if (v)
            AddGlobal(name, std::move(v));
    }

    void Engine::AddRequire(const char* lib, int index) {
        if (detail->requires.size() <= index)
            detail->requires.resize(index*2+1);
        detail->requires[index] = detail->factory.make(lib, lib);
    }

    std::shared_ptr<VarObj> Engine::Require(int index) {
        if (index >= detail->requires.size())
            return std::shared_ptr<VarObj>();
        return detail->requires[index];
    }


    void Engine::AddGlobal(const char* name, std::shared_ptr<VarObj> v)
    { 
        if (!name || !v)
            return;
        
        std::map<std::string, std::shared_ptr<VarObj>>::iterator i = detail->globals.find(name);
        if (i != detail->globals.end())
            return;
        
        detail->globals[name] = v;
    }
    
    std::weak_ptr<VarObj> Engine::Global(const char* name)
    {
        std::map<std::string, std::shared_ptr<VarObj>>::iterator i = detail->globals.find(name);
        if (i == detail->globals.end()) {
            return std::weak_ptr<VarObj>();
        }
        return i->second;
    }

    //---------------------------------------------
    // Run

    
    void Engine::RunScript(const char* machineName)
    {
        std::shared_ptr<MachineCacheEntry> mce = _machineCache->findExemplar(machineName);
        if (mce) {
            std::shared_ptr<Fiber> vop = std::make_shared<Fiber>(machineName);
            Fiber* f = (Fiber*) vop.get();
            AddGlobal(machineName, vop);
            f->Create(factory(), MAX_STACK_DEPTH, MAX_VARS, mce);
            std::shared_ptr<LStack> stack = std::make_shared<LStack>();
            std::vector<std::shared_ptr<Fiber>> exeStack;
            f->Run(this, vop, detail->currentElapsedTime, f->EntryPoint(), stack.get(), 0, exeStack, 0);
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

EXTERNC void* landruCreateEngine(char const*const workingDir)
{
    Landru::Engine* e = new Landru::Engine("engine");
    e->workingDir.assign(workingDir);
    return e;
}

EXTERNC void landruDestroyEngine(void* e)
{
    Landru::Engine* engine = reinterpret_cast<Landru::Engine*>(e);
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
