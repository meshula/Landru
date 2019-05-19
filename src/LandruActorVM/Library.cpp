//
//  Library.cpp
//  Landru
//
//  Created by Nick Porcino on 11/26/14.
//
//

#include "Library.h"

#include "LandruAssembler/LandruAssembler.h"
#include "LandruCompiler/AST.h"
#include "LandruActorVM/VMContext.h"
#include "Landru/Landru.h"
#include <LabText/LabText.h>

namespace Landru 
{
    using lab::Text::StrView;

    std::function<std::shared_ptr<Wires::TypedData>()> Library::findFactory(const char* name) 
	{
        std::vector<StrView> nameParts = lab::Text::Split(StrView{ name, strlen(name) }, '.');
        if (nameParts.size() == 1) {
            auto f = factories.find(name);
            if (f == factories.end()) {
                VM_RAISE("Cannot construct object of type: " << std::string(name));
            }
            return f->second;
        }
        else {
            for (auto& i : libraries) {
                if (nameParts[0] == i.name.c_str()) {
                    auto f = i.factories.find(std::string(nameParts[1].curr, nameParts[1].sz));
                    if (f == i.factories.end()) {
                        VM_RAISE("Cannot construct object of type: " << std::string(name));
                    }
                    return f->second;
                }
            }
        }
        VM_RAISE("Cannot construct object of type: " << std::string(name));
    }
    
    Library::Vtable const*const Library::findVtable(const char* name) 
	{
        auto v = vtables.find(name);
        if (v != vtables.end())
            return v->second.get();
        
        for (auto& i : libraries) {
            auto v = i.vtables.find(name);
            if (v != i.vtables.end())
                return v->second.get();
        }
        VM_RAISE("Cannot find vtable for: " << std::string(name));
    }
    
} // Landru


#if defined(LANDRU_OS_WINDOWS)
 #include <Windows.h>
#else
 #include <dlfcn.h>
#endif

#include <set>

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

#if defined(LANDRU_OS_WINDOWS)
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
    #if defined(LANDRU_OS_WINDOWS)
        return LoadLibrary(filename.c_str());
    #else
        return dlopen(filename.c_str(), flag);
    #endif
    }

    const char* ArchLibraryError()
    {
    #if defined(LANDRU_OS_WINDOWS)
        DWORD error = ::GetLastError();
        return error ?  ArchStrSysError(error).c_str() : nullptr;
    #else
        return dlerror();
    #endif
    }

    int ArchLibraryClose(void* handle)
    {
    #if defined(LANDRU_OS_WINDOWS)
        int status = ::FreeLibrary(reinterpret_cast<HMODULE>(handle));
    #else
        int status = dlclose(handle);
    #endif
        return status;
    }

    void* ArchLibraryGetSymbol(void* handle, const char* name)
    {
    #if defined(LANDRU_OS_WINDOWS)
        return GetProcAddress((HMODULE) handle, name);
    #else
        return dlsym(handle, name);
    #endif
    }

void loadLibrary(Landru::Library & lib, const std::string & s, Landru::VMContext* vm)
{
	using Landru::LandruRequire;
    static std::set<string> loadedLibraries;
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

} // anon

extern "C"
LandruLibrary_t* landruCreateLibrary(char const*const name)
{
    return reinterpret_cast<LandruLibrary_t*>(new Landru::Library("landru"));
}

extern "C"
void landruReleaseLibrary(LandruLibrary_t* library_)
{
    Landru::Library* library = reinterpret_cast<Landru::Library*>(library_);
    delete library;
}

extern "C"
int landruLoadRequiredLibraries(LandruAssembler_t* assembler, LandruNode_t* root_node_, LandruLibrary_t* library_, LandruVMContext_t* vmContext_)
{
    Landru::Assembler* laa = reinterpret_cast<Landru::Assembler*>(assembler);
    Landru::ASTNode* root_node = reinterpret_cast<Landru::ASTNode*>(root_node_);
    Landru::VMContext* vmContext = reinterpret_cast<Landru::VMContext*>(vmContext_);
    Landru::Library* library = reinterpret_cast<Landru::Library*>(library_);

    std::vector<std::string> requires = laa->requires2(root_node);
    for (auto r : requires)
        loadLibrary(*library, r, vmContext);

    for (auto & r : vmContext->plugins)
        if (r.init)
            r.init(library);

    return static_cast<int>(requires.size());
}
