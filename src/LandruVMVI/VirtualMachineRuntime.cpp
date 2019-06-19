
#include "LandruVMVI/VirtualMachine.hpp"
#include "LandruVMVI/Library.h"
#include "LandruVMVI/StateContext.h"
#include "LandruLang/LandruMachineExemplar.h"
#include "LandruStd/IoLib.h"
#include "Landru/Landru.h"

#include <entt/entt.hpp>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <variant>
#include <vector>

using std::map;
using std::mutex;
using std::optional;
using std::scoped_lock;
using std::set;
using std::string;
using std::shared_ptr;
using std::variant;
using std::vector;

using entt::entity;

using namespace lvmvi::Exemplar;


namespace
{
    void findAndReplaceAll(std::string& data, std::string toSearch, std::string replaceStr)
    {
        // Get the first occurrence
        size_t pos = data.find(toSearch);

        // Repeat till end is reached
        while (pos != std::string::npos)
        {
            // Replace this occurrence of Sub String
            data.replace(pos, toSearch.size(), replaceStr);
            // Get the next occurrence from the current position
            pos = data.find(toSearch, pos + replaceStr.size());
        }
    }
}


/*

    Entity machine_class
        +-- machineClassC
        +-- nameC
        +-- varsC
        +-- statesC

    Entity machine
        +-- machineC
        +-- nameC
        +-- varsC
        +-- stateE

    Entity state
        +-- stateC
        +-- nameC
        +-- varsC
        +-- statementsC

 */

namespace lvmvi
{
    struct MachineRen 
    {
        entity state_e;
    };
    struct MachineClassRen {};
    struct StateRen {};

    struct Var
    {
        Exemplar::Variable::Type type;
        variant<int, float, string> value;
    };

    struct VarsRen
    {
        explicit VarsRen(map<string, Var>&& v)
        {
            std::swap(v, vars);
        }
        explicit VarsRen() = default;
        VarsRen(VarsRen&& rh)
        {
            std::swap(vars, rh.vars);
        }
        VarsRen& operator = (VarsRen&& rh)
        {
            std::swap(vars, rh.vars);
            return *this;
        }
        map<string, Var> vars;
    };

    struct NameRen
    {
        std::string name;
    };

    struct Statement
    {
        int argc;
        Library::Fn fn;
    };

    struct CompiledStateData
    {
        map<string, Var> vars;
        vector<Statement> exec;
    };

    struct LaunchRecord
    {
        entt::entity machine_e;
        string state;
    };

    struct ExecutionContext
    {
        ~ExecutionContext()
        {
        }

        entt::registry registry;
        mutex registry_mutex;

        map<string, map<string, CompiledStateData>> compiled_state_cache;
        map<string, shared_ptr<lvmvi::Library>> libraries;

        map<string, entt::entity> machine_class;
        vector<LaunchRecord> awaiting_launch;   // launch in the next update round
        vector<LaunchRecord> pending_launch;    // after the update round, pending become awaiting

        void remove(const vector<entt::entity> removals)
        {
            for (auto i : removals)
            {
                auto& m = registry.get<MachineRen>(i);
                registry.destroy(m.state_e);
                registry.destroy(i);
            }
        }

        map<string, string> get_requires(const vector<Exemplar::Require>& requires)
        {
            map<string, string> req;
            for (auto& r : requires)
                req[r.name] = r.module_name;
            return req;
        }

        optional<Library::Fn> find_function(const std::string& name)
        {
            vector<StrView> bc = lab::Text::Split({ name.c_str(), name.size() }, '.');

            if (bc.size() == 1)
            {
                // search for function in standard library
                return {};
            }
            else if (bc.size() == 2)
            {
                // search for function in library named by bc[0]
                string lib{ bc[0].curr, bc[0].sz };
                auto lib_it = libraries.find(lib);
                if (lib_it != libraries.end())
                {
                    return lib_it->second->get_fn(string{ bc[1].curr, bc[1].sz });
                }
                else
                {
                    // library not found error
                    return {};
                }
            }
            else
            {
                // it's a nested thing, like in the audio library
            }
            return {};
        }

        void add_state(string prefix, shared_ptr<Exemplar::State> state, map<string, CompiledStateData>& state_data)
        {
            map<string, Var> vars;
            for (auto& i : state->variables)
                vars[i->name] = { i->type, i->value };

            vector<Statement> exec;
            vector<int> stack_depth;
            for (auto& st : state->statements)
            {
                // intercept goto statements before allowing function evaluations
                // because goto is currently abusing the AST.
                if (st->function->name == "goto")
                {
                    std::string next_state = st->function->evals[0];
                    exec.push_back({ 0, [next_state](StateContext& sc, int argc)
                        {
                            sc.next_state = next_state;
                        } });
                    continue;
                }

                int pushed = 0;
                if (st->function->evals.size())
                {
                    for (auto& ev : st->function->evals)
                    {
                        if (ev == "(")
                        {
                            stack_depth.push_back(pushed);
                        }
                        else if (ev == ")")
                        {
                            // ignore; running the following function will clean off the stack
                        }
                        else if (ev.back() == '#')
                        {
                            string name = ev;
                            name.pop_back(); // remove #
                            optional<Library::Fn> fn = find_function(name);
                            int back = stack_depth.back();
                            if (fn)
                                exec.push_back({ back, fn.value() });

                            // push instructions to clean off the stack, in case not all return values were consumed
                            exec.push_back({ 0, [back](StateContext& sc, int argc)
                            {
                                while (sc.stack.size() > back)
                                    sc.stack.pop_back();
                            } });
                            stack_depth.pop_back();
                            pushed = back;
                        }
                        else if (ev.front() == '"')
                        {
                            ++pushed;
                            string val = ev.substr(1, ev.size() - 2);
                            findAndReplaceAll(val, "\\n", "\n");
                            findAndReplaceAll(val, "\\r", "\r");
                            findAndReplaceAll(val, "\\t", "\t");

                            exec.push_back({ 0, [val](StateContext& sc, int argc)
                            {
                                sc.stack.push_back(val);
                            } });
                        }
                        else if (ev.find('.') != std::string::npos)
                        {
                            ++pushed;
                            float val;
                            lab::Text::GetFloat({ ev.c_str(), ev.size() }, val);
                            exec.push_back({ 0, [val](StateContext& sc, int argc)
                            {
                                sc.stack.push_back(val);
                            } });
                        }
                        else if ((ev[0] >= '0' && ev[0] <= '9') || ev[0] == '-')
                        {
                            ++pushed;
                            int val;
                            lab::Text::GetInt32({ ev.c_str(), ev.size() }, val);
                            exec.push_back({ 0, [val](StateContext& sc, int argc)
                            {
                                sc.stack.push_back(val);
                            } });
                        }
                        else
                        {
                            // must be a variable to look up and push on the stack
                            if (vars.find(ev) != vars.end())
                            {
                                // it's a state local variable
                            }
                            else
                            {
                                // traverse all the scopes upwards
                            }
                        }
                    }
                }

                optional<Library::Fn> fn = find_function(st->function->name);
                StrView fnName{ st->function->name.c_str(), st->function->name.size() };
                if (fn)
                {
                    int argc = pushed;
                    exec.push_back({ argc, fn.value() });
                }
                else
                {
                    // function not found error
                }

                if (st->target.length() > 0)
                {
                    // what is the variable
                    // the result must be available from the stack so assign it to the variable
                }
            }

            state_data[prefix + ":" + state->name] = CompiledStateData{ vars, exec };

            for (auto& s : state->states)
                add_state(prefix + ":" + s->name, s, state_data);
        }

        void add_states(shared_ptr<Exemplar::Machine> machine, std::string prefix)
        {
            if (compiled_state_cache.find(machine->name) == compiled_state_cache.end())
            {
                map<string, CompiledStateData> state_map;
                for (auto& s : machine->states)
                    add_state("", s, state_map);

                compiled_state_cache[prefix + "/" + machine->name] = state_map;
            }
            for (auto& i : machine->machines)
            {
                add_states(i, prefix + "/" + machine->name);
            }
        }

        void enter_state(entt::entity machine_e, CompiledStateData& state)
        {
            auto& name = registry.get<NameRen>(machine_e);
            auto states_it = compiled_state_cache.find(name.name);

            if (states_it == compiled_state_cache.end())
                return;

            auto& machine = registry.get<MachineRen>(machine_e);
            auto& state_vars = registry.get<VarsRen>(machine.state_e);

            //clear any outstanding triggers on the current state.
            /* when HFSM is implemented do all this:
            is the new state part of the previous state's subtree?
                cool
            else is the new state deeper?
                cool
            must be shallower
                cancel everything at the same depth
                while not correct depth
                    go up
                    if correct depth break
                    cancel everything at this depth
            */

            // instantiate variables whose lifespan ends when the state is left
            map<string, lvmvi::Var> fresh_state_vars;
            for (auto& i : state.vars)
            {
                fresh_state_vars[i.first] = { i.second.type, i.second.value };
            }
            std::swap(fresh_state_vars, state_vars.vars);

            StateContext sc;
            for (Statement& e : state.exec)
            {
                e.fn(sc, e.argc);
                if (!sc.next_state.empty())
                    break;
            }
            if (!sc.next_state.empty())
            {
                // enter_state is called from the awaiting list; pending will replace waiting in the next epoch
                pending_launch.push_back({ machine_e, sc.next_state });
            }
        }

        entity get_machine_class(std::shared_ptr<Exemplar::Machine> machine_exemplar, const std::string& prefix)
        {
            std::string name = prefix + "/" + machine_exemplar->name;
            entity machine_class_e;
            auto machine_class_it = machine_class.find(name);
            if (machine_class_it != machine_class.end())
                return machine_class_it->second;

            machine_class_e = registry.create();
            registry.assign<MachineClassRen>(machine_class_e);
            registry.assign<NameRen>(machine_class_e, name);
            {
                map<string, Var> shared_vars;

                for (auto& i : machine_exemplar->variables)
                {
                    shared_ptr<Exemplar::Variable> v = std::make_shared<Exemplar::Variable>();
                    *v = *i.get();

                    switch (i->scope)
                    {
                    case Exemplar::Variable::Scope::global:
                        shared_vars[i->name] = { v->type, v->value };
                        break;
                    }
                }

                registry.assign<VarsRen>(machine_class_e, std::move(shared_vars));
            }
            return machine_class_e;
        }

        void launch(shared_ptr<Exemplar::Machine> machine, std::string prefix)
        {
            string prefixed_name = prefix + "/" + machine->name;

            entt::entity machine_e = registry.create();
            entt::entity state_e = registry.create();
            registry.assign<StateRen>(state_e);                 // state data
            registry.assign<VarsRen>(state_e);                  // state local vars
            registry.assign<MachineRen>(machine_e, state_e);    // machine has state
            registry.assign<NameRen>(machine_e, prefixed_name); // machine has name

            // There is one machine class entity per machine class; it's where variables
            // shared across all machine instances are stored.
            // getting the machine class, as a side-effect, will instantiate the shared variables if necessary
            get_machine_class(machine, prefix);

            // instantiate machine local variables
            map<string, Var> machine_local_vars;
            for (auto& i : machine->variables)
            {
                shared_ptr<Exemplar::Variable> v = std::make_shared<Exemplar::Variable>();
                *v = *i.get();

                if (i->scope == Exemplar::Variable::Scope::local)
                    machine_local_vars[i->name] = { v->type, v->value };
            }
            registry.assign<VarsRen>(machine_e, std::move(machine_local_vars)); // machine has local vars

            auto requires = get_requires(machine->requires);
            add_states(machine, prefix);
            awaiting_launch.push_back({ machine_e, ":main" }); // nested machines launch from outer to inner.

            for (auto& i : machine->machines)
            {
                // recurse as many nested mains as there are
                if (i->name == "main")
                    launch(i, prefixed_name);
            }
        }


    };

}

using lvmvi::ExecutionContext;
using lvmvi::MachineRen;
using lvmvi::MachineClassRen;
using lvmvi::NameRen;
using lvmvi::StateRen;
using lvmvi::CompiledStateData;
using lvmvi::VarsRen;

struct LandruEC
{
    uint32_t magic = 0xcafebabe;
    shared_ptr<ExecutionContext> ec;
    static bool valid(LandruEC* ec)
    {
        return ec->magic == 0xcafebabe;
    }
    ~LandruEC()
    {
        magic = 0xdddddddd;
    }
};

EXTERNC
LandruEC* landru_create_execution_context()
{
    LandruEC* w = new LandruEC();
    w->ec = std::make_shared<lvmvi::ExecutionContext>();
    static shared_ptr<lvmvi::IoLib> io_lib = std::make_shared<lvmvi::IoLib>();
    w->ec->libraries[io_lib->name()] = io_lib;
    return w;
}

EXTERNC
void landru_destroy_execution_context(LandruEC* v)
{
    if (LandruEC::valid(v))
        delete v;
}

EXTERNC
void landru_destroy_machine(LandruMachineExemplar* v)
{
    if (LandruMachineExemplar::valid(v))
        delete v;
}


EXTERNC
void landru_machine_instantiate(LandruEC* wcontext, LandruMachineExemplar* wmachine)
{
    if (!LandruEC::valid(wcontext))
        return;

    if (!LandruMachineExemplar::valid(wmachine))
        return;

    using namespace lvmvi;

    std::shared_ptr<lvmvi::ExecutionContext> ec = wcontext->ec;
    std::shared_ptr<Machine> machine = wmachine->machine;
    std::lock_guard lock(ec->registry_mutex);
    ec->launch(machine, "");
}

void landru_runtime_update(LandruEC* ecptr)
{
    if (!LandruEC::valid(ecptr))
        return;

    auto ec = ecptr->ec;
    std::lock_guard lock(ec->registry_mutex);

    vector<entt::entity> removals;

    // this loop can be parallel_for'd.
    //
    for (auto& i : ec->awaiting_launch)
    {
        if (!ec->registry.valid(i.machine_e))
            continue;

        auto &name = ec->registry.get<NameRen>(i.machine_e);
        auto states_it = ec->compiled_state_cache.find(name.name);
        if (states_it == ec->compiled_state_cache.end())
            continue;

        auto& states = states_it->second;
        auto state_it = states.find(i.state);
        if (state_it == states.end())
            continue;

        ec->enter_state(i.machine_e, state_it->second);
    }

    std::swap(ec->awaiting_launch, ec->pending_launch);
    ec->awaiting_launch.clear();
    ec->remove(removals);
}
