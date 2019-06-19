#ifndef LANDRUAST_H
#define LANDRUAST_H

namespace llp
{
    struct AST;
}

struct LandruAST
{
    uint32_t magic = 0xcafef00d;
    std::shared_ptr<llp::AST> ast;
    static bool valid(LandruAST* ast)
    {
        return ast->magic = 0xcafef00d;
    }
    ~LandruAST()
    {
        magic = 0xdddddddd; // thwart magic
    }
};

#endif
