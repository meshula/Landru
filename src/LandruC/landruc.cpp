
#include "OptionParser.h"

#include "LandruCompiler/LandruCompiler.h"
#include "LandruAssembler/LandruAssembler.h"
#include "LandruAssembler/LandruActorAssembler.h"
#include "LandruVM/Engine.h"
#include "LandruStd/IoVarObj.h"

#include <iostream>
#include <vector>
#include <string>

int main(int argc, char** argv)
{
    
    OptionParser op("landruc");
    bool json;
    op.AddTrueOption("j", "json", json, "Output AST as Json");
    std::string path;
    op.AddStringOption("f", "file", path, "Compile this file");
    if (op.Parse(argc, argv)) {
        if (path.length() == 0) {
            op.Usage();
            exit(1);
        }
        std::cout << "Compiling " << path << std::endl;
        
        FILE* f = fopen(path.c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            size_t len = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* text = new char[len+1];
            fread(text, 1, len, f);
            text[len] = '\0';
            fclose(f);
        
            void* rootNode = landruCreateRootNode();
            std::vector<std::pair<std::string, Json::Value*> > jsonVars;
            landruParseProgram(rootNode, &jsonVars, text, len);
            
            Landru::ActorAssembler laa;
            laa.assemble((Landru::ASTNode*) rootNode);
            
            if (json)
                landruToJson(rootNode);
            else
                landruPrintRawAST(rootNode);
            
            delete [] text;
        }
    }
    return 0;
}
