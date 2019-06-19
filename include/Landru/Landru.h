
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_H
#define LANDRU_H

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include "export.h"

struct LandruAST;
struct LandruEC;
struct LandruMachineExemplar;

EXTERNC LandruAST* landru_lex_parse(char const*const in, size_t sz);
EXTERNC void landru_ast_print(LandruAST*);
EXTERNC void landru_destroy_ast(LandruAST*);

EXTERNC LandruEC* landru_create_execution_context();
EXTERNC void landru_runtime_update(LandruEC*);
EXTERNC void landru_destroy_execution_context(LandruEC*);
EXTERNC void landru_machine_instantiate(LandruEC* context, LandruMachineExemplar* machine);

LandruMachineExemplar* landru_compile(LandruAST*);
void landru_machine_print(LandruMachineExemplar*);

#endif
