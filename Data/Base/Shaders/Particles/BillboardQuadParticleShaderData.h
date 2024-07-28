#pragma once

#include "BaseParticleShaderData.h"
#include "ParticleSystemConstants.h"

struct NS_SHADER_STRUCT nsBillboardQuadParticleShaderData
{
  FLOAT3(Position);
  PACKEDHALF2(RotationOffset, RotationSpeed, RotationOffsetAndSpeed);
};

// this is only defined during shader compilation
#if NS_ENABLED(PLATFORM_SHADER)

StructuredBuffer<nsBillboardQuadParticleShaderData> particleBillboardQuadData;

#else // C++

NS_CHECK_AT_COMPILETIME(sizeof(nsBillboardQuadParticleShaderData) == 16);

#endif
