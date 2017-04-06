
#pragma once

#include "grapheditor.h"
#include <new>

namespace lab
{

    class GraphNode: public ImGui::Node
    {
    public:
		template <typename NodeType>
		static NodeType * new_node() 
		{
			// The graph editor deletes nodes using ImGui::MemFree after expliciting calling the destructor
			// so nodes must be allocated with new_node.
			NodeType * node = (NodeType*)ImGui::MemAlloc(sizeof(NodeType)); 

			new(node) NodeType();
			return node;
		}


    };

} // lab
