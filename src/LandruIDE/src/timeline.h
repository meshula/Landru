
#pragma once

#include "modes.h"
#include "events/event.hpp"
#include <chrono>

namespace lab {

	enum class PlayMode { NotStarted, Playing, Paused };

	class Timeline : public MinorMode
	{
		PlayMode _playMode = PlayMode::NotStarted;
		double _current_time = 100.0;

		void play();
		void pause();
		void rewind();

		void update(std::chrono::steady_clock::time_point&);

	public:
		Timeline();
		virtual ~Timeline();

		virtual const char * name() const override { return "timeline"; }

		virtual void ui(lab::EditState& edit_state,
			lab::CursorManager& cursorManager,
			lab::FontManager& fontManager,
			float width, float height) override;
	};

	extern event<void()> evt_timeline_play;
	extern event<void()> evt_timeline_pause;
	extern event<void()> evt_timeline_rewind;
	extern event<void(std::chrono::steady_clock::time_point&)> evt_timeline_update;

} // lab
