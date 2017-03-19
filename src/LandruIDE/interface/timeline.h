
#pragma once

#include <imgui.h>

namespace ImGui {
    bool BeginTimeline(const char* str_id, float pixel_offset, float max_time_value);
    bool TimelineEvent(const char* str_id, float & val1, float & val2);
    void EndTimeline();
}

