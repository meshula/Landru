
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

// name, operand count, description
INSTR_DECL(Nop,							0, ". -> .")
INSTR_DECL(PopStore,					0, "var varIndex -> .")
INSTR_DECL(LaunchMachine,				0, "instanceHandle -> .")
INSTR_DECL(PushConstant,				1, ". -> constant")
INSTR_DECL(PushFloatConstant,			1, ". -> floatConstant")
INSTR_DECL(PushIntZero,					0, ". -> 0")
INSTR_DECL(PushIntOne,					0, ". -> 1")
INSTR_DECL(PushVar,						0, "varIndex -> var")
INSTR_DECL(CreateTempString,            0, "stringIndx -> StringVarObj")
INSTR_DECL(GetGlobalVar,				0, "stringIndx -> VarObj")
INSTR_DECL(GetSharedVar,				0, "stringIndx -> VarObj")
INSTR_DECL(GetVarFromVar,               0, "stringIndx -> VarObj")
INSTR_DECL(GetSelfVar,                  0, "varIdx -> VarObj")
INSTR_DECL(GetLocalString,              0, ". -> StringVarObj Top 16b is param index")
INSTR_DECL(GetLocalVarObj,              0, ". -> VarObj Top 16b is param index")
INSTR_DECL(GetLocalExeVarObj,           0, ". -> . push to VarObj to exe stack Top 16b is param index")
INSTR_DECL(StateEnd,					0, ". -> .")
INSTR_DECL(SubStateEnd,					0, ". -> .")
INSTR_DECL(StateSuspend,				0, ". -> .")
INSTR_DECL(ParamsStart,                 0, ". -> paramMark")
INSTR_DECL(ParamsEnd,                   0, "paramMark ... -> VarArrayObj")
INSTR_DECL(GotoState,					0, "stateIndex -> .")
INSTR_DECL(GotoAddr,					1, ". -> .")
INSTR_DECL(OnMessage,					0, "stringIndx -> .")
INSTR_DECL(OnTick,						0, ". -> .")
INSTR_DECL(CallFunction,				0, "stringIndex(ignored) -> .")
INSTR_DECL(DotChain,                    0, "varObj -> . / VarObj moved to execution stack")
INSTR_DECL(GetRequire,                  0, ". -> VarObj")
INSTR_DECL(IfEq,						0, "var var -> .")
INSTR_DECL(IfLte0,						0, "float32 -> .")
INSTR_DECL(IfGte0,						0, "float32 -> .")
INSTR_DECL(IfLt0,						0, "float32 -> .")
INSTR_DECL(IfGt0,						0, "float32 -> .")
INSTR_DECL(IfEq0,						0, "float32 -> .")
INSTR_DECL(IfNotEq0,					0, "float32 -> .")
INSTR_DECL(OpAdd,					    0, "float32 float32 -> float32")
INSTR_DECL(OpSubtract,				    0, "float32 float32 -> float32")
INSTR_DECL(OpMultiply,				    0, "float32 float32 -> float32")
INSTR_DECL(OpDivide,				    0, "float32 float32 -> float32")
INSTR_DECL(OpNegate,				    0, "float32 float32 -> float32")
INSTR_DECL(OpModulus,				    0, "float32 float32 -> float32")
INSTR_DECL(ForEach,                     0, "array of varobjs -> .")
INSTR_DECL(RangedRandom,				0, "maxfloat32, minFloat32 -> rand(min, max)")
INSTR_DECL(Unknown,						0, "? -> ?")
