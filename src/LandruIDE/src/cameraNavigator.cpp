
#include "cameraNavigator.h"
#include "events/event.hpp"
#include <imgui.h>

namespace gui = ImGui;

namespace lab {

}

#if 0
	void camera_navigator(lab::FontManager& fontManager, const unsigned int frameRate)
	{
		SetFont small(fontManager.mono_small_font);

		ImVec2 pos = gui::GetCursorScreenPos();
		gui::SetNextWindowPos(pos);
		gui::SetNextWindowCollapsed(true, ImGuiSetCond_FirstUseEver);
		if (gui::Begin("Statistics", nullptr,
						ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_AlwaysAutoResize))
		{
			gui::Text("FPS  : %u", frameRate);
			gui::Separator();
			gui::Text("MSPF : %.3f ms ", 1000.0f / float(frameRate));
			gui::Separator();

			auto stats = gfx::getStats();
			gui::Text("Wait Render : %fms", stats->waitToRender);
			gui::Text("Wait Submit : %fms", stats->waitToSubmit);
			gui::Text("Draw calls: %u", stats->numDraw);
			gui::Text("Compute calls: %u", stats->numCompute);

			if (gui::Checkbox("More Stats", &ui_more_stats))
			{
				/*			if (more_stats)
								gfx::setDebug(BGFX_DEBUG_STATS);
							else
								gfx::setDebug(BGFX_DEBUG_NONE);
								*/
			}
			gui::Separator();
			gui::Checkbox("Show G-Buffer", &ui_show_gbuffer);
		}
		gui::End();
	}

#endif
