
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

}
