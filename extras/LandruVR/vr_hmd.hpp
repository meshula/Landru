
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#pragma once

#ifndef vr_hmd_hpp
#define vr_hmd_hpp

#include <openvr.h>
#include "linalg_util.hpp"
#include "geometric.hpp"
#include "geometry.hpp"

#include <memory>
#include <vector>

using namespace avl;

inline Pose make_pose(const vr::HmdMatrix34_t & m)
{
	return {
		make_rotation_quat_from_rotation_matrix({ { m.m[0][0], m.m[1][0], m.m[2][0] },{ m.m[0][1], m.m[1][1], m.m[2][1] },{ m.m[0][2], m.m[1][2], m.m[2][2] } }),
		{ m.m[0][3], m.m[1][3], m.m[2][3] }
	};
}

struct ControllerRenderData
{
	GlMesh mesh;
	std::vector<float3> verts;
	GlTexture2D tex;
	bool loaded = false;
};

struct Controller
{
	struct ButtonState
	{
		bool down = false;
		bool lastDown = false;
		bool pressed = false;
		bool released = false;

		void update(bool state)
		{
			lastDown = down;
			down = state;
			pressed = (!lastDown) && state;
			released = lastDown && (!state);
		}
	};

	ButtonState pad;
	ButtonState trigger;
	float2 touchpad = float2(0.0f, 0.0f);

	Pose p;

	Ray forward_ray() const { return Ray(p.position, p.transform_vector(float3(0.0f, 0.0f, -1.0f))); }

	std::shared_ptr<ControllerRenderData> renderData;
};

class OpenVR_HMD
{
	vr::IVRSystem * hmd = nullptr;
	vr::IVRRenderModels * renderModels = nullptr;

	uint2 renderTargetSize;
	Pose hmdPose;
	Pose worldPose;

	GlFramebuffer eyeFramebuffers[2];
	GlTexture2D eyeTextures[2];
	GlRenderbuffer multisampleRenderbuffers[2];
	GlFramebuffer multisampleFramebuffer;

	std::shared_ptr<ControllerRenderData> controllerRenderData;

	Controller controllers[2];

public:

	OpenVR_HMD();
	~OpenVR_HMD();

	const Controller & get_controller(const vr::ETrackedControllerRole controller) const
	{
		if (controller == vr::TrackedControllerRole_LeftHand) return controllers[0];
		if (controller == vr::TrackedControllerRole_RightHand) return controllers[1];
	}

	std::shared_ptr<ControllerRenderData> get_controller_render_data()
	{
		return controllerRenderData;
	}

	Pose get_hmd_pose() { return worldPose * hmdPose; }
	void set_hmd_pose(Pose p) { hmdPose = p; }

	GLuint get_eye_texture(vr::Hmd_Eye eye) const { return eyeTextures[eye]; }

	float4x4 get_proj_matrix(vr::Hmd_Eye eye, float near_clip, float far_clip) {
		return transpose(reinterpret_cast<const float4x4 &>(hmd->GetProjectionMatrix(eye, near_clip, far_clip))); }

	Pose get_eye_pose(vr::Hmd_Eye eye) { return get_hmd_pose() * make_pose(hmd->GetEyeToHeadTransform(eye)); }

	void update();

	void render(float near, float far, std::function<void(Pose eye, float4x4 projMat)> renderFunc);
};

#endif
