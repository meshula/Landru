//
//  IoLib.cpp
//  Landru
//
//  Created by Nick Porcino on 10/29/14.
//
//


#include "IoLib.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
using namespace std;

namespace Landru {
    namespace Std {


        //-----------
        // Io Libary \__________________________________________

        void IoLib::registerLib(Library& l) {
            auto u = unique_ptr<Library::Vtable>(new Library::Vtable("io"));
            u->registerFn("2.0", "print", "...", "", print);
            l.registerVtable(move(u));
        }
        void IoLib::print(FnContext& run) {
            auto& params = run.self->stack.back();
            for (auto p : params) {
                if (p->type() == typeid(string)) {
                    string s = dynamic_cast<Wires::Data<string>*>(p.get())->value();
					size_t i;
                    while ((i = s.find("\\n")) != string::npos)
                        s.replace(i, 2, 1, '\n');
                    cout << s;
                }
                else if (p->type() == typeid(float)) {
                    float f = dynamic_cast<Wires::Data<float>*>(p.get())->value();
                    cout << f;
                }
                else if (p->type() == typeid(int)) {
                    int i = (int) dynamic_cast<Wires::Data<int>*>(p.get())->value();
                    cout << i;
                }
            }
            int pop = params.size();
            for (int i = 0; i < pop; ++i)
                run.self->popVar();
        }


    } // Std
} // Landru
