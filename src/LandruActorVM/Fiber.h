//
//  Fiber.h
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//

#include "LandruActorVM/Exception.h"
#include "LandruActorVM/FnContext.h"
#include "LandruActorVM/MachineDefinition.h"
#include "LandruActorVM/Property.h"
#include "LandruActorVM/State.h"
#include "LandruActorVM/VMContext.h"
#include "LandruActorVM/WiresTypedData.h"
#include "LandruActorVM/Detail/Comparable.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Landru {

    struct InvalidId {
        constexpr InvalidId() {}
    };

    // Id class inspired by libcaf's node_id
    class Id : caf::detail::comparable<Id>, caf::detail::comparable<Id, InvalidId> {
        uint32_t _pid;
        unsigned char _uuid[16];

    public:
        Id();
        Id(const Id&);
        Id(const InvalidId&);
        ~Id() = default;

        Id& operator=(const Id&);
        Id& operator=(const InvalidId&);

        // Barton–Nackman comparable implementation.
        // `Subclass` must provide a compare member function that compares
        // to instances of `T` and returns an integer x with:
        // - `x < 0</tt> if <tt>*this < other
        // - `x > 0</tt> if <tt>*this > other
        // - `x == 0</tt> if <tt>*this == other
        int compare(const Id& other) const;
        int compare(const InvalidId&) const;
    };

    class Fiber {
        Id _id;

    public:
        Fiber(std::shared_ptr<MachineDefinition> m, VMContext& vm);
        ~Fiber();

        const Id& id() const { return _id; }

        void gotoState(FnContext& run, const char* name) {
            auto mainState = machineDefinition->states.find(name);
            if (mainState == machineDefinition->states.end())
                VM_RAISE("main not found on machine");

            run.run(mainState->second->instructions);
        }

        template <typename T>
        T top() {
            if (stack.empty() || stack.back().empty())
                VM_RAISE("stack underflow");
            std::shared_ptr<Wires::TypedData>& data = stack.back().back();
            if (data->type() != typeid(T))
                return 0;
            Wires::Data<T>* val = reinterpret_cast<Wires::Data<T>*>(data.get());
            return val->value();
        }

        std::shared_ptr<Wires::TypedData> topVar() const {
            if (stack.empty() || stack.back().empty())
                VM_RAISE("stack underflow");
            return stack.back().back();
        }

        template <typename T>
        T pop() {
            if (stack.empty() || stack.back().empty())
                VM_RAISE("stack underflow");
            std::shared_ptr<Wires::TypedData>& data = stack.back().back();
            if (!data)
                VM_RAISE("nullptr on stack (variable not found?)");
            if (data->type() == typeid(T)) {
                Wires::Data<T>* val = reinterpret_cast<Wires::Data<T>*>(data.get());
                T result = val->value();
                stack.back().pop_back();
                return result;
            }
            Wires::Data<T>* val = dynamic_cast<Wires::Data<T>*>(data.get());
            if (!val)
                VM_RAISE("Wrong type on stack to pop");
            T result= val->value();
            stack.back().pop_back();
            return result;
        }

        std::shared_ptr<Wires::TypedData> popVar() {
            if (stack.empty() || stack.back().empty())
                VM_RAISE("stack underflow");
            std::shared_ptr<Wires::TypedData> result = stack.back().back();
            stack.back().pop_back();
            return result;
        }

        template <typename T>
        T back(int i) {
            if (stack.empty())
                VM_RAISE("stack underflow");
            int sz = (int) stack.back().size();
            if (sz + i < 0)
                VM_RAISE("stack underflow");
            std::shared_ptr<Wires::TypedData>& data = stack.back()[sz + i];
            if (data->type() != typeid(T))
                return T(0);
            Wires::Data<T>* val = reinterpret_cast<Wires::Data<T>*>(data.get());
            return val->value();
        }

        template <typename T>
        void push(const T& val) {
            stack.back().push_back(std::make_shared<Wires::Data<T>>(val));
        }

        void pushVar(std::shared_ptr<Wires::TypedData> v) {
            stack.back().push_back(v);
        }

        std::shared_ptr<MachineDefinition> machineDefinition;
        std::map<std::string, Property*> properties;
        std::vector<std::shared_ptr<Wires::TypedData>> locals;

        typedef std::vector<std::shared_ptr<Wires::TypedData>> Stack;
        std::vector<Stack> stack;
    };

}
