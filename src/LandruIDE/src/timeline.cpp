

#include "timeline.h"
#include "timingMode.h"
#include "interface/animation.h"
#include "interface/timeline.h"

namespace lab
{
	event<void()> evt_timeline_play;
	event<void()> evt_timeline_pause;
	event<void()> evt_timeline_rewind;

	Timeline::Timeline()
	{
		evt_timeline_play.connect(this, &Timeline::play);
		evt_timeline_pause.connect(this, &Timeline::pause);
		evt_timeline_rewind.connect(this, &Timeline::rewind);
		evt_timing_update.connect(this, &Timeline::update);
	}

	Timeline::~Timeline()
	{
		evt_timeline_play.disconnect(this, &Timeline::play);
		evt_timeline_pause.disconnect(this, &Timeline::pause);
		evt_timeline_rewind.disconnect(this, &Timeline::rewind);
		evt_timing_update.disconnect(this, &Timeline::update);
	}

	void Timeline::pause()
	{
		_playMode = PlayMode::Paused;
	}

	void Timeline::play()
	{
		_playMode = PlayMode::Playing;
	}

	void Timeline::rewind()
	{
		_playMode = PlayMode::Paused;
		_current_time = 0;
	}

	void Timeline::update(std::chrono::steady_clock::time_point & now)
	{
		static std::chrono::steady_clock::time_point prev;
		auto delta = now - prev;
		prev = now;

		static bool first = true;
		if (first)
		{
			first = false;
			return;
		}

		if (_playMode == PlayMode::Playing)
		{
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
			_current_time += microseconds * 1.0e-6;
		}
	}


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
		ImGui::EndTimeline(5, _current_time, timeline_running_color);
	}

}

