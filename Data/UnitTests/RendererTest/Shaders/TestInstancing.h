#pragma once

#include "../../../Base/Shaders/Common/Platforms.h"

#include "../../../Base/Shaders/Common/ConstantBufferMacros.h"

struct NS_SHADER_STRUCT nsTestShaderData
{
  FLOAT4(InstanceColor);
  TRANSFORM(InstanceTransform);
};

// this is only defined during shader compilation
#if NS_ENABLED(PLATFORM_SHADER)

StructuredBuffer<nsTestShaderData> instancingData;

#else // C++

NS_CHECK_AT_COMPILETIME(sizeof(nsTestShaderData) == 64);

#endif