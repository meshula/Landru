
// Copyright (c) 2013 Nick Porcino, All rights reserved.
// License is MIT: http://opensource.org/licenses/MIT

#pragma once

#include "LandruCompiler/Parser.h"
#include <vector>
#include <string>

int parseExpression(CurrPtr& curr, EndPtr end, std::vector<std::string>& outposVec);
