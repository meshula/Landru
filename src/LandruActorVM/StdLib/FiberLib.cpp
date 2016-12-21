//
//  FiberLib.cpp
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#include "FiberLib.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include <cmath>
#include <memory>
using namespace std;

namespace Landru {
    namespace Std {
        
        void FiberLib::registerLib(Library& l) {
            auto u = unique_ptr<Library::Vtable>(new Library::Vtable("fiber"));
            u->registerFn("2.0", "eval", "f", "f", eval);
            u->registerFn("2.0", "add", "ff", "f", add);
            u->registerFn("2.0", "sub", "ff", "f", sub);
            u->registerFn("2.0", "mul", "ff", "f", mul);
            u->registerFn("2.0", "div", "ff", "f", div);
            u->registerFn("2.0", "min", "ff", "f", min);
            u->registerFn("2.0", "max", "ff", "f", max);
            u->registerFn("2.0", "int", "f", "i", toInt);
            u->registerFn("2.0", "sqrt", "f", "f", sqrt);
            u->registerFn("2.0", "toggle", "i", "i", toggle);
            u->registerFn("2.0", "new", "s", "*", newFn);
            l.registerVtable(move(u));
        }
        
		RunState FiberLib::eval(FnContext& run) {
            // eval doesn't have to do anything, it's sugar.
			return RunState::Continue;
		}
        
		RunState FiberLib::add(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 + f2);
			return RunState::Continue;
		}
        
		RunState FiberLib::sub(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 - f2);
			return RunState::Continue;
		}
        
		RunState FiberLib::mul(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 * f2);
			return RunState::Continue;
		}
        
		RunState FiberLib::div(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 / f2);
			return RunState::Continue;
		}
        
		RunState FiberLib::min(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 > f2 ? f2 : f1);
			return RunState::Continue;
		}
        
		RunState FiberLib::max(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 > f2 ? f1 : f2);
			return RunState::Continue;
		}
        
		RunState FiberLib::toInt(FnContext& run) {
            float i = run.self->pop<float>();
            run.self->push<int>(int(i));
			return RunState::Continue;
		}
        
		RunState FiberLib::sqrt(FnContext& run) {
            float f = run.self->back<float>(-1);
            auto a = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(a.get());
            v1->setValue(sqrtf(f));
			return RunState::Continue;
		}
        
		RunState FiberLib::toggle(FnContext& run) {
            int i = run.self->back<int>(-1);
            auto a = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(a.get());
            v1->setValue(1 - i);
			return RunState::Continue;
		}
        
        //	LANDRU_DECL_FN(Fiber, send)
        //    {
        //		int message = LStackTopInt(stack); stack->pop();
        //		const char* messageString = &f->exemplar->stringData[f->exemplar->stringArray[message]];
        //		const char* machineTypeNameString = f->TopString(stack);  stack->pop();
        //		f->_engine->SendMessageToAll(machineTypeNameString, messageString);
        //	}
        
		RunState FiberLib::newFn(FnContext& run) {
            string type = run.self->pop<string>();
            auto factory = run.vm->libs->findFactory(type.c_str());
            auto v = factory(*run.vm);
            run.self->push(v);
			return RunState::Continue;
        }
        
    } // Std
} // Landru
