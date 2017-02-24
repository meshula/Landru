
#include "editState.h"
#include <LabRender/Camera.h>

namespace lab {

	event<void(float deltax, float deltay)> evt_camera_mouse;
    event<void(std::shared_ptr<Camera>)> evt_bind_view_camera;
	event<void(CameraRig::Mode)> evt_set_camera_mode;


struct EditState::_Detail
{
    _Detail()
    {
        evt_camera_mouse.connect(this, &_Detail::rig_mouse_move);
        evt_bind_view_camera.connect(this, &_Detail::bind_view_camera);
		evt_set_camera_mode.connect(this, &_Detail::set_camera_mode);
    }
    ~_Detail()
    {
        evt_camera_mouse.disconnect(this, &_Detail::rig_mouse_move);
		evt_bind_view_camera.disconnect(this, &_Detail::bind_view_camera);
		evt_set_camera_mode.disconnect(this, &_Detail::set_camera_mode);
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

    std::shared_ptr<Camera> _currentCamera;
    CameraRig rig;
};

EditState::EditState()
: _detail(new _Detail())
{
}

EditState::~EditState()
{
    delete _detail;
}


} // lab


