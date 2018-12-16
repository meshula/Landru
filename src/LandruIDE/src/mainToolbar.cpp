
#include "editState.h"
#include "interface/toolbar.h"
#include "interface/labFontManager.h"
#include "interface/materialIconDefinitions.h"
#include "timeline.h"
#include <LabRender/Camera.h>
#include <imgui.h>

// toolbar adapated from EtherealEngine

namespace lab
{
	namespace gui = ImGui;

	PlayMode playMode = PlayMode::NotStarted;

	CameraRig::Mode navMode = CameraRig::Mode::TurnTableOrbit;

	EditState::ManipSpace space = EditState::ManipSpace::Local;

    void toolbar(EditState & edit_state, FontManager* fm)
    {
		SetFont f(fm->mono_font);
        float width = gui::GetContentRegionAvailWidth();
		EditState::ManipulatorMode operationMode = edit_state.manipulator_mode();

		{
			SetFont icon(fm->icon_font);

			if (lab::ToolbarTextButton(" " ICON_MD_CREATE_NEW_FOLDER " ", "New", false))
			{
				evt_new_stage();
			}
			gui::SameLine(0.0f);
			if (lab::ToolbarTextButton(" " ICON_MD_CLOUD_DOWNLOAD " ", "Open", false))
			{
//				evt_set_manipulator_mode(EditState::ManipulatorMode::Translate);
			}
			gui::SameLine(0.0f);
			if (lab::ToolbarTextButton(" " ICON_MD_CLOUD_UPLOAD " ", "Save", false))
			{
				//				evt_set_manipulator_mode(EditState::ManipulatorMode::Translate);
			}


			gui::SameLine(0.0f, 50.0f);
			if (lab::ToolbarTextButton(" " ICON_MD_GAMEPAD " ", "Translate", operationMode == EditState::ManipulatorMode::Translate))
			{
				evt_set_manipulator_mode(EditState::ManipulatorMode::Translate);
			}
			gui::SameLine(0.0f);
			if (lab::ToolbarTextButton(" " ICON_MD_SYNC " ", "Rotate", operationMode == EditState::ManipulatorMode::Rotate))
			{
				evt_set_manipulator_mode(EditState::ManipulatorMode::Rotate);
			}
			gui::SameLine(0.0f);
			if (lab::ToolbarTextButton(" " ICON_MD_ZOOM_OUT_MAP " ", "Scale", operationMode == EditState::ManipulatorMode::Scale))
			{
				evt_set_manipulator_mode(EditState::ManipulatorMode::Scale);
				space = EditState::ManipSpace::Local;
			}
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
		if (lab::ToolbarTextButton(" L ", "Local Coordinate System", space == EditState::ManipSpace::Local))
        {
			space = EditState::ManipSpace::Local;
        }
        gui::SameLine(0.0f);
        if (lab::ToolbarTextButton(" G ", "Global Coordinate System", space == EditState::ManipSpace::Global, operationMode == EditState::ManipulatorMode::Scale))
        {
			space = EditState::ManipSpace::Global;
        }
        gui::SameLine(0.0f, 2.f);
        gui::SameLine(width / 2.0f - 36.0f);
		{
			SetFont icon(fm->icon_font);
			if (lab::ToolbarTextButton(" " ICON_MD_REPLAY " ", "Rewind", playMode != PlayMode::Playing, playMode != PlayMode::Playing))
			{
				evt_timeline_rewind();
				playMode = PlayMode::Paused;
			}
			gui::SameLine(0.0f, 2.f);
			if (lab::ToolbarTextButton(" " ICON_MD_PLAY_ARROW " ", "Play", playMode != PlayMode::Playing, playMode != PlayMode::Playing)) // selected, enabled
			{
				evt_timeline_play();
				playMode = PlayMode::Playing;
			}
			gui::SameLine(0.0f, 2.f);
			if (lab::ToolbarTextButton(" " ICON_MD_PAUSE " ", "Pause", playMode == PlayMode::Playing, playMode == PlayMode::Playing))
			{
				evt_timeline_pause();
				playMode = PlayMode::Paused;
			}
			gui::SameLine(0.0f, 2.f);
			if (lab::ToolbarTextButton(" " ICON_MD_SKIP_NEXT " ", "Step", playMode != PlayMode::Playing, playMode != PlayMode::Playing))
			{
			}
		}
	}

} // lab
