#pragma once

#include <memory>

namespace llp
{
    struct AST;
}

namespace lvmvi
{
    namespace Exemplar
    {
        struct Machine;
    }

    struct ExecutionContext;
}

std::shared_ptr<lvmvi::Exemplar::Machine> landru_compile(std::shared_ptr<llp::AST>);
void landru_machine_print(std::shared_ptr<lvmvi::Exemplar::Machine>);
void landru_machine_instance(std::shared_ptr<lvmvi::ExecutionContext>, std::shared_ptr<lvmvi::Exemplar::Machine>);
