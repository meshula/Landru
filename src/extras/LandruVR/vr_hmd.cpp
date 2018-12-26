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

#include "vr_hmd.hpp"

std::string get_tracked_device_string(vr::IVRSystem * pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0) return "";
	std::vector<char> pchBuffer(unRequiredBufferLen);
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer.data(), unRequiredBufferLen, peError);
	std::string result = { pchBuffer.begin(), pchBuffer.end() };
	return result;
}

OpenVR_HMD::OpenVR_HMD()
{
	vr::EVRInitError eError = vr::VRInitError_None;
	hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None) throw std::runtime_error("Unable to init VR runtime: " + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(eError)));

	std::cout << "VR Driver:  " << get_tracked_device_string(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String) << std::endl;
	std::cout << "VR Display: " << get_tracked_device_string(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String) << std::endl;

	controllerRenderData = std::make_shared<ControllerRenderData>();
	controllers[0].renderData = controllerRenderData;
	controllers[1].renderData = controllerRenderData;

	renderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!renderModels)
	{
		vr::VR_Shutdown();
		throw std::runtime_error("Unable to get render model interface: " + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(eError)));
	}

	// Setup Framebuffers
	hmd->GetRecommendedRenderTargetSize(&renderTargetSize.x, &renderTargetSize.y);

	// Gen intermediate tex
	std::cout << controllerRenderData->tex << std::endl;

	// Generate multisample render buffers for color and depth
	glNamedRenderbufferStorageMultisampleEXT(multisampleRenderbuffers[0], 4, GL_RGBA8, renderTargetSize.x, renderTargetSize.y);
	glNamedRenderbufferStorageMultisampleEXT(multisampleRenderbuffers[1], 4, GL_DEPTH_COMPONENT, renderTargetSize.x, renderTargetSize.y);

	// Generate a framebuffer for multisample rendering
	glNamedFramebufferRenderbufferEXT(multisampleFramebuffer, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, multisampleRenderbuffers[0]);
	glNamedFramebufferRenderbufferEXT(multisampleFramebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, multisampleRenderbuffers[1]);
	if (glCheckNamedFramebufferStatusEXT(multisampleFramebuffer, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("Framebuffer incomplete!");

	// Generate textures and framebuffers for the left and right eye images
	for (int i : {0, 1})
	{
		glTextureImage2DEXT(eyeTextures[i], GL_TEXTURE_2D, 0, GL_RGBA8, renderTargetSize.x, renderTargetSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTextureParameteriEXT(eyeTextures[i], GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteriEXT(eyeTextures[i], GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteriEXT(eyeTextures[i], GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteriEXT(eyeTextures[i], GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteriEXT(eyeTextures[i], GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glNamedFramebufferTexture2DEXT(eyeFramebuffers[i], GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eyeTextures[i], 0);
		eyeFramebuffers[i].check_complete();
	}

	// Setup the compositor
	if (!vr::VRCompositor())
	{
		throw std::runtime_error("could not initialize VRCompositor");
	}
}

OpenVR_HMD::~OpenVR_HMD()
{
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
	glDebugMessageCallback(nullptr, nullptr);
	if (hmd) vr::VR_Shutdown();
}

void OpenVR_HMD::update()
{
	// Handle events
	vr::VREvent_t event;
	while (hmd->PollNextEvent(&event, sizeof(event)))
	{
		switch (event.eventType)
		{

		case vr::VREvent_TrackedDeviceActivated:
		{
			std::cout << "Device " << event.trackedDeviceIndex << " attached." << std::endl;

			if (hmd->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::TrackedDeviceClass_Controller && controllerRenderData->loaded == false)
			{
				vr::EVRInitError eError = vr::VRInitError_None;

				vr::RenderModel_t * model = nullptr;
				vr::RenderModel_TextureMap_t * texture = nullptr;

				while (true)
				{
					std::string sRenderModelName = get_tracked_device_string(hmd, event.trackedDeviceIndex, vr::Prop_RenderModelName_String);
					renderModels->LoadRenderModel_Async(sRenderModelName.c_str(), &model);
					if (model) renderModels->LoadTexture_Async(model->diffuseTextureId, &texture);
					if (model && texture) break;
				}

				controllerRenderData->mesh.set_vertex_data(sizeof(vr::RenderModel_Vertex_t)*model->unVertexCount, model->rVertexData, GL_STATIC_DRAW);
				controllerRenderData->mesh.set_attribute(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (const GLvoid *)offsetof(vr::RenderModel_Vertex_t, vPosition));
				controllerRenderData->mesh.set_attribute(1, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (const GLvoid *)offsetof(vr::RenderModel_Vertex_t, vNormal));
				controllerRenderData->mesh.set_attribute(3, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (const GLvoid *)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));
				controllerRenderData->mesh.set_index_data(GL_TRIANGLES, GL_UNSIGNED_SHORT, model->unTriangleCount * 3, model->rIndexData, GL_STATIC_DRAW);

				glTextureImage2DEXT(controllerRenderData->tex, GL_TEXTURE_2D, 0, GL_RGBA, texture->unWidth, texture->unHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->rubTextureMapData);
				glGenerateTextureMipmapEXT(controllerRenderData->tex, GL_TEXTURE_2D);
				glTextureParameteriEXT(controllerRenderData->tex, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteriEXT(controllerRenderData->tex, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

				for (uint32_t i = 0; i<model->unVertexCount; ++i)
					controllerRenderData->verts.push_back(reinterpret_cast<const float3 &>(model->rVertexData[i].vPosition));

				renderModels->FreeTexture(texture);
				renderModels->FreeRenderModel(model);

				controllerRenderData->loaded = true;
			}

			break;
		}
		case vr::VREvent_TrackedDeviceDeactivated: std::cout << "Device " << event.trackedDeviceIndex << " detached." << std::endl; break;
		case vr::VREvent_TrackedDeviceUpdated: std::cout << "Device " << event.trackedDeviceIndex << " updated." << std::endl; break;
		}
	}

	// Get HMD pose
	std::array<vr::TrackedDevicePose_t, 16> poses;
	vr::VRCompositor()->WaitGetPoses(poses.data(), static_cast<uint32_t>(poses.size()), nullptr, 0);
	for (vr::TrackedDeviceIndex_t i = 0; i < poses.size(); ++i)
	{
		if (!poses[i].bPoseIsValid) continue;
		switch (hmd->GetTrackedDeviceClass(i))
		{
		case vr::TrackedDeviceClass_HMD:
		{
			hmdPose = make_pose(poses[i].mDeviceToAbsoluteTracking);
			break;
		}
		case vr::TrackedDeviceClass_Controller:
		{
			vr::VRControllerState_t controllerState = vr::VRControllerState_t();
			switch (hmd->GetControllerRoleForTrackedDeviceIndex(i))
			{
			case vr::TrackedControllerRole_LeftHand:
			{
				if (hmd->GetControllerState(i, &controllerState, sizeof(vr::VRControllerState_t)))
				{
					controllers[0].trigger.update(!!(controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)));
					controllers[0].pad.update(!!(controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)));
					controllers[0].touchpad.x = controllerState.rAxis[vr::k_eControllerAxis_TrackPad].x;
					controllers[0].touchpad.y = controllerState.rAxis[vr::k_eControllerAxis_TrackPad].y;
					controllers[0].p = make_pose(poses[i].mDeviceToAbsoluteTracking);
				}
				break;
			}
			case vr::TrackedControllerRole_RightHand:
			{
				if (hmd->GetControllerState(i, &controllerState, sizeof(vr::VRControllerState_t)))
				{
					controllers[1].trigger.update(!!(controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger)));
					controllers[1].pad.update(!!(controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad)));
					controllers[1].touchpad.x = controllerState.rAxis[vr::k_eControllerAxis_TrackPad].x;
					controllers[1].touchpad.y = controllerState.rAxis[vr::k_eControllerAxis_TrackPad].y;
					controllers[1].p = make_pose(poses[i].mDeviceToAbsoluteTracking);
				}
				break;
			}
			}
			break;
		}
		}
	}
}

void OpenVR_HMD::render(float near, float far, std::function<void(::Pose eyePose, float4x4 projMatrix)> renderFunc)
{
	for (auto eye : { vr::Eye_Left, vr::Eye_Right })
	{
		glViewport(0, 0, renderTargetSize.x, renderTargetSize.y);

		// Render into single 4x multisampled fbo
		glEnable(GL_MULTISAMPLE);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampleFramebuffer);

		renderFunc(get_eye_pose(eye), get_proj_matrix(eye, near, far));

		glDisable(GL_MULTISAMPLE);

		// Resolve multisample into per-eye textures
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eyeTextures[eye]);
		glBlitFramebuffer(0, 0, renderTargetSize.x, renderTargetSize.y, 0, 0, renderTargetSize.x, renderTargetSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	// Submit
	for (auto eye : { vr::Eye_Left, vr::Eye_Right })
	{
		const vr::Texture_t texture = { (void*)(intptr_t)(GLuint) eyeTextures[eye], vr::ETextureType::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(eye, &texture);
	}

	glFlush();
}
