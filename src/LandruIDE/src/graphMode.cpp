

#include "graphMode.h"
#include "interface/graph/graphEditor.h"

#define IMGUI_DEFINE_PLACEMENT_NEW
#include "imgui_internal.h"

namespace lab
{

    enum MyNodeTypes {
        MNT_COLOR_NODE = 0,
        MNT_COMBINE_NODE,
        MNT_COMMENT_NODE,
        MNT_COMPLEX_NODE,
        MNT_OUTPUT_NODE,    // One problem here when adding new values is backward compatibility with old saved files: they rely on the previously used int values (it should be OK only if we append new values at the end).
        MNT_COUNT
    };



    class ColorNode : public ImGui::Node {
        protected:
        typedef Node Base;  //Base Class
        typedef ColorNode ThisClass;
        ColorNode() : Base() {}
        static const int TYPE = MNT_COLOR_NODE;

        ImVec4 Color;       // field

        // Support static method for enumIndex (the signature is the same used by ImGui::Combo(...))
        static bool GetTextFromEnumIndex(void* ,int value,const char** pTxt) {
            if (!pTxt) return false;
            static const char* values[] = {"APPLE","LEMON","ORANGE"};
            static int numValues = (int)(sizeof(values)/sizeof(values[0]));
            if (value>=0 && value<numValues) *pTxt = values[value];
            else *pTxt = "UNKNOWN";
            return true;
        }

        virtual const char* getTooltip() const {return "ColorNode tooltip.";}
        virtual const char* getInfo() const {return "ColorNode info.\n\nThis is supposed to display some info about this node.";}
        /*virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
            // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
            defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(0,75,0,255);defaultTitleBgColorGradientOut = -1.f;
        }*/

        public:

        // create:
        static ThisClass* Create(const ImVec2& pos) {
            // 1) allocation
            // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW (node) ThisClass();

            // 2) main init
            node->init("ColorNode",pos,"","r;g;b;a",TYPE);

            // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addFieldColor(&node->Color.x,true,"Color","color with alpha");

            // 4) set (or load) field values
            node->Color = ImColor(255,255,0,255);

            return node;
        }


        // casts:
        inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
        inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    };
    class CombineNode : public ImGui::Node {
        protected:
        typedef Node Base;  //Base Class
        typedef CombineNode ThisClass;
        CombineNode() : Base() {}
        static const int TYPE = MNT_COMBINE_NODE;

        float fraction;

        virtual const char* getTooltip() const {return "CombineNode tooltip.";}
        virtual const char* getInfo() const {return "CombineNode info.\n\nThis is supposed to display some info about this node.";}
        /*virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
            // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
            defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(0,75,0,255);defaultTitleBgColorGradientOut = -1.f;
        }*/

        public:

        // create:
        static ThisClass* Create(const ImVec2& pos) {
            // 1) allocation
            // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW(node) ThisClass();

            // 2) main init
            node->init("CombineNode",pos,"in1;in2","out",TYPE);

            // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addField(&node->fraction,1,"Fraction","Fraction of in1 that is mixed with in2",2,0,1);

            // 4) set (or load) field values
            node->fraction = 0.5f;

            return node;
        }

        // casts:
        inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
        inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}


    };
    class CommentNode : public ImGui::Node {
        protected:
        typedef Node Base;  //Base Class
        typedef CommentNode ThisClass;
        CommentNode() : Base() {}
        static const int TYPE = MNT_COMMENT_NODE;
        static const int TextBufferSize = 128;

        char comment[TextBufferSize];			    // field 1
        char comment2[TextBufferSize];			    // field 2
        char comment3[TextBufferSize];			    // field 3
        char comment4[TextBufferSize];			    // field 4
        bool flag;                                  // field 5

        virtual const char* getTooltip() const {return "CommentNode tooltip.";}
        virtual const char* getInfo() const {return "CommentNode info.\n\nThis is supposed to display some info about this node.";}
        /*virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
            // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
            defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(0,75,0,255);defaultTitleBgColorGradientOut = -1.f;
        }*/

        public:

        // create:
        static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW(node) ThisClass();

        // 2) main init
        node->init("CommentNode",pos,"","",TYPE);
        node->baseWidthOverride = 200.f;    // (optional) default base node width is 120.f;


        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addFieldTextEdit(		&node->comment[0],TextBufferSize,"Single Line","A single line editable field",ImGuiInputTextFlags_EnterReturnsTrue);
        node->fields.addFieldTextEditMultiline(&node->comment2[0],TextBufferSize,"Multi Line","A multi line editable field",ImGuiInputTextFlags_AllowTabInput,50);
        node->fields.addFieldTextEditMultiline(&node->comment3[0],TextBufferSize,"Multi Line 2","A multi line read-only field",ImGuiInputTextFlags_ReadOnly,50);
        node->fields.addFieldTextWrapped(      &node->comment4[0],TextBufferSize,"Text Wrapped ReadOnly","A text wrapped field");
        node->fields.addField(&node->flag,"Flag","A boolean field");

        // 4) set (or load) field values
        strcpy(node->comment,"Initial Text Line.");
        strcpy(node->comment2,"Initial Text Multiline.");
        static const char* tiger = "Tiger, tiger, burning bright\nIn the forests of the night,\nWhat immortal hand or eye\nCould frame thy fearful symmetry?";
        strncpy(node->comment3,tiger,TextBufferSize);
        static const char* txtWrapped = "I hope this text gets wrapped gracefully. But I'm not sure about it.";
        strncpy(node->comment4,txtWrapped,TextBufferSize);
        node->flag = true;

        return node;
        }

        // helper casts:
        inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
        inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    };
    class ComplexNode : public ImGui::Node {
        protected:
        typedef Node Base;  //Base Class
        typedef ComplexNode ThisClass;
        ComplexNode() : Base() {}
        static const int TYPE = MNT_COMPLEX_NODE;

        float Value[3];     // field 1
        ImVec4 Color;       // field 2
        int enumIndex;      // field 3

        // Support static method for enumIndex (the signature is the same used by ImGui::Combo(...))
        static bool GetTextFromEnumIndex(void* ,int value,const char** pTxt) {
            if (!pTxt) return false;
            static const char* values[] = {"APPLE","LEMON","ORANGE"};
            static int numValues = (int)(sizeof(values)/sizeof(values[0]));
            if (value>=0 && value<numValues) *pTxt = values[value];
            else *pTxt = "UNKNOWN";
            return true;
        }

        virtual const char* getTooltip() const {return "ComplexNode tooltip.";}
        virtual const char* getInfo() const {return "ComplexNode info.\n\nThis is supposed to display some info about this node.";}
        virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
            // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
            defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(125,35,0,255);defaultTitleBgColorGradientOut = -1.f;
        }

        public:

        // create:
        static ThisClass* Create(const ImVec2& pos) {
            // 1) allocation
            // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs.  Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW(node) ThisClass();

            // 2) main init
            node->init("ComplexNode",pos,"in1;in2;in3","out1;out2",TYPE);

            // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addField(&node->Value[0],3,"Angles","Three floats that are stored in radiant units internally",2,0,360,NULL,true);
        node->fields.addFieldColor(&node->Color.x,true,"Color","color with alpha");
            node->fields.addFieldEnum(&node->enumIndex,3,&GetTextFromEnumIndex,"Fruit","Choose your favourite");

            // 4) set (or load) field values
            node->Value[0] = 0;node->Value[1] = 3.14f; node->Value[2] = 4.68f;
            node->Color = ImColor(126,200,124,230);
            node->enumIndex = 1;

            return node;
        }

        // helper casts:
        inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
        inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    };


    class OutputNode : public ImGui::Node {
        protected:
        typedef Node Base;  //Base Class
        typedef OutputNode ThisClass;
        OutputNode() : Base() {}
        static const int TYPE = MNT_OUTPUT_NODE;

        // No field values in this class

        virtual const char* getTooltip() const {return "OutputNode tooltip.";}
        virtual const char* getInfo() const {return "OutputNode info.\n\nThis is supposed to display some info about this node.";}
        virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
            // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
            defaultTitleTextColorOut = IM_COL32(230,180,180,255);defaultTitleBgColorOut = IM_COL32(40,55,55,200);defaultTitleBgColorGradientOut = 0.025f;
        }
        virtual bool canBeCopied() const {return false;}

        public:

        // create:
        static ThisClass* Create(const ImVec2& pos) {
            // 1) allocation
            // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW (node) ThisClass();

            // 2) main init
            node->init("OutputNode",pos,"ch1;ch2;ch3;ch4","",TYPE);

            // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )

            // 4) set (or load) field values

            return node;
        }

        // casts:
        inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
        inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}

        protected:
        bool render(float /*nodeWidth*/)   {
            ImGui::Text("There can be a single\ninstance of this class.\nTry and see if it's true!");
            return false;
        }
    };


    class GraphNodeFactory
    {
    public:
        virtual const char ** node_type_names() = 0;
        virtual size_t node_type_name_count() const = 0;

        virtual std::function <ImGui::Node * (int, const ImVec2&)> node_factory() = 0;
    };

    class Sample_GraphNodeFactory : public GraphNodeFactory
    {
		const char * node_names[MNT_COUNT] = { "Color","Combine","Comment","Complex", "Output" };

    public:
        Sample_GraphNodeFactory()
        {
        }

        virtual const char ** node_type_names() override { return &node_names[0]; }
        virtual size_t node_type_name_count() const override { return MNT_COUNT; }
        virtual std::function <ImGui::Node * (int, const ImVec2&)> node_factory() override
        {
			Sample_GraphNodeFactory * me = this;
			return [me](int nt, const ImVec2 & pos) -> ImGui::Node *
			{
				switch (nt) {
				case MNT_COLOR_NODE: return ColorNode::Create(pos);
				case MNT_COMBINE_NODE: return CombineNode::Create(pos);
				case MNT_COMMENT_NODE: return CommentNode::Create(pos);
				case MNT_COMPLEX_NODE: return ComplexNode::Create(pos);
				case MNT_OUTPUT_NODE: return OutputNode::Create(pos);
				default: return nullptr;
				}
				return nullptr;
			};
		}
    };

    class GraphMode::Detail
    {
    public:
        ImGui::NodeGraphEditor nge;
        std::shared_ptr<GraphNodeFactory> node_factory;

        Detail(std::shared_ptr<GraphNodeFactory> gnf)
        : node_factory(gnf) {}

        void render()
        {
            if (nge.isInited())
            {
                // This adds entries to the "add node" context menu
                // last 2 args can be used to add only a subset of nodes (or to sort their order inside the context menu)
                nge.registerNodeTypes(node_factory->node_type_names(), (int) node_factory->node_type_name_count(),
                                      node_factory->node_factory(), nullptr, -1);

                // restrict the count of output nodes to one
                nge.registerNodeTypeMaxAllowedInstances(MNT_OUTPUT_NODE,1);

                nge.show_style_editor = false;
                nge.show_load_save_buttons = false;
                //--------------------------------------------------------------------------------
            }
            nge.render();
        }
    };

	GraphMode::GraphMode()
	{
		auto gnf = std::make_shared<Sample_GraphNodeFactory>();
		_detail = new Detail(gnf);
	}

	GraphMode::GraphMode(std::shared_ptr<GraphNodeFactory> gnf)
    : _detail(new Detail(gnf))
	{
	}

	GraphMode::~GraphMode()
	{
        delete _detail;
	}


    void GraphMode::ui(lab::EditState& edit_state,
        lab::CursorManager& cursorManager,
        lab::FontManager& fontManager,
        float width, float height)
    {
		_detail->render();
//		ImGui::TestNodeGraphEditor();
	}

} // lab

