
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_REFCOUNTED_H
#define LANDRU_REFCOUNTED_H

#include "LandruVM/RaiseError.h"

namespace Landru
{
    class RefCounted
    {
    public:
        RefCounted() : refCount(0) { }
        virtual ~RefCounted() { if (refCount > 0) RaiseError(0, "bad ref count", 0); }

        int RefCount() const { return refCount; }
        
        int IncRefCount() 
        { 
            ++refCount; 
            return refCount;
        }

        int DecRefCount() 
        { 
            --refCount; 
            if (refCount < 0)
            {
                RaiseError(0, "bad ref count", 0);
                refCount = 0;
            }
            return refCount; 
        }

    private:
        int refCount;
    };

}

#endif
