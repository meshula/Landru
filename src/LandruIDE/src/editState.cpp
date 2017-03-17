
#include "editState.h"
#include "interface/boxer.h"
#include <imgui.h>
#include <LabRender/Camera.h>
#include <ctime>

#ifdef _MSC_VER
// suppress warnings until USD is cleaned up
#  pragma warning(push)
#  pragma warning(disable : 4244 4305)
#endif

#include <pxr/usd/usd/stage.h>
#include <pxr/base/arch/nap.h>

#ifdef _MSC_VER
// suppress warnings until USD is cleaned up
#  pragma warning(pop)
#endif

namespace lab {

	// camera
	event<void(float deltax, float deltay)> evt_camera_mouse;
    event<void(std::shared_ptr<Camera>)> evt_bind_view_camera;
	event<void(CameraRig::Mode)> evt_set_camera_mode;

	// manipulator
	event<void(EditState::ManipulatorMode)> evt_set_manipulator_mode;
	event<void(const EditState::float16 &)> evt_set_manipulated_matrix;

	// stage
	event<void()> evt_new_stage;
	event<void(UsdStageRefPtr new_stage)> evt_stage_created;
	event<void(UsdStageRefPtr new_stage)> evt_stage_closing;
	event<void(const std::string&)> evt_new_layer;
	event<void(const std::string&)> evt_layer_created;


struct EditState::_Detail
{
    _Detail()
    {
        evt_camera_mouse.connect(this, &_Detail::rig_mouse_move);
        evt_bind_view_camera.connect(this, &_Detail::bind_view_camera);
		evt_set_camera_mode.connect(this, &_Detail::set_camera_mode);
		evt_set_manipulator_mode.connect(this, &_Detail::set_manipulator_mode);
		evt_set_manipulated_matrix.connect(this, &_Detail::set_manipulated_matrix);
		evt_new_stage.connect(this, &_Detail::new_stage);
		evt_new_layer.connect(this, &_Detail::new_layer);

    }
    ~_Detail()
    {
        evt_camera_mouse.disconnect(this, &_Detail::rig_mouse_move);
		evt_bind_view_camera.disconnect(this, &_Detail::bind_view_camera);
		evt_set_camera_mode.disconnect(this, &_Detail::set_camera_mode);
		evt_set_manipulator_mode.disconnect(this, &_Detail::set_manipulator_mode);
		evt_set_manipulated_matrix.disconnect(this, &_Detail::set_manipulated_matrix);
		evt_new_stage.connect(this, &_Detail::new_stage);
		evt_new_layer.disconnect(this, &_Detail::new_layer);
	}

	void set_manipulator_mode(ManipulatorMode m)
	{
		mode = m;
	}

	void set_manipulated_matrix(const EditState::float16 & m)
	{
		memcpy(&manipulator_matrix, &m, sizeof(EditState::float16));
	}

	void set_camera_mode(CameraRig::Mode mode)
	{
		rig.set_mode(mode);
	}

    void rig_mouse_move(float deltax, float deltay)
    {
        v2f delta{deltax, deltay};
        rig.interact(_currentCamera.get(), delta);
    }

    void bind_view_camera(std::shared_ptr<Camera> camera)
    {
        _currentCamera = camera;
    }

	void new_stage()
	{
		if (_stage)
		{
			boxer::Selection choice = boxer::show("Create new stage?", "Closing stage", boxer::Style::Warning, boxer::Buttons::OKCancel);
			if (choice == boxer::Selection::Cancel)
				return;

			evt_stage_closing(_stage);
			_stage->Close();
		}

		std::time_t now = std::time(nullptr);
		std::tm * ltime = localtime(&now);
		char tbuff[80];
		sprintf(tbuff, "session_%04d%02d%02d_%02d%02d.usda", ltime->tm_year + 1900, ltime->tm_mon + 1, ltime->tm_mday,
			ltime->tm_hour, ltime->tm_min);

		_stage = UsdStage::CreateNew(tbuff);
		evt_stage_created(_stage);
	}

	void new_layer(const std::string & layer)
	{
		if (!_stage)
			return;

		auto root_layer = _stage->GetRootLayer();
		auto new_layer = SdfLayer::CreateNew(layer);
		root_layer->GetSubLayerPaths().push_back(layer);
		evt_layer_created(layer);
	}

    std::shared_ptr<Camera> _currentCamera;
    CameraRig rig;
	ManipulatorMode mode = ManipulatorMode::Translate;

	float manipulator_matrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	UsdStageRefPtr _stage;
};

EditState::EditState()
: _detail(new _Detail())
{
}

EditState::~EditState()
{
    delete _detail;
}

UsdStageRefPtr EditState::stage()
{
	return _detail->_stage;
}

EditState::ManipulatorMode EditState::manipulator_mode() const
{
	return _detail->mode;
}

const EditState::float16 & EditState::manipulated_matrix() const
{
	return _detail->manipulator_matrix;
}


} // lab


