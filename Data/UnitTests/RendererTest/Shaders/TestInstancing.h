#pragma once

#include "../../../Base/Shaders/Common/Platforms.h"

#include "../../../Base/Shaders/Common/ConstantBufferMacros.h"

struct WD_SHADER_STRUCT wdTestShaderData
{
  FLOAT4(InstanceColor);
  TRANSFORM(InstanceTransform);
};

// this is only defined during shader compilation
#if WD_ENABLED(PLATFORM_SHADER)

StructuredBuffer<wdTestShaderData> instancingData;

#else // C++

WD_CHECK_AT_COMPILETIME(sizeof(wdTestShaderData) == 64);

#endif