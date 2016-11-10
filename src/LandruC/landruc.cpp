
#include "OptionParser.h"

#include "LandruCompiler/LandruCompiler.h"
#include "LandruAssembler/LandruAssembler.h"
#include "LandruAssembler/LandruActorAssembler.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include "LandruActorVM/StdLib/StdLib.h"
//#include "LandruLabSound.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

int main(int argc, char** argv) 
{
    OptionParser op("landruc");
    bool json = false;
    op.AddTrueOption("j", "json", json, "Output AST as Json");
    std::string path;
    op.AddStringOption("f", "file", path, "Compile this file");
    bool printAst = false;
    op.AddTrueOption("p", "print", printAst, "Print AST to console");
    bool run = false;
    op.AddTrueOption("r", "run", run, "Run on succesful compilation");
    bool verbose = false;
    op.AddTrueOption("v", "verbose", verbose, "Verbose output");
    int breakPoint = ~0;
    op.AddIntOption("b", "breakpoint", breakPoint, "Breakpoint");
    if (op.Parse(argc, argv)) {
        if (path.length() == 0) {
            op.Usage();
            exit(1);
        }
        std::cout << "Compiling " << path << std::endl;
        
        FILE* f = fopen(path.c_str(), "rb");
		if (!f) {
			std::cout << path << " not found" << std::endl;
			exit(1);
		}
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
            Landru::Std::populateLibrary(laa.library());
            //Landru::Audio::LabSoundLib::registerLib(laa.library());
            bool success = true;
            try {
                laa.assemble((Landru::ASTNode*) rootNode);
            }
            catch (const std::exception& exc) {
                std::cout << "Compilation error occured: " << exc.what() << std::endl;
                success = false;
            }

            delete [] text;
            
            if (json)
                landruToJson(rootNode);
            if (printAst)
                landruPrintRawAST(rootNode);
            if (run && success) {
                Landru::VMContext vmContext;
                vmContext.activateMeta = verbose;
                vmContext.breakPoint = breakPoint;
                Landru::Std::populateLibrary(*vmContext.libs.get());
                //Landru::Audio::LabSoundLib::registerLib(*vmContext.libs.get());
                //vmContext.bsonGlobals = laa.assembledGlobalVariables(); @TODO need to wrap Bson with WiredData
                vmContext.machineDefinitions = laa.assembledMachineDefinitions();
                vmContext.instantiateLibs();
                vmContext.launchQueue.push_back(Landru::VMContext::LaunchRecord("main", Landru::Fiber::Stack()));
                do {
                    chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
                    chrono::duration<double> time_span = chrono::duration_cast<chrono::seconds>(now.time_since_epoch());
                    vmContext.update(time_span.count());
                    if (vmContext.undeferredMessagesPending()) {
                        continue;
                    }
					else if (!vmContext.launchQueue.empty()) {
						continue;
					}
					else if (vmContext.deferredMessagesPending()) {
                        std::this_thread::sleep_for(std::chrono::microseconds(2000));
                    }
                    else
                        break;
                } while (true);
            }
        }
    }
    return 0;
}
