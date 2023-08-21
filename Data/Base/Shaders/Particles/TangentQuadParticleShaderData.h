#pragma once

#include "BaseParticleShaderData.h"

struct WD_SHADER_STRUCT wdTangentQuadParticleShaderData
{
  FLOAT3(Position);
  FLOAT1(dummy1);

  FLOAT3(TangentX);
  FLOAT1(dummy2);

  FLOAT3(TangentZ);
  FLOAT1(dummy3);
};

// this is only defined during shader compilation
#if WD_ENABLED(PLATFORM_SHADER)

StructuredBuffer<wdTangentQuadParticleShaderData> particleTangentQuadData;

#else // C++

WD_CHECK_AT_COMPILETIME(sizeof(wdTangentQuadParticleShaderData) == 48);

#endif

