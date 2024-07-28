#pragma once

#include "BaseParticleShaderData.h"

struct NS_SHADER_STRUCT nsTangentQuadParticleShaderData
{
  FLOAT3(Position);
  FLOAT1(dummy1);

  FLOAT3(TangentX);
  FLOAT1(dummy2);

  FLOAT3(TangentZ);
  FLOAT1(dummy3);
};

// this is only defined during shader compilation
#if NS_ENABLED(PLATFORM_SHADER)

StructuredBuffer<nsTangentQuadParticleShaderData> particleTangentQuadData;

#else // C++

NS_CHECK_AT_COMPILETIME(sizeof(nsTangentQuadParticleShaderData) == 48);

#endif
