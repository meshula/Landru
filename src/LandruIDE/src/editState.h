
#pragma once

#include "events/event.hpp"
#include <LabRender/Camera.h>
#include <memory>

namespace lab {

	class Camera;

    class EditState
    {
        struct _Detail;
        _Detail * _detail = nullptr;

    public:
        EditState();
        ~EditState();
    };

    extern event<void(float deltax, float deltay)> evt_camera_mouse;
    extern event<void(std::shared_ptr<Camera>)> evt_bind_view_camera;
	extern event<void(CameraRig::Mode)> evt_set_camera_mode;

}
