#pragma once

#include "BaseParticleShaderData.h"
#include "ParticleSystemConstants.h"

struct NS_SHADER_STRUCT nsTrailParticleShaderData
{
  INT1(NumPoints);
  FLOAT3(dummy);
};

struct NS_SHADER_STRUCT nsTrailParticlePointsData8
{
  FLOAT4(Positions[8]);
};

struct NS_SHADER_STRUCT nsTrailParticlePointsData16
{
  FLOAT4(Positions[16]);
};

struct NS_SHADER_STRUCT nsTrailParticlePointsData32
{
  FLOAT4(Positions[32]);
};

struct NS_SHADER_STRUCT nsTrailParticlePointsData64
{
  FLOAT4(Positions[64]);
};

// this is only defined during shader compilation
#if NS_ENABLED(PLATFORM_SHADER)

StructuredBuffer<nsTrailParticleShaderData> particleTrailData;

#  if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT8
StructuredBuffer<nsTrailParticlePointsData8> particlePointsData;
#  endif

#  if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT16
StructuredBuffer<nsTrailParticlePointsData16> particlePointsData;
#  endif

#  if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT32
StructuredBuffer<nsTrailParticlePointsData32> particlePointsData;
#  endif

#  if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT64
StructuredBuffer<nsTrailParticlePointsData64> particlePointsData;
#  endif

#else // C++

#endif
