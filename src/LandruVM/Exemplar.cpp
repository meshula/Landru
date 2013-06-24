
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruVM/Exemplar.h"
#include "LandruVM/Instructions.h"

#include <string.h>
#include <stdlib.h>

namespace Landru
{
	Exemplar::Exemplar()
	:nameStr(0)
	,stateTableLength(0)
	,stateTable(0)
	,stateNameIndex(0)
	,programStoreLength(0)
	,programStore(0)	
	,stringArrayLength(0)
	,stringArray(0)
	,stringDataLength(0)
	,stringData(0)
	,varCount(0)
	,varNameIndex(0)
	,varTypeIndex(0)
	,sharedVarCount(0)
	,sharedVarNameIndex(0)
	,sharedVarTypeIndex(0)
	{
	}
	
	Exemplar::~Exemplar()
	{
		Exemplar::deallocate();
	}
	
	void Exemplar::deallocate()
	{
		free(stateTable);
        free((void*)stateNameIndex);
		free((void*)programStore);
		free(stringArray);
		free((void*)stringData);
		free((void*)varNameIndex);
		free((void*)varTypeIndex);
        nameStr = 0;
		stateTable = 0;
		stateNameIndex = 0;
		programStore = 0;
		stringArray = 0;
		stringData = 0;
		varNameIndex = 0;
		varTypeIndex = 0;
        nameStr = 0;
	}
	
	
	void Exemplar::copy(Exemplar* m)
	{
		stateTableLength = m->stateTableLength;
		programStoreLength = m->programStoreLength;
		stringArrayLength = m->stringArrayLength;
		stringDataLength = m->stringDataLength;
		stateTable = (unsigned int*) malloc(sizeof(unsigned int) * stateTableLength);
		memcpy(stateTable, m->stateTable, sizeof(unsigned int) * stateTableLength);
        stateNameIndex = (int*) malloc(sizeof(int) * stateTableLength);
        memcpy(stateNameIndex, m->stateNameIndex, sizeof(int) * stateTableLength);
		programStore = (unsigned int*) malloc(sizeof(unsigned int) * m->programStoreLength);
		memcpy(programStore, m->programStore, sizeof(unsigned int) * m->programStoreLength);
		stringArray = (int*) malloc(sizeof(int) * stringArrayLength);
		memcpy(stringArray, m->stringArray, sizeof(int) * stringArrayLength);
		stringData = (char*) malloc(stringDataLength);
		memcpy((void*)stringData, m->stringData, stringDataLength);
		nameStr = stringData;

		varCount = m->varCount;
		varNameIndex = (int*) malloc(sizeof(int) * varCount);
		memcpy(varNameIndex, m->varNameIndex, sizeof(int) * varCount);
		if (m->varTypeIndex)
		{
			varTypeIndex = (int*) malloc(sizeof(int) * varCount);
			memcpy(varTypeIndex, m->varTypeIndex, sizeof(int) * varCount);
		}
		else
		{
			varTypeIndex = 0;
		}

		sharedVarCount = m->sharedVarCount;
		if (sharedVarCount > 0)
		{
			sharedVarNameIndex = (int*) malloc(sizeof(int) * sharedVarCount);
			memcpy(sharedVarNameIndex, m->sharedVarNameIndex, sizeof(int) * sharedVarCount);
		}
		else 
		{
			sharedVarNameIndex = 0;
		}

		if (m->sharedVarTypeIndex)
		{
			sharedVarTypeIndex = (int*) malloc(sizeof(int) * sharedVarCount);
			memcpy(sharedVarTypeIndex, m->sharedVarTypeIndex, sizeof(int) * sharedVarCount);
		}
		else
		{
			sharedVarTypeIndex = 0;
		}
	}
	
	int Exemplar::varIndex(const char* varName)
	{
		int retval = -1;
		
		// is it a local var on the current fiber?
		for (int i = 0; i < varCount; ++i)
		{
			int index = varNameIndex[i];
			const char* thisVarName = &stringData[stringArray[index]];
			if (!strcmp(varName, thisVarName))
			{
				retval = i;
				break;
			}
		}
		return retval;
	}
	
	int Exemplar::sharedVarIndex(const char* varName)
	{
		int retval = -1;
		
		// is it a local var on the current fiber?
		for (int i = 0; i < sharedVarCount; ++i)
		{
			int index = sharedVarNameIndex[i];
			const char* thisVarName = &stringData[stringArray[index]];
			if (!strcmp(varName, thisVarName))
			{
				retval = i;
				break;
			}
		}
		return retval;
	}
	
	int Exemplar::stateIndex(const char* stateName)
	{
		int retval = -1;
		for (int i = 0; i < stateTableLength; ++i)
		{
			int index = stateNameIndex[i];
			const char* thisStateName = &stringData[stringArray[index]];
			if (!strcmp(stateName, thisStateName))
			{
				retval = i;
				break;
			}
		}
		return retval;
	}
	
	void Exemplar::disassemble(FILE* f)
	{
		fprintf(f, "\nMachine: %s\n", name());
		
		fprintf(f, "\tStrings:\n");
		for (size_t i = 0; i < stringArrayLength; ++i)
		{
			int strIndex = stringArray[i];
			const char* str = &stringData[strIndex];
			fprintf(f, "\t\t%d:\"%s\"\n", (int) i, str);
		}
		
		fprintf(f, "\tLocal Variables:\n");
		for (size_t i = 0; i < varCount; ++i)
		{
			int varNameIdx = varNameIndex[i];
			int strIndex = stringArray[varNameIdx];
			const char* str = &stringData[strIndex];
			int varTypeIdx = varTypeIndex[i];
			strIndex = stringArray[varTypeIdx];
			const char* typeStr = &stringData[strIndex];
			fprintf(f, "\t\t%d:%s \"%s\"\n", (int) i, typeStr, str);
		}
		
		fprintf(f, "\tShared Variables:\n");
		for (size_t i = 0; i < sharedVarCount; ++i)
		{
			int varNameIdx = sharedVarNameIndex[i];
			int strIndex = stringArray[varNameIdx];
			const char* str = &stringData[strIndex];
			int varTypeIdx = sharedVarTypeIndex[i];
			strIndex = stringArray[varTypeIdx];
			const char* typeStr = &stringData[strIndex];
			fprintf(f, "\t\t%d:%s \"%s\"\n", (int) i, typeStr, str);
		}

		fprintf(f, "\tStates:\n");
		for (size_t i = 0; i < stateTableLength; ++i)
		{
			int addr = stateTable[i];
			int stateNameIdx = stateNameIndex[i];
			int strIndex = stringArray[stateNameIdx];
			const char* str = &stringData[strIndex];
			fprintf(f, "\t\t%02d:[%04d]%s\n", (int) i, addr, str);
		}
		
		for (int i = 0; i < programStoreLength; ++i)
		{
			int skip;
			char const*const dis = Instructions::disassemble(programStore, i, stringData, stringArray, skip);
			fprintf(f, "%s\n", dis);
			i += skip;
		}
		fflush(f);
	}
	

} // Landru

