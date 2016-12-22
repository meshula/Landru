//
//  LandruActorAssembler.cpp
//  Landru
//
//  Created by Nick Porcino on 9/2/14.
//
//

#include "LandruActorAssembler.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Generator.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/MachineDefinition.h"
#include "LandruActorVM/Property.h"
#include "LandruActorVM/State.h"
#include "LandruActorVM/StdLib/StdLib.h"
#include "LandruActorVM/VMContext.h"
#include "LandruActorVM/WiresTypedData.h"
#include "LabText/TextScanner.hpp"
#include "LabJson/LabBson.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <exception>
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cstdlib>

using namespace std;
using Lab::Bson;

namespace Landru {


	//-------------------
	// assembler context \______________________________________________

	class ActorAssembler::Context {
	public:
		// the result of the assembly
		//
		map<string, shared_ptr<MachineDefinition>> machineDefinitions;
		map<string, shared_ptr<Bson>> bsonGlobals;
		//

		// current state of assembly
		//
		Library* libs = nullptr;
		map<string, string> requireAliases;
		shared_ptr<MachineDefinition> currMachineDefinition;
		vector<State*> currState;

		vector<vector<Instruction>*> currInstr;

		struct Conditional {
			Conditional() : op(Op::unknown) {}

			enum class Op { unknown, eq, eq0, lte0, gte0, lt0, gt0, neq0 };

			Op op;
			vector<Instruction> conditionalInstructions;
			vector<Instruction> contraConditionalInstructions;
		};
		vector<shared_ptr<Conditional>> currConditional;

		vector<pair<string, string>> localVariables; // vector[name-> name,type]
		vector<int> localVariableState;
		//

		Context(Library* l) : libs(l) {}
		~Context() {}

		void beginMachine(const char* name) {
			currMachineDefinition = make_shared<MachineDefinition>();
			currMachineDefinition->name.assign(name);
			machineDefinitions[currMachineDefinition->name] = currMachineDefinition;
		}
		void endMachine() {
			if (!currState.empty())
				AB_RAISE("badly formed state" << currState.back()->name);
			if (!currConditional.empty())
				AB_RAISE("badly formed conditional");

			currMachineDefinition = nullptr;
		}

		void beginState(const char* name) {
			State *s = new State();
			s->name = name;
			currMachineDefinition->states[s->name] = s;
			currState.emplace_back(s);
			currInstr.emplace_back(&s->instructions);
		}

		void endState() {
			currState.pop_back();
			currInstr.pop_back();
		}

		int localVariableIndex(const char* name) {
			if (localVariables.empty())
				return -1;

			// parameters were parsed in order into the vector;
			// they are indexed backwards from the local stack frame, so if the parameters were a, b, c
			// the runtime indices are a:2 b:1 c:0
			// search from most recently pushed because of scoping, eg in this case
			// for x in range(): for y in range(): local real x ...... ; ;
			// the innermost x should be found within the y loop, and the outermost in the x loop
			int m = (int)localVariables.size() - 1;
			for (int i = m; i >= 0; --i)
				if (!strcmp(name, localVariables[i].first.c_str()))
					return i;
			return -1;
		}

	};

	ActorAssembler::ActorAssembler(Library* l) : _context(new Context(l)) {}
	ActorAssembler::~ActorAssembler() {}

	Library* ActorAssembler::library() const {
		return _context->libs;
	}


	const std::map<std::string, std::shared_ptr<Lab::Bson>>& ActorAssembler::assembledGlobalBsonVariables() const {
		return _context->bsonGlobals;
	}
	const std::map<std::string, std::shared_ptr<MachineDefinition>>& ActorAssembler::assembledMachineDefinitions() const {
		return _context->machineDefinitions;
	}
	const std::map<std::string, std::shared_ptr<Landru::Property>>& ActorAssembler::assembledGlobalVariables() const {
		return globals;
	}



	void ActorAssembler::beginMachine(const char* name) {
		_context->beginMachine(name);
	}

	void ActorAssembler::endMachine() {
		_context->endMachine();
	}

	void ActorAssembler::beginState(const char *name) {
		_context->beginState(name);
	}

	void ActorAssembler::endState() {
		_context->endState();
	}

	void ActorAssembler::beginConditionalClause() {
	}

	void ActorAssembler::beginContraConditionalClause() {
		_context->currInstr.pop_back();
		_context->currInstr.emplace_back(&_context->currConditional.back()->contraConditionalInstructions);
	}

	void ActorAssembler::endConditionalClause() {
		_context->currInstr.pop_back();
		shared_ptr<Context::Conditional> conditional = _context->currConditional.back();
		_context->currConditional.pop_back();
		switch (conditional->op) {
		case Context::Conditional::Op::unknown:
			break;
		case Context::Conditional::Op::eq:
			_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState {
				float test1 = run.self->pop<float>();
				float test2 = run.self->pop<float>();

				RunState runstate = RunState::Continue;
				if (run.vm->activateMeta) {
					if (test1 == test2) {
						for (auto& i : conditional->conditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
					else {
						for (auto i : conditional->contraConditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
				}
				else {
					if (test1 == test2) {
						for (auto& i : conditional->conditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
					else {
						for (auto i : conditional->contraConditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
				}
				return runstate;
			}, "Op::Eq"));
			break;
		case Context::Conditional::Op::eq0:
			_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState {
				RunState runstate = RunState::Continue;
				float test = run.self->pop<float>();
				if (run.vm->activateMeta) {
					if (test == 0) {
						for (auto& i : conditional->conditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
					else {
						for (auto i : conditional->contraConditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
				}
				else {
					if (test == 0) {
						for (auto& i : conditional->conditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
					else {
						for (auto i : conditional->contraConditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
				}
				return runstate;
			}, "Op::Eq0"));
			break;
		case Context::Conditional::Op::lte0:
			_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState {
				RunState runstate = RunState::Continue;
				float test = run.self->pop<float>();
				if (run.vm->activateMeta) {
					if (test <= 0) {
						for (auto& i : conditional->conditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
					else {
						for (auto i : conditional->contraConditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
				}
				else {
					if (test <= 0) {
						for (auto& i : conditional->conditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
					else {
						for (auto i : conditional->contraConditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
				}
				return runstate;
			}, "Op::lte0"));
			break;
		case Context::Conditional::Op::gte0:
			_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState {
				RunState runstate = RunState::Continue;
				float test = run.self->pop<float>();
				if (run.vm->activateMeta) {
					if (test >= 0) {
						for (auto& i : conditional->conditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
					else {
						for (auto i : conditional->contraConditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
				}
				else {
					if (test >= 0) {
						for (auto& i : conditional->conditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
					else {
						for (auto i : conditional->contraConditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
				}
				return runstate;
			}, "Op::gte0"));
			break;
		case Context::Conditional::Op::lt0:
			_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState {
				RunState runstate = RunState::Continue;
				float test = run.self->pop<float>();
				if (run.vm->activateMeta) {
					if (test < 0) {
						for (auto& i : conditional->conditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
					else {
						for (auto i : conditional->contraConditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
				}
				else {
					if (test < 0) {
						for (auto& i : conditional->conditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
					else {
						for (auto i : conditional->contraConditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
				}
				return runstate;
			}, "Op::lte0"));
			break;
		case Context::Conditional::Op::gt0:
			_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState {
				RunState runstate = RunState::Continue;
				float test = run.self->pop<float>();
				if (run.vm->activateMeta) {
					if (test > 0) {
						for (auto& i : conditional->conditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
					else {
						for (auto i : conditional->contraConditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
				}
				else {
					if (test > 0) {
						for (auto& i : conditional->conditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
					else {
						for (auto i : conditional->contraConditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
				}
				return runstate;
			}, "Op::gte0"));
			break;
		case Context::Conditional::Op::neq0:
			_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState {
				RunState runstate = RunState::Continue;
				float test = run.self->pop<float>();
				if (run.vm->activateMeta) {
					if (test != 0) {
						for (auto& i : conditional->conditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
					else {
						for (auto i : conditional->contraConditionalInstructions) {
							i.second.exec(run);
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
						}
					}
				}
				else {
					if (test != 0) {
						for (auto& i : conditional->conditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
					else {
						for (auto i : conditional->contraConditionalInstructions)
							if ((runstate = i.first(run)) != RunState::Continue)
								break;
					}
				}
				return runstate;
			}, "Op::neq0"));
			break;
		}
	}

	void ActorAssembler::beginForEach(const char *name, const char *type) {
		// for each will call Run as if executing substates
		// in the case of for body in bodies, type will be varobj and name will be body
		// the local parameters will be created, pushed, and cleaned up by the forEach instruction itself
		//
		_context->localVariables.emplace_back(make_pair(name, type));
		_context->currConditional.emplace_back(make_shared<Context::Conditional>());
		_context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
	}

	void ActorAssembler::endForEach()
	{
		shared_ptr<Context::Conditional> conditional = _context->currConditional.back();
		_context->currConditional.pop_back();
		_context->currInstr.pop_back();
		_context->currInstr.back()->emplace_back(Instruction([conditional](FnContext& run)->RunState
		{
			RunState runstate = RunState::Continue;
			auto genVarPtr = run.self->popVar();
			auto generatorVar = reinterpret_cast<Wires::Data<shared_ptr<Generator>>*>(genVarPtr.get());
			auto generator = generatorVar->value();

			auto factory = run.vm->libs->findFactory(generator->typeName());
			auto local = factory(*run.vm);
			auto var = run.self->push_local(std::string("gen"), std::string(generator->typeName()), local);

			for (generator->begin(); !generator->done(); generator->next())
			{
				generator->generate(var.get());
				if (run.vm->activateMeta) {
					for (auto& i : conditional->conditionalInstructions) {
						i.second.exec(run);
						if ((runstate = i.first(run)) != RunState::Continue)
							break;
					}
				}
				else
					for (auto& i : conditional->conditionalInstructions)
						if ((runstate = i.first(run)) != RunState::Continue)
							break;
				generator->finalize(run);
			}

			run.self->pop_local(); // discard the local variable
			return runstate;
		}, "endForEach"));
		_context->localVariables.pop_back();
	}

	void ActorAssembler::beginOn() {
		_context->currConditional.emplace_back(make_shared<Context::Conditional>());
		_context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
		// the instructions compiled next will be the on clause, for example
		// on time.after(3)
	}

	void ActorAssembler::beginOnStatements() {
		_context->currConditional.emplace_back(make_shared<Context::Conditional>());
		_context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
		// the statements following are what to do when the on clause is satisfied
	}

	void ActorAssembler::endOnStatements() {
		shared_ptr<Context::Conditional> onStatements = _context->currConditional.back();
		_context->currConditional.pop_back();
		_context->currInstr.pop_back();
		shared_ptr<Context::Conditional> onStatement = _context->currConditional.back();
		_context->currConditional.pop_back();
		_context->currInstr.pop_back();
		_context->currInstr.back()->emplace_back(Instruction([onStatement, onStatements](FnContext& run)->RunState
		{
			RunState runstate = RunState::Continue;
			// push the statements to execute if the on fires
			run.self->stack.back().emplace_back(make_shared<Wires::Data<vector<Instruction>>>(onStatements->conditionalInstructions));

			// the on-statement must consume the on-statements
			if (run.vm->activateMeta) {
				for (auto& i : onStatement->conditionalInstructions) {
					i.second.exec(run);
					if ((runstate = i.first(run)) != RunState::Continue)
						break;
				}
			}
			else
				for (auto& i : onStatement->conditionalInstructions)
					if ((runstate = i.first(run)) != RunState::Continue)
						break;
			return runstate;
		}, "endOn"));
	}


	void ActorAssembler::paramsStart() {
	}
	void ActorAssembler::paramsEnd() {
	}


	void ActorAssembler::beginLocalVariableScope() {
		_context->localVariableState.emplace_back(0);
	}

	void ActorAssembler::addLocalVariable(const char* name, const char* type) {
		string nStr(name);
		string tStr(type);
		_context->localVariables.emplace_back(make_pair(name, type));
		_context->currInstr.back()->emplace_back(Instruction([nStr, tStr](FnContext& run)->RunState
		{
			auto factory = run.vm->libs->findFactory(tStr.c_str());
			auto local = factory(*run.vm);
			run.self->push_local(nStr, tStr, local);
			return RunState::Continue;
		}, "addLocalVariable"));
	}

	void ActorAssembler::endLocalVariableScope() {
		int localsCount = _context->localVariableState[_context->localVariableState.size() - 1];
		_context->localVariableState.pop_back();

		for (int i = 0; i < localsCount; ++i)
			_context->localVariables.pop_back();

		_context->currInstr.back()->emplace_back(Instruction([localsCount](FnContext& run)->RunState
		{
			for (int i = 0; i < localsCount; ++i)
				run.self->locals.pop_back();
			return RunState::Continue;
		}, "endLocalVariableScope"));
	}


	void ActorAssembler::addRequire(const char* name, const char* module)
	{
		_context->requireAliases[name] = module;
	}

	std::vector<std::string> ActorAssembler::requires()
	{
		std::vector<std::string> r;
		for (auto i : _context->requireAliases)
			r.push_back(i.second);
		return r;
	}

	void ActorAssembler::callFunction(const char *fnName)
	{
		string f(fnName);
		vector<string> parts = TextScanner::Split(f, string("."));
		if (parts.size() == 0)
			AB_RAISE("Invalid function name" << f);

		enum class CallOn {
			callOnSelf, callOnProperty, callOnRequire
		};

		CallOn callOn = CallOn::callOnSelf;

		int index = 0;
		while (index < parts.size() - 1) {
			if (_context->currMachineDefinition->properties.find(parts[index]) != _context->currMachineDefinition->properties.end()) {
				callOn = CallOn::callOnProperty;
				break;
			}
			if (_context->requireAliases.find(parts[index]) != _context->requireAliases.end()) {
				callOn = CallOn::callOnRequire;
				break;
			}
			AB_RAISE("Unknown identifier " << parts[index] << " while parsing " << fnName);
		}

		// finally got to a function to call
		switch (callOn) {
		case CallOn::callOnSelf: {
			Library::Vtable const*const lib = _context->libs->findVtable("fiber");
			if (!lib) {
				AB_RAISE("No library named fiber exists");
			}

			auto fnEntry = lib->function(parts[index].c_str());
			if (!fnEntry)
				AB_RAISE("Function named \"" << parts[index] << "\" does not exist on library: fiber");

			auto fn = fnEntry->fn;
			string str = "self call to " + parts[index];
			_context->currInstr.back()->emplace_back(Instruction([fn, str](FnContext& run)->RunState
			{
				return fn(run);
			}, str.c_str()));
			break;
		}

		case CallOn::callOnRequire: {
			auto requiresDef = _context->requireAliases.find(parts[0]);
			if (requiresDef == _context->requireAliases.end()) {
				// don't allow use of libs unless referenced by a require statement
				AB_RAISE("Can't find require library " << parts[0]);
			}

			Library::Vtable const* lib = nullptr;
			if (parts.size() == 1) {
				lib = _context->libs->findVtable(parts[0].c_str());
				if (!lib)
					AB_RAISE("No std library named " << parts[0] << " exists");
			}
			else if (parts.size() == 2) {
				lib = _context->libs->findVtable(parts[0].c_str());
				if (!lib)
					for (auto& i : _context->libs->libraries) {
						if (i.name == parts[0]) {
							lib = i.findVtable(parts[0].c_str());
							if (!lib)
								AB_RAISE("No vtable for library named " << parts[0]);
						}
					}
			}
			else
				AB_RAISE("Couldn't find function " << f);

			auto fnEntry = lib->function(parts[1].c_str());
			if (!fnEntry)
				AB_RAISE("Function " << parts[1] << " does not exist on library: " << parts[0]);

			auto fn = fnEntry->fn;
			string str = "library call on " + parts[0] + " to " + parts[1];
			string libraryName = lib->name;
			_context->currInstr.back()->emplace_back(Instruction([fn, libraryName](FnContext& run)->RunState 
			{
				FnContext fnRun(run);
				fnRun.library = run.vm->getLibraryInstanceData(libraryName).get();
				return fn(fnRun);
			}, str.c_str()));
			break;
		}

		case CallOn::callOnProperty:
			auto property = _context->currMachineDefinition->properties.find(parts[0]);
			if (property == _context->currMachineDefinition->properties.end()) {
				AB_RAISE("Property " << parts[0] << " not found for function call " << f);
			}
			string type = property->second->type;
			vector<string> typeParts = TextScanner::Split(type, ".");

			if (typeParts.size() == 0) {
				// type must be a std type
				Library::Vtable const*const lib = _context->libs->findVtable(typeParts[0].c_str());
				if (!lib) {
					AB_RAISE("No std library named " << typeParts[0] << " exists for function call " << f << " on property of type " << type);
				}
				AB_RAISE("@TODO callOnProperty");
				_context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState 
				{
					// we know what the lambda to call is, but we're going to have to look up the property named parts[0] at runtime
					// call function on require parts[index] and then clear the params
					// &&& @TODO
					return RunState::UndefinedBehavior;
				}, "callOnStdProperty"));
			}
			else {
				bool found = false;
				for (auto& i : _context->libs->libraries) {
					if (i.name == typeParts[0]) {
						// found the right library, find the vtable on the library, eg audio.buffer
						Library::Vtable const*const lib = i.findVtable(typeParts[1].c_str());
						if (!lib)
							AB_RAISE("No library named " << typeParts[0] << " exists for function call " << f << " on property of type " << type);
						auto fnEntry = lib->function(parts[1].c_str());
						if (!fnEntry)
							AB_RAISE("Function " << parts[1] << " does not exist on library: " << parts[0]);

						auto fn = fnEntry->fn;
						string str = "library call on property '" + parts[0] + "' to " + type + "." + parts[1];
						string propertyName = parts[0];
						string libName = i.name;
						shared_ptr<Wires::TypedData> libInstance = i.libraryInstanceData;
						_context->currInstr.back()->emplace_back(Instruction([propertyName, libName, fn](FnContext& run)->RunState
						{
							FnContext fnRun(run);
							fnRun.library = run.vm->getLibraryInstanceData(libName).get();
							auto property = run.vm->findInstance(run.self, propertyName);
							if (!property)
								VM_RAISE("Couldn't find property: " << propertyName);

							fnRun.var = property->data.get();
							return fn(fnRun);
						}, str.c_str()));
						found = true;
						break;
					}
				}
				if (!found)
					AB_RAISE("No library named " << typeParts[0] << " exists for function call " << f << " on property of type " << type);
			}
			break;
		}
	}

	void ActorAssembler::storeToVar(const char *name)
	{
		/// @TODO need to decompose name, discover where the variable is and store the appropriate lambda
		string parts(name);
		string str = "store to " + parts;

		// prefer the local scope
		int localIndex = _context->localVariableIndex(name);
		if (localIndex >= 0)
		{
			_context->currInstr.back()->emplace_back(Instruction([localIndex](FnContext& run)->RunState
			{
				auto data = run.self->popVar();
				run.self->locals[localIndex]->assign(data, true);
				return RunState::Continue;
			}, str.c_str()));
		}
		else if (_context->currMachineDefinition->properties.find(parts) != _context->currMachineDefinition->properties.end())
		{
			_context->currInstr.back()->emplace_back(Instruction([parts, str](FnContext& run)->RunState
			{
				auto prop = run.vm->findInstance(run.self, parts); // already checked at compile time
				auto data = run.self->popVar();
				prop->copy(data, true);
				return RunState::Continue;
			}, str.c_str()));
		}
		else {
			AB_RAISE("Couldn't find variable: " << string(name));
		}
	}

	void ActorAssembler::initializeSharedVarIfNecessary(const char * name)
	{
		/// @TODO need to decompose name, discover where the variable is and store the appropriate lambda
		string parts(name);
		string str = "initialize shared var " + parts;
		if (_context->currMachineDefinition->properties.find(parts) == _context->currMachineDefinition->properties.end()) {
			AB_RAISE("Couldn't find shared variable: " << string(name));
			return;
		}
		_context->currInstr.back()->emplace_back(Instruction([parts, str](FnContext& run)->RunState
		{
			auto prop = run.vm->findInstance(run.self, parts); // already checked at compile time
			auto data = run.self->popVar();
			if (!prop->assignCount) {
				prop->data->copy(data.get());
				++prop->assignCount;
			}
			return RunState::Continue;
		}, str.c_str()));
	}

    void ActorAssembler::addGlobalBson(const char *name, std::shared_ptr<Lab::Bson> bson) 
	{
        _context->bsonGlobals[name] = bson;
    }

	void ActorAssembler::addGlobalString(const char* name, const char* value) {
		std::shared_ptr<Property> prop = std::make_shared<Property>();
		prop->name.assign(name);
		prop->type.assign("string");
		prop->visibility = Property::Visibility::Global;
		std::shared_ptr<Wires::TypedData> val = std::make_shared<Wires::Data<std::string>>(string(value));
		prop->assign(val, false);
		globals[name] = prop;
	}
	void ActorAssembler::addGlobalInt(const char* name, int value) {
		std::shared_ptr<Property> prop = std::make_shared<Property>();
		prop->name.assign(name);
		prop->type.assign("int");
		prop->visibility = Property::Visibility::Global;
		std::shared_ptr<Wires::TypedData> val = std::make_shared<Wires::Data<int>>(value);
		prop->assign(val, false);
		globals[name] = prop;
	}
	void ActorAssembler::addGlobalFloat(const char* name, float value) {
		std::shared_ptr<Property> prop = std::make_shared<Property>();
		prop->name.assign(name);
		prop->type.assign("float");
		prop->visibility = Property::Visibility::Global;
		std::shared_ptr<Wires::TypedData> val = std::make_shared<Wires::Data<float>>(value);
		prop->assign(val, false);
		globals[name] = prop;
	}

    void ActorAssembler::addSharedVariable(const char *name, const char *type) {
        Property *prop = new Property();
        prop->name.assign(name);
        prop->type.assign(type);
        prop->visibility = Property::Visibility::Shared;
        _context->currMachineDefinition->properties[name] = prop;
    }

    void ActorAssembler::addInstanceVariable(const char *name, const char *type) {
        Property *prop = new Property();
        prop->name.assign(name);
        prop->type.assign(type);
        prop->visibility = Property::Visibility::ActorLocal;
        _context->currMachineDefinition->properties[name] = prop;
    }

    void ActorAssembler::pushConstant(int i) {
        _context->currInstr.back()->emplace_back(Instruction([i](FnContext& run)->RunState
		{
            run.self->stack.back().emplace_back(make_shared<Wires::Data<int>>(i));
			return RunState::Continue;
		}, "pushIntConstant"));
    }

    void ActorAssembler::pushFloatConstant(float f) {
        char buff[32];
        sprintf(buff, "%f", f);
        string str("push float constant: ");
        str += buff;
		string s(str);
        _context->currInstr.back()->emplace_back(Instruction([f, s](FnContext& run)->RunState
		{
            run.self->stack.back().emplace_back(make_shared<Wires::Data<float>>(f));
			return RunState::Continue;
		}, str.c_str()));
    }

    void ActorAssembler::pushStringConstant(const char *str) {
        string s(str);
        string verbose = "push string constant: " + s;
        _context->currInstr.back()->emplace_back(Instruction([s, verbose](FnContext& run)->RunState
		{
            run.self->stack.back().emplace_back(make_shared<Wires::Data<string>>(s));
			return RunState::Continue;
		}, verbose.c_str()));
    }

    void ActorAssembler::pushRangedRandom(float r1, float r2) {
        _context->currInstr.back()->emplace_back(Instruction([r1, r2](FnContext& run)->RunState
		{
            float r = (float)rand()/RAND_MAX;
            r *= (r2 - r1);
            r += r1;
            run.self->stack.back().emplace_back(make_shared<Wires::Data<float>>(r));
			return RunState::Continue;
		}, "pushRangedRandom"));
    }

    void ActorAssembler::pushInstanceVar(const char* name) {
        string str(name);
        if (_context->currMachineDefinition->properties.find(str) == _context->currMachineDefinition->properties.end())
            AB_RAISE("Instance variable " << str << " not found on machine" << _context->currMachineDefinition->name);

        _context->currInstr.back()->emplace_back(Instruction([str](FnContext& run)->RunState
		{
			auto i = run.vm->findInstance(run.self, str);
            run.self->stack.back().emplace_back(i->data);
			return RunState::Continue;
		}, "pushInstanceVar"));
    }

    void ActorAssembler::pushLocalVar(const char *varName) {
        int var = _context->localVariableIndex(varName);
        if (var < 0)
            AB_RAISE("Unknown local variable " << varName << " on machine" << _context->currMachineDefinition->name);
        _context->currInstr.back()->emplace_back(Instruction([var](FnContext& run)->RunState
		{
            run.self->stack.back().emplace_back(run.self->locals[var]->data);
			return RunState::Continue;
		}, "pushLocalVar"));
    }

    void ActorAssembler::pushSharedVar(const char* name) {
        string str(name);
        if (_context->currMachineDefinition->properties.find(str) == _context->currMachineDefinition->properties.end())
            AB_RAISE("Shared variable " << str << " not found on machine" << _context->currMachineDefinition->name);

        _context->currInstr.back()->emplace_back(Instruction([str](FnContext& run)->RunState
		{
			auto i = run.vm->findInstance(run.self, str);
			run.self->stack.back().emplace_back(i->data);
			return RunState::Continue;
		}, "pushSharedVar"));
    }

    void ActorAssembler::pushGlobalBsonVar(const char *varName) {
        string str(varName);
        if (_context->currMachineDefinition->properties.find(str) == _context->currMachineDefinition->properties.end())
            AB_RAISE("Shared variable " << str << " not found on machine" << _context->currMachineDefinition->name);

        _context->currInstr.back()->emplace_back(Instruction([str](FnContext& run)->RunState
		{
            auto i = run.vm->bsonGlobals.find(str);
            run.self->stack.back().emplace_back(i->second);
			return RunState::Continue;
		}, "pushGlobalBsonVar"));
    }

	void ActorAssembler::pushGlobalVar(const char *varName) {
		string str(varName);
		if (globals.find(str) == globals.end())
			AB_RAISE("Global variable " << str << " not found");

		_context->currInstr.back()->emplace_back(Instruction([str](FnContext & run)->RunState
		{
			auto i = run.vm->findGlobal(str);
			run.self->stack.back().emplace_back(i->data);
			return RunState::Continue;
		}, "pushGlobalVar"));
	}

    void ActorAssembler::pushInstanceVarReference(const char* varName)
	{
		string str(varName);
		if (_context->currMachineDefinition->properties.find(str) == _context->currMachineDefinition->properties.end())
			AB_RAISE("Instance variable " << str << " not found on machine" << _context->currMachineDefinition->name);

		_context->currInstr.back()->emplace_back(Instruction([str](FnContext & run)->RunState
		{
			auto i = run.vm->findInstance(run.self, str);
			run.self->stack.back().emplace_back(make_shared<Wires::Data<shared_ptr<Property>>>(i));
			return RunState::Continue;
		}, "pushInstanceVarReference"));
	}

    void ActorAssembler::pushGlobalVarReference(const char* varName)
	{
		string str(varName);
		if (globals.find(str) == globals.end())
			AB_RAISE("Global variable " << str << " not found");

		_context->currInstr.back()->emplace_back(Instruction([str](FnContext & run)->RunState
		{
			auto i = run.vm->findGlobal(str);
			run.self->stack.back().emplace_back(make_shared<Wires::Data<shared_ptr<Property>>>(i));
			return RunState::Continue;
		}, "pushGlobalVarReference"));
	}

    void ActorAssembler::pushSharedVarReference(const char* varName)
	{
		AB_RAISE("pushSharedVarReference not implemented" << varName);
	}

    void ActorAssembler::ifEq() {
        _context->currConditional.emplace_back(make_shared<Context::Conditional>());
        _context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
        _context->currConditional.back()->op = Context::Conditional::Op::eq;
    }

    void ActorAssembler::ifLte0() {
        _context->currConditional.emplace_back(make_shared<Context::Conditional>());
        _context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
        _context->currConditional.back()->op = Context::Conditional::Op::lte0;
    }

    void ActorAssembler::ifGte0() {
        _context->currConditional.emplace_back(make_shared<Context::Conditional>());
        _context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
        _context->currConditional.back()->op = Context::Conditional::Op::gte0;
    }

    void ActorAssembler::ifLt0() {
        _context->currConditional.emplace_back(make_shared<Context::Conditional>());
        _context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
        _context->currConditional.back()->op = Context::Conditional::Op::lt0;
    }

    void ActorAssembler::ifGt0() {
        _context->currConditional.emplace_back(make_shared<Context::Conditional>());
        _context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
        _context->currConditional.back()->op = Context::Conditional::Op::gt0;
    }

    void ActorAssembler::ifEq0() {
        _context->currConditional.emplace_back(make_shared<Context::Conditional>());
        _context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
        _context->currConditional.back()->op = Context::Conditional::Op::eq0;
    }

    void ActorAssembler::ifNotEq0() {
        _context->currConditional.emplace_back(make_shared<Context::Conditional>());
        _context->currInstr.emplace_back(&_context->currConditional.back()->conditionalInstructions);
        _context->currConditional.back()->op = Context::Conditional::Op::neq0;
    }

    void ActorAssembler::opAdd() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            float v2 = run.self->pop<float>();
            float v1 = run.self->pop<float>();
            run.self->push<float>(v1 + v2);
			return RunState::Continue;
		}, "opAdd"));
    }

    void ActorAssembler::opSubtract() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            float v2 = run.self->pop<float>();
            float v1 = run.self->pop<float>();
            run.self->push<float>(v1 - v2);
			return RunState::Continue;
		}, "opSubtract"));
    }

    void ActorAssembler::opMultiply() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            float v2 = run.self->pop<float>();
            float v1 = run.self->pop<float>();
            run.self->push<float>(v1 * v2);
			return RunState::Continue;
		}, "opMultiply"));
    }

    void ActorAssembler::opDivide() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState {
            float v2 = run.self->pop<float>();
            float v1 = run.self->pop<float>();
            run.self->push<float>(v1 / v2);
			return RunState::Continue;
		}, "opDivide"));
    }

    void ActorAssembler::opNegate() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            float v1 = run.self->pop<float>();
            run.self->push<float>(-v1);
			return RunState::Continue;
		}, "opNegate"));
    }

    void ActorAssembler::opModulus() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            float v2 = run.self->pop<float>();
            float v1 = run.self->pop<float>();
            run.self->push<float>(fmodf(v1, v2));
			return RunState::Continue;
		}, "opModulus"));
    }

    void ActorAssembler::opGreaterThan() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            float v2 = run.self->pop<float>();
            float v1 = run.self->pop<float>();
            run.self->push<float>(v1 > v2 ? 1.f : 0.f);
			return RunState::Continue;
		}, "opGreaterThan"));
    }

    void ActorAssembler::opLessThan() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            float v2 = run.self->pop<float>();
            float v1 = run.self->pop<float>();
            run.self->push<float>(v1 < v2 ? 1.f : 0.f);
			return RunState::Continue;
		}, "opLessThan"));
    }

    void ActorAssembler::launchMachine() {
        _context->currInstr.back()->emplace_back(Instruction([](FnContext& run)->RunState
		{
            string machine = run.self->pop<string>();
            run.vm->launchQueue.emplace_back(Landru::VMContext::LaunchRecord(machine, Landru::Fiber::Stack()));
			return RunState::Continue;
		}, "launchMachine"));
    }

    void ActorAssembler::gotoState(const char *stateName) {
        string s(stateName);
        _context->currInstr.back()->emplace_back(Instruction([s](FnContext& run)->RunState
		{
			run.vm->enqueueGoto(run.self, s);
//            run.self->gotoState(run, s.c_str(), true);
			return RunState::Goto;
		}, "gotoState"));
    }

} // Landru
