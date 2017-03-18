
#include "labAppWindow.h"

#include "interface/labFontManager.h"
#include "interface/imguidock.h"
#include "landruConsole.h"
#include "mainToolbar.h"
#include "modes.h"
#include "outliner.h"
#include "propertyPanel.h"
#include "renderingView.h"


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

	class Login_MajorMode : public MajorMode
	{
	public:
		Login_MajorMode(shared_ptr<lab::CursorManager> cm, shared_ptr<lab::FontManager> fm)
		{
			fontMgr = fm;
		}

		virtual const char * name() const override { return "login"; }

		virtual void ui(EditState & es, GraphicsWindowManager & mgr, ImGuiDock::Dockspace & dockspace) override
		{
			lab::SetFont sf(fontMgr->regular_font);

			ImGui::Columns(2);
			ImGui::Separator();
			ImGui::SetColumnOffset(1, 100);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

			ImVec4 col{ 0.3f,0.3f,0.3f,1 };
			ImGui::PushStyleColor(ImGuiCol_FrameBg, col);


			ImGui::Text("Server"); ImGui::NextColumn();
			static char server[256] = "localhost:2323";
			ImGui::PushID(1);
			ImGui::InputText("", server, 256); ImGui::NextColumn();
			ImGui::PopID();
			ImGui::Text("User"); ImGui::NextColumn();
			static char user[256];
			ImGui::PushID(2);
			ImGui::InputText("", user, 256); ImGui::NextColumn();
			ImGui::PopID();
			ImGui::Text("Auth"); ImGui::NextColumn();
			static char pass[256];
			ImGui::PushID(3);
			ImGui::InputText("", pass, 256, ImGuiInputTextFlags_Password); ImGui::NextColumn();
			ImGui::PopID();

			ImGui::PopStyleColor();
			ImGui::PopStyleVar();

			ImGui::Columns(1);

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

	class ChooseProject_MajorMode : public MajorMode
	{
	public:
		ChooseProject_MajorMode(shared_ptr<lab::CursorManager> cm, shared_ptr<lab::FontManager> fm)
		{
			fontMgr = fm;
		}

		virtual const char * name() const override { return "chooseProject"; }


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
        Edit_MajorMode(shared_ptr<RenderingView> rpv, shared_ptr<lab::CursorManager> cm, std::shared_ptr<lab::FontManager> fm)
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

			lab::EditState * es = &editState;
            view_dock->initialize("View", true, ImVec2(), [es, fm, cm, rpv](ImVec2 area) {
                rpv->ui(*es, *cm.get(), *fm.get(), area.x, area.y);
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

		virtual const char * name() const override { return "edit"; }


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

        Detail(lab::ModeManager & mm, shared_ptr<lab::CursorManager> cm, std::shared_ptr<lab::FontManager> fm)
        {
			view_minorMode = std::make_shared<RenderingView>();
			edit_majorMode = std::make_shared<Edit_MajorMode>(view_minorMode, cm, fm);
			login_majorMode = std::make_shared<Login_MajorMode>(cm, fm);
			chooseProject_majorMode = std::make_shared<ChooseProject_MajorMode>(cm, fm);

			mm.add_mode(edit_majorMode);
			mm.add_mode(login_majorMode);
			mm.add_mode(chooseProject_majorMode);
			mm.add_mode(view_minorMode);

			evt_set_major_mode.connect(this, &AppWindow::Detail::set_major_mode);
			majorMode = login_majorMode.get();
        }

		~Detail()
		{
		}

		void set_major_mode(const std::string& name)
		{
			if (name == "edit")
				majorMode = edit_majorMode.get();
			else if (name == "chooseProject")
				majorMode = chooseProject_majorMode.get();
		}

		MajorMode * majorMode = nullptr;
        std::shared_ptr<Edit_MajorMode> edit_majorMode;
		std::shared_ptr<Login_MajorMode> login_majorMode;
		std::shared_ptr<ChooseProject_MajorMode> chooseProject_majorMode;

		std::shared_ptr<RenderingView> view_minorMode;
	};

	AppWindow::AppWindow(std::shared_ptr<GraphicsRootWindow> grw,
		const std::string & window_name, int width, int height,
		lab::ModeManager & mm,
		std::shared_ptr<lab::CursorManager> cm,
		std::shared_ptr<lab::FontManager> fm)
		: DockingWindow(grw, window_name, width, height, cm, fm)
		, _detail(new Detail(mm, cm, fm))
	{
		ImGuiDock::Dockspace & dockspace = get_dockspace();
        _detail->edit_majorMode->dock(dockspace);
	}

	AppWindow::~AppWindow()
	{
		delete _detail;
	}

	lab::EditState & AppWindow::editState()
	{
		return _detail->edit_majorMode->editState;
	}

	void AppWindow::ui(EditState & es, GraphicsWindowManager & mgr)
	{
		_detail->majorMode->ui(es, mgr, get_dockspace());
	}


} // lab
