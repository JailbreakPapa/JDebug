#pragma once

#include "ParticleSystemConstants.h"

struct WD_SHADER_STRUCT wdBaseParticleShaderData
{
  PACKEDCOLOR4H(Color);
  PACKEDHALF2(Life, Size, LifeAndSize); // Life: 1 to 0
  UINT1(Variation); // only lower 8 bit
};

// this is only defined during shader compilation
#if WD_ENABLED(PLATFORM_SHADER)

StructuredBuffer<wdBaseParticleShaderData> particleBaseData;

#else // C++

WD_CHECK_AT_COMPILETIME(sizeof(wdBaseParticleShaderData) == 16);

#endif

