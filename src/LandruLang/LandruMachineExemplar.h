#pragma once

#include <memory>

struct LandruMachineExemplar
{
    uint32_t magic = 0xcafed00d;
    std::shared_ptr<lvmvi::Exemplar::Machine> machine;
    static bool valid(LandruMachineExemplar* ec)
    {
        return ec->magic == 0xcafed00d;
    }
    ~LandruMachineExemplar()
    {
        magic = 0xdddddddd;
    }
};

