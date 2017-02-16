#pragma once

#include <string>

namespace lab
{
    class PassRenderer;
    class DrawList;
    class Camera;

    class RenderEngine
	{
        class Detail;
        Detail * _detail = nullptr;

	public:
		RenderEngine();
        ~RenderEngine();
		void create_scene();
		void render(int width, int height);
		int output_texture_id();
		void save_output_texture(std::string path);
	};
}