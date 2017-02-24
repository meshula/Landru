
#include "editState.h"
#include "interface/toolbar.h"
#include "interface/labFontManager.h"
#include <LabRender/Camera.h>
#include <imgui.h>

// toolbar adapated from EtherealEngine

namespace lab
{
	namespace gui = ImGui;

    enum ManipMode { Translate, Rotate, Scale, Local, Global };
    ManipMode operationMode = ManipMode::Translate;
    ManipMode space = ManipMode::Local;

	enum PlayMode { NotStarted, Playing, Paused };
	PlayMode playMode = PlayMode::NotStarted;

	CameraRig::Mode navMode = CameraRig::Mode::TurnTableOrbit;

    void toolbar(FontManager* fm)
    {
		SetFont f(fm->mono_font);
        float width = gui::GetContentRegionAvailWidth();
        if (lab::ToolbarTextButton(" T ", "Translate", operationMode == ManipMode::Translate))
        {
			operationMode = ManipMode::Translate;
        }
        gui::SameLine(0.0f);
        if (lab::ToolbarTextButton(" R ", "Rotate", operationMode == ManipMode::Rotate))
        {
			operationMode = ManipMode::Rotate;
		}
        gui::SameLine(0.0f);
        if (lab::ToolbarTextButton(" S ", "Scale", operationMode == ManipMode::Scale))
        {
			operationMode = ManipMode::Scale;
			space = ManipMode::Local;
        }

		gui::SameLine(0.0f, 50.0f);
		if (lab::ToolbarTextButton(" O ", "Orbit", navMode == CameraRig::Mode::TurnTableOrbit))
		{
			navMode = CameraRig::Mode::TurnTableOrbit;
			evt_set_camera_mode(navMode);
		}
		gui::SameLine(0.0f);
		if (lab::ToolbarTextButton(" D ", "Dolly", navMode == CameraRig::Mode::Dolly))
		{
			navMode = CameraRig::Mode::Dolly;
			evt_set_camera_mode(navMode);
		}
		gui::SameLine(0.0f);
		if (lab::ToolbarTextButton(" C ", "Crane", navMode == CameraRig::Mode::Crane))
		{
			navMode = CameraRig::Mode::Crane;
			evt_set_camera_mode(navMode);
		}
		gui::SameLine(0.0f);
		if (lab::ToolbarTextButton(" F ", "Fly", navMode == CameraRig::Mode::Fly))
		{
			navMode = CameraRig::Mode::Fly;
			evt_set_camera_mode(navMode);
		}



		gui::SameLine(0.0f, 50.0f);
		if (lab::ToolbarTextButton(" L ", "Local Coordinate System", space == ManipMode::Local))
        {
			space = ManipMode::Local;
        }
        gui::SameLine(0.0f);
        if (lab::ToolbarTextButton(" G ", "Global Coordinate System", space == ManipMode::Global, operationMode == ManipMode::Scale))
        {
			space = ManipMode::Global;
        }
        gui::SameLine(0.0f, 2.f);
        gui::SameLine(width / 2.0f - 36.0f);
		if (lab::ToolbarTextButton(" << ", "Rewind", playMode != PlayMode::Playing, playMode != PlayMode::Playing))
		{
			playMode = PlayMode::Paused;
		}
		gui::SameLine(0.0f, 2.f);
		if (lab::ToolbarTextButton(" > ", "Play", playMode != PlayMode::Playing, playMode != PlayMode::Playing)) // selected, enabled
        {
			playMode = PlayMode::Playing;
        }
        gui::SameLine(0.0f, 2.f);
        if (lab::ToolbarTextButton(" || ", "Pause", playMode == PlayMode::Playing, playMode == PlayMode::Playing))
        {
			playMode = PlayMode::Paused;
		}
        gui::SameLine(0.0f, 2.f);
        if (lab::ToolbarTextButton(" >| ", "Step", playMode != PlayMode::Playing, playMode != PlayMode::Playing))
        {
        }
    }

} // lab
