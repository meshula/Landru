

#include "timeline.h"
#include "interface/timeline.h"

namespace lab
{

    void Timeline::ui(lab::EditState& edit_state,
        lab::CursorManager& cursorManager,
        lab::FontManager& fontManager,
        float width, float height)
    {
		constexpr float column_x = 160.f;

        ImGui::BeginTimeline("Foo", column_x, 123.f);

		ImGui::Columns(2);
		ImGui::SetColumnOffset(1, column_x);

		ImGui::Text("Raisins");
		ImGui::NextColumn();

		static float a1 = 0;
		static float a2 = 50;
        ImGui::TimelineEvent("Raisins", a1, a2);

		ImGui::NextColumn();
		ImGui::Text("Curry");
		ImGui::NextColumn();

		static float b1 = 10;
		static float b2 = 78;
        ImGui::TimelineEvent("Curry", b1, b2);

		ImGui::NextColumn();
		ImGui::Text("Mental");
		ImGui::NextColumn();

		static float c1 = 52;
		static float c2 = 175;
		ImGui::TimelineEvent("Mental", c1, c2);

        ImGui::EndTimeline();
    }

}

