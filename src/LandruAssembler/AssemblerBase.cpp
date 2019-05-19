
// Copyright (c) 2011 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT


//#define ASSEMBLER_TRACE(a) printf("Assembler: %s\n", #a)
#define ASSEMBLER_TRACE(a)

#include "AssemblerBase.h"
#include "LandruCompiler/AST.h"
#include "LandruCompiler/lcRaiseError.h"
#include "LandruCompiler/Parser.h"

#ifdef LANDRU_HAVE_BSON
#include "LabJson/LabBson.h"
#include "BsonCompiler.h"
#endif
#include "LabText/LabText.h"

#include <string.h>
#include <map>

using namespace std;
#ifdef LANDRU_HAVE_BSON
using Lab::Bson;
#endif

/*
 The Assembler knows the actual details of machine code, and string and variable indexing
 */

namespace Landru {

    using lab::Text::StrView;

    AssemblerBase::~AssemblerBase() {
    }

    bool AssemblerBase::isLocalVar(const char* name) const {
        for (auto scope : scopedVariables)
            for (auto var : scope)
                if (!strcmp(name, var.first.c_str()))
                    return true;
        return false;
    }


void AssemblerBase::assembleNode(ASTNode* root) {
    switch (root->token) {
        case kTokenProgram:
            ASSEMBLER_TRACE(kTokenProgram);
            assembleStatements(root); // parameters
            break;

        case kTokenMachine:
            ASSEMBLER_TRACE(kTokenMachine);
            assembleMachine(root);
            break;

        case kTokenLaunch:
            ASSEMBLER_TRACE(kTokenLaunch);
            assembleStatements(root); // parameters
            launchMachine();
            break;

        case kTokenLocalVariable:
            // this was handled in assembleStatements
            break;

        case kTokenIf: {
            // children: qualifier, statements, [else, statements]
            ASSEMBLER_TRACE(kTokenIf);
            ASTConstIter j = root->children.begin();
            assembleNode(*j++); // qualifier such as Eq, Lte0, etc.
            if ((*j)->token != kTokenStatements) {
                lcRaiseError("Expected statements", 0, 0);
                break;
            }
            beginConditionalClause();
            assembleStatements(*j++);
            if (j != root->children.end()) {
                if ((*j)->token != kTokenElse) {
                    lcRaiseError("Expected else", 0, 0);
                    break;
                }
                beginContraConditionalClause();
                ASTConstIter k = (*j)->children.begin();
                if (k != (*j)->children.end()) {
                    assembleStatements(*k); // else
                }
            }
            endConditionalClause();
            break;
        }

        case kTokenEq: ASSEMBLER_TRACE(kTokenEq); assembleStatements(root); ifEq(); break;
        case kTokenLte0: ASSEMBLER_TRACE(kTokenLte0); assembleStatements(root); ifLte0(); break;
        case kTokenGte0: ASSEMBLER_TRACE(kTokenGte0); assembleStatements(root); ifGte0(); break;
        case kTokenLt0: ASSEMBLER_TRACE(kTokenLt0); assembleStatements(root); ifLt0(); break;
        case kTokenGt0: ASSEMBLER_TRACE(kTokenGt0); assembleStatements(root); ifGt0(); break;
        case kTokenEq0: ASSEMBLER_TRACE(kTokenEq0); assembleStatements(root); ifEq0(); break;
        case kTokenNotEq0: ASSEMBLER_TRACE(kTokenNotEq0); assembleStatements(root); ifNotEq0(); break;

        case kTokenParameters:
            ASSEMBLER_TRACE(kTokenParameters);
            paramsStart();
            assembleStatements(root);           // recursively assemble the code for everything that was between parens
            paramsEnd();
            break;

        case kTokenFunction:
            ASSEMBLER_TRACE(kTokenFunction);
            for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i)
                assembleNode(*i);
            callFunction(root->str2.c_str());
            break;

        case kTokenDotChain:
            ASSEMBLER_TRACE(kTokenDotChain);
            dotChain();
            break;

        case kTokenIntLiteral:
            ASSEMBLER_TRACE(kTokenIntLiteral);
            pushConstant(root->intVal);
            break;

        case kTokenFloatLiteral:
            ASSEMBLER_TRACE(kTokenFloatLiteral);
            pushFloatConstant(root->floatVal1);
            break;

        case kTokenRangedLiteral:
            ASSEMBLER_TRACE(kTokenRangedLiteral);
            pushRangedRandom(root->floatVal1, root->floatVal2);
            break;

        case kTokenStringLiteral:
            ASSEMBLER_TRACE(kTokenStringLiteral);
            pushStringConstant(root->str2.c_str());
            break;

        case kTokenTrue:
            ASSEMBLER_TRACE(kTokenTrue);
            pushConstant(1);
            break;

        case kTokenFalse:
            ASSEMBLER_TRACE(kTokenFalse);
            pushConstant(0);
            break;

        case kTokenGetVariable: {
            ASSEMBLER_TRACE(kTokenGetVariable);
            vector<StrView> parts = lab::Text::Split(StrView{ root->str2.c_str(), root->str2.size() }, '.');
            std::string tname(parts[0].curr, parts[0].sz);
            const char* name = tname.c_str();
            if (isLocalVar(name)) {
                pushLocalVar(name);
            }
            else if (selfVarNames.find(name) != selfVarNames.end()) {
                pushInstanceVar(name);
            }
            else if (sharedVars.find(name) != sharedVars.end()) {
                pushSharedVar(name);
            }
			else if (globals.find(name) != globals.end()) {
				pushGlobalVar(name);
			}
            else {
                AB_RAISE("Unknown variable named " << root->str2);
            }
            break;
        }

		case kTokenGetVariableReference: {
            ASSEMBLER_TRACE(kTokenGetVariableReference);
            vector<StrView> parts = lab::Text::Split(StrView{ root->str2.c_str(), root->str2.size() }, '.');
            std::string tname(parts[0].curr, parts[0].sz);
            const char* name = tname.c_str();
            if (isLocalVar(name)) {
                AB_RAISE("Cannot reference local variables. " << root->str2);
            }
            else if (selfVarNames.find(name) != selfVarNames.end()) {
                pushInstanceVarReference(name);
            }
            else if (sharedVars.find(name) != sharedVars.end()) {
                pushSharedVarReference(name);
            }
			else if (globals.find(name) != globals.end()) {
                pushGlobalVarReference(name);
			}
            else {
                AB_RAISE("Unknown variable named " << root->str2);
            }
            break;
        }

        case kTokenAssignment:
            ASSEMBLER_TRACE(kTokenAssignment);
            assembleStatements(root); //function(new)
            storeToVar(root->str2.c_str());
            break;

		case kTokenInitialAssignment:
			ASSEMBLER_TRACE(kTokenInitialAssignment);
			assembleStatements(root); //function(new)
			initializeSharedVarIfNecessary(root->str2.c_str());
			break;

        case kTokenGoto:
            ASSEMBLER_TRACE(kTokenGoto);
            gotoState(root->str2.c_str());
            break;

        case kTokenFor: {
            ASSEMBLER_TRACE(kTokenFor);
            ASTConstIter j = root->children.begin();
            assembleNode(*j++); // the function that returns a range iterator

            const char* type = root->str1.c_str(); // type
            const char* name = root->str2.c_str(); // name
            scopedVariables.push_back(vector<pair<string, string>>());
            scopedVariables.back().push_back(pair<string,string>(name, type));
            beginForEach(name, type);
            assembleStatements(*j); // statements
            endForEach();
            scopedVariables.pop_back();
            break;
        }

        case kTokenOn: {
            ASSEMBLER_TRACE(kTokenOn);
            beginOn();
            ASTConstIter j = root->children.begin();
            assembleNode(*j++); // the event part of the on statement
            beginOnStatements();
            assembleStatements(*j); // statements
            endOnStatements();
            break;
        }

        case kTokenOpAdd: ASSEMBLER_TRACE(kTokenOpAdd); opAdd(); break;
        case kTokenOpSubtract: ASSEMBLER_TRACE(kTokenOpSubtract); opSubtract(); break;
        case kTokenOpMultiply: ASSEMBLER_TRACE(kTokenOpMultiply); opMultiply(); break;
        case kTokenOpDivide: ASSEMBLER_TRACE(kTokenOpDivide); opDivide(); break;
        case kTokenOpNegate: ASSEMBLER_TRACE(kTokenOpNegate); opNegate(); break;
        case kTokenOpModulus: ASSEMBLER_TRACE(kTokenOpModulus); opModulus(); break;
        case kTokenOpGreaterThan: ASSEMBLER_TRACE(kTokenOpGreaterThan); opGreaterThan(); break;
        case kTokenOpLessThan: ASSEMBLER_TRACE(kTokenOpLessThan); opLessThan(); break;

        default:
            lcRaiseError("Compile Error: unknown node encountered\n", 0, 0);
            break;
    }
}


void AssemblerBase::assembleStatements(ASTNode* root) {
    beginLocalVariableScope();

    scopedVariables.push_back(vector<pair<string, string>>());

	/*
	this loop should be

	for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i)
		assembleNode(*i);

	and assemble node should have all the smarts for variable handling.
	&&&
	*/
    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
		if ((*i)->token == kTokenLocalVariable) {
			for (ASTConstIter j = (*i)->children.begin(); j != (*i)->children.end(); ++j) {
				if (!(*j)->str1.length()) // unnamed variable? how did that happen??
					continue;

				const char* type = (*j)->str1.c_str(); // type
				const char* name = (*j)->str2.c_str(); // name
				addLocalVariable(name, type);
				scopedVariables.back().push_back(pair<string, string>(name, type));

				if ((*j)->token == kTokenParam)
					storeToVar(name);

				if ((*j)->children.size()) {
					ASTConstIter k = (*j)->children.begin();
					if ((*k)->token == kTokenAssignment) {
						// An assignment is part of the local variable declaration
						assembleNode(*k);
					}
				}
			}
		}
		else if ((*i)->token == kTokenSharedVariable) {
			for (ASTConstIter j = (*i)->children.begin(); j != (*i)->children.end(); ++j) {
				if (!(*j)->str1.length()) // unnamed variable? how did that happen??
					continue;

				const char* type = (*j)->str1.c_str(); // type
				const char* name = (*j)->str2.c_str(); // name
				addSharedVariable(name, type);
				scopedVariables.back().push_back(pair<string, string>(name, type));

				if ((*j)->children.size()) {
					ASTConstIter k = (*j)->children.begin();
					if ((*k)->token == kTokenEq) {
						// An assignment is part of the local variable declaration
						assembleNode(*k);
						initializeSharedVarIfNecessary(name);
					}
				}
			}
		}
		else
			assembleNode(*i);
	}

    scopedVariables.pop_back();

    endLocalVariableScope();
}

void AssemblerBase::assembleState(ASTNode* root) {
    beginState(root->str2.c_str());   // this will update the program index

    if (root->token != kTokenState) {
        lcRaiseError("Compile Error: node is not a state\n", 0, 0);
        return;
    }

    bool once = true;
	for (auto i : root->children) {
		if (once == false) {
            lcRaiseError("Compile Error: should only be one statements node under a state node\n", 0, 0);
            return;
        }
        once = false;

        assembleStatements(i);
    }
    endState();
}

void AssemblerBase::assembleDeclarations(ASTNode* root)
{
	std::vector<ASTNode*> declares;

	// gather all global declarations
	for (auto i : root->children) {
		if (i->token == kTokenDeclare) {
			declares.push_back(i);
		}
	}

	if (!declares.size())
		return;

	for (auto i : declares) {
		for (auto j : i->children)
			if (j->token == kTokenSharedVariable) {
				addSharedVariable(j->str2.c_str(), j->str1.c_str());
				sharedVars[j->str2] = j->str1.c_str();
			}
			else {
				addInstanceVariable(j->str2.c_str(), j->str1.c_str());
				selfVarNames.insert(j->str2);
			}
	}

	beginState("__auto__");
	for (auto i : declares)
		for (auto j : i->children)
			assembleStatements(j);
	endState();
}

	void AssemblerBase::assembleMachine(ASTNode* root)
	{
		beginMachine(root->str2.c_str());

		assembleDeclarations(root);

		// compile main state first, then all other states
		for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
			if ((*i)->token == kTokenDeclare)
				continue;

			if ((*i)->str2 != "main")
				continue;

			assembleState(*i);
			break;
		}
		for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
			if ((*i)->token == kTokenDeclare)
				continue;

			if ((*i)->str2 == "main")
				continue;

			assembleState(*i);
		}
		endMachine();
	}



vector<string> AssemblerBase::requires2(ASTNode* root)
{
    vector<string> result;

    for (auto i : root->children) {
        if (i->token == kTokenGlobalVariable && i->children.size()) {
            ASTConstIter kind = i->children.begin();
            if ((*kind)->token == kTokenRequire)
                result.push_back((*kind)->str2);
        }
    }

    return result;
}

void AssemblerBase::assemble(ASTNode* root)
{
    //landruPrintRawAST(rootNode);
    //landruPrintAST(root);

	{   // gather all machine-scoped declarations
		std::vector<ASTNode*> declarations;

		for (auto i : root->children) {
			if (i->token == kTokenGlobalVariable) {
				if (i->children.size()) {
					ASTConstIter kind = i->children.begin();
					if ((*kind)->token == kTokenRequire) {
						_requires[i->str2.c_str()] = (*kind)->str2.c_str();
						addRequire(i->str2.c_str(), (*kind)->str2.c_str());
					}
					else {
#ifdef LANDRU_HAVE_BSON
						BsonCompiler bc;
						addGlobalBson(i->str2.c_str(), bc.compileBson(*kind));
#endif
					}
				}
			}
			else if (i->token == kTokenDeclare) 
            {
				// gather all global declarations
				for (auto j : i->children)
					if (j->token == kTokenLocalVariable) 
                    {
						declarations.push_back(j);
					}
					else if (j->token == kTokenAssignment) 
                    {
						for (auto k : j->children)
							declarations.push_back(k);
					}
			}
		}

        // instantiate the globals
		if (declarations.size() > 0) 
        {
			for (auto i : declarations) {
				if (i->token == kTokenLocalVariable) {
					string type = i->str1;
					string name = i->str2;
                    bool set_val = false;
                    if (i->children.size())
                    {
                        auto i_assign = *i->children.begin();
                        if (i_assign->token == kTokenAssignment && i_assign->children.size())
                        {
						    auto j = *i_assign->children.begin();
						    if (j->token == kTokenStringLiteral) {
							    string val = j->str2;
							    // create string property named(name);
							    addGlobalString(name.c_str(), val.c_str());
                                set_val = true;
						    }
						    else if (j->token == kTokenIntLiteral) {
							    int val = j->intVal;
							    addGlobalInt(name.c_str(), val);
                                set_val = true;
						    }
						    else if (j->token == kTokenFloatLiteral) {
							    float val = j->floatVal1;
							    addGlobalFloat(name.c_str(), val);
                                set_val = true;
						    }
						    // @todo ranges, what else?
                        }
                    }
                    if (!set_val)
    					addGlobal(name.c_str(), type.c_str());
				}
				else if (i->token == kTokenAssignment) {
					string name = i->str2;
					if (i->children.size()) {
						auto j = *i->children.begin();
						if (j->token == kTokenStringLiteral) {
							string val = j->str2;
							// create string property named(name);
							addGlobalString(name.c_str(), val.c_str());
						}
						else if (j->token == kTokenIntLiteral) {
							int val = j->intVal;
							addGlobalInt(name.c_str(), val);
						}
						else if (j->token == kTokenFloatLiteral) {
							float val = j->floatVal1;
							addGlobalFloat(name.c_str(), val);
						}
						// @todo ranges, what else?
					}
				}
			}
		}
	}

    // assemble all the machines
	for (auto i : root->children) {
		if (i->token == kTokenMachine) {
            startAssembling();  // reset the assembler so that it is ready to compile something new
            assembleNode(i);
            finalizeAssembling(); // the assembler needs to record its results in finalize
        }
    }

    if (false) {
        FILE* f = fopen("/Users/dp/lasm.txt", "at");
        disassemble(root->str2, f);
        fflush(f);
        fclose(f);
    }
}

} // Landru

