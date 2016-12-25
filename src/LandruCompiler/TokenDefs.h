
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

TOKEN_DECL(Unknown,			"unknown"	)
TOKEN_DECL(Declare,			"declare"	)
TOKEN_DECL(Shared,			"shared"	)
TOKEN_DECL(Dot,				"."			)
TOKEN_DECL(Else,			"else"		)
TOKEN_DECL(For,				"for"		)
TOKEN_DECL(In,				"in"		)
TOKEN_DECL(Goto,			"goto"		)
TOKEN_DECL(If,				"if"		)
TOKEN_DECL(Machine,			"machine"	)
TOKEN_DECL(Message,			"message"	)
TOKEN_DECL(New,				"new"		)
TOKEN_DECL(On,				"on"		)
TOKEN_DECL(Require,			"require"	)
TOKEN_DECL(State,			"state"		)
TOKEN_DECL(Tick,			"tick"		)
TOKEN_DECL(Symbols,			"symbols"	)
TOKEN_DECL(Lib,				"lib"		)
TOKEN_DECL(Launch,			"launch"	)
TOKEN_DECL(Break,			"break"		)
TOKEN_DECL(True,			"true"		)
TOKEN_DECL(False,			"false"		)
TOKEN_DECL(Eq,				"="			)
TOKEN_DECL(Lte0,			"<=0"		)
TOKEN_DECL(Gte0,			">=0"		)
TOKEN_DECL(Lt0,				"<0"		)
TOKEN_DECL(Gt0,				">0"		)
TOKEN_DECL(Eq0,				"=0"		)
TOKEN_DECL(NotEq0,			"!=0"		)
TOKEN_DECL(OpenBrace,       "{"         )
TOKEN_DECL(OpenBracket,     "["         )

// AST tokens
TOKEN_DECL(Program,			"program"		  )
TOKEN_DECL(Assignment,		"assignment"	  )
TOKEN_DECL(InitialAssignment, "initialAssignment" )
TOKEN_DECL(Function,        "function"		  )
TOKEN_DECL(DotChain,        "dotChain"        ) // example dot chain: foo().bar()
TOKEN_DECL(GlobalVariable,  "globalVariable"  )
TOKEN_DECL(LocalVariable,   "localVariable"   )
TOKEN_DECL(Parameters,		"parameters"    )
TOKEN_DECL(GetVariable,     "getVariable"   )
TOKEN_DECL(GetVariableReference, "getVariableReference")
TOKEN_DECL(Statements,		"statements"    )

// Operators
TOKEN_DECL(OpMultiply,		"opMultiply"    )
TOKEN_DECL(OpAdd,	     	"opAdd"         )
TOKEN_DECL(OpDivide,        "opDivide"      )
TOKEN_DECL(OpSubtract,		"opSubtract"    )
TOKEN_DECL(OpNegate,		"opNegate"      )
TOKEN_DECL(OpModulus,		"opModulus"     )
TOKEN_DECL(OpGreaterThan,   "opGreaterThan" )
TOKEN_DECL(OpLessThan,      "opLessThan"    )

// Literals
TOKEN_DECL(FloatLiteral,    "float"	)
TOKEN_DECL(IntLiteral,      "int"    )
TOKEN_DECL(RangedLiteral,	"rangedLiteral"	)
TOKEN_DECL(SharedVariable,	"sharedVariable")
TOKEN_DECL(StringLiteral,	"string"	)
TOKEN_DECL(NullLiteral,     "null"   )

// Data tokens
TOKEN_DECL(StaticData,        "staticData"        )
TOKEN_DECL(DataObject,        "dataObject"        )
TOKEN_DECL(DataElement,       "dataElement"       )
TOKEN_DECL(DataArray,         "dataArray"         )
TOKEN_DECL(DataFloatLiteral,  "dataFloatLiteral"  )
TOKEN_DECL(DataIntLiteral,    "dataIntLiteral"    )
TOKEN_DECL(DataNullLiteral,   "dataNullLiteral"   )
TOKEN_DECL(DataStringLiteral, "dataStringLiteral" )
