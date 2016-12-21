//
//  IntLib.cpp
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//


#include "IntLib.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"

#include <algorithm>
#include <cmath>
#include <memory>
using namespace std;

namespace Landru {
    namespace Std {
        
        //-----------
        // Int Libary \__________________________________________
        void IntLib::registerLib(Library& l) {
            auto u = unique_ptr<Library::Vtable>(new Library::Vtable("int"));
            u->registerFn("2.0", "add", "ii", "i", add);
            u->registerFn("2.0", "sub", "ii", "i", sub);
            u->registerFn("2.0", "mul", "ii", "i", mul);
            u->registerFn("2.0", "div", "ii", "i", div);
            u->registerFn("2.0", "mod", "ii", "i", mod);
            u->registerFn("2.0", "max", "ii", "i", min);
            u->registerFn("2.0", "min", "ii", "i", max);
            l.registerVtable(move(u));
            l.registerFactory("int", [](VMContext&)->std::shared_ptr<Wires::TypedData>{ return std::make_shared<Wires::Data<int>>(0); });
        }
		RunState IntLib::add(FnContext& run) {
            int i1 = run.self->back<int>(-2);
            int i2 = run.self->pop<int>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(param.get());
            v1->setValue(i1 + i2);
			return RunState::Continue;
        }
		RunState IntLib::sub(FnContext& run) {
            int i1 = run.self->back<int>(-2);
            int i2 = run.self->pop<int>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(param.get());
            v1->setValue(i1 - i2);
			return RunState::Continue;
		}
		RunState IntLib::mul(FnContext& run) {
            int i1 = run.self->back<int>(-2);
            int i2 = run.self->pop<int>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(param.get());
            v1->setValue(i1 * i2);
			return RunState::Continue;
		}
		RunState IntLib::div(FnContext& run) {
            int i1 = run.self->back<int>(-2);
            int i2 = run.self->pop<int>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(param.get());
            v1->setValue(i1 / i2);
			return RunState::Continue;
		}
		RunState IntLib::mod(FnContext& run) {
            int i1 = run.self->back<int>(-2);
            int i2 = run.self->pop<int>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(param.get());
            v1->setValue(i1 % i2);
			return RunState::Continue;
		}
		RunState IntLib::min(FnContext& run) {
            int i1 = run.self->back<int>(-2);
            int i2 = run.self->pop<int>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(param.get());
            v1->setValue(std::min(i1, i2));
			return RunState::Continue;
		}
		RunState IntLib::max(FnContext& run) {
            int i1 = run.self->back<int>(-2);
            int i2 = run.self->pop<int>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<int>*>(param.get());
            v1->setValue(std::max(i1, i2));
			return RunState::Continue;
		}
        
    } // Std
} // Landru
