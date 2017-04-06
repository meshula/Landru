

#pragma once

#include "graphMode.h"

namespace lab
{


    class Sample_GraphNodeFactory : public GraphNodeFactory
    {
        class Detail;
        Detail * _detail;

    public:
        Sample_GraphNodeFactory();
        ~Sample_GraphNodeFactory();

        virtual const char ** node_type_names() override;
        virtual size_t node_type_name_count() const override;
        virtual std::function <ImGui::Node * (int, const ImVec2&)> node_factory() override;
		virtual std::vector<std::pair<int, int>> node_limits() override;

    };


} // lab
