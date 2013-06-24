
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_RUNTIME_INSTRUCTIONS_H
#define LANDRU_RUNTIME_INSTRUCTIONS_H

#include <string>
#include <vector>

namespace Landru
{

	namespace Instructions
	{
		
		#define INSTR_DECL(a, b, c) i##a,		
		enum Op 
		{
			#include "InstructionsDef.h"
		};
		#undef INSTR_DECL

		struct Info
		{
			int				instruction;
			const char*		name;
			int				argCount;
			const char*		stackDoc;
		};

		const Info& getInfo(Op instruction);

        char const*const disassemble(unsigned int const*const program, int pc, 
									 const std::vector<std::string>& stringTable, int& skip);
		
        char const*const disassemble(unsigned int const*const program, int pc, 
									 const char* stringData, int* stringArray, int& skip);
        
        char const*const opName(int op);

	} // Instructions


}

#endif
