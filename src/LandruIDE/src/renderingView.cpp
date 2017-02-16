
#include "renderingView.h"
#include "renderer.h"
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
						ImGuiWindowFlags_NoMove |
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

//		auto pos = gui::GetWindowPos();
		auto pos = gui::GetCursorScreenPos();
//		pos.x += 20.f;
//		pos.y += 40.f;
		ImVec2 bounds = size;// (size.x - 40.f, size.y - 40.f);
	//	gui::SetNextWindowPos(pos);
		//ImGui::GetWindowDrawList()->AddRectFilled(pos, bounds + pos, ImColor(0, 0, 0, 255));

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


	void manipulation_gizmos()
	{
#if 0
		auto input = core::get_subsystem<runtime::Input>();
		auto es = core::get_subsystem<editor::EditState>();
		auto& selected = es->selection_data.object;
		auto& editor_camera = es->camera;
		auto& operation = es->operation;
		auto& mode = es->mode;

		if (!input->is_mouse_button_down(sf::Mouse::Right) && !gui::IsAnyItemActive() && !imguizmo::is_using())
		{
			if (input->is_key_pressed(sf::Keyboard::W))
			{
				operation = imguizmo::OPERATION::TRANSLATE;
			}
			if (input->is_key_pressed(sf::Keyboard::E))
			{
				operation = imguizmo::OPERATION::ROTATE;
			}
			if (input->is_key_pressed(sf::Keyboard::R))
			{
				operation = imguizmo::OPERATION::SCALE;
				mode = imguizmo::MODE::LOCAL;
			}
			if (input->is_key_pressed(sf::Keyboard::T))
			{
				mode = imguizmo::MODE::LOCAL;
			}
			if (input->is_key_pressed(sf::Keyboard::Y) && operation != imguizmo::OPERATION::SCALE)
			{
				mode = imguizmo::MODE::WORLD;
			}
		}

		if (selected && selected.is_type<runtime::Entity>())
		{
			auto sel = selected.get_value<runtime::Entity>();
			if (sel && sel != editor_camera && sel.has_component<TransformComponent>())
			{
				auto p = gui::GetItemRectMin();
				auto s = gui::GetItemRectSize();
				imguizmo::set_view_rect(p.x, p.y, s.x, s.y);
				auto camera_component = editor_camera.component<CameraComponent>().lock();
				auto transform_component = sel.component<TransformComponent>().lock();
				transform_component->resolve(true);
				auto transform = transform_component->get_transform();
				math::transform_t delta;
				math::transform_t inputTransform = transform;
				float* snap = nullptr;
				if (input->is_key_down(sf::Keyboard::LControl))
				{
					if (operation == imguizmo::OPERATION::TRANSLATE)
						snap = &es->snap_data.translation_snap[0];
					else if (operation == imguizmo::OPERATION::ROTATE)
						snap = &es->snap_data.rotation_degree_snap;
					else if (operation == imguizmo::OPERATION::SCALE)
						snap = &es->snap_data.scale_snap;
				}

				imguizmo::manipulate(
					camera_component->get_camera().get_view(),
					camera_component->get_camera().get_projection(),
					operation,
					mode,
					transform,
					nullptr,
					snap);


				transform_component->set_transform(transform);
			}
		}
#endif
	}

	void handle_camera_movement()
	{
#if 0
		if (!gui::IsWindowFocused())
			return;

		auto es = core::get_subsystem<editor::EditState>();
		auto input = core::get_subsystem<runtime::Input>();
		auto engine = core::get_subsystem<runtime::Engine>();

		auto& editor_camera = es->camera;
		auto dt = engine->get_delta_time().count();

		auto transform = editor_camera.component<TransformComponent>().lock();
		float movement_speed = 5.0f;
		float rotation_speed = 0.2f;
		float multiplier = 5.0f;
		iPoint delta_move = input->get_cursor_delta_move();

		if (input->is_mouse_button_down(sf::Mouse::Middle))
		{
			if (input->is_key_down(sf::Keyboard::LShift))
			{
				movement_speed *= multiplier;
			}

			if (delta_move.x != 0)
			{
				transform->move_local({ -1 * delta_move.x * movement_speed * dt, 0.0f, 0.0f });
			}
			if (delta_move.y != 0)
			{
				transform->move_local({ 0.0f, delta_move.y * movement_speed * dt, 0.0f });
			}
		}

		if (input->is_mouse_button_down(sf::Mouse::Right))
		{
			if (input->is_key_down(sf::Keyboard::LShift))
			{
				movement_speed *= multiplier;
			}

			if (input->is_key_down(sf::Keyboard::W))
			{
				transform->move_local({ 0.0f, 0.0f, movement_speed * dt });
			}

			if (input->is_key_down(sf::Keyboard::S))
			{
				transform->move_local({ 0.0f, 0.0f, -movement_speed * dt });
			}

			if (input->is_key_down(sf::Keyboard::A))
			{
				transform->move_local({ -movement_speed * dt, 0.0f, 0.0f });
			}

			if (input->is_key_down(sf::Keyboard::D))
			{
				transform->move_local({ movement_speed * dt, 0.0f, 0.0f });
			}
			if (input->is_key_down(sf::Keyboard::Up))
			{
				transform->move_local({ 0.0f, 0.0f, movement_speed * dt });
			}

			if (input->is_key_down(sf::Keyboard::Down))
			{
				transform->move_local({ 0.0f, 0.0f, -movement_speed * dt });
			}

			if (input->is_key_down(sf::Keyboard::Left))
			{
				transform->move_local({ -movement_speed * dt, 0.0f, 0.0f });
			}

			if (input->is_key_down(sf::Keyboard::Right))
			{
				transform->move_local({ movement_speed * dt, 0.0f, 0.0f });
			}

			if (input->is_key_down(sf::Keyboard::Space))
			{
				transform->move_local({ 0.0f, movement_speed * dt, 0.0f });
			}

			if (input->is_key_down(sf::Keyboard::LControl))
			{
				transform->move_local({ 0.0f, -movement_speed * dt, 0.0f });
			}

			float x = static_cast<float>(delta_move.x);
			float y = static_cast<float>(delta_move.y);

			// Make each pixel correspond to a quarter of a degree.
			float dx = x * rotation_speed;
			float dy = y * rotation_speed;

			transform->resolve(true);
			transform->rotate(0.0f, dx, 0.0f);
			transform->rotate_local(dy, 0.0f, 0.0f);


			float delta_wheel = input->get_mouse_wheel_scroll_delta_move();
			transform->move_local({ 0.0f, 0.0f, 14.0f * movement_speed * delta_wheel * dt });
		}
#endif
	}

	void RenderingView::render_ui(lab::CursorManager& cursorManager, lab::FontManager& fontManager, ImVec2 area)
	{
#		if 0
			auto es = core::get_subsystem<editor::EditState>();
			auto engine = core::get_subsystem<runtime::Engine>();
			auto ecs = core::get_subsystem<runtime::EntityComponentSystem>();
			auto input = core::get_subsystem<runtime::Input>();

			auto window = engine->get_focused_window();
			auto& editor_camera = es->camera;
			auto& selected = es->selection_data.object;
			auto& dragged = es->drag_data.object;

			bool has_edit_camera = editor_camera
				&& editor_camera.has_component<CameraComponent>()
				&& editor_camera.has_component<TransformComponent>();
#		endif


#		if 0
			if (!has_edit_camera)
				return;
#		endif

		auto size = gui::GetContentRegionAvail();

		width = size.x;
		height = size.y;

		auto pos = gui::GetCursorScreenPos();
		draw_view_content(fontManager, size);

		show_statistics(fontManager, 90);// engine->get_fps());

#		if 0
			auto camera_component = editor_camera.component<CameraComponent>().lock();
#		endif

		if (size.x > 0 && size.y > 0)
		{
#if 0
			camera_component->get_camera().set_viewport_pos({ static_cast<uint32_t>(pos.x), static_cast<uint32_t>(pos.y) });
			camera_component->set_viewport_size({ static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) });

			const auto surface = camera_component->get_output_buffer();
			gui::Image(surface, size);
#endif

			if (gui::IsItemClicked(1) || gui::IsItemClicked(2))
			{
				gui::SetWindowFocus();
				cursorManager.hide();
			}

			manipulation_gizmos();
			handle_camera_movement();

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
