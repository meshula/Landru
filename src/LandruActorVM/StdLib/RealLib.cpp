//
//  RealLib.cpp
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//


#include "RealLib.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Generator.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"

#include <algorithm>
#include <cmath>
#include <memory>
using namespace std;

namespace Landru {
    namespace Std {
        
        class RealRangeGenerator : public Generator {
        public:
            RealRangeGenerator(float first, float last, float incr)
            : first(first), last(last), incr(incr), curr(first) {}
            virtual ~RealRangeGenerator() {}
            
            virtual void begin() override {}
            virtual bool done() override { return curr >= last; }
            virtual void next() override { curr += incr; }
            
            virtual void generate(Wires::TypedData* var) override {
                Wires::Data<float>* floatVar = dynamic_cast<Wires::Data<float>*>(var);
                floatVar->setValue(curr);
            }
            virtual void finalize(FnContext&) override {}
            
            virtual const char* typeName() const override { return "real"; }
            
            float first, last, incr, curr;
        };

        //-------------
        // Real Libary \__________________________________________
        void RealLib::registerLib(Library& l) {
            auto u = unique_ptr<Library::Vtable>(new Library::Vtable("real"));
            u->registerFn("2.0", "add", "ff", "f", add);
            u->registerFn("2.0", "sub", "ff", "f", sub);
            u->registerFn("2.0", "mul", "ff", "f", mul);
            u->registerFn("2.0", "div", "ff", "f", div);
            u->registerFn("2.0", "mod", "ff", "f", mod);
            u->registerFn("2.0", "max", "ff", "f", min);
            u->registerFn("2.0", "min", "ff", "f", max);
            u->registerFn("2.0", "range", "fff", "o", range);
            l.registerVtable(move(u));
            l.registerFactory("real", [](VMContext&)->std::shared_ptr<Wires::TypedData>{ return std::make_shared<Wires::Data<float>>(0.f); });
			l.registerFactory("float", [](VMContext&)->std::shared_ptr<Wires::TypedData> { return std::make_shared<Wires::Data<float>>(0.f); });
		}
        RunState RealLib::add(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 + f2);
			return RunState::Continue;
        }
		RunState RealLib::sub(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 - f2);
			return RunState::Continue;
        }
		RunState RealLib::mul(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 * f2);
			return RunState::Continue;
        }
		RunState RealLib::div(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(f1 / f2);
			return RunState::Continue;
        }
		RunState RealLib::mod(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(fmodf(f1, f2));
			return RunState::Continue;
		}
		RunState RealLib::min(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(std::min(f1, f2));
			return RunState::Continue;
		}
		RunState RealLib::max(FnContext& run) {
            float f1 = run.self->back<float>(-2);
            float f2 = run.self->pop<float>();
            auto param = run.self->stack.back().back();
            auto v1 = reinterpret_cast<Wires::Data<float>*>(param.get());
            v1->setValue(std::max(f1, f2));
			return RunState::Continue;
		}
		RunState RealLib::range(FnContext& run) {
            auto& params = run.self->stack.back();
            float incr = 1.f;
            float first = 0.f;
            float last = 0.f;
            if (params.size() == 3) {
                incr = run.self->pop<float>();
            }
            if (params.size() == 2) {
                last = run.self->pop<float>();
            }
            if (params.size() == 1) {
                first = run.self->pop<float>();
            }
            auto gen = make_shared<Wires::Data<shared_ptr<RealRangeGenerator>>>();
            gen->setValue(make_shared<RealRangeGenerator>(first, last, incr));
            run.self->pushVar(gen);
			return RunState::Continue;
		}
        
    } // Std
} // Landru
