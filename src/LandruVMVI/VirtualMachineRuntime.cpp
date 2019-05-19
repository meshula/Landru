
#include "LandruVMVI/VirtualMachine.hpp"
#include <entt/entt.hpp>
#include <algorithm>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <variant>
#include <vector>

using std::map;
using std::mutex;
using std::scoped_lock;
using std::set;
using std::string;
using std::shared_ptr;
using std::variant;
using std::vector;

namespace lvmvi
{
	struct MachineRen
	{
		string name;
	};
	struct StateRen
	{
		map<string, entt::entity> state_vars;
	};
	struct VarRen
	{
		Exemplar::Variable::Type type;
		variant<int, float, string> value;
	};

	struct StateData
	{
		map<string, shared_ptr<Exemplar::Variable>> state_vars;
	};

    struct ExecutionContext
    {
        entt::registry registry;
        mutex registry_mutex;

        map<entt::entity, map<string, shared_ptr<Exemplar::Variable>>> self_vars;
        map<entt::entity, map<string, shared_ptr<Exemplar::Variable>>> local_vars;
        map<entt::entity, map<string, shared_ptr<Exemplar::Variable>>> global_vars;
        map<string, map<string, StateData>> states;

        vector<entt::entity> awaiting_launch;   // launch in the next update round
        vector<entt::entity> pending_launch;    // after the update round, pending become awaiting

        void remove(const vector<entt::entity> removals)
        {
            for (auto i : removals)
            {
                auto globals_it = global_vars.find(i);
                if (globals_it != global_vars.end())
                    global_vars.erase(globals_it);
                auto shared_it = local_vars.find(i);
                if (shared_it != local_vars.end())
                    local_vars.erase(shared_it);
                auto self_it = self_vars.find(i);
                if (self_it != self_vars.end())
                    self_vars.erase(self_it);
            }
        }

        void add_vars(entt::entity e, vector<shared_ptr<Exemplar::Variable>>& in_vars)
        {
            map<string, shared_ptr<Exemplar::Variable>> svars;
            map<string, shared_ptr<Exemplar::Variable>> lvars;
            map<string, shared_ptr<Exemplar::Variable>> gvars;

            for (auto& i : in_vars)
            {
                shared_ptr<Exemplar::Variable> v = std::make_shared<Exemplar::Variable>();
                *v = *i.get();

                switch (i->scope)
                {
                case Exemplar::Variable::Scope::self:   svars[i->name] = v; break;
                case Exemplar::Variable::Scope::local:  lvars[i->name] = v; break;
                case Exemplar::Variable::Scope::global: gvars[i->name] = v; break;
                }
            }

            self_vars[e] = svars;
            local_vars[e] = lvars;
            global_vars[e] = gvars;
        }

        map<string, string> get_requires(const vector<Exemplar::Require>& requires)
        {
            map<string, string> req;
            for (auto& r: requires)
                req[r.name] = r.module_name;
            return req;
        }

        void add_state(string prefix, shared_ptr<Exemplar::State> state, map<string, StateData>& state_data)
        {
            map<string, shared_ptr<Exemplar::Variable>> vars;
            for (auto& i : state->variables)
                vars[i->name] = i;

            state_data[prefix + ":" + state->name] = { vars };
            for (auto& s : state->states)
                add_state(prefix + ":" + s->name, s, state_data);
        }
        void add_states(shared_ptr<Exemplar::Machine> machine)
        {
            map<string, StateData> mstates;
            for (auto& s : machine->states)
                add_state("", s, mstates);

            states[machine->name] = mstates;
        }
    };




    shared_ptr<ExecutionContext> get_ec()
    {
        static shared_ptr<ExecutionContext> ec = std::make_shared<ExecutionContext>();
        static std::once_flag flag;
        std::call_once(flag, []() 
        {

        });
        return ec;
    }
}

using namespace lvmvi::Exemplar;
using lvmvi::MachineRen;
using lvmvi::StateRen;
using lvmvi::VarRen;

using lvmvi::StateData;

void landru_machine_instance(
    std::shared_ptr<lvmvi::ExecutionContext> ec, 
    std::shared_ptr<lvmvi::Exemplar::Machine> machine)
{
    std::lock_guard lock(ec->registry_mutex);

    entt::entity machine_entity = ec->registry.create();
    ec->registry.assign<MachineRen>(machine_entity, machine->name);
	ec->registry.assign<StateRen>(machine_entity);

	auto requires = ec->get_requires(machine->requires);
    ec->add_vars(machine_entity, machine->variables);
    ec->add_states(machine);
    ec->awaiting_launch.push_back(machine_entity);
}

namespace {
void enter_state(
	std::shared_ptr<lvmvi::ExecutionContext> ec, 
	entt::entity machine_entity, MachineRen& machine, StateData& state)
{
	auto& state_ren = ec->registry.get<StateRen>(machine_entity);
	for (auto& i : state_ren.state_vars)
		ec->registry.destroy(i.second);
	state_ren.state_vars.clear();

	// instantiate variables whose lifespan ends when the state is left
    //
    for (auto& i : state.state_vars)
    {
		entt::entity e = ec->registry.create();
		ec->registry.assign<VarRen>(e, i.second->type, i.second->value);
		state_ren.state_vars[i.first] = e;
    }

    // execute all statements
    //
	#if 0    
	for (auto s : state.statements)
    {
        if (s->target.size())
            cout << s->target << " = ";
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
#endif
}
} // anon

void landru_runtime_update(
    std::shared_ptr<lvmvi::ExecutionContext> ec)
{
    std::lock_guard lock(ec->registry_mutex);

    vector<entt::entity> removals;

    // instantiate all global variables as this must be done serially
    // because if two machines of the same exemplar are instantiated, only one should instantiate the globals
    //
    for (auto i : ec->awaiting_launch)
    {
        if (!ec->registry.valid(i))
            continue;

        auto &machine = ec->registry.get<MachineRen>(i);
        auto globals_it = ec->global_vars.find(i);
        if (globals_it != ec->global_vars.end())
        {

        }
    }

    // everything else can be done concurrently; so this loop can be parallel_for'd.
    //
    for (auto i : ec->awaiting_launch)
    {
        if (!ec->registry.valid(i))
            continue;

        auto &machine = ec->registry.get<MachineRen>(i);

        auto shared_it = ec->local_vars.find(i);
        if (shared_it != ec->local_vars.end())
        {

        }
        auto self_it = ec->self_vars.find(i);
        if (self_it != ec->self_vars.end())
        {

        }
        auto states_it = ec->states.find(machine.name);
        if (states_it == ec->states.end())
            continue;

        auto& states = states_it->second;
        auto main_it = states.find("main");
        if (main_it == states.end())
            continue;

        enter_state(ec, i, machine, main_it->second);
    }

    std::swap(ec->awaiting_launch, ec->pending_launch);
    ec->awaiting_launch.clear();
    ec->remove(removals);
}
