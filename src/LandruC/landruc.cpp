
#include "OptionParser.h"

#include "LandruCompiler/LandruCompiler.h"
#include "LandruAssembler/LandruAssembler.h"
#include "LandruVM/Engine.h"
#include "LandruStd/IoVarObj.h"

#include <iostream>
#include <vector>
#include <string>

int main(int argc, char** argv)
{
    
    OptionParser op("MidiPlayer");
    bool json;
    op.AddTrueOption("j", "json", json, "Output AST as Json");
    std::string path;
    op.AddStringOption("f", "file", path, "Compile this file");
    if (op.Parse(argc, argv)) {
        std::cout << "Compiling " << path << std::endl;
        
        FILE* f = fopen(path.c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            int len = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* text = new char[len+1];
            fread(text, 1, len, f);
            text[len] = '\0';
            fclose(f);
        
            void* rootNode = landruCreateRootNode();
            std::vector<std::pair<std::string, Json::Value*> > jsonVars;
            landruParseProgram(rootNode, &jsonVars, text, len);
            
            if (json)
                landruToJson(rootNode);
            else
                landruPrintRawAST(rootNode);
            
            delete [] text;
        }
    }
    return 0;
}
