
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Instructions.h"
#include <stdio.h>

namespace Landru
{


	namespace Instructions
	{
		#define INSTR_DECL(a, b, c) i##a, #a, b, c,
		Info instructionInfo[] =
		{
			#include "LandruVM/InstructionsDef.h"
		};
		#undef INSTR_DECL
		
        char const*const opName(int op)
        {
		    const Instructions::Info& info = Instructions::getInfo((Instructions::Op) op);
            return info.name;   
        }
        
		const Info& getInfo(Op instruction_)
		{
            Op instruction = (Op) (0xffff & (int)instruction_);
			for (int i = 0; i <= iUnknown; ++i)
			{
                Op test = (Op) instructionInfo[i].instruction;
				if (test == instruction)
					return instructionInfo[i];
			}

			static Info unknown = { iUnknown, "Unknown", 0, "? > ?" };
			return unknown;
		}

        char const*const disassemble(unsigned int const*const program, int pc, 
                                     const std::vector<std::string>& stringTable, int& skip)
        {
            static char buff[1024];
            int op = program[pc];
		    const Instructions::Info& info = Instructions::getInfo((Instructions::Op) op);

		    switch(op & 0xffff) {
                case Instructions::iGotoAddr:
                    sprintf(buff, "%04d:%s %d", pc, info.name, program[pc+1]);
                    break;

                case Instructions::iPushVar:
                case Instructions::iPushConstant:
                    sprintf(buff, "%04d:%s %d \t// Stack:. > %d", pc, info.name, program[pc+1], program[pc+1]);
                    break;

                case Instructions::iPushFloatConstant: {
                    union { int i; float f; } u;
                    u.i = program[pc+1];
                    sprintf(buff, "%04d:%s %f \t// Stack:. > %f", pc, info.name, u.f, u.f); }
                    break;

                case Instructions::iGetRequire:
                    sprintf(buff, "%04d:GetRequire %d \t// Stack:. > x", pc, op>>16);
                    break;

                case Instructions::iCallFunction:
                    sprintf(buff, "%04d:Call %s()", pc, stringTable[op>>16].c_str());
                    break;

                case Instructions::iStateEnd:
                case Instructions::iSubStateEnd:
                case Instructions::iStateSuspend:
                    sprintf(buff, "%04d:%s\n", pc, info.name);
                    break;

                case Instructions::iGetLocalString:
                case Instructions::iGetLocalVarObj:
                case Instructions::iGetLocalExeVarObj:
                    sprintf(buff, "%04d:%s %d", pc, info.name, op >> 16);
                    break;
                    
                default:
                    sprintf(buff, "%04d:%s \t// Stack:%s", pc, info.name, info.stackDoc);
                    break;
		    }
            skip = info.argCount;
            return buff;
        }

        char const*const disassemble(unsigned int const*const program, int pc, 
									 const char* stringData, int* stringArray, int& skip)
        {
            static char buff[1024];
            int op = program[pc];
		    const Instructions::Info& info = Instructions::getInfo((Instructions::Op) op);
			
		    switch(op & 0xffff)
		    {
				case Instructions::iGotoAddr:
					sprintf(buff, "%04d:%s %d", pc, info.name, program[pc+1]);
					break;
					
				case Instructions::iPushVar:
				case Instructions::iPushConstant:
					sprintf(buff, "%04d:%s %d \t// Stack:. > %d", pc, info.name, program[pc+1], program[pc+1]);
					break;
					
				case Instructions::iPushFloatConstant:
					{
						union { int i; float f; } u;
						u.i = program[pc+1];
						sprintf(buff, "%04d:%s %f \t// Stack:. > %f", pc, info.name, u.f, u.f);
					}
					break;
					
                case Instructions::iGetRequire:
                    sprintf(buff, "%04d:GetRequire %d \t// Stack:. >x", pc, op>>16);
                    break;
					
				case Instructions::iCallFunction:
					{
						sprintf(buff, "%04d:Call %s()", pc, &stringData[stringArray[op>>16]]);
					}
					break;

				case Instructions::iStateEnd:
				case Instructions::iSubStateEnd:
				case Instructions::iStateSuspend:
					sprintf(buff, "%04d:%s\n", pc, info.name);
					break;
					
                case Instructions::iGetLocalString:
                case Instructions::iGetLocalVarObj:
                case Instructions::iGetLocalExeVarObj:
                    sprintf(buff, "%04d:%s %d", pc, info.name, op >> 16);
                    break;
                    
				default:
					sprintf(buff, "%04d:%s \t// Stack:%s", pc, info.name, info.stackDoc);
					break;
		    }
            skip = info.argCount;
            return buff;
        }

    } // Instructions

}
