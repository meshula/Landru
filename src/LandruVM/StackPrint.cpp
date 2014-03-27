
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "StackPrint.h"
#include "LandruStd/StringVarObj.h"

#include <string.h>
#include <stdio.h>

using namespace Landru;

void stackPrint(LStack* s)
{
    printf("+------------------------------------+\n");
    printf("| Depth: %03d                       |\n", s->voa->size());
    printf("+------------------------------------+\n");
    for (int i = s->voa->size() - 1; i >=0; --i) {
        std::shared_ptr<VarObj> vop = LStackVarObjAt(s, i).lock();
        if (!vop)
            printf(">--- Param mark ----->\n");
        else if (vop->TypeId() == Landru::StringVarObj::StaticTypeId()) {
            Landru::StringVarObj* svo = (Landru::StringVarObj*) vop.get();
            printf("\"%s\"\n", svo->getCstr());
        }
        else
            printf("%s: %s\n", vop->TypeName(), vop->name());
    }
    printf("+-------------------------------------\n");
}
