
#include "labAppWindow.h"

#include "interface/labFontManager.h"
#include "interface/imguidock.h"
#include "mainToolbar.h"
#include "outliner.h"
#include "propertyPanel.h"


using namespace std;

namespace lab
{

	static const int drag_button = 0;

	struct DragObject {
		void * data;
		string name;
		void unselect() {}
		void drop() {}
	};
	static DragObject * drag_object = nullptr;

	event<void(const std::string&)> evt_set_major_mode;


	class MajorMode
	{
	public:
		virtual void ui(EditState & es, GraphicsWindowManager & mgr, ImGuiDock::Dockspace & dockspace) = 0;
	};

	class Login_MajorMode : public MajorMode
	{
	public:
		Login_MajorMode(shared_ptr<lab::CursorManager> cm, shared_ptr<lab::FontManager> fm)
		{
			fontMgr = fm;
		}

		virtual void ui(EditState & es, GraphicsWindowManager & mgr, ImGuiDock::Dockspace & dockspace) override
		{
			lab::SetFont sf(fontMgr->regular_font);

			static char user[256];
			ImGui::InputText("User", user, 256);
			static char pass[256];
			ImGui::InputText("Auth", pass, 256, ImGuiInputTextFlags_Password);
			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				evt_set_major_mode("chooseProject");
			}
		}

		std::shared_ptr<lab::FontManager> fontMgr;
	};

	class ChooseProject_MajorMode : public MajorMode
	{
	public:
		ChooseProject_MajorMode(shared_ptr<lab::CursorManager> cm, shared_ptr<lab::FontManager> fm)
		{
			fontMgr = fm;
		}

		virtual void ui(EditState & es, GraphicsWindowManager & mgr, ImGuiDock::Dockspace & dockspace) override
		{
			lab::SetFont sf(fontMgr->regular_font);

			static char server[256] = "localhost:2323";
			ImGui::InputText("Server", server, 256);
			static char pass[256];
			ImGui::InputText("Auth", pass, 256, ImGuiInputTextFlags_Password);
			ImGui::Separator();
			static char show[256];
			if (ImGui::BeginMenu("Shows"))
			{
				if (ImGui::MenuItem("Sample")) 
				{
					strcpy(show, "Sample");
				}
				if (ImGui::MenuItem("Dune"))
				{
					strcpy(show, "Dune");
				}
				ImGui::EndMenu();
			}
			ImGui::SameLine();
			ImGui::Text(show);
			if (strlen(show) && ImGui::Button("Start", ImVec2(120, 0)))
			{
				evt_set_major_mode("edit");
			}
		}

		std::shared_ptr<lab::FontManager> fontMgr;
	};

    class Edit_MajorMode : public MajorMode
    {
    public:
        Edit_MajorMode(shared_ptr<lab::CursorManager> cm, std::shared_ptr<lab::FontManager> fm)
        {
			fontMgr = fm;

            outliner = new Outliner();
			console = new LandruConsole(fm);
            docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
            docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
            docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
            docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
            docks.emplace_back(std::make_unique<ImGuiDock::Dock>());

            console_dock = docks[0].get();
			view_dock = docks[1].get();
			outliner_dock = docks[2].get();
			properties_dock = docks[3].get();
			timeline_dock = docks[4].get();

            console_dock->initialize("Console", true, ImVec2(400, 200), [this](ImVec2 area)
            {
                this->console->draw_contents();
            });

			lab::RenderingView * rpv = &renderingView;
			lab::EditState * es = &editState;
            view_dock->initialize("View", true, ImVec2(), [es, fm, cm, rpv](ImVec2 area) {
                rpv->render_ui(*es, *cm.get(), *fm.get(), area);
            });

			Outliner * op = outliner;
            outliner_dock->initialize("Outliner", true, ImVec2(100, 100), [op, es, fm](ImVec2 area)
            {
                op->ui(*es, *fm.get());
            });

            properties_dock->initialize("Properties", true, ImVec2(100, 100), [](ImVec2 area)
            {
                property_panel();
            });

            timeline_dock->initialize("Timeline", true, ImVec2(100, 100), [](ImVec2 area)
            {
                ImGui::Text("this is the timeline");
            });
        }

        virtual ~Edit_MajorMode()
        {
            delete console;
            delete outliner;
        }

		void dock(ImGuiDock::Dockspace & dockspace)
		{
			dockspace.dock(view_dock, ImGuiDock::DockSlot::None, 300, true);

			dockspace.dock(console_dock, ImGuiDock::DockSlot::Bottom, 300, true);
			dockspace.dock_with(timeline_dock, console_dock, ImGuiDock::DockSlot::Tab, 250, true);

			dockspace.dock_with(outliner_dock, view_dock, ImGuiDock::DockSlot::Left, 300, true);
			dockspace.dock_with(properties_dock, view_dock, ImGuiDock::DockSlot::Right, 300, true);
		}

		virtual void ui(EditState & es, GraphicsWindowManager & mgr, ImGuiDock::Dockspace & dockspace) override
		{
			lab::toolbar(es, fontMgr.get());
			dockspace.update_and_draw(ImGui::GetContentRegionAvail(), mgr);

			if (ImGui::IsMouseDragging(drag_button) && drag_object)
			{
				ImGui::SetTooltip(drag_object->name.c_str());

				//if (ImGui::GetMouseCursor() == ImGuiMouseCursor_Arrow)
				//					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
			}

			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered())
			{
				/* for when there is imguizmo
				if (ImGui::IsMouseDoubleClicked(0) && !imguizmo::is_over() && drag_object)
				{
				drag_object->unselect();
				drag_object->drop();
				}
				*/
			}
			if (ImGui::IsMouseReleased(drag_button))
			{
				drag_object->drop();
			}
		}



		LandruConsole * console = nullptr;
		Outliner * outliner = nullptr;

		lab::RenderingView renderingView;
		lab::EditState editState;

		std::shared_ptr<FontManager> fontMgr;

		ImGuiDock::Dock * console_dock;
		ImGuiDock::Dock * view_dock;
		ImGuiDock::Dock * outliner_dock;
		ImGuiDock::Dock * properties_dock;
		ImGuiDock::Dock * timeline_dock;

        std::vector<std::unique_ptr<ImGuiDock::Dock>> docks;
    };

    class AppWindow::Detail
    {
    public:

        Detail(shared_ptr<lab::CursorManager> cm, std::shared_ptr<lab::FontManager> fm)
        : edit_majorMode(cm, fm)
		, login_majorMode(cm, fm)
		, chooseProject_majorMode(cm, fm)
        {
			evt_set_major_mode.connect(this, &AppWindow::Detail::set_major_mode);
        }

		~Detail()
		{
		}

		void set_major_mode(const std::string& name)
		{
			if (name == "edit")
				majorMode = &edit_majorMode;
			else if (name == "chooseProject")
				majorMode = &chooseProject_majorMode;
		}

		MajorMode * majorMode = &login_majorMode;
        Edit_MajorMode edit_majorMode;
		Login_MajorMode login_majorMode;
		ChooseProject_MajorMode chooseProject_majorMode;
	};

	AppWindow::AppWindow(const std::string & window_name, int width, int height,
		std::shared_ptr<lab::CursorManager> cm,
		std::shared_ptr<lab::FontManager> fm)
		: GraphicsWindow(window_name, width, height, cm, fm)
		, _detail(new Detail(cm, fm))
	{
		ImGuiDock::Dockspace & dockspace = get_dockspace();
        _detail->edit_majorMode.dock(dockspace);
	}

	AppWindow::~AppWindow()
	{
		delete _detail;
	}

	void AppWindow::render_scene()
	{
		if (_detail->majorMode == &_detail->edit_majorMode)
			_detail->edit_majorMode.renderingView.render_scene(); // width and height were recorded during UI rendering
	}

	lab::EditState & AppWindow::editState()
	{
		return _detail->edit_majorMode.editState;
	}

	void AppWindow::ui(EditState & es, GraphicsWindowManager & mgr)
	{
		_detail->majorMode->ui(es, mgr, get_dockspace());
	}


} // lab
