
#include "VirtualMachine.hpp"
#include "LandruLang/LandruAST.h"
#include "LandruLang/LandruLexParse.h"
#include "LandruLang/LandruMachineExemplar.h"

#include <LabText/LabText.h>

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace lvmvi
{
    using std::string;
    using std::vector;
    using std::shared_ptr;
    using std::variant;
    using lab::Text::StrView;

    template<typename T>
    struct CompileResult
    {
        bool valid;
        shared_ptr<T> value;
        char* msg;
    };

    CompileResult<Exemplar::Function> compile_function(shared_ptr<llp::AST> ast)
    {
        if (ast->scope != llp::Scope::function)
            return { false, {}, "ast node must be a function" };

        shared_ptr<Exemplar::Function> fn = std::make_shared<Exemplar::Function>();
        fn->name = ast->name;
        fn->evals = ast->values;
        return { true, fn, "" };
    }

    CompileResult<Exemplar::Variable> compile_declaration(shared_ptr<llp::AST> ast)
    {
        if (ast->scope != llp::Scope::declaration && ast->scope != llp::Scope::assignment)
            return { false, {}, "ast node must be a declaration" };

        if (ast->values.size() != 3)
            return { false, {}, "declaration must have three values" };

        shared_ptr<Exemplar::Variable> var = std::make_shared<Exemplar::Variable>();
        var->name = ast->name;
        var->scope = ast->values[0] == "local" ? Exemplar::Variable::Scope::local : Exemplar::Variable::Scope::global;
        if (ast->values[1] == "int")
        {
            StrView curr{ ast->values[2].c_str(), ast->values[2].size() };
            int32_t val;
            GetInt32(curr, val);
            var->value = val;
            var->type = Exemplar::Variable::Type::int_type;
        }
        else if (ast->values[1] == "float")
        {
            StrView curr{ ast->values[2].c_str(), ast->values[2].size() };
            float val;
            GetFloat(curr, val);
            var->value = val;
            var->type = Exemplar::Variable::Type::float_type;
        }
        else
        {
            var->value = ast->values[2];
            var->type = Exemplar::Variable::Type::string_type;
        }
        return { true, var, "" };
    }

    CompileResult<Exemplar::State> compile_state(shared_ptr<llp::AST> ast)
    {
        if (ast->scope != llp::Scope::state)
            return { false, {}, "ast node must be a machine" };

        std::shared_ptr<Exemplar::State> state = std::make_shared<Exemplar::State>();

        state->name = ast->name;

        for (auto& i : ast->children)
        {
            switch (i->scope)
            {
            case llp::Scope::declaration:
            {
                for (auto c : i->children)
                {
                    CompileResult<Exemplar::Variable> result = compile_declaration(c);
                    if (!result.valid)
                        return { false, {}, "Could not compile declaration" };

                    state->variables.push_back(result.value);
                }
                break;
            }
            case llp::Scope::state:
            {
                CompileResult<Exemplar::State> result = compile_state(i);
                if (!result.valid)
                    return result;

                state->states.push_back(result.value);
                break;
            }

            case llp::Scope::assignment:
            {
                if (i->children.size() < 1)
                    return { false, {}, "assignment cannot be empty" };

                CompileResult<Exemplar::Function> result = compile_function(i->children[0]);
                if (!result.valid)
                    return { false, {}, "Could not compile assignment function" };

                shared_ptr<Exemplar::Statement> statement = std::make_shared<Exemplar::Statement>();
                statement->target = i->name;
                statement->function = result.value;
                state->statements.push_back(statement);
                break;
            }

            case llp::Scope::function:
            {
                CompileResult<Exemplar::Function> result = compile_function(i);
                if (!result.valid)
                    return { false, {}, "Could not require on's functional condition" };

                shared_ptr<Exemplar::Statement> statement = std::make_shared<Exemplar::Statement>();
                statement->function = result.value;
                state->statements.push_back(statement);
                break;
            }

            case llp::Scope::statement:
                if (i->name == "on")
                {
                    if (i->children.size() < 1)
                        return { false, {}, "on requires a functional condition" };
                    
                    CompileResult<Exemplar::Function> result = compile_function(i->children[0]);
                    if (!result.valid)
                        return { false, {}, "Could not require on's functional condition" };

                    shared_ptr<Exemplar::Statement> statement = std::make_shared<Exemplar::Statement>();
                    statement->function = result.value;
                    state->statements.push_back(statement);
                }
                else if (i->name == "goto")
                {
                    if (i->values.size() < 1)
                        return { false, {}, "Goto requires a target" };

                    shared_ptr<Exemplar::Statement> st = std::make_shared<Exemplar::Statement>();
                    std::shared_ptr<Exemplar::Function> fn = std::make_shared<Exemplar::Function>();
                    fn->name = "goto";
                    fn->evals.push_back(i->values[0]);
                    st->function = fn;
                    state->statements.push_back(st);
                }
                break;

            default:
                return { false, {}, "Unexpected scope encountered" };
            }
        }
        return { true, state, "" };
    }

    CompileResult<Exemplar::Machine> compile_machine(shared_ptr<llp::AST> ast)
    {
        if (ast->scope != llp::Scope::machine)
            return { false, {}, "ast node must be a machine" };

        std::shared_ptr<Exemplar::Machine> machine = std::make_shared<Exemplar::Machine>();
        machine->name = ast->name;
        for (auto& i : ast->children)
        {
            switch (i->scope)
            {
                case llp::Scope::require:
                {
                    if (i->values.size())
                        machine->requires.push_back({ i->name, i->values[0] });
                    else
                        machine->requires.push_back({ i->name, i->name });
                    break;
                }
                case llp::Scope::machine:
                {
                    CompileResult<Exemplar::Machine> result = compile_machine(i);
                    if (!result.valid)
                        return result;

                    machine->machines.push_back(result.value);
                    break;
                }
                case llp::Scope::assignment:
                {
                    for (auto c : i->children)
                    {
                        CompileResult<Exemplar::Variable> result = compile_declaration(c);
                        if (!result.valid)
                            return { false, {}, "Could not compile declaration" };

                        machine->variables.push_back(result.value);
                    }
                    break;
                }
                case llp::Scope::declaration:
                {
                    for (auto c : i->children)
                    {
                        CompileResult<Exemplar::Variable> result = compile_declaration(c);
                        if (!result.valid)
                            return { false, {}, "Could not compile declaration" };

                        machine->variables.push_back(result.value);
                    }
                    break;
                }
                case llp::Scope::state:
                {
                    CompileResult<Exemplar::State> result = compile_state(i);
                    if (!result.valid)
                        return { false, {}, "Could not compile state" };

                    machine->states.push_back(result.value);
                    break;
                }
                default:
                    return { false, {}, "Unexpected scope encountered" };
            }
        }

        return { true, machine, "" };
    }

}

LandruMachineExemplar* landru_compile(LandruAST* ast)
{
    lvmvi::CompileResult<lvmvi::Exemplar::Machine> result = lvmvi::compile_machine(ast->ast);
    if (!result.valid)
        return nullptr;

    LandruMachineExemplar* machine = new LandruMachineExemplar();
    machine->machine = result.value;
    return machine;
}
