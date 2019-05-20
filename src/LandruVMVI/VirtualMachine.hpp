#pragma once

#include <LabText/LabText.h>
#include <memory>
#include <string>
#include <vector>
#include <variant>

namespace lvmvi
{
    using std::string;
    using std::vector;
    using std::shared_ptr;
    using std::variant;
    using lab::Text::StrView;

    namespace Exemplar
    {
        struct Require
        {
            string name;
            string module_name;
        };

        struct Function
        {
            string name;
            vector<string> evals;
        };

        struct Statement
        {
            string target;
            shared_ptr<Function> function;
        };

        struct Variable
        {
            enum class Scope { self, local, global };
            Scope             scope = Scope::self;
            string            name;
            enum class Type { int_type, float_type, string_type };
			Type              type = Type::int_type;
			variant<int, float, string> value = int(0);
        };

        struct State
        {
            string            name;
            vector<shared_ptr<Variable>>  variables;
            vector<shared_ptr<State>>     states;

            vector<shared_ptr<Statement>> statements;
        };

        struct Machine
        {
            vector<Require>   requires;

            string            name;
            vector<shared_ptr<Variable>>  variables;
            vector<shared_ptr<State>>     states;

            vector<shared_ptr<Machine>>   machines;
        };
    } // Exemplar

} // namespace
