#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsReflectionProbeMode, 1)
  NS_BITFLAGS_CONSTANTS(nsReflectionProbeMode::Static, nsReflectionProbeMode::Dynamic)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_BITFLAGS(nsProbeFlags, 1)
  NS_BITFLAGS_CONSTANTS(nsProbeFlags::SkyLight, nsProbeFlags::HasCustomCubeMap, nsProbeFlags::Sphere, nsProbeFlags::Box, nsProbeFlags::Dynamic)
NS_END_STATIC_REFLECTED_BITFLAGS;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsReflectionProbeRenderData, 1, nsRTTIDefaultAllocator<nsReflectionProbeRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_ReflectionProbeData);
