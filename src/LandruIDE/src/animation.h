
#pragma once

#include <string>
#include <vector>

class AnimationContent
{
public:
    std::string name;
}

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
    std:string name;
    std::shared_ptr<AnimationClip> clip;
    double offset_time = 0.;
    double clip_scale = 1.;

    // trim offset from start of clip, after clip has been scaled
    double in_trim = 0.;

    // trim offset from end of clip, after clip has been scaled
    double out_trim = 0.;

    // a block may nest other blocks
    std::vector<AnimationBlock> blocks;
};

class AnimationTrack
{
public:
    std::string name;

    // a track has blocks
    std::vector<AnimationBlock> blocks;

    // a track may nest other blocks
    std::vector<AnimationTrack> tracks;
};


