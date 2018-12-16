
#pragma once

#include <imgui.h>

namespace ImGui
{
//    bool BeginTimeline(const char* str_id, float max_value, int num_visible_rows, int opt_exact_num_rows, ImVec2 *popt_offset_and_scale);
//    bool TimelineEvent(const char* str_id, float* values, bool keep_range_constant);
//    void EndTimeline(int num_vertical_grid_lines, float current_time, ImU32 timeline_running_color);

    bool BeginTimeline(const char* str_id, float column_pixel_offset, float max_time_value, double * time_in, double * time_out);
    bool TimelineEvent(const char* str_id, double & val1, double & val2);
    void EndTimeline(int num_vertical_lines, float current_time, ImU32 timeline_indicator_color);
}
