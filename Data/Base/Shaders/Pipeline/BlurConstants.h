#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(nsBlurConstants, 3)
{
  INT1(BlurRadius);
};
