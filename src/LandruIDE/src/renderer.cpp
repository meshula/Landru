
#include "renderer.h"
#include "editState.h"
#include <LabRender/Camera.h>
#include <LabRender/gl4.h> // temp, shouldn't be using GL directly here
#include <LabRender/PassRenderer.h>
#include <LabRender/UtilityModel.h>
#include <LabRender/Utils.h>
#include <LabCmd/FFI.h>
#include <memory>

namespace lab
{
	using namespace std;

    class RenderEngine::Detail
    {
    public:
        shared_ptr<lab::PassRenderer> dr;
		lab::DrawList drawList;
		shared_ptr<Camera> camera;

		lab::OSCServer oscServer;
		lab::WebSocketsServer wsServer;

        Detail()
        : oscServer("renderingView")
        , wsServer("renderingView")
		, camera(make_shared<Camera>())
        {
#	ifdef _WIN32
			lab::addPathVariable("$(ASSET_ROOT)", "C:/Projects/landru-stage/Landru/thirdparty/prereq/labrender/assets");
#	else
			lab::addPathVariable("$(ASSET_ROOT)", "/Users/dp/Projects/LabRender/assets");
#	endif

			dr = make_shared<lab::PassRenderer>();
			//dr->configure("$(ASSET_ROOT)/pipelines/deferred.json");
			dr->configure("$(ASSET_ROOT)/pipelines/deferred_2_fxaa.json");
			//dr->configure("$(ASSET_ROOT)/pipelines/shadertoy.json");
			dr->self = dr;

            //shared_ptr<lab::Command> command = make_shared<PingCommand>();
            //oscServer.registerCommand(command);
            //command = make_shared<LoadMeshCommand>(dr.get(), &drawList);
            //oscServer.registerCommand(command);

            const int PORT_NUM = 9109;
            oscServer.start(PORT_NUM);
            wsServer.start(PORT_NUM + 1);
        }

		void render_start(lab::PassRenderer::RenderLock& rl, double time, v2i fbOffset, v2i fbSize)
		{
			if (!rl.valid() || rl.renderInProgress())
				return;

			const double globalTime = 0;

			rl.setRenderInProgress(true);
			rl.context.renderTime = globalTime;

			// do render start
			//glViewport(fbOffset.x, fbOffset.y, fbSize.x, fbSize.y);
			//glClearColor(0.5f, 0, 0, 1);
			//glClear(GL_COLOR_BUFFER_BIT);
			// done
		}

		void render_end(PassRenderer::RenderLock& rl)
		{
			if (!rl.valid() || !rl.renderInProgress())
				return;

			// custom bit
			//glfwSwapBuffers(window);
			// end

			rl.setRenderInProgress(false);
		}
    };

	RenderEngine::RenderEngine()
    : _detail(new Detail())
    {
    }

	RenderEngine::~RenderEngine()
    {
        delete _detail;
    }

    void RenderEngine::create_scene()
    {
        std::vector<std::shared_ptr<lab::ModelBase>>& meshes = _detail->drawList.deferredMeshes;
        //shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("$(ASSET_ROOT)/models/starfire.25.obj");
        shared_ptr<lab::ModelBase> model = lab::Model::loadMesh("$(ASSET_ROOT)/models/ShaderBall/shaderBallNoCrease/shaderBall.obj");
        meshes.push_back(model);

        shared_ptr<lab::UtilityModel> cube = make_shared<lab::UtilityModel>();
        cube->createCylinder(0.5f, 0.5f, 2.f, 16, 3, false);
        meshes.push_back(cube);

        static float foo = 0.f;
        _detail->camera->position = { foo, 0, -1000 };
        lab::Bounds bounds = model->localBounds();
        bounds = model->transform.transformBounds(bounds);
        _detail->camera->frame(bounds);
    }

    void RenderEngine::render(int width, int height)
    {
        lab::checkError(lab::ErrorPolicy::onErrorLog,
            lab::TestConditions::exhaustive, "main loop start");

        v2i fbSize = { width, height };

        _detail->drawList.jacobian = _detail->camera->mount.jacobian();
        _detail->drawList.view = _detail->camera->mount.viewTransform();
        _detail->drawList.proj = _detail->camera->optics.perspective(float(width) / float(height));

        const float time = 0.f; //=renderTime()
        const v2f mousePosition = { 0,0 }; // = mousePosition();

        lab::PassRenderer::RenderLock rl(_detail->dr->self.lock(), time, mousePosition);
        v2i fbOffset = {0,0};
        _detail->render_start(rl, time, fbOffset, fbSize);
        _detail->dr->render(rl, fbSize, _detail->drawList);
        _detail->render_end(rl);

        lab::checkError(lab::ErrorPolicy::onErrorThrow,
            lab::TestConditions::exhaustive, "main loop end");
    }

	m44f RenderEngine::camera_view()
	{
		return _detail->camera->mount.viewTransform();
	}
	m44f RenderEngine::camera_projection(int width, int height)
	{
		return _detail->camera->optics.perspective(float(width) / float(height));
	}


    int RenderEngine::output_texture_id()
    {
        auto fb = _detail->dr->framebuffer("gbuffer");
		if (!fb || !fb->textures.size())
			return 0;
        return fb->textures[3]->id;
    }

    void RenderEngine::save_output_texture(std::string path)
    {
        auto fb = _detail->dr->framebuffer("gbuffer");
		if (!fb || !fb->textures.size())
			return;
		fb->textures[3]->save(path.c_str());
    }

	void RenderEngine::camera_interact(int delta_x, int delta_y)
	{
		evt_bind_view_camera(_detail->camera);
		evt_camera_mouse((float)delta_x * 0.02f, (float)delta_y * -0.02f);
	}

}

