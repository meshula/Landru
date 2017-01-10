#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include <mutex>
#include <GLFW/glfw3.h>

#define LANDRUVR_EXPORTS
#include "api.h"

using namespace std;
using namespace Landru;

namespace {


extern "C"
LANDRUVR_API
void landru_vr_init(void* vl)
{
    Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
    Landru::Library gl_lib("vr");

    auto vtable = unique_ptr<Library::Vtable>(new Library::Vtable("vr"));
	gl_lib.registerVtable(move(vtable));

    lib->libraries.emplace_back(std::move(gl_lib));
}

extern "C"
LANDRUVR_API
RunState landru_vr_update(double now, VMContext* vm)
{
	return RunState::Continue;
}

extern "C"
LANDRUVR_API
void landru_vr_finish(void* vl)
{
    Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
}

extern "C"
LANDRUVR_API
void landru_vr_fiberExpiring(Fiber* f)
{
//    called when Fibers are destroyed so that pending items like onWindowsClosed can be removed
}

extern "C"
LANDRUVR_API
void landru_vr_clearContinuations(Fiber* f, int level)
{
	// called when continuations must be cleared, for example before executing a goto statement
}

extern "C"
LANDRUVR_API
bool landru_vr_pendingContinuations(Fiber * f)
{
	return false;
}

} // anon
