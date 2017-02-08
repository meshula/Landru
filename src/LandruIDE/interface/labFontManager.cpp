
#include "labFontManager.h"
#include <stdint.h>
#include "interface/imgui/roboto_regular.ttf.h"
#include "interface/imgui/robotomono_regular.ttf.h"

namespace lab {

FontManager::FontManager()
{

    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

	uint8_t* data = nullptr;
	int32_t width = 0;
	int32_t height = 0;

	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;
	config.MergeMode = false;
	config.MergeGlyphCenterV = true;

	auto io = ImGui::GetIO();
	default_font = io.Fonts->AddFontDefault(&config);
	regular_font = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoRegularTtf, sizeof(s_robotoRegularTtf), 28, &config);
	mono_font = io.Fonts->AddFontFromMemoryTTF((void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), 28.0f, &config);
	io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

	/*
	s_font_texture = std::make_shared<Texture>(
		static_cast<std::uint16_t>(width)
		, static_cast<std::uint16_t>(height)
		, false
		, 1
		, gfx::TextureFormat::BGRA8
		, 0
		, gfx::copy(data, width*height * 4)
		);

	// Store our identifier
	io.Fonts->SetTexID(s_font_texture.get());
*/

}


}
