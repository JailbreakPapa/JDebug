#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(wdReflectionFilteredSpecularConstants, 3)
{
  UINT1(MipLevel);
  FLOAT1(Intensity);
  FLOAT1(Saturation);
};
