
#include "LandruActorVM/Fiber.h"
#include "LandruActorVM/Library.h"
#include "LandruActorVM/VMContext.h"
#include <mutex>
#include <unordered_map>

#include <pxr/base/arch/env.h>
//#include <pxr/base/work/work.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/treeIterator.h>
#include <pxr/usdImaging/usdImagingGL/gl.h>

#define LANDRUHYDRA_EXPORTS
#include "api.h"

using namespace std;
using namespace Landru;

namespace {

	unordered_map<size_t, UsdStageRefPtr> _stages;
	unordered_map<size_t, shared_ptr<UsdImagingGL>> _renderers;

    UsdStageRefPtr _openStage(const std::string& path)
    {
        FILE * f = fopen(path.c_str(), "r");
        if (!f)
            return nullptr;
        fclose(f);

        ArResolver& ar = ArGetResolver();
        ar.ConfigureResolverForAsset(path);
        ArResolverContext& arc = ar.CreateDefaultContextForAsset(path);

        UsdStageRefPtr stage = UsdStage::Open(path, UsdStage::LoadAll);

        // per bug described in mainWindow.py
        // as described in bug #99309, UsdStage change processing is expensive
        // so set up things manually
        SdfLayerHandle session = stage->GetSessionLayer();

		std::vector<SdfPath> paths;
        UsdTreeIterator iter = stage->Traverse(UsdPrimIsGroup);
		WorkParallelForEach(
			iter, iter.GetEnd(),
			[&paths](UsdPrim const &prim) {
			paths.push_back(prim.GetPath());
		});

		// defer change notifications until block exits scope
		SdfChangeBlock block;

		for (auto p : paths) {
			auto parent = session->GetPrimAtPath(p.GetParentPath());
			SdfPrimSpec::New(parent, p.GetName(), SdfSpecifierOver);
		}

		return stage;
    }

	RunState open_stage(FnContext& run)
    {
		Wires::Data<size_t>* stage_var = dynamic_cast<Wires::Data<size_t>*>(run.var);
		string path = run.self->pop<string>();
        UsdStageRefPtr stage = _openStage(path);
		auto s = TfTypeFunctions<UsdStageRefPtr>::GetRawPtr(stage);

		if (!s || !s->GetPseudoRoot()) {
			std::cerr << "stage is empty: " << path << endl;
			return RunState::Continue;
		}

		size_t h = std::hash<UsdStage*>{}(s);
		_stages[h] = stage;
		stage_var->setValue(h);
		return RunState::Continue;
	}

	RunState close_stage(FnContext& run)
	{
		Wires::Data<size_t>* stage_var = dynamic_cast<Wires::Data<size_t>*>(run.var);
		size_t h = stage_var->value();
		auto i = _stages.find(h);
		if (i != _stages.end()) {
			if (i->second) {
				i->second->Close();
			}
			_stages.erase(i);
		}
		stage_var->setValue(0);
		return RunState::Continue;
	}

	RunState renderer_create(FnContext& run)
	{
		Wires::Data<size_t>* render_var = dynamic_cast<Wires::Data<size_t>*>(run.var);
		shared_ptr<UsdImagingGL> renderer = make_shared<UsdImagingGL>();
		size_t h = hash<UsdImagingGL*>{}(renderer.get());
		_renderers[h] = renderer;
		render_var->setValue(h);
		return RunState::Continue;
	}

	RunState renderer_render(FnContext& run)
	{
		size_t render_h = (dynamic_cast<Wires::Data<size_t>*>(run.var))->value();
		auto render_i = _renderers.find(render_h);
		if (render_i == _renderers.find(render_h))
			return RunState::Continue;

		if (!render_i->second)
			return RunState::Continue;

		size_t stage_h = run.self->pop<size_t>();
		auto stage_i = _stages.find(stage_h);
		if (stage_i == _stages.end())
			return RunState::Continue;

		if (!stage_i->second)
			return RunState::Continue;

		UsdImagingGLEngine::RenderParams params;
		params.frame = 0;
		params.complexity = 1.f;
		params.drawMode = UsdImagingGLEngine::DRAW_SHADED_SMOOTH;
		params.showGuides = false;
		params.showRenderGuides = false;
		params.forceRefresh = false;
		params.cullStyle = UsdImagingGLEngine::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED;
		params.gammaCorrectColors = true; // hydra doesn't implement yet
		params.enableIdRender = true;
		params.enableSampleAlphaToCoverage = false;
		params.highlight = false;
		params.enableHardwareShading = true;

		auto pseudRoot = stage_i->second->GetPseudoRoot();
		shared_ptr<UsdImagingGL> renderer;
		renderer->SetSelectionColor({ 0.8f, 0.3f, 0.0f, 1.0f });
		renderer->Render(pseudRoot, params);
		return RunState::Continue;
	}

} // anon



extern "C"
LANDRUHYDRA_API
void landru_hydra_init(void* vl)
{
    const int maxThreads = 0; // let's start with max threads
    WorkSetConcurrencyLimitArgument(0);

    const bool simpleRenderer = false;
    if (simpleRenderer)
        ArchSetEnv("HD_ENABLED", "0", false);

    Landru::Library * root_lib = reinterpret_cast<Landru::Library*>(vl);
    Landru::Library lib("hydra");

    auto vtable = unique_ptr<Library::Vtable>(new Library::Vtable("hydra"));
	lib.registerVtable(move(vtable));

	lib.registerFactory("stage", []()->std::shared_ptr<Wires::Data<size_t>>
	{
		return std::make_shared<Wires::Data<size_t>>(0);
	});
	{
		Landru::Library sub_lib("stage");
		auto sub_vt = unique_ptr<Library::Vtable>(new Library::Vtable("stage"));
		sub_vt->registerFn("1.0", "open", "s", "", open_stage);
		sub_vt->registerFn("1.0", "close", "", "", close_stage);
		sub_lib.registerVtable(move(sub_vt));
		lib.libraries.emplace_back(std::move(sub_lib));
	}

	lib.registerFactory("renderer", []()->shared_ptr<Wires::Data<size_t>>
	{
		return make_shared<Wires::Data<size_t>>(0);
	});
	{
		Landru::Library sub_lib("renderer");
		auto sub_vt = unique_ptr<Library::Vtable>(new Library::Vtable("renderer"));
		sub_vt->registerFn("1.0", "create", "", "", renderer_create);
		sub_vt->registerFn("1.0", "render", "o", "", renderer_render);
		sub_lib.registerVtable(move(sub_vt));
		lib.libraries.emplace_back(std::move(sub_lib));
	}

    root_lib->libraries.emplace_back(std::move(lib));
}

extern "C"
LANDRUHYDRA_API
RunState landru_hydra_update(double now, VMContext* vm)
{
	return RunState::Continue;
}

extern "C"
LANDRUHYDRA_API
void landru_hydra_finish(void* vl)
{
    Landru::Library * lib = reinterpret_cast<Landru::Library*>(vl);
}

extern "C"
LANDRUHYDRA_API
void landru_hydra_fiberExpiring(Fiber* f)
{
//    called when Fibers are destroyed so that pending items like onWindowsClosed can be removed
}

extern "C"
LANDRUHYDRA_API
void landru_hydra_clearContinuations(Fiber* f, int level)
{
}

extern "C"
LANDRUHYDRA_API
bool landru_hydra_pendingContinuations(Fiber * f)
{
    return false;
}
