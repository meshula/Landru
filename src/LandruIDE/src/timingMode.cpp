
#include "timingMode.h"

#include <chrono>

namespace lab
{

    using namespace std::chrono_literals;
    constexpr std::chrono::nanoseconds timestep(16ms);


    void TimingMode::update(lab::GraphicsRootWindow&)
    {
        using clock = std::chrono::high_resolution_clock;
        static auto time_start = clock::now();
        static std::chrono::nanoseconds lag(0ns);

        auto delta_time = clock::now() - time_start;
        time_start = clock::now();
        lag += std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);

        while (lag >= timestep)
        {
            lag -= timestep;
			evt_timeline_update(time_start);
            time_start += timestep;
        }

        // calculate residual fraction
        //auto alpha = (float) lag.count / timestep.count;
    }

} // lab
