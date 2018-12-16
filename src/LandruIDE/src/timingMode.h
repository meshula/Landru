
#include "modes.h"

namespace lab
{
	// the timing mode emits timing updates with the current time
	// whenever the mode's update() is called.
	//
	// things like the timeline listen to the timing mode and update
	// their status accordingly
	//
	extern event<void(std::chrono::steady_clock::time_point&)> evt_timing_update;

    class TimingMode : public MinorMode
    {
    public:
        virtual const char * name() const override { return "timing"; }
        virtual void update(lab::GraphicsRootWindow&);
    };

} // lab
