
#include "Landru/defines.h"
#include "OptionParser.h"

#include "LandruCompiler/LandruCompiler.h"
#include "LandruCompiler/lcRaiseError.h"
#include "LandruAssembler/LandruAssembler.h"
#include "LandruAssembler/LandruActorAssembler.h"
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include "LandruActorVM/StdLib/StdLib.h"
//#include "LandruLabSound.h"

#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

#if defined(ARCH_OS_WINDOWS)
 #include <Windows.h>
#else
 #include <dlfcn.h>
#endif

using namespace std;

namespace {
    #if defined(ARCH_OS_LINUX) || defined(ARCH_OS_DARWIN)
    #   define ARCH_LIBRARY_LAZY    RTLD_LAZY
    #   define ARCH_LIBRARY_NOW     RTLD_NOW
    #   define ARCH_LIBRARY_LOCAL   RTLD_LOCAL
    #   define ARCH_LIBRARY_GLOBAL  RTLD_GLOBAL
    #else
    #   define ARCH_LIBRARY_LAZY    0
    #   define ARCH_LIBRARY_NOW     0
    #   define ARCH_LIBRARY_LOCAL   0
    #   define ARCH_LIBRARY_GLOBAL  0
    #endif

#if defined(ARCH_OS_WINDOWS)
	std::string ArchStrSysError(unsigned long errorCode)
	{
		if (errorCode == 0)
			return std::string();

		LPSTR buffer = nullptr;
		size_t len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&buffer,
			0,
			nullptr);

		std::string message(buffer, len);

		OutputDebugString(buffer);
		LocalFree(buffer);

		return message;
		return std::string();
	}
#endif


    void* ArchLibraryOpen(const std::string &filename, int flag)
    {
    #if defined(ARCH_OS_WINDOWS)
        return LoadLibrary(filename.c_str());
    #else
        return dlopen(filename.c_str(), flag);
    #endif
    }

    const char* ArchLibraryError()
    {
    #if defined(ARCH_OS_WINDOWS)
        DWORD error = ::GetLastError();
        return error ?  ArchStrSysError(error).c_str() : nullptr;
    #else
        return dlerror();
    #endif
    }

    int ArchLibraryClose(void* handle)
    {
    #if defined(ARCH_OS_WINDOWS)
        int status = ::FreeLibrary(reinterpret_cast<HMODULE>(handle));
    #else
        int status = dlclose(handle);
    #endif
        return status;
    }

    void* ArchLibraryGetSymbol(void* handle, const char* name)
    {
    #if defined(ARCH_OS_WINDOWS)
        return GetProcAddress((HMODULE) handle, name);
    #else
        return dlsym(handle, name);
    #endif
    }
}

void loadLibrary(Landru::Library & lib, const std::string & s, Landru::VMContext* vm)
{
	using Landru::LandruRequire;
    static set<string> loadedLibraries;
    if (loadedLibraries.find(s) == loadedLibraries.end()) {
        if (s.length()) {
			LandruRequire req;
			req.name = "landru_" + s;
            req.plugin = ArchLibraryOpen(req.name + ".dll", ARCH_LIBRARY_LAZY);
            if (!req.plugin) {
                // raise error
            }
            else {
                req.init = (LandruRequire::InitFn) ArchLibraryGetSymbol(req.plugin, (req.name + "_init").c_str());
                req.update = (LandruRequire::UpdateFn) ArchLibraryGetSymbol(req.plugin, (req.name + "_update").c_str());
                req.finish = (LandruRequire::FinishFn) ArchLibraryGetSymbol(req.plugin, (req.name + "_finish").c_str());
                req.fiberExpiring = (LandruRequire::FiberExpiringFn) ArchLibraryGetSymbol(req.plugin, (req.name + "_fiberExpiring").c_str());
				req.clearContinuations = (LandruRequire::ClearContinuationsFn) ArchLibraryGetSymbol(req.plugin, (req.name + "_clearContinuations").c_str());
				req.pendingContinuations = (LandruRequire::PendingContinuationsFn) ArchLibraryGetSymbol(req.plugin, (req.name + "_pendingContinuations").c_str());
				vm->plugins.push_back(req);
            }
        }
    }
}

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

			bool success = !lcCurrentError();

			Landru::Library library("landru");
            Landru::ActorAssembler laa(&library);
            Landru::Std::populateLibrary(library);
			//Landru::Audio::LabSoundLib::registerLib(library());

			Landru::VMContext vmContext(&library);

			try {

				vector<string> requires = laa.requires2((Landru::ASTNode*) rootNode);
				for (auto r : requires) {
					loadLibrary(library, r, &vmContext);
				}

				for (auto & r : vmContext.plugins) {
					if (r.init)
						r.init(&library);
				}

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

            if (run && success)
			{
                vmContext.traceEnabled = verbose;
                vmContext.breakPoint = breakPoint;

                //Landru::Audio::LabSoundLib::registerLib(*vmContext.libs.get());
                //vmContext.bsonGlobals = laa.assembledGlobalBsonVariables();
                vmContext.machineDefinitions = laa.assembledMachineDefinitions();

				for (auto i : laa.assembledGlobalVariables()) {
					vmContext.storeGlobal(i.first, i.second);
				}

                vmContext.instantiateLibs();
                vmContext.launchQueue.push_back(Landru::VMContext::LaunchRecord("main", Landru::Fiber::Stack()));

				bool run = true;
                do {
                    chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
                    chrono::duration<double> time_span = chrono::duration_cast<chrono::seconds>(now.time_since_epoch());
					try {
						vmContext.update(time_span.count());
					}
					catch (Landru::Exception & exc) {
						std::cerr << "Caught Landru exception: " << std::endl << exc.s << std::endl;
						run = false;
					}
					catch (...) {
						std::cerr << "Exception caught, exiting" << std::endl;
						run = false;
					}
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
                } while (run);
            }
        }
    }
    return 0;
}
