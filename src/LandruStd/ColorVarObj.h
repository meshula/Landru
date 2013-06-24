
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRURUNTIME_COLORVAROBJ_H
#define LANDRURUNTIME_COLORVAROBJ_H

#include "LandruVM/VarObj.h"


namespace Landru {

	class ColorVarObj : public Landru::VarObj
	{
	public:
		ColorVarObj(const char* name);
		virtual ~ColorVarObj();
		
		virtual void Update(float /*elapsedTime*/) { }
        VAROBJ_FACTORY(color, ColorVarObj)
		
		enum Mode {
			RGB, HSB
		};

		float r, g, b, a;
		
	private:
		unsigned int c;
		Mode mode;
		
        LANDRU_DECL_BINDING_BEGIN
		LANDRU_DECL_BINDING(mode)
		LANDRU_DECL_BINDING(getInt)
		LANDRU_DECL_BINDING(getV1f)
		LANDRU_DECL_BINDING(getV2f)
		LANDRU_DECL_BINDING(getV3f)
		LANDRU_DECL_BINDING(getV4f)
		LANDRU_DECL_BINDING(setInt)
		LANDRU_DECL_BINDING(setV1f)
		LANDRU_DECL_BINDING(setV2f)
		LANDRU_DECL_BINDING(setV3f)
		LANDRU_DECL_BINDING(setV4f)
		LANDRU_DECL_BINDING_END
	};
	
	
}

#endif
