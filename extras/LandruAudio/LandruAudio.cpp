
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"

#include "labsoundc.h"

#define LANDRUAUDIO_EXPORTS
#include "api.h"

using namespace std;
using namespace Landru;

namespace {

	RunState audioBuffer_Create(FnContext& run)
	{
		float sampleRate = run.self->pop<float>();
		int numberOfFrames = run.self->pop<int>();
		int numberOfChannels = run.self->pop<int>();
		LS_Handle h = ls_AudioBuffer_Create(numberOfChannels, numberOfFrames, sampleRate);
		run.self->push<LS_Handle>(h);
		return RunState::Continue;
	}

	RunState audioBuffer_Length(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<int>((int)ls_AudioBuffer_Length(h));
		return RunState::Continue;
	}

	RunState audioBuffer_Duration(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<float>((float)ls_AudioBuffer_Duration(h));
		return RunState::Continue;
	}

	RunState audioBuffer_SampleRate(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<float>(ls_AudioBuffer_SampleRate(h));
		return RunState::Continue;
	}

	RunState audioBuffer_NumberOfChannels(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<int>(ls_AudioBuffer_NumberOfChannels(h));
		return RunState::Continue;
	}

	//LS_API float*    ls_AudioBuffer_GetChannelData(LS_Handle, unsigned channelIndex);

	RunState audioBuffer_Zero(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_AudioBuffer_Zero(h);
		return RunState::Continue;
	}

	RunState audioBuffer_GetGain(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<float>((float) ls_AudioBuffer_GetGain(h));
		return RunState::Continue;
	}

	RunState audioBuffer_SetGain(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_AudioBuffer_SetGain(h, run.self->pop<float>());
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_Create(FnContext& run)
	{
		ls_AudioBufferSourceNode_Create(run.self->pop<float>());
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_SetBuffer(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle b = run.self->pop<LS_Handle>();
		ls_AudioBufferSourceNode_SetBuffer(h, b);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_GetBuffer(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle b = ls_AudioBufferSourceNode_GetBuffer(h);
		run.self->push<LS_Handle>(b);
		return RunState::Continue;
	}

	// zero duration means play to end
	RunState audioBufferSourceNode_StartGrain(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float duration = run.self->pop<float>();
		float offset = run.self->pop<float>();
		float when = run.self->pop<float>();
		ls_AudioBufferSourceNode_StartGrain(h, when, offset, duration);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_SetLooping(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle b = run.self->pop<int>();
		ls_AudioBufferSourceNode_SetLooping(h, !!b);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_GetLooping(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		bool b = ls_AudioBufferSourceNode_GetLooping(h);
		run.self->push<int>(b);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_SetLoop(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float end = run.self->pop<float>();
		float start = run.self->pop<float>();
		ls_AudioBufferSourceNode_SetLoop(h, start, end);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_GetLoop(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		double start, end;
		ls_AudioBufferSourceNode_GetLoop(h, &start, &end);
		run.self->push<float>((float) end);
		run.self->push<float>((float) start);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_GainParam(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle param = ls_AudioBufferSourceNode_GainParam(h);
		run.self->push<LS_Handle>(param);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_PlaybackRateParam(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle param = ls_AudioBufferSourceNode_PlaybackRateParam(h);
		run.self->push<LS_Handle>(param);
		return RunState::Continue;
	}

	RunState audioBufferSourceNode_SetPannerNode(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle n = run.self->pop<LS_Handle>();
		ls_AudioBufferSourceNode_SetPannerNode(h, n);
		return RunState::Continue;
	}


	RunState contextSampleRate(FnContext& run)
	{
		LS_Handle ac = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<float>(ls_AudioContext_SampleRate(ac));
		return RunState::Continue;
	}

	RunState audioContext_DestinationNode(FnContext& run)
	{
		LS_Handle ac = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<LS_Handle>(ls_AudioContext_DestinationNode(ac));
		return RunState::Continue;
	}

	RunState audioContext_CurrentTime(FnContext& run)
	{
		LS_Handle ac = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<float>((float) ls_AudioContext_CurrentTime(ac));
		return RunState::Continue;
	}

	RunState audioContext_Listener(FnContext& run)
	{
		LS_Handle ac = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		run.self->push<LS_Handle>(ls_AudioContext_Listener(ac));
		return RunState::Continue;
	}

	RunState bufferLoad(FnContext& run)
	{
		float sampleRate = run.self->pop<float>();
		string path = run.self->pop<string>();
		LS_Handle ac = run.self->pop<LS_Handle>();
		run.self->push<LS_Handle>(ls_Buffer_Load(ac, path.c_str(), sampleRate));
		return RunState::Continue;
	}

	RunState bufferPlay(FnContext& run)
	{
		LS_Handle buffer = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle node = ls_Buffer_Play(ls_Buffer_Context(buffer), 0, buffer, 0, 0, 0);
		run.self->push<LS_Handle>(node);
		return RunState::Continue;
	}

	RunState bufferCreateSourceNode(FnContext& run)
	{
		LS_Handle buffer = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float sampleRate = run.self->pop<float>();
		LS_Handle node = ls_Buffer_CreateSourceNode(buffer, sampleRate);
		run.self->push<LS_Handle>(node);
		return RunState::Continue;
	}

	RunState bufferAudioBuffer(FnContext& run)
	{
		LS_Handle buffer = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		LS_Handle audioBuffer = ls_Buffer_AudioBuffer(buffer);
		run.self->push<LS_Handle>(audioBuffer);
		return RunState::Continue;
	}

	RunState listener_SetPosition(FnContext& run)
	{
		float z = run.self->pop<float>();
		float y = run.self->pop<float>();
		float x = run.self->pop<float>();
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_Listener_SetPosition(h, x, y, z);
		return RunState::Continue;
	}

	RunState listener_GetPosition(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float x, y, z;
		ls_Listener_GetPosition(h, &x, &y, &z);
		run.self->push<float>(z);
		run.self->push<float>(y);
		run.self->push<float>(x);
		return RunState::Continue;
	}

	RunState listener_SetFacing(FnContext& run)
	{
		float z = run.self->pop<float>();
		float y = run.self->pop<float>();
		float x = run.self->pop<float>();
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_Listener_SetFacing(h, x, y, z);
		return RunState::Continue;
	}

	RunState listener_GetFacing(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float x, y, z;
		ls_Listener_GetFacing(h, &x, &y, &z);
		run.self->push<float>(z);
		run.self->push<float>(y);
		run.self->push<float>(x);
		return RunState::Continue;
	}

	RunState listener_SetUp(FnContext& run)
	{
		float z = run.self->pop<float>();
		float y = run.self->pop<float>();
		float x = run.self->pop<float>();
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_Listener_SetUp(h, x, y, z);
		return RunState::Continue;
	}

	RunState listener_GetUp(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float x, y, z;
		ls_Listener_GetUp(h, &x, &y, &z);
		run.self->push<float>(z);
		run.self->push<float>(y);
		run.self->push<float>(x);
		return RunState::Continue;
	}

	RunState listener_SetVelocity(FnContext& run)
	{
		float z = run.self->pop<float>();
		float y = run.self->pop<float>();
		float x = run.self->pop<float>();
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_Listener_SetVelocity(h, x, y, z);
		return RunState::Continue;
	}

	RunState listener_GetVelocity(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float x, y, z;
		ls_Listener_GetVelocity(h, &x, &y, &z);
		run.self->push<float>(z);
		run.self->push<float>(y);
		run.self->push<float>(x);
		return RunState::Continue;
	}

	RunState listener_SetDopplerFactor(FnContext& run)
	{
		float x = run.self->pop<float>();
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_Listener_SetDopplerFactor(h, x);
		return RunState::Continue;
	}

	RunState listener_GetDopplerFactor(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float x = ls_Listener_GetDopplerFactor(h);
		run.self->push<float>(x);
		return RunState::Continue;
	}

	RunState listener_SetSpeedOfSound(FnContext& run)
	{
		float x = run.self->pop<float>();
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		ls_Listener_SetSpeedOfSound(h, x);
		return RunState::Continue;
	}

	RunState listener_GetSpeedOfSound(FnContext& run)
	{
		LS_Handle h = (dynamic_cast<Wires::Data<LS_Handle>*>(run.var))->value();
		float x = ls_Listener_GetSpeedOfSound(h);
		run.self->push<float>(x);
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

	audio_lib.registerFactory("context", []()->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>(ls_AudioContext_Create());
	});
	{
		Landru::Library context_lib("context");
		auto context_vt = unique_ptr<Library::Vtable>(new Library::Vtable("context"));
		context_vt->registerFn("1.0", "sampleRate", "o", "f", contextSampleRate);
		context_vt->registerFn("1.0", "destinationNode", "o", "f", audioContext_DestinationNode);
		context_vt->registerFn("1.0", "currentTime", "o", "f", audioContext_CurrentTime);
		context_vt->registerFn("1.0", "listener", "o", "f", audioContext_Listener);

		context_lib.registerVtable(move(context_vt));
		audio_lib.libraries.emplace_back(std::move(context_lib));
	}

	audio_lib.registerFactory("audioBuffer", []()->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>();
	});
	{
		Landru::Library buffer_lib("audioBuffer");
		auto buffer_vt = unique_ptr<Library::Vtable>(new Library::Vtable("audioBuffer"));

		buffer_vt->registerFn("1.0", "create", "o", "o", audioBuffer_Create);
		buffer_vt->registerFn("1.0", "length", "", "i", audioBuffer_Length);
		buffer_vt->registerFn("1.0", "duration", "", "f", audioBuffer_Duration);
		buffer_vt->registerFn("1.0", "sampleRate", "", "f", audioBuffer_SampleRate);
		buffer_vt->registerFn("1.0", "numberOfChannels", "", "i", audioBuffer_NumberOfChannels);
		buffer_vt->registerFn("1.0", "zero", "", "", audioBuffer_Zero);
		buffer_vt->registerFn("1.0", "zero", "f", "", audioBuffer_SetGain);
		buffer_vt->registerFn("1.0", "zero", "", "f", audioBuffer_GetGain);

		buffer_lib.registerVtable(move(buffer_vt));
		audio_lib.libraries.emplace_back(std::move(buffer_lib));
	}

	audio_lib.registerFactory("audioBufferSourceNode", []()->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>();
	});
	{
		Landru::Library buffer_lib("audioBufferSourceNode");
		auto buffer_vt = unique_ptr<Library::Vtable>(new Library::Vtable("audioBufferSourceNode"));

		buffer_vt->registerFn("1.0", "create", "f", "o", audioBufferSourceNode_Create);
		buffer_vt->registerFn("1.0", "setBuffer", "o", "", audioBufferSourceNode_SetBuffer);
		buffer_vt->registerFn("1.0", "getBuffer", "o", "o", audioBufferSourceNode_GetBuffer);
		buffer_vt->registerFn("1.0", "getBuffer", "offf", "", audioBufferSourceNode_StartGrain);
		buffer_vt->registerFn("1.0", "getBuffer", "oi", "", audioBufferSourceNode_SetLooping);
		buffer_vt->registerFn("1.0", "getBuffer", "o", "i", audioBufferSourceNode_GetLooping);
		buffer_vt->registerFn("1.0", "getBuffer", "off", "", audioBufferSourceNode_SetLoop);
		buffer_vt->registerFn("1.0", "getBuffer", "o", "ff", audioBufferSourceNode_GetLoop);
		buffer_vt->registerFn("1.0", "getBuffer", "o", "o", audioBufferSourceNode_GainParam);
		buffer_vt->registerFn("1.0", "getBuffer", "o", "o", audioBufferSourceNode_PlaybackRateParam);
		buffer_vt->registerFn("1.0", "getBuffer", "oo", "", audioBufferSourceNode_SetPannerNode);

		buffer_lib.registerVtable(move(buffer_vt));
		audio_lib.libraries.emplace_back(std::move(buffer_lib));
	}

	audio_lib.registerFactory("buffer", []()->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>();
	});
	{
		Landru::Library buffer_lib("buffer");
		auto buffer_vt = unique_ptr<Library::Vtable>(new Library::Vtable("buffer"));
		buffer_vt->registerFn("1.0", "load", "osf", "o", bufferLoad);
		buffer_vt->registerFn("1.0", "play", "o", "o", bufferPlay);
		buffer_vt->registerFn("1.0", "createSourceNode", "f", "o", bufferCreateSourceNode);
		buffer_vt->registerFn("1.0", "audioBuffer", "o", "o", bufferAudioBuffer);
		buffer_lib.registerVtable(move(buffer_vt));
		audio_lib.libraries.emplace_back(std::move(buffer_lib));
	}

	audio_lib.registerFactory("convolver", []()->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>();
	});

	audio_lib.registerFactory("listener", []()->std::shared_ptr<Wires::TypedData>
	{
		return std::make_shared<Wires::Data<LS_Handle>>();
	});
	{
		Landru::Library buffer_lib("listener");
		auto buffer_vt = unique_ptr<Library::Vtable>(new Library::Vtable("listener"));

		buffer_vt->registerFn("1.0", "setPosition", "offf", "", listener_SetPosition);
		buffer_vt->registerFn("1.0", "getPosition", "o", "fff", listener_GetPosition);
		buffer_vt->registerFn("1.0", "setFacing", "offf", "", listener_SetFacing);
		buffer_vt->registerFn("1.0", "getFacing", "o", "fff", listener_GetFacing);
		buffer_vt->registerFn("1.0", "setUp", "offf", "", listener_SetUp);
		buffer_vt->registerFn("1.0", "getUp", "o", "fff", listener_GetUp);
		buffer_vt->registerFn("1.0", "setVelocity", "offf", "", listener_SetVelocity);
		buffer_vt->registerFn("1.0", "getVelocity", "o", "fff", listener_GetVelocity);
		buffer_vt->registerFn("1.0", "setDopplerFactor", "offf", "", listener_SetDopplerFactor);
		buffer_vt->registerFn("1.0", "getDopplerFactor", "o", "fff", listener_GetDopplerFactor);
		buffer_vt->registerFn("1.0", "setSpeedOfSound", "offf", "", listener_SetSpeedOfSound);
		buffer_vt->registerFn("1.0", "getSpeedOfSound", "o", "fff", listener_GetSpeedOfSound);
		buffer_lib.registerVtable(move(buffer_vt));
		audio_lib.libraries.emplace_back(std::move(buffer_lib));
	}

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

