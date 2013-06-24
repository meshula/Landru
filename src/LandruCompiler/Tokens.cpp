
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#include "LandruCompiler/Tokens.h"

namespace Landru
{
	
	
#ifdef TOKEN_DECL
#undef TOKEN_DECL
#endif
#define TOKEN_DECL(a, b) case kToken##a: return b;
	char const* tokenName(TokenId t)
	{
		switch (t)
		{
#include "LandruCompiler/TokenDefs.h"
		}
		return "";
	}
#undef TOKEN_DECL
	
}


