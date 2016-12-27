
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"

#include "labsoundc.h"

#define LANDRUAUDIO_EXPORTS
#include "api.h"

using namespace std;
using namespace Landru;

/*

LS_API LS_Handle ls_Convolver_Create(float sampleRate);
LS_API bool ls_Convolver_SetBuffer(LS_Handle context, LS_Handle convolver, LS_Handle buffer);
LS_API void ls_release(LS_Handle h);
LS_API LS_Handle ls_AudioContext_Create();
LS_API float ls_AudioContext_SampleRate(LS_Handle h);
LS_API LS_Handle ls_AudioContext_DestinationNode(LS_Handle audioContext);
LS_API bool ls_AudioContext_Connect(LS_Handle audioContext, LS_Handle from, LS_Handle to);
LS_API LS_Handle ls_Buffer_Load(LS_Handle context, const char * filepath, float sampleRate);
LS_API LS_Handle ls_Buffer_Play(LS_Handle context, LS_Handle node,
LS_Handle buffer, float start, float end, float when);

*/

namespace {

	RunState sampleRate(FnContext& run)
	{
		LS_Handle ac = run.self->pop<LS_Handle>();
		run.self->push<float>(ls_AudioContext_SampleRate(ac));
		return RunState::Continue;
	}

}

extern "C"
LANDRUAUDIO_API
void landru_audio_init(void* vl)
{
	Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
	
	Landru::Library audio_lib("audio");
	auto audio_vt = unique_ptr<Library::Vtable>(new Library::Vtable("audio"));
	audio_lib.registerVtable(move(audio_vt));

	audio_lib.registerFactory("context", [](VMContext&)->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>(ls_AudioContext_Create());
	});
	{
		Landru::Library context_lib("context");
		auto context_vt = unique_ptr<Library::Vtable>(new Library::Vtable("context"));
		context_vt->registerFn("1.0", "sampleRate", "o", "f", sampleRate);
		context_lib.registerVtable(move(context_vt));
		audio_lib.libraries.emplace_back(std::move(context_lib));
	}

	audio_lib.registerFactory("buffer", [](VMContext&)->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>();
	});

	audio_lib.registerFactory("convolver", [](VMContext&)->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>();
	});

	lib->libraries.emplace_back(std::move(audio_lib));
}

extern "C"
LANDRUAUDIO_API
RunState landru_audio_update(double now, VMContext* vm)
{
	return RunState::Continue;
}

extern "C"
LANDRUAUDIO_API
void landru_audio_finish(void* vl)
{
	Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
}

extern "C"
LANDRUAUDIO_API
void landru_audio_fiberExpiring(Fiber* f)
{
	//    called when Fibers are destroyed so that pending items like onWindowsClosed can be removed
}

extern "C"
LANDRUAUDIO_API
void landru_audio_clearContinuations(Fiber* f, int level)
{
}

extern "C"
LANDRUAUDIO_API
bool landru_audio_pendingContinuations(Fiber * f)
{
	return false;
}

