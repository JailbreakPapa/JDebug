#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(wdSelectionHighlightConstants, 3)
{
  COLOR4F(HighlightColor);
  FLOAT1(OverlayOpacity);
};

