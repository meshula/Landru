
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/VarObj.h"
#include "LandruStd/IntVarObj.h"
#include "LandruStd/RealVarObj.h"
#include <vector>

namespace Landru {
    
    class VarObjArray : public VarObj
    {
    public:
        VAROBJ_FACTORY(array, VarObjArray)
        
		VarObjArray(const char* name) : VarObj(name, &Landru::VarObjArray::functions) { }
		virtual ~VarObjArray() { clear(); }

        void add(VarObjPtr* vop)
        {
            if (vop && vop->vo)
                vop->vo->varPool()->addStrongRef(vop);
            
            _array.push_back(vop);
        }

        // return the top of the stack
        VarObjPtr* top() const
        {
            return _array.back();
        }
        
        void pop()
        {
            VarObjPtr* vop = top();
            if (vop && vop->vo)
                vop->vo->varPool()->releaseStrongRef(vop);
            
            _array.pop_back();
        }
        
        void pop_front()
        {
            if (!_array.empty()) {
                auto i = _array.begin();
                if ((*i) && (*i)->vo)
                    (*i)->vo->varPool()->releaseStrongRef(*i);
                _array.erase(i);
            }
        }
        
        void push_back(VarObjPtr* vop)
        {
            add(vop);
        }
        
        bool empty() const
        {
            return _array.empty();
        }
        
        void clear()
        {
            for (std::vector<VarObjPtr*>::iterator i = _array.begin(); i != _array.end(); ++i)
                if (*i)
                    (*i)->vo->varPool()->releaseStrongRef(*i);
            
            _array.clear();
        }
        
        int size() const
        {
            return int(_array.size());
        }
        
        VarObjPtr* get(int i)
        {
            if (i < 0)
                i = size() + i;
            if (i < size())
                return _array[i];
            
            return 0;
        }
        
        int getInt(int i)
        {
            if (i < 0)
                i = size() + i;
            // if i < 0 || i > size() raise error
            // if _array[i] ! int, raise error
            IntVarObj* ivo = dynamic_cast<IntVarObj*> (_array[i]->vo);
            return ivo->value();
        }
        
        float getReal(int i)
        {
            if (i < 0)
                i = size() + i;
            // if i < 0 || i > size() raise error
            // if _array[i] ! int, raise error
            RealVarObj* ivo = dynamic_cast<RealVarObj*> (_array[i]->vo);
            return ivo->value();
        }

        LANDRU_DECL_BINDING_BEGIN
        LANDRU_DECL_BINDING(create)
        LANDRU_DECL_BINDING_END
    
    private:
        std::vector<VarObjPtr*> _array;
    };
    
    
}