
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#ifndef LANDRU_TOKENS_H
#define LANDRU_TOKENS_H

namespace Landru
{
#ifdef TOKEN_DECL
#undef TOKEN_DECL
#endif
#define TOKEN_DECL(a,b) kToken##a, 
    enum TokenId
    {
#include "Landrucompiler/TokenDefs.h"
    };
#undef TOKEN_DECL

	char const* tokenName(TokenId t);
}

#endif
