
#include <imgui.h>
#include <filesystem>

namespace lab {

	namespace fs = std::experimental::filesystem;


class FontManager
{
public:
    FontManager(const fs::path & resource_path);

    ImFont * default_font;
    ImFont * regular_font;
	ImFont * mono_font;
	ImFont * mono_small_font;
	ImFont * icon_font;
};

class SetFont
{
public:
	SetFont(ImFont * f) { ImGui::PushFont(f); }
	~SetFont() { ImGui::PopFont(); }
};

}



