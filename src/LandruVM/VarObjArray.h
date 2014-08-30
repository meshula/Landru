
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

        void add(std::shared_ptr<VarObj> vop) {
            _array.push_back(vop);
        }

        // return the top of the stack
        std::weak_ptr<VarObj> top() const {
            return _array.back();
        }
        
        void pop() {
            _array.pop_back();
        }
        
        void pop_front() {
            if (!_array.empty()) {
                auto i = _array.begin();
                _array.erase(i);
            }
        }
        
        void push_back(std::shared_ptr<VarObj> vop) {
            add(vop);
        }
        
        bool empty() const
        {
            return _array.empty();
        }
        
        void clear()
        {
            _array.clear();
        }
        
        int size() const
        {
            return int(_array.size());
        }
        
        std::weak_ptr<VarObj> get(int i)
        {
            if (i < 0)
                i = size() + i;
            if (i < size())
                return _array[i];
            
            return std::weak_ptr<VarObj>();
        }

        void set(std::shared_ptr<VarObj> v, int i) {
            if (i < 0)
                i = size() + i;
            if (i < size())
                _array[i] = v;
        }
        
        int getInt(int i)
        {
            if (i < 0)
                i = size() + i;
            // if i < 0 || i > size() raise error
            // if _array[i] ! int, raise error
            IntVarObj* ivo = dynamic_cast<IntVarObj*>(_array[i].get());
            return ivo ? ivo->value() : 0;
        }
        
        float getReal(int i)
        {
            if (i < 0)
                i = size() + i;
            // if i < 0 || i > size() raise error
            // if _array[i] ! int, raise error
            RealVarObj* ivo = dynamic_cast<RealVarObj*> (_array[i].get());
            return ivo ? ivo->value() : 0;
        }

        LANDRU_DECL_BINDING_BEGIN
        LANDRU_DECL_BINDING(create)
        LANDRU_DECL_BINDING(pushBack)
        LANDRU_DECL_BINDING_END
    
    private:
        std::vector<std::shared_ptr<VarObj>> _array;
    };
    
    
}