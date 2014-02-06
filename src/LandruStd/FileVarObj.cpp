//
//  FileVarObj.cpp
//  Landru
//
//  Created by Nick Porcino on 2014 01/1.
//
//

#include "FileVarObj.h"
#include "LandruVM/VarObjStackUtil.h"
#include "LandruStd/StringVarObj.h"

namespace Landru {

    class FileVarObj::Detail
    {
    public:
        Detail() : buffer(0), bufferSize(0) { }
        ~Detail() { free(buffer); }

        uint8_t* buffer;
        size_t bufferSize;
    };

    FileVarObj::FileVarObj(const char* name)
    : VarObj(name, &functions)
    , detail(new Detail)
	{
	}

    FileVarObj::~FileVarObj()
    {
        delete detail;
    }

    size_t FileVarObj::size() const {
        return detail->bufferSize;
    }

    uint8_t* FileVarObj::data() const {
        return detail->buffer;
    }

    LANDRU_DECL_FN(FileVarObj, read)
    {
		FileVarObj* o = (FileVarObj*) p->vo;
        if (o->detail->buffer) {
            o->detail->bufferSize = 0;
            free(o->detail->buffer);
            o->detail->buffer = 0;
        }
        VarObjArray* voa;
        Pop<VarObjArray> t1(p, voa);
        StringVarObj* svo = dynamic_cast<StringVarObj*>(voa->get(-1)->vo);
        char const*const filename = svo->getCstr();
        FILE* f = fopen(filename, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            o->detail->bufferSize = ftell(f);
            rewind(f);
            if (o->detail->bufferSize) {
                o->detail->buffer = (uint8_t*) malloc(o->detail->bufferSize + 1);
                fread(o->detail->buffer, 1, o->detail->bufferSize, f);
                o->detail->buffer[o->detail->bufferSize] = '\0';
            }
            fclose(f);
        }
    }

    LANDRU_DECL_FN(FileVarObj, size)
    {
		FileVarObj* o = (FileVarObj*) p->vo;
        pushInt(p, o->detail->bufferSize);
    }

    LANDRU_DECL_TABLE_BEGIN(FileVarObj)
    LANDRU_DECL_ENTRY(FileVarObj, read)
    LANDRU_DECL_ENTRY(FileVarObj, size)
    LANDRU_DECL_TABLE_END(FileVarObj)
    
}
