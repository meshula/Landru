#pragma once

#include "LandruVMVI/Library.h"
#include "LandruVMVI/StateContext.h"

namespace lvmvi
{
    struct IoLib : public Library
    {
        IoLib() : Library(io_lib_name())
        {
            functions["print"] = [](StateContext& sc, int argc)
            {
                int base = static_cast<int>(sc.stack.size()) - argc;
                if (base < 0)
                {
                    /// @TODO post an error...
                    return;
                }
                for (int i = 0; i < argc; ++i)
                {
                    std::variant<int, float, std::string>& j = sc.stack[base + i];
					if (auto pval = std::get_if<int>(&j))
						printf("%d", *pval);
					else if (auto pval = std::get_if<float>(&j))
						printf("%f", *pval);
					else if (auto pval = std::get_if<std::string>(&j))
						printf("%s", (*pval).c_str());
                }
                sc.stack.resize(sc.stack.size() - argc);
            };
        }

        ~IoLib() = default;

        static char const*const io_lib_name() { return "io"; }
    };
}

