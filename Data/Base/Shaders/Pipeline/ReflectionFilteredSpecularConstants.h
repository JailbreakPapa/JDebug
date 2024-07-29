#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(nsReflectionFilteredSpecularConstants, 3)
{
  UINT1(MipLevel);
  FLOAT1(Intensity);
  FLOAT1(Saturation);
};