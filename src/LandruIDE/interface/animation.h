
#pragma once

/*

    The structures in this file are meant to provide the look of a timeline's
    interface, but are not intended to encode animation.

*/

#include <string>
#include <vector>

namespace lab {

	class AnimationContent
	{
	public:
		std::string name;

		enum class Draw { Bar, Block };
		Draw draw = Draw::Bar;
	};

	class AnimationMark
	{
	public:
		std::string name;
		double t;
	};

	class AnimationClip
	{
	public:
		std::string name;
		AnimationContent content;
		double in_time = 0.;
		double out_time = 0;
		std::vector<AnimationMark> marks;
	};

	class AnimationBlock
	{
	public:
		std::string name;
		std::shared_ptr<AnimationClip> clip;
		double offset_time = 0.;
		double clip_scale = 1.;

		// trim offset from start of clip, after clip has been scaled
		double in_trim = 0.;

		// trim offset from end of clip, after clip has been scaled
		double out_trim = 0.;

		// a block may nest other blocks
		std::vector<AnimationBlock> blocks;

		// if the blocks vector is empty, the clip content draw type governs the look.
		// otherwise:
		// if collapsed, nested blocks are not expanded, and the clip content draw type governs the look
		// If not collapsed, the block is drawn as a block and the contents
		// rendering is governed by the same rule.
		bool collapsed = true;
	};

	class AnimationTrack
	{
	public:
		std::string name;

		void append_block(const std::string & name, double t0, double t1)
		{
			auto c = std::make_shared<AnimationClip>();
			c->in_time = t0;
			c->out_time = t1;

			AnimationBlock b;
			b.name = name;
			b.clip = c;

			blocks.emplace_back(b);
		}

		// a track has blocks
		std::vector<AnimationBlock> blocks;

		// a track may nest other blocks
		std::vector<AnimationTrack> tracks;
	};


} // lab
