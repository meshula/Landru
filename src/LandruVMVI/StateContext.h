#ifndef LANDRU_STATECONTEXT_H
#define LANDRU_STATECONTEXT_H

#include <string>
#include <variant>
#include <vector>

namespace lvmvi
{
    
    struct StateContext
    {
        std::vector<std::variant<int, float, std::string>> stack;
        std::string next_state;
    };

}
#endif
