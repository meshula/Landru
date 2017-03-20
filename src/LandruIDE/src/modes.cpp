
#include "modes.h"

namespace lab
{
	void ModeManager::add_mode(std::shared_ptr<MinorMode> m)
    {
        _minorModes[m->name()] = m;
    }

	void ModeManager::add_mode(std::shared_ptr<MajorMode> m)
    {
        _majorModes[m->name()] = m;
    }

	void ModeManager::update(lab::GraphicsRootWindow & grw)
	{
		for (auto i : _minorModes)
			i.second->update(grw);
	}

	Mode * ModeManager::find_mode(const std::string & m)
	{
		auto maj = _majorModes.find(m);
		if (maj != _majorModes.end())
			return maj->second.get();
		auto mnr = _minorModes.find(m);
		if (mnr != _minorModes.end())
			return mnr->second.get();
		return nullptr;
	}


}
