
#include "modes.h"
#include "timeline.h"

namespace lab
{

    class TimingMode : public MinorMode
    {
    public:
        virtual const char * name() const override { return "timing"; }
        virtual void update(lab::GraphicsRootWindow&);
    };

} // lab
