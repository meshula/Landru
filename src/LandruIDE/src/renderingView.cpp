
#include "renderingView.h"
#include "editState.h"
#include "rendererMode.h"
#include "interface/imguizmo.h"
#include "interface/labCursorManager.h"
#include "interface/labFontManager.h"
#include "interface/imguimath.h"
#include "interface/imguiRenderFrameEx.h"


#include <LabRender/Camera.h>
#include <LabRender/gl4.h> // temp, shouldn't be using GL directly here
#include <LabRender/PassRenderer.h>
#include <LabRender/UtilityModel.h>
#include <LabRender/Utils.h>

#include <LabCmd/FFI.h>

#include <imgui.h>

#include <memory>


// adapted from https://github.com/volcoma/EtherealEngine, BSD License

namespace gfx
{
	struct Stats
	{
		double waitToRender = 5.0e-3; //ms
		double waitToSubmit = 10.0e-3; //ms
		unsigned int numDraw = 32;
		unsigned int numCompute = 0;
	};

	Stats * getStats()
	{
		static Stats stats;
		return &stats;
	}
}

namespace lab
{

	namespace gui = ImGui;
	using namespace std;



	RenderingView::RenderingView(ModeManager & modeMgr)
	{
		_rendererModePtr = modeMgr.find_mode("renderer");
		if (!_rendererModePtr)
		{
			_rendererModePtr = make_shared<RendererMode>();
			modeMgr.add_mode(_rendererModePtr);
		}
		_rendererMode = dynamic_cast<RendererMode*>(_rendererModePtr.get());
	}

	RenderingView::~RenderingView()
	{
	}

	void RenderingView::show_statistics(lab::FontManager& fontManager, const unsigned int frameRate)
	{
		SetFont small(fontManager.mono_small_font);

		ImVec2 pos = gui::GetCursorScreenPos();
		gui::SetNextWindowPos(pos);
		gui::SetNextWindowCollapsed(true, ImGuiSetCond_FirstUseEver);
		if (gui::Begin("Statistics", nullptr,
						ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_AlwaysAutoResize))
		{
			gui::Text("FPS  : %u", frameRate);
			gui::Separator();
			gui::Text("MSPF : %.3f ms ", 1000.0f / float(frameRate));
			gui::Separator();

			auto stats = gfx::getStats();
			gui::Text("Wait Render : %fms", stats->waitToRender);
			gui::Text("Wait Submit : %fms", stats->waitToSubmit);
			gui::Text("Draw calls: %u", stats->numDraw);
			gui::Text("Compute calls: %u", stats->numCompute);

			if (gui::Checkbox("More Stats", &ui_more_stats))
			{
				/*			if (more_stats)
								gfx::setDebug(BGFX_DEBUG_STATS);
							else
								gfx::setDebug(BGFX_DEBUG_NONE);
								*/
			}
		}
		gui::End();
	}

	void RenderingView::draw_view_content(lab::FontManager& fontManager, const ImVec2& size)
	{
		static std::once_flag once;
		std::call_once(once, [this]()
        {
			_rendererMode->create_scene();
        });

		auto pos = gui::GetCursorScreenPos();
		ImVec2 bounds = size;// (size.x - 40.f, size.y - 40.f);

		ImTextureID tex_id = (ImTextureID) uintptr_t(_rendererMode->output_texture_id());
		gui::Image(tex_id, bounds, ImVec2(0,1), ImVec2(1,0));

		//_detail->save_output_texture("c:\\Projects\\foo.png");

		gui::SetCursorScreenPos(pos);
	}


	void RenderingView::manipulation_gizmos(lab::EditState& edit_state)
	{
		auto io = gui::GetIO();
		const bool object_selected = true;

		if (object_selected)
		{
			float* snap = nullptr;

			auto p = gui::GetItemRectMin();
			auto s = gui::GetItemRectSize();
			ImGuizmo::SetRect(p.x, p.y, s.x, s.y);

			m44f view = _rendererMode->camera_view();
			m44f proj = _rendererMode->camera_projection(width, height);

			ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
			switch (edit_state.manipulator_mode())
			{
			case EditState::ManipulatorMode::Translate: operation = ImGuizmo::TRANSLATE; break;
			case EditState::ManipulatorMode::Rotate: operation = ImGuizmo::ROTATE; break;
			case EditState::ManipulatorMode::Scale: operation = ImGuizmo::SCALE; break;
			}

			float source_matrix[16];
			float edit_matrix[16];
			memcpy(source_matrix, edit_state.manipulated_matrix(), sizeof(float[16]));
			memcpy(edit_matrix, source_matrix, sizeof(float[16]));

			ImGuizmo::Manipulate(
				&view.m00,
				&proj.m00,
				operation,
				ImGuizmo::LOCAL,
				edit_matrix,
				nullptr,
				snap);

			bool changed = false;
			for (int i = 0; i < 16 && !changed; ++i)
				changed |= source_matrix[i] != edit_matrix[i];

			if (changed)
				evt_set_manipulated_matrix(edit_matrix);
		}
	}

	void RenderingView::ui(lab::EditState& edit_state,
		lab::CursorManager& cursorManager,
		lab::FontManager& fontManager,
		float width_, float height_)
	{
		ImVec2 area{ width_, height_ };

		auto size = gui::GetContentRegionAvail();

		width = (int) size.x;
		height = (int) size.y;

		auto pos = gui::GetCursorScreenPos();
		draw_view_content(fontManager, size);

		show_statistics(fontManager, 90);// engine->get_fps());

		if (size.x > 0 && size.y > 0)
		{
			if (gui::IsItemClicked(1) || gui::IsItemClicked(2))
			{
				gui::SetWindowFocus();
				cursorManager.hide();
			}

			static bool tumbling = false;

			if (gui::IsWindowFocused())
			{
				auto & io = gui::GetIO();

				ImVec2 mousePos = io.MousePos;
				ImGuiWindow* window = gui::GetCurrentWindow();
				mousePos = mousePos - window->Pos - window->WindowPadding;

				if (io.MouseDown[0] && !left_mouse)
				{
					left_mouse = true;
					previousMousePosition = mousePos;
					initialMousePosition = mousePos;
					//printf("rui CLICK %f %f\n", (float)mousePos.x, (float)mousePos.y);
				}
				else if (!io.MouseDown[0] && left_mouse)
				{
					left_mouse = false;
					//printf("rui RELEASE %f %f\n", (float)mousePos.x, (float)mousePos.y);
				}

				ImVec2 delta = mousePos - previousMousePosition;
				previousMousePosition = mousePos;

				if (io.MouseDown[0])
				{
					if (!tumbling && ImGuizmo::IsOver() || ImGuizmo::IsUsing())
					{
					}
					else
					{
						tumbling = true;
						_rendererMode->camera_interact((int)delta.x, (int)delta.y);
					}
				}
				else
				{
					tumbling = false;
				}
			}

			ImGuizmo::Enable(!tumbling);
			manipulation_gizmos(edit_state);

			if (gui::IsWindowFocused())
			{
				ImGui::PushStyleColor(ImGuiCol_Border, gui::GetStyle().Colors[ImGuiCol_Button]);
				ImGui::RenderFrameEx(gui::GetItemRectMin(), gui::GetItemRectMax(), true, 0.0f, 2.0f);
				ImGui::PopStyleColor();
			}

			if (gui::IsMouseReleased(1) || gui::IsMouseReleased(2))
			{
				cursorManager.set_cursor(CursorManager::Cursor::Arrow);
			}

		}

		if (gui::IsWindowHovered())
		{
#			if 0
				if (dragged)
				{
					math::vec3 projected_pos;

					if (gui::IsMouseReleased(gui::drag_button))
					{
						auto cursor_pos = gui::GetMousePos();
						camera_component->get_camera().viewport_to_world(
							math::vec2{ cursor_pos.x, cursor_pos.y },
							math::plane::fromPointNormal(math::vec3{ 0.0f, 0.0f, 0.0f }, math::vec3{ 0.0f, 1.0f, 0.0f }),
							projected_pos,
							false);
					}

					if (dragged.is_type<runtime::Entity>())
					{
						gui::SetMouseCursor(ImGuiMouseCursor_Move);
						if (gui::IsMouseReleased(gui::drag_button))
						{
							auto dragged_entity = dragged.get_value<runtime::Entity>();
							dragged_entity.component<TransformComponent>().lock()
								->set_parent(runtime::CHandle<TransformComponent>());

							es->drop();
						}
					}
					if (dragged.is_type<AssetHandle<Prefab>>())
					{
						gui::SetMouseCursor(ImGuiMouseCursor_Move);
						if (gui::IsMouseReleased(gui::drag_button))
						{
							auto prefab = dragged.get_value<AssetHandle<Prefab>>();
							auto object = prefab->instantiate();
							object.component<TransformComponent>().lock()
								->set_position(projected_pos);
							es->drop();
							es->select(object);
						}
					}
					if (dragged.is_type<AssetHandle<Mesh>>())
					{
						gui::SetMouseCursor(ImGuiMouseCursor_Move);
						if (gui::IsMouseReleased(gui::drag_button))
						{
							auto mesh = dragged.get_value<AssetHandle<Mesh>>();
							Model model;
							model.set_lod(mesh, 0);

							auto object = ecs->create();
							//Add component and configure it.
							object.assign<TransformComponent>().lock()
								->set_position(projected_pos);
							//Add component and configure it.
							object.assign<ModelComponent>().lock()
								->set_casts_shadow(true)
								.set_casts_reflection(false)
								.set_model(model);

							es->drop();
							es->select(object);
						}
					}
				}
#			endif
		}
	}

	void RenderingView::update(lab::GraphicsRootWindow & grw)
	{
		if (suspended())
			return;

		grw.make_current();
		_rendererMode->render(width, height);
	}

} // lab
