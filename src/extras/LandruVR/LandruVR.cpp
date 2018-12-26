#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include <mutex>
#include "vr_hmd.hpp"
#include <GLFW/glfw3.h>

#define LANDRUVR_EXPORTS
#include "api.h"

using namespace std;
using namespace Landru;

namespace {

size_t hmd_h = 0;
std::shared_ptr<OpenVR_HMD> hmd()
{
    static std::shared_ptr<OpenVR_HMD> ptr;
    static std::once_flag once;
    std::call_once(once, []()
    {
        try
        {
            ptr = make_shared<OpenVR_HMD>();
            hmd_h = std::hash<OpenVR_HMD*>{}(ptr.get());
        }
		catch (const std::exception & e)
		{
			std::cout << "OpenVR Exception: " << e.what() << std::endl;
		}
	});
    return ptr;
}

RunState get_hmd(FnContext& run)
{
    hmd().get();
    run.self->push<size_t>(hmd_h);
    return RunState::Continue;
}

RunState hmd_pose(FnContext& run)
{
    avl::Pose p = hmd()->get_hmd_pose();
    run.self->push<Pose>(p);
    return RunState::Continue;
}

RunState pose_print(FnContext& run)
{
    Pose p = (dynamic_cast<Wires::Data<Pose>*>(run.var))->value();
    float3 pos = p.position;
    printf("%f %f %f\n", pos.x, pos.y, pos.z);
    return RunState::Continue;
}

} // anon

extern "C"
LANDRUVR_API
void landru_vr_init(void* vl)
{
    Landru::Library * root_lib = reinterpret_cast<Landru::Library*>(vl);
    Landru::Library lib("vr");

    auto vtable = unique_ptr<Library::Vtable>(new Library::Vtable("vr"));
	lib.registerVtable(move(vtable));

	lib.registerFactory("hmd", []()->std::shared_ptr<Wires::Data<size_t>>
	{
        hmd();
		return std::make_shared<Wires::Data<size_t>>(hmd_h);
	});
	{
		Landru::Library sub_lib("hmd");
		auto sub_vt = unique_ptr<Library::Vtable>(new Library::Vtable("hmd"));
		sub_vt->registerFn("1.0", "pose", "", "o", hmd_pose);
		sub_lib.registerVtable(move(sub_vt));
		lib.libraries.emplace_back(std::move(sub_lib));
	}

    lib.registerFactory("pose", []()->std::shared_ptr<Wires::Data<avl::Pose>>
    {
        return std::make_shared<Wires::Data<avl::Pose>>();
    });
	{
		Landru::Library sub_lib("pose");
		auto sub_vt = unique_ptr<Library::Vtable>(new Library::Vtable("pose"));
		sub_vt->registerFn("1.0", "print", "", "o", pose_print);
		sub_lib.registerVtable(move(sub_vt));
		lib.libraries.emplace_back(std::move(sub_lib));
	}

    root_lib->libraries.emplace_back(std::move(lib));
}

extern "C"
LANDRUVR_API
RunState landru_vr_update(double now, VMContext* vm)
{
    hmd()->update();        /// @TODO the hmd update loop needs to be split between update and render so both can be threaded
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

