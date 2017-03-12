
#pragma once

#include "events/event.hpp"
#include <LabRender/Camera.h>
#include <memory>

#include <pxr/usd/usd/stage.h>

namespace lab {

	class Camera;

    class EditState
    {
        struct _Detail;
        _Detail * _detail = nullptr;

    public:
		enum ManipulatorMode { Translate, Rotate, Scale };
		enum ManipSpace { Local, Global };
		
		EditState();
        ~EditState();

		ManipulatorMode manipulator_mode() const;

		UsdStageRefPtr stage();
    };

    extern event<void(float deltax, float deltay)> evt_camera_mouse;
    extern event<void(std::shared_ptr<Camera>)> evt_bind_view_camera;
	extern event<void(CameraRig::Mode)> evt_set_camera_mode;

	extern event<void(EditState::ManipulatorMode)> evt_set_manipulator_mode;

} // lab
