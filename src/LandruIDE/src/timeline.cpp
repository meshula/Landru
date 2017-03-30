

#include "timeline.h"
#include "interface/animation.h"
#include "interface/timeline.h"

namespace lab
{

    void Timeline::ui(lab::EditState& edit_state,
        lab::CursorManager& cursorManager,
        lab::FontManager& fontManager,
        float width, float height)
    {
		static float column_x = 160.f;
		static double visible_time_in = 0;
		static double visible_time_out = 1;

		AnimationTrack & root = edit_state.animation_root();

		//static ImVec2 offset_scale = { 0, 1 };
		//ImGui::BeginTimeline("foo", 150.f, 5, 0, &offset_scale);
		ImGui::BeginTimeline("Foo", column_x, 150.f, &visible_time_in, &visible_time_out);

		// @TODO recursive drawing goes here
		for (auto & t : root.blocks)
		{
			ImGui::TimelineEvent(t.name.c_str(), t.clip->in_time, t.clip->out_time);
		}
		ImU32 timeline_running_color = 0xff00ffff;
		ImGui::EndTimeline(5, 100.f, timeline_running_color);
		//ImGui::EndTimeline(10, 22.5f, timeline_running_color);
	}

}

