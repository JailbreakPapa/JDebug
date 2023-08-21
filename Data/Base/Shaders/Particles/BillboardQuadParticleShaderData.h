#pragma once

#include "ParticleSystemConstants.h"
#include "BaseParticleShaderData.h"

struct WD_SHADER_STRUCT wdBillboardQuadParticleShaderData
{
  FLOAT3(Position);
  PACKEDHALF2(RotationOffset, RotationSpeed, RotationOffsetAndSpeed);
};

// this is only defined during shader compilation
#if WD_ENABLED(PLATFORM_SHADER)

StructuredBuffer<wdBillboardQuadParticleShaderData> particleBillboardQuadData;

#else // C++

WD_CHECK_AT_COMPILETIME(sizeof(wdBillboardQuadParticleShaderData) == 16);

#endif

