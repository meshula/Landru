//
//  FileVarObj.h
//  Landru
//
//  Created by Nick Porcino on 2014 01/1.
//
//

#pragma once
#include "LandruVM/VarObj.h"

namespace Landru {
    class FileVarObj : public Landru::VarObj
    {
    public:
        FileVarObj(const char* name);
        virtual ~FileVarObj();

        size_t size() const;
        uint8_t* data() const;

        VAROBJ_FACTORY(file, FileVarObj)

        LANDRU_DECL_BINDING_BEGIN
        LANDRU_DECL_BINDING(read)
        LANDRU_DECL_BINDING(size)
        LANDRU_DECL_BINDING_END
        
        class Detail;
        Detail* detail;
    };
} // Landru
