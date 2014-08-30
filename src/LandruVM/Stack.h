
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRURUNTIME_STACK_H
#define LANDRURUNTIME_STACK_H

#include <stddef.h>
#include "LandruVM/VarObj.h"
#include "LandruVM/VarObjArray.h"

/// next rename LStack to LandruVM::Stack


class LStack
{
public:
    LStack() {
        voa = std::make_shared<Landru::VarObjArray>("stack");
        _curr = voa;
    }

    std::shared_ptr<Landru::VarObjArray> curr() const { return _curr; }
    std::shared_ptr<Landru::VarObjArray> voa;

    void nestStack() {
        _curr->push_back(std::make_shared<Landru::VarObjArray>("stack"));
        _stackstack.push_back(_curr);
        _curr = std::dynamic_pointer_cast<Landru::VarObjArray>(_curr->top().lock());
    }

    void unnestStack() {
        _curr = _stackstack.back();
        _stackstack.pop_back();
    }

    template<typename T>
    std::shared_ptr<T> top() {
        return std::dynamic_pointer_cast<T>(_curr->top().lock());
    }

    void push(std::shared_ptr<Landru::VarObj> v) { _curr->push_back(v); }
    void pop() { _curr->pop(); }

    void clear() { _curr->clear(); }

private:
    std::shared_ptr<Landru::VarObjArray> _curr;
    std::vector<std::shared_ptr<Landru::VarObjArray>> _stackstack;
};

inline int LStackDepth(LStack* ls) {
    return ls->curr()->size();
}
inline void LStackPushVarObj(LStack* ls, std::shared_ptr<Landru::VarObj> vo) {
    ls->curr()->push_back(vo);
}
inline void LStackPushParamMark(LStack* ls) {
    ls->nestStack();
}
inline void LStackFinalizeParamMark(LStack* ls) {
    ls->unnestStack();
}
inline int LStackTopStringIndex(LStack* ls) {
    Landru::VarObj* vo = ls->curr()->top().lock().get();
    Landru::IntVarObj* ivo = dynamic_cast<Landru::IntVarObj*>(vo);
    return ivo? ivo->value() : -1;
}
inline std::weak_ptr<Landru::VarObj> LStackTopFiber(LStack* ls) {
    return ls->curr()->top();
}
inline std::weak_ptr<Landru::VarObj> LStackTopVarObj(LStack* ls) {
    return ls->curr()->top();
}
inline std::weak_ptr<Landru::VarObj> LStackVarObjAt(LStack* ls, int index) {
    return ls->curr()->get(index);
}

#endif
