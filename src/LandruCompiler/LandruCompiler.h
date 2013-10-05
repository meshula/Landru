
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include <string>
#include <vector>

namespace Json { class Value; }

EXTERNC void* landruCreateRootNode();
EXTERNC void  landruParseProgram(void* rootNode, 
                                 std::vector<std::pair<std::string, Json::Value*> >* jsonVars,
                                 char const* buff, size_t len);
EXTERNC void  landruPrintAST(void* rootNode);
EXTERNC void  landruPrintRawAST(void* rootNode);
EXTERNC void  landruToJson(void* rootNode);

