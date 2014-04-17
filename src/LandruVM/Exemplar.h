
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_EXEMPLAR_H
#define LANDRU_EXEMPLAR_H

#include <LandruVM/VarObjArray.h>
#include <stdio.h>
#include <vector>

namespace Landru
{
    class VarObj;

	class Exemplar
	{
	public:
		Exemplar();
		~Exemplar();

		void            copy(Exemplar*);
		void            disassemble(FILE* output);

		void			deallocate();
		
        int				varIndex(const char* varName);
        int				sharedVarIndex(const char* varName);
        int				stateIndex(const char* stateName);
		const char*		name() const { return nameStr; }

		const char*		nameStr;				///< Pointer to 0th string, convenience
		int				stateTableLength;		///< Number of states in the machine
		unsigned int*	stateTable;				///< Index to first statement in each state
        int*            stateNameIndex;			///< state name = stringData[stringArray[stateNameIndex[stateIndex]]]

		int				programStoreLength;		///< Size, in ints, of programStore
		unsigned int*	programStore;			///< Opcodes

		int				stringArrayLength;		///< Number of strings in array
		int*			stringArray;			///< Offset into string data
		int				stringDataLength;		///< Size, in bytes of stringData
		const char*		stringData;				///< String data

		int				varCount;				///< Number of vars
		int*			varNameIndex;			///< var name = stringData[stringArray[varNameIndex[varIndex]]]
		int*			varTypeIndex;			///< var type = stringData[stringArray[varTypeIndex[varIndex]]]
		int				sharedVarCount;			///< Number of shared vars
		int*			sharedVarNameIndex;		///< var name = stringData[stringArray[varNameIndex[varIndex]]]
		int*			sharedVarTypeIndex;		///< var type = stringData[stringArray[varTypeIndex[varIndex]]]
    };

}

#endif
