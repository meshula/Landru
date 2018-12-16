
#include "modes.h"

namespace lab
{
	using namespace std;

	void ModeManager::add_mode(std::shared_ptr<Mode> m)
    {
		if (dynamic_cast<MinorMode*>(m.get()) != nullptr)
			_minorModes[m->name()] = dynamic_pointer_cast<MinorMode>(m);
		else if (dynamic_cast<MajorMode*>(m.get()) != nullptr)
			_majorModes[m->name()] = dynamic_pointer_cast<MajorMode>(m);
    }

	void ModeManager::update(lab::GraphicsRootWindow & grw)
	{
		for (auto i : _minorModes)
			i.second->update(grw);
		for (auto i : _majorModes)
			i.second->update(grw);
	}

	std::shared_ptr<Mode> ModeManager::find_mode(const std::string & m)
	{
		auto maj = _majorModes.find(m);
		if (maj != _majorModes.end())
			return maj->second;
		auto mnr = _minorModes.find(m);
		if (mnr != _minorModes.end())
			return mnr->second;
		return std::shared_ptr<Mode>();
	}


}
