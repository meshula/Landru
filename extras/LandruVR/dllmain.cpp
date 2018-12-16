
#include "api.h"

#ifdef ARCH_OS_WINDOWS

#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx

BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);

    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            // dll is being loaded.
            // load instance data, or use TlsAlloc here
            break;

        case DLL_PROCESS_DETACH:
            // dll is being unloaded.
            // call TlsFree here
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

#endif
