
// Copyright (c) 2014 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/VarObj.h"
#include "LandruVM/VarObjArray.h"

namespace Landru {

    class GeneratorVarObj : public VarObj
    {
    public:
        class Generator {
        public:
            virtual ~Generator() { }

            virtual void begin(VarObjArray* params) = 0;
            virtual bool done() = 0;
            virtual void next() = 0;

            virtual void generate(VarObjArray& locals) = 0;
            virtual void generateDone(VarObjArray& locals) = 0;
        };

        VAROBJ_FACTORY(generator, GeneratorVarObj)

		GeneratorVarObj(const char* name) : VarObj(name, &Landru::GeneratorVarObj::functions), generator(0) { }
		virtual ~GeneratorVarObj() { }

        void begin(VarObjArray* params) { if (generator) generator->begin(params); }
        bool done() { if (generator) return generator->done(); else return true; }
        void next() { if (generator) generator->next(); }

        void generate(VarObjArray& locals) { if (generator) generator->generate(locals); }
        void generateDone(VarObjArray& locals) { if (generator) generator->generateDone(locals); }

        Generator* generator;

        LANDRU_DECL_BINDING_BEGIN
        LANDRU_DECL_BINDING_END
    };
}
