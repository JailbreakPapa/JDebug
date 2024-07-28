#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(nsChromaticAberrationConstants, 3)
{
  FLOAT1(Strength);
};