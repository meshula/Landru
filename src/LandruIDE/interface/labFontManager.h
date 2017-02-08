
#include <imgui.h>

namespace lab {

class FontManager
{
public:
    FontManager();

    ImFont * default_font;
    ImFont * regular_font;
    ImFont * mono_font;
};

class SetFont
{
public:
	SetFont(ImFont * f) { ImGui::PushFont(f); }
	~SetFont() { ImGui::PopFont(); }
};

}



