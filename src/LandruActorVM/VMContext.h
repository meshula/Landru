//
//  VMContext.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include "WiresTypedData.h"
#include "FnContext.h"
#include "State.h"

#include <deque>
#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace Landru {
    class Library;
    class MachineDefinition;
    class Fiber;
    class Property;
    class VMContext;

	class LandruRequire
	{
	public:
		typedef void(*InitFn)(Landru::Library*);
		typedef RunState(*UpdateFn)(double, Landru::VMContext*);
		typedef void(*FinishFn)(Landru::Library*);
		typedef void(*FiberExpiringFn)(Landru::Fiber*);
		typedef void(*ClearContinuationsFn)(Landru::Fiber*, int level);
		typedef bool(*PendingContinuationsFn)(Landru::Fiber*);

		explicit LandruRequire() {}
		explicit LandruRequire(const LandruRequire & rh)
		{
			*this = rh;
		}

		LandruRequire & operator=(const LandruRequire & rh)
		{
			name = rh.name;
			plugin = rh.plugin;
			init = rh.init;
			update = rh.update;
			finish = rh.finish;
			fiberExpiring = rh.fiberExpiring;
			clearContinuations = rh.clearContinuations;
			pendingContinuations = rh.pendingContinuations;
			return *this;
		}

		std::string name;
		void* plugin = nullptr;
		InitFn init = nullptr;
		UpdateFn update = nullptr;
		FinishFn finish = nullptr;
		FiberExpiringFn fiberExpiring = nullptr;
		ClearContinuationsFn clearContinuations = nullptr;
		PendingContinuationsFn pendingContinuations = nullptr;

		RunState runState = RunState::Continue;
	};



    class OnEventEvaluator
    {
	protected:
		class Detail
		{
		public:
			std::shared_ptr<Fiber> fiber;
			std::vector<Instruction> instructions;
		};
        Detail* _detail;

        friend class VMContext;
        OnEventEvaluator(std::shared_ptr<Fiber>, std::vector<Instruction>&);

    public:
        OnEventEvaluator();
        ~OnEventEvaluator();

        OnEventEvaluator(OnEventEvaluator&& rhs) : _detail(rhs._detail)
		{
			rhs._detail = nullptr;
		}
		OnEventEvaluator & operator=(OnEventEvaluator && rhs)
		{
			_detail = rhs._detail;
			rhs._detail = nullptr;
			return *this;
		}

		std::vector<Instruction>& instructions() { return _detail->instructions;  }
		Fiber* fiber() const { return _detail->fiber.get(); }
		std::shared_ptr<Fiber> fiberPtr() const { return _detail->fiber; }
    };

    class VMContext
    {
        class Detail;
        std::unique_ptr<Detail> _detail;

    public:
		const float TIME_QUANTA = 1.e-4f;

		VMContext(Library*);
        ~VMContext();

		double now() const;

		//--------------\_____________________________________________________
		// Libraries
        void instantiateLibs();
		Library* libs;							// holds all vtables
		std::vector<LandruRequire> plugins;		// holds the implementations

		//--------------\_____________________________________________________
		// Update
		bool traceEnabled;
		uint32_t breakPoint;
		void update(double now);

		//--------------\_____________________________________________________
		// Events
		OnEventEvaluator registerOnEvent(const Fiber& self, std::vector<Instruction> instr);

        bool deferredMessagesPending() const;
        bool undeferredMessagesPending() const;

		void clearContinuations(Fiber* f, int level);

		//--------------\_____________________________________________________
		// Machines
        typedef std::pair<std::string,
                          std::vector<std::shared_ptr<Wires::TypedData>>> LaunchRecord;

        std::deque<LaunchRecord> launchQueue;
		std::map<std::string, std::shared_ptr<MachineDefinition>> machineDefinitions;
		std::shared_ptr<Fiber> fiberPtr(Fiber*);
		void enqueueGoto(Fiber * f, const std::string & state);
		void finalizeGotos();

		//--------------\_____________________________________________________
		// Properties
		typedef size_t LandruIndex;
		LandruIndex propertyIndex(const std::string & str, LandruIndex parent = 0)
		{
			return std::hash<std::string>{}(str) ^ (parent << 1);
		}

		std::unordered_map<LandruIndex, std::shared_ptr<Landru::Property>> properties;

		// &&& @todo replace str with LandruIndex
		std::shared_ptr<Landru::Property> findGlobal(const std::string & str)
		{
			LandruIndex i = propertyIndex(str, 0);
			std::unordered_map<LandruIndex, std::shared_ptr<Landru::Property>>::const_iterator j = properties.find(i);
			if (j != properties.end())
				return j->second;

			return std::shared_ptr<Landru::Property>();
		}

		void storeGlobal(const std::string & str, std::shared_ptr<Landru::Property> p)
		{
			LandruIndex i = propertyIndex(str, 0);
			properties[i] = p;
		}

		// &&& @todo replace str with LandruIndex
		std::shared_ptr<Landru::Property> findInstance(const Fiber * f, const std::string & str)
		{
			LandruIndex i = propertyIndex(str, std::hash<const Fiber *>{}(f));
			std::unordered_map<LandruIndex, std::shared_ptr<Landru::Property>>::const_iterator j = properties.find(i);
			if (j != properties.end())
				return j->second;

			return std::shared_ptr<Landru::Property>();
		}

		void storeInstance(const Fiber * f, const std::string & str, std::shared_ptr<Landru::Property> p)
		{
			LandruIndex i = propertyIndex(str, std::hash<const Fiber *>{}(f));
			properties[i] = p;
		}
	};

}