//
//  VMContext.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include "WiresTypedData.h"
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
		typedef void(*UpdateFn)(double, Landru::VMContext*);
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
		Fiber* fiber() { return _detail->fiber.get(); }
    };

    class VMContext
    {
        class Detail;
        std::unique_ptr<Detail> _detail;

    public:
        VMContext(Library*);
        ~VMContext();

        const float TIME_QUANTA = 1.e-4f;

        void instantiateLibs();
        void update(double now);

        OnEventEvaluator registerOnEvent(const Fiber& self, std::vector<Instruction> instr);

        void onTimeout(float delay, int recurrences, const Fiber& self,
                       std::vector<Instruction> instr);

        bool deferredMessagesPending() const;
        bool undeferredMessagesPending() const;

		void clearContinuations(Fiber* f, int level);

        std::shared_ptr<Wires::TypedData> getLibraryInstanceData(const std::string& name);

        typedef std::pair<std::string,
                          std::vector<std::shared_ptr<Wires::TypedData>>> LaunchRecord;

        std::deque<LaunchRecord> launchQueue;
        Library* libs;
        std::map<std::string, std::shared_ptr<MachineDefinition>> machineDefinitions;
        std::vector<Fiber*> fibers;
		std::vector<LandruRequire> plugins;

		std::shared_ptr<Fiber> fiberPtr(Fiber*);

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

#ifdef HAVE_VMCONTEXT_REQUIRES
        std::map<std::string, std::string> requireDefinitions;
        std::map<std::string, Property*> requires;
#endif
        std::map<std::string, std::shared_ptr<Wires::TypedData>> bsonGlobals; // when VM is instantiated make TypedData's around the bson globals
	//	std::map<std::string, std::shared_ptr<Landru::Property>> globals;
        bool activateMeta;
        uint32_t breakPoint;
    };

}