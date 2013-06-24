
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruVM/Continuation.h"

namespace Landru {	
    class Engine;
    
	class TimeVarObj : public VarObj
	{		
	public:
		TimeVarObj(const char* name);
		virtual ~TimeVarObj();
        
        VAROBJ_FACTORY(time, TimeVarObj);        
        
        LANDRU_DECL_BINDING_BEGIN
            LANDRU_DECL_BINDING(after)
            LANDRU_DECL_BINDING(every)
            LANDRU_DECL_BINDING(recur)
        LANDRU_DECL_BINDING_END
	};
    
    class TimeEngine : public ContinuationList
    {
        struct TimeFunctor;
        
    public:
        TimeEngine(VarPool*);
        
        // this update is called from updateQueues in Continuation.cpp
		virtual void update(Engine* engine, float elapsedTime);
    };
	
}

