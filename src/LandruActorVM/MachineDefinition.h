//
//  MachineDefinition.h
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#pragma once

#include <map>
#include <string>

namespace Landru {
    class State;
    class Property;
    
    class MachineDefinition {
    public:
        MachineDefinition() {}
        MachineDefinition(const MachineDefinition& rhs) {
            states = rhs.states;
            name = rhs.name;
        }
        ~MachineDefinition();
        
        std::string name;
        std::map<std::string, State*> states;
        std::map<std::string, Property*> properties;
    };
    
} // Landru

