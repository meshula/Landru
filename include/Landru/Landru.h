
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include "defines.h"
#include "export.h"

#include <string>
#include <vector>

//namespace Json { class Value; }

struct LandruNode_t {};
struct LandruLibrary_t {};
struct LandruVMContext_t {};
struct LandruAssembler_t {};

EXTERNC LandruNode_t* landruCreateRootNode();
EXTERNC int   landruParseProgram(LandruNode_t* rootNode, 
//                                 std::vector<std::pair<std::string, Json::Value*> >* jsonVars,
                                 char const* buff, size_t len);
EXTERNC void  landruPrintAST(LandruNode_t* rootNode);
EXTERNC void  landruPrintRawAST(LandruNode_t* rootNode);
EXTERNC void  landruToJson(LandruNode_t* rootNode);

EXTERNC LandruLibrary_t* landruCreateLibrary(char const*const name);
EXTERNC LandruVMContext_t* landruCreateVMContext(LandruLibrary_t* lib);
EXTERNC void landruInitializeStdLib(LandruLibrary_t* library, LandruVMContext_t* vmContext);

EXTERNC LandruAssembler_t* landruCreateAssembler(LandruLibrary_t*);
EXTERNC int landruLoadRequiredLibraries(LandruAssembler_t*, LandruNode_t* root_node, LandruLibrary_t* library, LandruVMContext_t* vmContext);
EXTERNC void landruAssemble(LandruAssembler_t*, LandruNode_t* rootNode);
EXTERNC void landruVMContextSetTraceEnabled(LandruVMContext_t*, bool);
EXTERNC void landruInitializeContext(LandruAssembler_t*, LandruVMContext_t*);
EXTERNC void landruLaunchMachine(LandruVMContext_t*, char const*const name);

EXTERNC bool landruUpdate(LandruVMContext_t*, double now);

EXTERNC void landruReleaseAssembler(LandruAssembler_t*);
EXTERNC void landruReleaseRootNode(LandruNode_t*);
EXTERNC void landruReleaseLibrary(LandruLibrary_t*);
EXTERNC void landruReleaseVMContext(LandruVMContext_t*);
