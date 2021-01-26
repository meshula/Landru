
#include "VirtualMachine.hpp"
#include "LandruLang/LandruMachineExemplar.h"
#include <iostream>
#include <string>

namespace lvmvi
{
    using std::cout;

    void machine_print(int indent, std::shared_ptr<lvmvi::Exemplar::Machine> machine);

    void variable_print(int indent, std::shared_ptr<lvmvi::Exemplar::Variable> var)
    {
        std::string pad(indent, ' ');
        cout << pad;
        if (var->scope == Exemplar::Variable::Scope::global)
            cout << "shared ";
        switch (var->type)
        {
            case Exemplar::Variable::Type::int_type: cout << "int "; break;
            case Exemplar::Variable::Type::float_type: cout << "float "; break;
            case Exemplar::Variable::Type::string_type: cout << "string"; break;
        }
        cout << " " << var->name << " = ";
        switch (var->type)
        {
            case Exemplar::Variable::Type::int_type: cout << std::get<int>(var->value); break;
            case Exemplar::Variable::Type::float_type: cout << std::get<float>(var->value); break;
            case Exemplar::Variable::Type::string_type: cout << "\"" << std::get<string>(var->value) << "\""; break;
        }
        cout << "\n";
    }

    void state_print(int indent, std::shared_ptr<lvmvi::Exemplar::State> state)
    {
        std::string pad(indent, ' ');
        cout << pad << "state " << state->name << "\n" << pad << "{\n";

        if (state->variables.size() > 0)
        {
            cout << pad << "declare\n" << pad << "{\n";
            for (auto v: state->variables)
                variable_print(indent + 4, v);
            cout << pad << "}\n";
        }

        for (auto s: state->states)
            state_print(indent + 4, s);

        for (auto s: state->statements)
        {
            cout << pad << "    ";
            if (s->target.size())
                cout << s->target << " = " ;
            size_t evals = s->function->evals.size() - 1;
            if (evals > 1)
            {
                cout << s->function->name << "(\n";
                for (size_t i = 1; i < evals; ++i)
                    cout << pad << "        " << s->function->evals[i] << "\n";
                cout << pad << "     )\n";
            }
            else 
                cout << s->function->name << "()\n";            
        }

        cout << pad << "}\n";
    }

    void machine_print(int indent, std::shared_ptr<lvmvi::Exemplar::Machine> machine)
    {
        std::string pad(indent, ' ');

        if (indent > 0)
            cout << pad << "machine " << machine->name << "\n" << pad << "{\n";

        for (auto& r : machine->requires)
            cout << pad << r.name << " = require \"" << r.module_name << "\"\n";

        if (machine->variables.size() > 0)
        {
            cout << pad << "declare\n" << pad << "{\n";
            for (auto v: machine->variables)
                variable_print(indent + 4, v.second);
            cout << pad << "}\n";
        }

        for (auto s: machine->states)
            state_print(indent + 4, s);

        for (auto m : machine->machines)
            machine_print(indent + 4, m);

        if (indent > 0)
            cout << pad << "}\n";
    }

}

void landru_machine_print(LandruMachineExemplar* machine)
{
    lvmvi::machine_print(0, machine->machine);
}
