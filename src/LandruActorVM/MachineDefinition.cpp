//
//  MachineDefinition.cpp
//  Landru
//
//  Created by Nick Porcino on 10/28/14.
//
//

#include "MachineDefinition.h"
#include "Property.h"
#include "State.h"

namespace Landru {
    
    MachineDefinition::~MachineDefinition() {
        for (auto i : states)
            delete i.second;
        for (auto i : properties)
            delete i.second;
    }
    
}
