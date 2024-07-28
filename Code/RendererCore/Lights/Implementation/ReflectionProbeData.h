#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/TagSet.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderData.h>

struct nsReflectionProbeMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Static,
    Dynamic,

    Default = Static
  };
};
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsReflectionProbeMode);

/// \brief Describes how a cube map should be generated.
struct NS_RENDERERCORE_DLL nsReflectionProbeDesc
{
  nsUuid m_uniqueID;

  nsTagSet m_IncludeTags;
  nsTagSet m_ExcludeTags;

  nsEnum<nsReflectionProbeMode> m_Mode;

  bool m_bShowDebugInfo = false;
  bool m_bShowMipMaps = false;

  float m_fIntensity = 1.0f;
  float m_fSaturation = 1.0f;
  float m_fNearPlane = 0.0f;
  float m_fFarPlane = 100.0f;
  nsVec3 m_vCaptureOffset = nsVec3::MakeZero();
};

using nsReflectionProbeId = nsGenericId<24, 8>;

template <>
struct nsHashHelper<nsReflectionProbeId>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsReflectionProbeId value) { return nsHashHelper<nsUInt32>::Hash(value.m_Data); }

  NS_ALWAYS_INLINE static bool Equal(nsReflectionProbeId a, nsReflectionProbeId b) { return a == b; }
};

/// \brief Render data for a reflection probe.
class NS_RENDERERCORE_DLL nsReflectionProbeRenderData : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsReflectionProbeRenderData, nsRenderData);

public:
  nsReflectionProbeRenderData()
  {
    m_Id.Invalidate();
    m_vHalfExtents.SetZero();
  }

  nsReflectionProbeId m_Id;
  nsUInt32 m_uiIndex = 0;
  nsVec3 m_vProbePosition; ///< Probe position in world space.
  nsVec3 m_vHalfExtents;
  nsVec3 m_vPositiveFalloff;
  nsVec3 m_vNegativeFalloff;
  nsVec3 m_vInfluenceScale;
  nsVec3 m_vInfluenceShift;
};

/// \brief A unique reference to a reflection probe.
struct nsReflectionProbeRef
{
  bool operator==(const nsReflectionProbeRef& b) const
  {
    return m_Id == b.m_Id && m_uiWorldIndex == b.m_uiWorldIndex;
  }

  nsUInt32 m_uiWorldIndex = 0;
  nsReflectionProbeId m_Id;
};
NS_CHECK_AT_COMPILETIME(sizeof(nsReflectionProbeRef) == 8);

template <>
struct nsHashHelper<nsReflectionProbeRef>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsReflectionProbeRef value) { return nsHashHelper<nsUInt64>::Hash(reinterpret_cast<nsUInt64&>(value)); }

  NS_ALWAYS_INLINE static bool Equal(nsReflectionProbeRef a, nsReflectionProbeRef b) { return a.m_Id == b.m_Id && a.m_uiWorldIndex == b.m_uiWorldIndex; }
};

/// \brief Flags that describe a reflection probe.
struct nsProbeFlags
{
  using StorageType = nsUInt8;

  enum Enum
  {
    SkyLight = NS_BIT(0),
    HasCustomCubeMap = NS_BIT(1),
    Sphere = NS_BIT(2),
    Box = NS_BIT(3),
    Dynamic = NS_BIT(4),
    Default = 0
  };

  struct Bits
  {
    StorageType SkyLight : 1;
    StorageType HasCustomCubeMap : 1;
    StorageType Sphere : 1;
    StorageType Box : 1;
    StorageType Dynamic : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsProbeFlags);

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsProbeFlags);
