#pragma once

#include "ParticleSystemConstants.h"

struct NS_SHADER_STRUCT nsBaseParticleShaderData
{
  PACKEDCOLOR4H(Color);
  PACKEDHALF2(Life, Size, LifeAndSize); // Life: 1 to 0
  UINT1(Variation);                     // only lower 8 bit
};

// this is only defined during shader compilation
#if NS_ENABLED(PLATFORM_SHADER)

StructuredBuffer<nsBaseParticleShaderData> particleBaseData;

#else // C++

NS_CHECK_AT_COMPILETIME(sizeof(nsBaseParticleShaderData) == 16);

#endif
