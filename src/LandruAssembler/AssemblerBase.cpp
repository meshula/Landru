
// Copyright (c) 2011 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "AssemblerBase.h"
#include "LandruCompiler/AST.h"
#include "LandruCompiler/lcRaiseError.h"
#include "LabJson/LabBson.h"
#include "LabText/itoa.h"
#include "LabText/TextScanner.hpp"

#include <string.h>
#include <map>

using namespace std;
using Lab::Bson;

#ifdef _MSC_VER
# define itoa _itoa
#endif

/*
 The Assembler knows the actual details of machine code, and string and variable indexing
 */

//#define ASSEMBLER_TRACE(a) printf("Assembler: %s\n", #a)
#define ASSEMBLER_TRACE(a)

namespace Landru {

    AssemblerBase::~AssemblerBase() {
    }

    bool AssemblerBase::isLocalVar(const char* name) const {
        for (auto scope : locals)
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
            vector<string> parts = TextScanner::Split(root->str2, '.', false, false);
            const char* name = parts[0].c_str();
            if (isLocalVar(name)) {
                pushLocalVar(name);
            }
            else if (selfVarNames.find(name) != selfVarNames.end()) {
                pushInstanceVar(name);
            }
            else if (sharedVars.find(name) != sharedVars.end()) {
                pushSharedVar(name);
            }
            else if (requires.find(name) != requires.end()) {
                pushRequire(name);
            }
            else if (globals.find(name) != globals.end()) {
                pushGlobalVar(name);
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
            locals.push_back(vector<pair<string, string>>());
            locals.back().push_back(pair<string,string>(name, type));
            beginForEach(name, type);
            assembleStatements(*j); // statements
            endForEach();
            locals.pop_back();
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

    locals.push_back(vector<pair<string, string>>());

    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
        if ((*i)->token == kTokenLocalVariable) {
            for (ASTConstIter j = (*i)->children.begin(); j != (*i)->children.end(); ++j) {
                if (!(*j)->str1.length()) // unnamed variable? how did that happen??
                    continue;

                const char* type = (*j)->str1.c_str(); // type
                const char* name = (*j)->str2.c_str(); // name
                addLocalVariable(name, type);
                locals.back().push_back(pair<string,string>(name, type));

                if ((*j)->children.size()) {
                    ASTConstIter k = (*j)->children.begin();
                    if ((*k)->token == kTokenEq) {
                        // An assignment is part of the local variable declaration
                        assembleNode(*k);
                        storeToVar(name);
                    }
                }
            }
        }
    }

    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i)
        assembleNode(*i);

    locals.pop_back();

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

void AssemblerBase::assembleDeclarations(ASTNode* root) {
	for (auto i : root->children) {
		if (i->token == kTokenSharedVariable) {
            addSharedVariable(i->str2.c_str(), i->str1.c_str());
            sharedVars[i->str2] = i->str1.c_str();
        }
        else {
            addInstanceVariable(i->str2.c_str(), i->str1.c_str());
            selfVarNames.insert(i->str2);
        }
    }

	beginState("__auto__");
	for (auto i : root->children) {
		assembleStatements(i);
	}
	endState();
}

void AssemblerBase::assembleMachine(ASTNode* root) {
    beginMachine(root->str2.c_str());

    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) 
		if ((*i)->token == kTokenDeclare) {
			assembleDeclarations(*i);
		}

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

    class BsonCompiler {
    public:
        // temp for bson parsing
        std::vector<int> bsonArrayNesting;
        int bsonCurrArrayIndex;

        BsonCompiler() : bsonCurrArrayIndex(0) {}

        shared_ptr<Bson> compileBson(Landru::ASTNode* rootNode) {
            bsonArrayNesting.clear();
            bsonCurrArrayIndex = 0;
            shared_ptr<Bson> b = make_shared<Bson>();
            compileBsonData(rootNode, b->b);
            bson_finish(b->b);
            bson_print(b->b);
            return b;
        }

        void compileBsonData(Landru::ASTNode* rootNode, bson* b)
        {
            using namespace Landru;
            for (ASTConstIter i = rootNode->children.begin(); i != rootNode->children.end(); ++i)
                compileBsonDataElement(*i, b);
        }

        void compileBsonDataElement(Landru::ASTNode* rootNode, bson* b)
        {
            using namespace Landru;
            switch(rootNode->token) {

                case kTokenDataObject:
                {
                    ASSEMBLER_TRACE(kTokenDataObject);
                    if (bsonArrayNesting.size() > 0) {
                        char indexStr[16];
                        itoa(bsonCurrArrayIndex, indexStr, 10);
                        //printf("----> %s\n", indexStr);
                        bson_append_start_object(b, indexStr);
                        compileBsonData(rootNode, b); // recurse
                        bson_append_finish_object(b);
                        ++bsonCurrArrayIndex;
                    }
                    else
                        compileBsonData(rootNode, b); // recurse
                    break;
                }

                case kTokenDataArray:
                {
                    ASSEMBLER_TRACE(kTokenDataArray);
                    bsonArrayNesting.push_back(bsonCurrArrayIndex);
                    bsonCurrArrayIndex = 0;
                    compileBsonData(rootNode, b); // recurse
                    bsonCurrArrayIndex = bsonArrayNesting.back();
                    bsonArrayNesting.pop_back();
                    break;
                }

                case kTokenDataElement:
                {
                    ASSEMBLER_TRACE(kTokenDataElement);
                    ASTConstIter i = rootNode->children.begin();
                    if ((*i)->token == kTokenDataIntLiteral) {
                        int val = atoi((*i)->str2.c_str());
                        bson_append_int(b, rootNode->str2.c_str(), val);
                    }
                    else if ((*i)->token == kTokenDataFloatLiteral)
                    {
                        float val = (float) atof(rootNode->str2.c_str());
                        bson_append_double(b, rootNode->str2.c_str(), val);
                    }
                    else if ((*i)->token == kTokenDataNullLiteral) {
                        bson_append_null(b, rootNode->str2.c_str());
                    }
                    else {
                        bson_append_start_object(b, rootNode->str2.c_str());
                        compileBsonData(rootNode, b); // recurse
                        bson_append_finish_object(b);
                    }
                    break;
                }

                case kTokenDataFloatLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataFloatLiteral);
                    float val = (float) atof(rootNode->str2.c_str());
                    bson_append_double(b, rootNode->str2.c_str(), val);
                    break;
                }

                case kTokenDataIntLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataIntLiteral);
                    int val = atoi(rootNode->str2.c_str());
                    bson_append_int(b, rootNode->str2.c_str(), val);
                    break;
                }

                case kTokenDataNullLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataNullLiteral);
                    bson_append_null(b, rootNode->str2.c_str());
                    break;
                }

                case kTokenDataStringLiteral:
                {
                    ASSEMBLER_TRACE(kTokenDataStringLiteral);
                    bson_append_string(b, rootNode->str2.c_str(), rootNode->str1.c_str());
                    break;
                }

                default:
                    break;
            }
        }

    };

void AssemblerBase::assemble(ASTNode* root) {
    //landruPrintRawAST(rootNode);
    //landruPrintAST(root);

    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
        if ((*i)->token == kTokenGlobalVariable) {
            ASTConstIter kind = (*i)->children.begin();
            if ((*kind)->token == kTokenRequire) {
                requires[(*i)->str2.c_str()] = (*kind)->str2.c_str();
                addRequire((*i)->str2.c_str(), (*kind)->str2.c_str());
            }
            else {
                BsonCompiler bc;
                addGlobalBson((*i)->str2.c_str(), bc.compileBson(*kind));
            }
        }
        else if ((*i)->token == kTokenMachine) {
            startAssembling();  // reset the assembler so that it is ready to compile something new
            assembleNode(*i);
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

