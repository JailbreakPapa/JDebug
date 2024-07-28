#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

#define USE_GGX 1
#define BLOCK_SIZE 16
#define MAX_STEPS 16
#define BINARY_SEARCH_STEPS 16

/// Mirror reflection constants. most of these we wont need to use, as SSR isnt as complex as SSAO.
CONSTANT_BUFFER(nsSSRConstants, 3)
{
  FLOAT1(SSRRayStep);
  FLOAT1(SSRRayHitThreshold);
  UINT1(depthIdx);
  UINT1(normalIdx);
  UINT1(diffuseIdx);
  UINT1(sceneIdx);
  UINT1(outputIdx);
};
