#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(nsSelectionHighlightConstants, 3)
{
  COLOR4F(HighlightColor);
  FLOAT1(OverlayOpacity);
};