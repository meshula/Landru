//
//  TimeLib.cpp
//  Landru
//
//  Created by Nick Porcino on 11/10/14.
//
//

#include "TimeLib.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/State.h"
#include "LandruActorVM/VMContext.h"
#include <cmath>
#include <memory>
#include <vector>
using namespace std;

namespace Landru {
    namespace Std {
        
        //-------------
        // Real Libary \__________________________________________
        void TimeLib::registerLib(Library& l) {
            auto u = unique_ptr<Library::Vtable>(new Library::Vtable("time"));
            u->registerFn("2.0", "after", "ff", "f", after);
            u->registerFn("2.0", "every", "ff", "f", every);
            u->registerFn("2.0", "recur", "ff", "f", recur);
            l.registerVtable(move(u));
        }
        void TimeLib::after(FnContext& run) {
            vector<Instruction> instr = run.self->back<vector<Instruction>>(-2);
            float delay = run.self->pop<float>();
            run.self->popVar(); // drop the instr
            int recurrences = 1;
            run.vm->onTimeout(delay, recurrences, *run.self, instr);
        }
        void TimeLib::every(FnContext& run) {
            vector<Instruction> instr = run.self->back<vector<Instruction>>(-2);
            float delay = run.self->pop<float>();
            run.self->popVar(); // drop the instr
            int recurrences = -1;
            run.vm->onTimeout(delay, recurrences, *run.self, instr);
        }
        void TimeLib::recur(FnContext& run) {
            vector<Instruction> instr = run.self->back<vector<Instruction>>(-3);
            int recurrences = run.self->pop<int>();
            float delay = run.self->pop<float>();
            run.self->popVar(); // drop the instr
            run.vm->onTimeout(delay, recurrences, *run.self, instr);
        }
        
    } // Std
} // Landru
