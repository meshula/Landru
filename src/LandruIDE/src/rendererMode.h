#pragma once

#include <string>
#include <LabRender/MathTypes.h>
#include "modes.h"

namespace lab
{
    class PassRenderer;
    class DrawList;
    class Camera;

    class RendererMode : public MinorMode
	{
        class Detail;
        Detail * _detail = nullptr;

	public:
		RendererMode();
        ~RendererMode();

		virtual const char * name() const override { return "renderer"; }

		void create_scene();
		void render(int width, int height);
		int output_texture_id();
		void save_output_texture(std::string path);

		void camera_interact(int delta_x, int delta_y);
		m44f camera_view();
		m44f camera_projection(int width, int height);
	};
}