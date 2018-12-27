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
#include "Landru/VMContext.h"
#include "LandruActorVM/WiresTypedData.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_COMPARABLE_HPP
#define CAF_COMPARABLE_HPP

namespace caf {
    namespace detail {
        
        /**
         * Barton–Nackman trick implementation.
         * `Subclass` must provide a compare member function that compares
         * to instances of `T` and returns an integer x with:
         * - `x < 0</tt> if <tt>*this < other
         * - `x > 0</tt> if <tt>*this > other
         * - `x == 0</tt> if <tt>*this == other
         */
        template <class Subclass, class T = Subclass>
        class comparable {
            
            friend bool operator==(const Subclass& lhs, const T& rhs) {
                return lhs.compare(rhs) == 0;
            }
            
            friend bool operator==(const T& lhs, const Subclass& rhs) {
                return rhs.compare(lhs) == 0;
            }
            
            friend bool operator!=(const Subclass& lhs, const T& rhs) {
                return lhs.compare(rhs) != 0;
            }
            
            friend bool operator!=(const T& lhs, const Subclass& rhs) {
                return rhs.compare(lhs) != 0;
            }
            
            friend bool operator<(const Subclass& lhs, const T& rhs) {
                return lhs.compare(rhs) < 0;
            }
            
            friend bool operator>(const Subclass& lhs, const T& rhs) {
                return lhs.compare(rhs) > 0;
            }
            
            friend bool operator<(const T& lhs, const Subclass& rhs) {
                return rhs > lhs;
            }
            
            friend bool operator>(const T& lhs, const Subclass& rhs) {
                return rhs < lhs;
            }
            
            friend bool operator<=(const Subclass& lhs, const T& rhs) {
                return lhs.compare(rhs) <= 0;
            }
            
            friend bool operator>=(const Subclass& lhs, const T& rhs) {
                return lhs.compare(rhs) >= 0;
            }
            
            friend bool operator<=(const T& lhs, const Subclass& rhs) {
                return rhs >= lhs;
            }
            
            friend bool operator>=(const T& lhs, const Subclass& rhs) {
                return rhs <= lhs;
            }
            
        };
        
        template <class Subclass>
        class comparable<Subclass, Subclass> {
            
            friend bool operator==(const Subclass& lhs, const Subclass& rhs) {
                return lhs.compare(rhs) == 0;
            }
            
            friend bool operator!=(const Subclass& lhs, const Subclass& rhs) {
                return lhs.compare(rhs) != 0;
            }
            
            friend bool operator<(const Subclass& lhs, const Subclass& rhs) {
                return lhs.compare(rhs) < 0;
            }
            
            friend bool operator<=(const Subclass& lhs, const Subclass& rhs) {
                return lhs.compare(rhs) <= 0;
            }
            
            friend bool operator>(const Subclass& lhs, const Subclass& rhs) {
                return lhs.compare(rhs) > 0;
            }
            
            friend bool operator>=(const Subclass& lhs, const Subclass& rhs) {
                return lhs.compare(rhs) >= 0;
            }
            
        };
        
    } // namespace details
} // namespace caf

#endif // CAF_COMPARABLE_HPP



namespace Landru {

    struct InvalidId
    {
        constexpr InvalidId() {}
    };

    // Id class inspired by libcaf's node_id
    class Id : caf::detail::comparable<Id>, caf::detail::comparable<Id, InvalidId>
    {
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

    class Fiber
    {
		Id _id;
		int scopeLevel = 1;	// In the future, 0 will mean continuations at machine scope, specified outside of a state
							// higher levels will indicate within hierarchies of states

        const char * _currentState = nullptr; // this will always point to a string in the machine definition

    public:
        Fiber(std::shared_ptr<MachineDefinition> m, VMContext *);
        ~Fiber();

        const Id& id() const { return _id; }

		const char * id_str() const { return "@TODO"; }

        void gotoState(FnContext& run, const char* name, bool raiseIfStateNotFound)
		{
            auto state = machineDefinition->states.find(name);
			if (state == machineDefinition->states.end()) {
				if (raiseIfStateNotFound) {
					VM_RAISE("main not found on machine");
                }
				return;
			}

            _currentState = state->first.c_str();

			run.clearContinuations(this, scopeLevel);
            run.run(state->second->instructions);
        }

        const char * currentState() const
        {
            return _currentState;
        }

        template <typename T>
        T top()
		{
            if (stack.empty() || stack.back().empty()) {
                VM_RAISE("stack underflow");
            }
            std::shared_ptr<Wires::TypedData>& data = stack.back().back();
            if (data->type() != typeid(T))
                return 0;
            Wires::Data<T>* val = reinterpret_cast<Wires::Data<T>*>(data.get());
            return val->value();
        }

        std::shared_ptr<Wires::TypedData> topVar() const
		{
            if (stack.empty() || stack.back().empty()) {
                VM_RAISE("stack underflow");
            }
            return stack.back().back();
        }

        template <typename T>
        T pop()
		{
            if (stack.empty() || stack.back().empty()) {
                VM_RAISE("stack underflow");
            }
            std::shared_ptr<Wires::TypedData>& data = stack.back().back();
            if (!data) {
                VM_RAISE("nullptr on stack (variable not found?)");
            }
            if (data->type() == typeid(T)) {
                Wires::Data<T>* val = reinterpret_cast<Wires::Data<T>*>(data.get());
                T result = val->value();
                stack.back().pop_back();
                return result;
            }
            Wires::Data<T>* val = dynamic_cast<Wires::Data<T>*>(data.get());
            if (!val) {
                VM_RAISE("Wrong type on stack to pop");
            }
            T result= val->value();
            stack.back().pop_back();
            return result;
        }

        std::shared_ptr<Wires::TypedData> popVar() {
            if (stack.empty() || stack.back().empty()) {
                VM_RAISE("stack underflow");
            }
            std::shared_ptr<Wires::TypedData> result = stack.back().back();
            stack.back().pop_back();
            return result;
        }

        template <typename T>
        T back(int i) {
            if (stack.empty()) {
                VM_RAISE("stack underflow");
            }
            int sz = (int) stack.back().size();
            if (sz + i < 0) {
                VM_RAISE("stack underflow");
            }
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

		std::shared_ptr<Wires::TypedData> push_local(const std::string & name, const std::string & type,
												     std::shared_ptr<Wires::TypedData> v)
		{
			locals.push_back(std::move(std::make_shared<Property>(name, type, v)));
			return (*locals.rbegin())->data;
		}

		void pop_local() {
			locals.pop_back();
		}

        std::shared_ptr<MachineDefinition> machineDefinition;

        //std::map<std::string, std::shared_ptr<Property>> properties;

		// vector, because local scopes push back their local variables, and pop them on exit
		std::vector<std::shared_ptr<Property>> locals;

        typedef std::vector<std::shared_ptr<Wires::TypedData>> Stack;
        std::vector<Stack> stack;
    };

}
