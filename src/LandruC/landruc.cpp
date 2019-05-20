
#include "OptionParser.h"

#include "LandruLang/LandruLexParse.h"
#include "LandruVMVI/VirtualMachine.h"

#include <LabText/LabText.h>

#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
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

#if defined(_WIN32)
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
    #if defined(_WIN32)
        return LoadLibrary(filename.c_str());
    #else
        return dlopen(filename.c_str(), flag);
    #endif
    }

    const char* ArchLibraryError()
    {
    #if defined(_WIN32)
        DWORD error = ::GetLastError();
        return error ?  ArchStrSysError(error).c_str() : nullptr;
    #else
        return dlerror();
    #endif
    }

    int ArchLibraryClose(void* handle)
    {
    #if defined(_WIN32)
        int status = ::FreeLibrary(reinterpret_cast<HMODULE>(handle));
    #else
        int status = dlclose(handle);
    #endif
        return status;
    }

    void* ArchLibraryGetSymbol(void* handle, const char* name)
    {
    #if defined(_WIN32)
        return GetProcAddress((HMODULE) handle, name);
    #else
        return dlsym(handle, name);
    #endif
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
	bool repl = false;
	op.AddTrueOption("l", "repl", repl, "Start a REPL");

	if (op.Parse(argc, argv))
	{
        if (path.length() == 0) {
            op.Usage();
            exit(1);
        }

        FILE* f = fopen(path.c_str(), "rb");
		if (!f) {
			std::cout << path << " not found" << std::endl;
			exit(1);
		}

		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);
		fseek(f, 0, SEEK_SET);
		char* text = new char[len + 1];
		fread(text, 1, len, f);
		text[len] = '\0';
		fclose(f);

		std::cout << "Compiling " << path << std::endl;

        std::shared_ptr<llp::AST> prg = landru_lex_parse(text, len);
        landru_ast_print(prg);
        std::shared_ptr<lvmvi::Exemplar::Machine> machine = landru_compile(prg);
        landru_machine_print(machine);
        return 1;
	}
	else
	{
		// no arguments supplied.
		op.Usage();
		exit(1);
	}
    return 0;
}
