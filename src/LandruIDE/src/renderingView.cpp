
#include "renderingView.h"
#include "editState.h"
#include "renderer.h"
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



	RenderingView::RenderingView()
	: _detail(new RenderEngine())
	{
	}

	RenderingView::~RenderingView()
	{
		delete _detail;
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
			gui::Separator();
			gui::Checkbox("Show G-Buffer", &ui_show_gbuffer);
		}
		gui::End();
	}

	void RenderingView::draw_view_content(lab::FontManager& fontManager, const ImVec2& size)
	{
		static std::once_flag once;
		std::call_once(once, [this]()
        {
			_detail->create_scene();
        });

		auto pos = gui::GetCursorScreenPos();
		ImVec2 bounds = size;// (size.x - 40.f, size.y - 40.f);

		ImTextureID tex_id = (ImTextureID) uintptr_t(_detail->output_texture_id());
		gui::Image(tex_id, bounds, ImVec2(0,1), ImVec2(1,0));

		//_detail->save_output_texture("c:\\Projects\\foo.png");

		gui::SetCursorScreenPos(pos);

		// this routine draws the rendered image from the camera, rather than drawing the scene directly
#if 0
		auto input = core::get_subsystem<runtime::Input>();
		auto es = core::get_subsystem<editor::EditState>();
		auto& selected = es->selection_data.object;
		auto& editor_camera = es->camera;

		if (selected.is_type<runtime::Entity>())
		{
			auto sel = selected.get_value<runtime::Entity>();

			if (sel && (editor_camera != sel) && sel.has_component<CameraComponent>())
			{
				const auto selected_camera = sel.component<CameraComponent>().lock();
				const auto& camera = selected_camera->get_camera();
				const auto surface = selected_camera->get_output_buffer();
				const auto view_size = camera.get_viewport_size();

				float factor = std::min(size.x / float(view_size.width), size.y / float(view_size.height)) / 4.0f;
				ImVec2 bounds(view_size.width * factor, view_size.height * factor);
				auto p = gui::GetWindowPos();
				p.x += size.x - bounds.x - 20.0f;
				p.y += size.y - bounds.y - 40.0f;
				gui::SetNextWindowPos(p);
				if (gui::Begin(
					"Camera Preview",
					nullptr,
					ImGuiWindowFlags_NoFocusOnAppearing |
					ImGuiWindowFlags_ShowBorders |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_AlwaysAutoResize))
				{
					gui::Image(surface, bounds);
				}
				gui::End();

				if (input->is_key_pressed(sf::Keyboard::F) && sel.has_component<TransformComponent>())
				{
					auto transform = editor_camera.component<TransformComponent>().lock();
					auto transform_selected = sel.component<TransformComponent>().lock();
					transform_selected->set_transform(transform->get_transform());
				}
			}
		}
#endif
	}


	void RenderingView::manipulation_gizmos(lab::EditState& edit_state)
	{
		auto io = gui::GetIO();
		const bool object_selected = true;

		static float mat[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

		if (object_selected)
		{
			float* snap = nullptr;

			auto p = gui::GetItemRectMin();
			auto s = gui::GetItemRectSize();
			ImGuizmo::SetRect(p.x, p.y, s.x, s.y);

			m44f view = _detail->camera_view();
			m44f proj = _detail->camera_projection(width, height);

			ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
			switch (edit_state.manipulator_mode())
			{
			case EditState::ManipulatorMode::Translate: operation = ImGuizmo::TRANSLATE; break;
			case EditState::ManipulatorMode::Rotate: operation = ImGuizmo::ROTATE; break;
			case EditState::ManipulatorMode::Scale: operation = ImGuizmo::SCALE; break;
			}

			ImGuizmo::Manipulate(
				&view.m00,
				&proj.m00,
				operation,
				ImGuizmo::LOCAL,
				mat,
				nullptr,
				snap);
		}
	}

	void RenderingView::render_ui(lab::EditState& edit_state, 
								  lab::CursorManager& cursorManager, lab::FontManager& fontManager, ImVec2 area)
	{

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
						_detail->camera_interact((int)delta.x, (int)delta.y);
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
#				if 0
					if (input->is_key_pressed(sf::Keyboard::Delete))
					{
						if (selected && selected.is_type<runtime::Entity>())
						{
							auto sel = selected.get_value<runtime::Entity>();
							if (sel != editor_camera)
							{
								sel.destroy();
								es->unselect();
							}
						}
					}

					if (input->is_key_pressed(sf::Keyboard::D))
					{
						if (input->is_key_down(sf::Keyboard::LControl))
						{
							if (selected && selected.is_type<runtime::Entity>())
							{
								auto sel = selected.get_value<runtime::Entity>();
								if (sel != editor_camera)
								{
									auto clone = ecs->create_from_copy(sel);
									clone.component<TransformComponent>().lock()
										->set_parent(sel.component<TransformComponent>().lock()->get_parent(), false, true);
									es->select(clone);
								}
							}
						}
					}
#				endif
			}

			if (gui::IsMouseReleased(1) || gui::IsMouseReleased(2))
			{
				cursorManager.set_cursor(CursorManager::Cursor::Arrow);
			}

			if (ui_show_gbuffer)
			{
#				if 0
					const auto g_buffer = camera_component->get_g_buffer();
					for (std::uint32_t i = 0; i < g_buffer->get_attachment_count(); ++i)
					{
						const auto attachment = g_buffer->get_attachment(i).texture;
						gui::Image(attachment, size);

						if (gui::IsItemClicked(1) || gui::IsItemClicked(2))
						{
							gui::SetWindowFocus();
							window->setMouseCursorVisible(false);
						}
					}
#				endif
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

	void RenderingView::render_scene()
	{
		_detail->render(width, height);
	}

} // lab
