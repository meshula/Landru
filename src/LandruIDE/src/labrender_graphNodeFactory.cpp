
#include "interface/graph/graphNode.h"
#include "labrender_graphNodeFactory.h"
#include <string>

constexpr int PassNodeId = 1;

#if 0
{ "name": "PostProcessAndBlit",
"type" : { "run": "PostProcess", "draw" : "quad" },
"depth" : { "test": "always", "write" : "no", "clear_buffer" : "no" },
"shader" : { "vertex_shader_path":   "$(ASSET_ROOT)/pipelines/deferred/full-screen-quad-vsh.glsl",
"fragment_shader_path" : "$(ASSET_ROOT)/pipelines/deferred/full-screen-deferred-blit-fsh.glsl",
"uniforms" : [{ "name": "u_normalTexture", "type" : "sampler2d" }],
"varyings" : [{ "name": "v_texCoord", "type" : "vec2" }] },
"inputs" : [{ "buffer": "gbuffer",
"render_textures" : ["diffuse", "position", "normal"] }],
"outputs" : { "buffer": "visible" } }
#endif


namespace lab
{
	using namespace std;

	class PassNode : public GraphNode
	{
		friend class GraphNode;

	protected:
		typedef PassNode ThisClass;
		PassNode() : GraphNode() {}
		static const int TYPE = 1;

		// No field values in this class

		virtual const char* getTooltip() const override { return "labRender Pass"; }
		virtual const char* getInfo() const override { return "labRender Pass"; }
		virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,
			ImU32& defaultTitleBgColorOut,
			float& defaultTitleBgColorGradientOut) const override
		{
			// [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
			defaultTitleTextColorOut = IM_COL32(230, 180, 180, 255);
			defaultTitleBgColorOut = IM_COL32(40, 55, 55, 200);
			defaultTitleBgColorGradientOut = 0.025f;
		}

		virtual bool canBeCopied() const override { return false; }

	public:

		enum class PassType { Generic, Geometry, PostProcess };
		PassType passType = PassType::Generic;
		string passTypeName;

		// create:
		static ThisClass* Create(const ImVec2& pos)
		{
			ThisClass * node = new_node<ThisClass>();
			node->init("Render Pass", pos, "in1;in2;in3", "out1;out2", TYPE);
			node->set_passType(PassType::Generic);
			return node;
		}

		void set_passType(PassType pt)
		{
			passType = pt;
			switch (pt)
			{
			case PassType::Generic: passTypeName = "Generic Pass"; break;
			case PassType::Geometry: passTypeName = "Geometry Pass"; break;
			case PassType::PostProcess: passTypeName = "Post Processing Pass"; break;
			}
		}

		virtual bool render(float /*nodeWidth*/) override
		{
			if (ImGui::BeginMenu(passTypeName.c_str()))
			{
				if (ImGui::MenuItem("Generic Pass"))
				{
					set_passType(PassType::Generic);
				}
				if (ImGui::MenuItem("Geometry Pass"))
				{
					set_passType(PassType::Geometry);
				}
				if (ImGui::MenuItem("Post Processing Pass"))
				{
					set_passType(PassType::PostProcess);
				}
				ImGui::EndMenu();
			}

			return false;
		}
	};

	constexpr int renderNodes = 1;
	static const char * node_names[renderNodes] = { "Pass" };

	enum class RenderNodeTypes { Pass };

	class LabRender_GraphNodeFactory::Detail
	{
	public:
	};

	LabRender_GraphNodeFactory::LabRender_GraphNodeFactory()
		: _detail(new LabRender_GraphNodeFactory::Detail())
	{}

	LabRender_GraphNodeFactory::~LabRender_GraphNodeFactory()
	{
		delete _detail;
	}

	const char ** LabRender_GraphNodeFactory::node_type_names() { return &node_names[0]; }
	size_t LabRender_GraphNodeFactory::node_type_name_count() const { return renderNodes; }

	auto LabRender_GraphNodeFactory::node_factory() -> std::function <ImGui::Node * (int, const ImVec2&)>
	{
		LabRender_GraphNodeFactory * me = this;
		return [me](int nt, const ImVec2 & pos) -> ImGui::Node *
		{
			switch ((RenderNodeTypes) nt) {
			case RenderNodeTypes::Pass: return PassNode::Create(pos);
			default: return nullptr;
			}
			return nullptr;
		};
	}

	std::vector<std::pair<int, int>> LabRender_GraphNodeFactory::node_limits()
	{
		std::vector<std::pair<int, int>> ret;
		//ret.push_back({ (int) RenderNodeTypes::Pass, 1 });
		return ret;
	}


} // lab
