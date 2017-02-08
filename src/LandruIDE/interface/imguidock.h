
// adapted from https://github.com/edin-p/ImGuiDock, MIT License
// and also https://github.com/volcoma/EtherealEngine, BSD License

#pragma once
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include <imgui.h>

namespace lab { class GraphicsWindow; class GraphicsWindowManager; class FontManager; }

namespace ImGuiDock
{
	enum class DockSlot { Left, Right, Top, Bottom, Tab, None };

	struct Dock;
	class Dockspace;

	struct Node
	{
		Node* splits[2] = { nullptr, nullptr };
		Node* parent = nullptr;

		// Only one dock is active at a time
		Dock* active_dock = nullptr;
		//Nodes can have multiple tabbed docks
		std::vector<Dock*> docks;

		// What kind of split is it
		bool vertical_split = false;

		bool always_auto_resize = true;

		//size of the node in pixels along its split
		float size = 0;
	};

	struct Dock
	{
		Dock* initialize(const std::string& dtitle, bool dcloseButton, ImVec2 dminSize, 
						 std::function<void(ImVec2)> ddrawFunction)
		{
			title = dtitle;
			close_button = dcloseButton;
			min_size = dminSize;
			drawFunction = ddrawFunction;
			return this;
		};
		~Dock()
		{
			title.clear();
		}

		//Container *parent = nullptr;
		Node* container = nullptr;
		Dockspace* redock_from = nullptr;
		lab::GraphicsWindow* redock_from_window = nullptr;
		Dock* redock_to = nullptr;

		DockSlot redock_slot = DockSlot::None;
		bool close_button = true;
		bool undockable = false;
		bool dragging = false;

		ImVec2 last_size;
		ImVec2 min_size;

		std::string title;
		std::function<void(ImVec2)> drawFunction;
		std::function<bool(void)> on_close_func;
	};

	class Dockspace
	{
	public:
		Dockspace(lab::GraphicsWindow* owner, std::shared_ptr<lab::FontManager> fm);
		~Dockspace();

		bool dock(Dock* dock, DockSlot dockSlot, float size = 0, bool active = false);
		bool dock_with(Dock* dock, Dock* dockTo, DockSlot dockSlot, float size = 0, bool active = false);
		bool undock(Dock* dock);

		void update_and_draw(ImVec2 size, lab::GraphicsWindowManager&);
		void clear();
		bool has_dock(const std::string& name);
		Node node;
		std::vector<Node*> nodes;
	protected:
		friend class lab::GraphicsWindow;

		std::shared_ptr<lab::FontManager> fontManager;

		DockSlot render_dock_slot_preview(const ImVec2& mousePos, const ImVec2& cPos, const ImVec2& cSize);
		void render_tab_bar(Node* container, const ImVec2& size, float height, const ImVec2& cursorPos);
		bool get_min_size(Node* container, ImVec2& min);
		lab::GraphicsWindow* is_any_window_dragged(lab::GraphicsWindowManager&);

		enum DockToAction
		{
			eUndock, eDrag, eClose, eNull
		};

		Dock* _current_dock_to = nullptr;
		DockToAction _current_dock_action = eNull;
	public:
		lab::GraphicsWindow* owner = nullptr;
	};
};