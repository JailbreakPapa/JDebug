#pragma once

#include "ParticleSystemConstants.h"
#include "BaseParticleShaderData.h"

struct WD_SHADER_STRUCT wdTrailParticleShaderData
{
  INT1(NumPoints);
  FLOAT3(dummy);
};

struct WD_SHADER_STRUCT wdTrailParticlePointsData8
{
  FLOAT4(Positions[8]);
};

struct WD_SHADER_STRUCT wdTrailParticlePointsData16
{
  FLOAT4(Positions[16]);
};

struct WD_SHADER_STRUCT wdTrailParticlePointsData32
{
  FLOAT4(Positions[32]);
};

struct WD_SHADER_STRUCT wdTrailParticlePointsData64
{
  FLOAT4(Positions[64]);
};

// this is only defined during shader compilation
#if WD_ENABLED(PLATFORM_SHADER)

StructuredBuffer<wdTrailParticleShaderData> particleTrailData;

#if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT8
  StructuredBuffer<wdTrailParticlePointsData8> particlePointsData;
#endif

#if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT16
  StructuredBuffer<wdTrailParticlePointsData16> particlePointsData;
#endif

#if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT32
  StructuredBuffer<wdTrailParticlePointsData32> particlePointsData;
#endif

#if PARTICLE_TRAIL_POINTS == PARTICLE_TRAIL_POINTS_COUNT64
  StructuredBuffer<wdTrailParticlePointsData64> particlePointsData;
#endif

#else // C++

#endif

