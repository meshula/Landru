
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruCompiler/Tokens.h"

typedef char const* CurrPtr;
typedef char const*const EndPtr;

bool peekIsLiteral(CurrPtr& curr, EndPtr end);
int literalLength(CurrPtr& curr, EndPtr end);

namespace Landru {
    TokenId getNameSpacedToken(CurrPtr& curr, EndPtr end);
    TokenId getToken(CurrPtr& curr, EndPtr end);
}