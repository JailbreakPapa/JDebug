#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(nsMotionBlurConstants, 3)
{
  FLOAT1(MotionBlurStrength);
};
