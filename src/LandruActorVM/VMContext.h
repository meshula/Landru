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
#include <memory>
#include <string>
#include <vector>

namespace Landru {
    class Library;
    class MachineDefinition;
    class Fiber;
    class Property;
    
    class VMContext {
        class Detail;
        std::unique_ptr<Detail> _detail;
        
    public:
        VMContext();
        ~VMContext();
        
        const float TIME_QUANTA = 1.e-4f;
        
        void instantiateLibs();
        void update(double now);
        
        void onTimeout(float delay, int recurrences, const Fiber& self,
                       std::vector<Instruction> instr);
        
        bool deferredMessagesPending() const;
        bool undeferredMessagesPending() const;
        
        std::shared_ptr<Wires::TypedData> getLibraryInstanceData(const std::string& name);
        
        typedef std::pair<std::string,
                          std::vector<std::shared_ptr<Wires::TypedData>>> LaunchRecord;
        std::deque<LaunchRecord> launchQueue;
        std::unique_ptr<Library> libs;
        std::map<std::string, std::shared_ptr<MachineDefinition>> machineDefinitions;
        std::vector<Fiber*> fibers;
        std::map<std::string, std::string> requireDefinitions;
        std::map<std::string, Property*> requires;
        std::map<std::string, std::shared_ptr<Wires::TypedData>> bsonGlobals; // when VM is instantiated make TypedData's around the bson globals
		std::map<std::string, std::shared_ptr<Landru::Property>> globals;
        bool activateMeta;
        uint32_t breakPoint;
    };
    
}