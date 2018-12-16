
#include "graphnode.h"
#include "imgui_internal.h"


namespace lab
{
    ImVec2 GraphNode::GetRenderedPos()
    {
        //auto & io = ImGui::GetIO();
        //const float currentFontWindowScale = !io.FontAllowUserScaling ? fontScaleStored : ImGui::GetCurrentWindow()->FontWindowScale;
        // FontAllowUserScaling doesn't work well with the graph editor's zooming.
        // see grapheditor.h TODO bullet point on Adjust zooming.
        const float currentFontWindowScale = ImGui::GetCurrentWindow()->FontWindowScale;
        return GetPos(currentFontWindowScale);
    }
    ImVec2 GraphNode::GetRenderedSize()
    {
        return Size;
    }

}
