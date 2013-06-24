
// Copyright (c) 2011 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "AssemblerBase.h"
#include "LandruCompiler/AST.h"
#include "LandruCompiler/lcRaiseError.h"

/*
 The Assembler knows the actual details of machine code, and string and variable indexing
 */

//#define ASSEMBLER_TRACE(a) printf("Assembler: %s\n", #a)
#define ASSEMBLER_TRACE(a)

namespace Landru {

    
void AssemblerBase::assembleNode(ASTNode* root)
{
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
            
        case kTokenIf:
            ASSEMBLER_TRACE(kTokenIf);
            {
                ASTConstIter j = root->children.begin();
                assembleNode(*j); // qualifier/parameters
                int patch = gotoAddr();
                ++j;
                if ((*j)->token != kTokenStatements) {
                    lcRaiseError("Expected statements", 0, 0);
                    break;
                }
                assembleStatements(*j);
                ++j;
                if (j != root->children.end()) {
                    if ((*j)->token != kTokenElse) {
                        lcRaiseError("Expected else", 0, 0);
                        break;
                    }
                    int patch2 = gotoAddr();
                    _patchGoto(patch);
                    ASTConstIter k = (*j)->children.begin();
                    if (k != (*j)->children.end()) {
                        assembleStatements(*k); // else
                    }
                    _patchGoto(patch2);
                }
                else
                    _patchGoto(patch);
            }
            break;
            
        case kTokenEq:		ASSEMBLER_TRACE(kTokenEq);     assembleStatements(root); ifEq();		break;
        case kTokenLte0:	ASSEMBLER_TRACE(kTokenLte0);   assembleStatements(root); ifLte0();	break;
        case kTokenGte0:	ASSEMBLER_TRACE(kTokenGte0);   assembleStatements(root); ifGte0();	break;
        case kTokenLt0:		ASSEMBLER_TRACE(kTokenLt0);    assembleStatements(root); ifLt0();     break;
        case kTokenGt0:		ASSEMBLER_TRACE(kTokenGt0);    assembleStatements(root); ifGt0();     break;
        case kTokenEq0:		ASSEMBLER_TRACE(kTokenEq0);    assembleStatements(root); ifEq0();     break;
        case kTokenNotEq0:	ASSEMBLER_TRACE(kTokenNotEq0); assembleStatements(root); ifNotEq0();	break;
            
        case kTokenParameters:
            ASSEMBLER_TRACE(kTokenParameters);
            paramsStart();
            assembleStatements(root);	// run through all children
            paramsEnd();
            break;
            
        case kTokenFunction:
            ASSEMBLER_TRACE(kTokenFunction);
            assembleStatements(root);	// parameters
            callFunction(root->str2.c_str());
            break;
            
        case kTokenDynamicFunction:
            ASSEMBLER_TRACE(kTokenDynamicFunction);
            assembleStatements(root);	// parameters
            callDynamicFunction(root->str2.c_str());
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
            pushFloatConstant(root->floatVal2);
            pushFloatConstant(root->floatVal1);
            rangedRandom();
            break;
            
        case kTokenStringLiteral:
            ASSEMBLER_TRACE(kTokenStringLiteral);
            pushStringConstant(root->str2.c_str());
            createTempString();  // creates a temp StringVarObj, valid only until the semicolon is hit, all temps are reaped at the end of this run loop
            break;

        case kTokenTrue:
            ASSEMBLER_TRACE(kTokenTrue);
            pushIntOne();
            break;
            
        case kTokenFalse:
            ASSEMBLER_TRACE(kTokenFalse);
            pushIntZero();
            break;
            
        case kTokenParam:
            ASSEMBLER_TRACE(kTokenParam);
            {
                for (ASTConstIter j = root->children.begin(); j != root->children.end(); ++j) {
                    const char* type = (*j)->str1.c_str(); // type
                    const char* name = (*j)->str2.c_str(); // name
                    addLocalParam(name, type);
                }
            }
            break;

            
        case kTokenType:
            ASSEMBLER_TRACE(kTokenType);
            {
                const char* name = root->str2.c_str(); // name
                if (isLocalParam(root->str2.c_str())) {
                    getLocalParam(localParamIndex(name)); // push local parameter op (with embedded index)
                }
                else if (isGlobalVariable(name)) {
                    pushGlobalVarIndex(name);	// actually pushes string index
                    getGlobalVar();
                }
                else if (isSharedVariable(name)) {
                    pushSharedVarIndex(name);	// actually pushes string index
                    getSharedVar();
                }
                else {
                    pushVarIndex(name);		// pushes true variable index
                    getSelfVar();
                }
            }
            break;
            
        case kTokenAssignment:
            ASSEMBLER_TRACE(kTokenAssignment);
            // &&& is this where to detect shared?
            // assignment(var)/function(new)/parameters/stringLiteral(physics2d.body)
            assembleStatements(root); //function(new)
            pushVarIndex(root->str2.c_str());
            popStore();
            break;
            
        case kTokenGoto:
            ASSEMBLER_TRACE(kTokenGoto);
            gotoState(root->str2.c_str());
            break;
            
        case kTokenRepeat:
            ASSEMBLER_TRACE(kTokenRepeat);
            {
                ASTConstIter j = root->children.begin();
                assembleNode(*j); // qualifier/parameters
                repeatBegin();
                int loopTop = programSize();
                repeatTest();
                int patch = gotoAddr();
                ++j;
                assembleStatements(*j); // statements
                gotoAddr(loopTop);
                _patchGoto(patch);
                repeatEnd();
            }
            break;
            
        case kTokenTick:
            ASSEMBLER_TRACE(kTokenTick);
            {
                ASTConstIter j = root->children.begin();
                // if it's true that tick has no parameter just remove this block and leave only the onTick
                if (j != root->children.end()) {
                    assembleNode(*j); // qualifier/parameters
                }
                onTick();
            }
            break;
            
        case kTokenMessage:
            ASSEMBLER_TRACE(kTokenMessage);
            {
                ASTConstIter j = root->children.begin();
                assembleNode(*j); // qualifier/parameters
                onMessage();
            }
            break;
            
        case kTokenLibEvent:
            ASSEMBLER_TRACE(kTokenLibEvent);
            {
                ASTConstIter j = root->children.begin();
                if (j != root->children.end())
                    assembleNode(*j); // qualifier/parameters
                onLibEvent(root->str2.c_str());
            }
            break;
            
        case kTokenFor:
            ASSEMBLER_TRACE(kTokenFor);
            {
                ASTConstIter j = root->children.begin();
                assembleNode(*j); // the function that returns an array of varobjs
                ++j;
                
                forEach();
                int patch = gotoAddr();
                
                // for each will call Run as if executing substates
                pushLocalParameters();
                
                // in the case of for body in bodies, type will be varobj and name will be body
                const char* type = root->str1.c_str(); // type
                const char* name = root->str2.c_str(); // name
                addLocalParam(name, type);
                
                assembleStatements(*j); // statements
                
                popLocalParameters();
                subStateEnd();
                
                _patchGoto(patch);
            }
            break;
            
        case kTokenOn:
            ASSEMBLER_TRACE(kTokenOn);
            {
                ASTConstIter j = root->children.begin();
                assembleNode(*j); // qualifier/parameters
                int patch = gotoAddr();
                ++j;

                pushLocalParameters();
                assembleStatements(*j); // statements
                popLocalParameters();
                
                subStateEnd();
                _patchGoto(patch);
            }
            break;
            
        case kTokenOpAdd:
            ASSEMBLER_TRACE(kTokenOpAdd);
            opAdd();
            break;
        case kTokenOpSubtract:
            ASSEMBLER_TRACE(kTokenOpSubtract);
            opSubtract();
            break;
        case kTokenOpMultiply:
            ASSEMBLER_TRACE(kTokenOpMultiply);
            opMultiply();
            break;
        case kTokenOpDivide:
            ASSEMBLER_TRACE(kTokenOpDivide);
            opDivide();
            break;
        case kTokenOpNegate:
            ASSEMBLER_TRACE(kTokenOpNegate);
            opNegate();
            break;
        case kTokenOpModulus:
            ASSEMBLER_TRACE(kTokenOpModulus);
            opModulus();
            break;
            
        default:
            lcRaiseError("Compile Error: unknown node encountered\n", 0, 0);
            break;
    }
}

void AssemblerBase::assembleStatements(ASTNode* root)
{
    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i)
        assembleNode(*i);
}

void AssemblerBase::assembleState(ASTNode* root)
{
    _addState(root->str2.c_str());   // this will update the program index
    stringIndex(root->str2.c_str()); 
    
    if (root->token != kTokenState) {
        lcRaiseError("Compile Error: node is not a state\n", 0, 0);
        return;
    }
    bool once = true;

    pushLocalParameters();
    
    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
        if (once == false) {
            popLocalParameters();
            lcRaiseError("Compile Error: should only be one statements node under a state node\n", 0, 0);
            return;
        }
        once = false;
        
        assembleStatements(*i);
    }
    popLocalParameters();
    stateEnd();
}

void AssemblerBase::assembleDeclarations(ASTNode* root)
{
    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i)
        _addVariable((*i)->str2.c_str(), (*i)->str1.c_str(), (*i)->token == kTokenSharedVariable);
}

void AssemblerBase::assembleMachine(ASTNode* root)
{
    if (root->token != kTokenMachine) {
        lcRaiseError("Compile Error: node is not a machine\n", 0, 0);
        return;
    }
    
    std::vector<std::string> states;
    ASTConstIter mainStateIter = root->children.end();
    
    stringIndex(root->str2.c_str());
    
    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
        if ((*i)->token == kTokenDeclare) {
            assembleDeclarations(*i);
        }
        else if ((*i)->token != kTokenState) {
            lcRaiseError("Compiler Error: node is not a state\n", 0, 0);
            return;
        }
        else {
            _addState((*i)->str2.c_str());
            if ((*i)->str2 == "main") {
                mainStateIter = i;
            }
            states.push_back((*i)->str2);
        }
    }
    
    if (mainStateIter == root->children.end()) {
        lcRaiseError("Compiler Error: main state not found\n", 0, 0);
        return;
    }
    
    // compile main state at address zero, then all other states
    assembleState(*mainStateIter);
    for (ASTConstIter i = root->children.begin(); i != root->children.end(); ++i) {
        if (i == mainStateIter)
            continue;
        else if ((*i)->token == kTokenDeclare)
            continue;
        assembleState(*i);
    }
}

void AssemblerBase::assemble(ASTNode* root)
{    
    if (root->token != kTokenMachine)
        lcRaiseError("Compile Error: Root node is not a Landru machine\n", 0, 0);
    else {
        reset();  // reset the assembler so that it is ready to compile something new
        assembleNode(root);
        if (true) {
            FILE* f = fopen("/Users/dp/lasm.txt", "at");
            disassemble(root->str2, f);
            fflush(f);
            fclose(f);
        }
        finalize(); // the assembler needs to record its results in finalize  
                    // (by pushing back the generated code into a list for example)
    }
}
    
} // Landru

