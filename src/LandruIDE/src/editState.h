
#pragma once

#include "events/event.hpp"
#include <LabRender/Camera.h>
#include <memory>


#ifdef _MSC_VER
// suppress warnings until USD is cleaned up
#  pragma warning(push)
#  pragma warning(disable : 4244 4305)
#endif

#include <pxr/usd/usd/stage.h>


#ifdef _MSC_VER
// suppress warnings until USD is cleaned up
#  pragma warning(pop)
#endif

namespace lab {

	class AnimationTrack;
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

		typedef float float16[16];
		const float16 & manipulated_matrix() const;

		AnimationTrack & animation_root();
    };

	// camera
    extern event<void(float deltax, float deltay)> evt_camera_mouse;
    extern event<void(std::shared_ptr<Camera>)> evt_bind_view_camera;
	extern event<void(CameraRig::Mode)> evt_set_camera_mode;

	// manipulator
	extern event<void(EditState::ManipulatorMode)> evt_set_manipulator_mode;
	extern event<void(const EditState::float16 &)> evt_set_manipulated_matrix;

	// stage
	extern event<void()> evt_new_stage;
	extern event<void(UsdStageRefPtr)> evt_stage_created;
	extern event<void(UsdStageRefPtr)> evt_stage_closing;
	extern event<void(const std::string&)> evt_new_layer;
	extern event<void(const std::string&)> evt_layer_created;

} // lab
