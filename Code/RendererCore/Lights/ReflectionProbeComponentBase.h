#pragma once

#include <Core/World/Component.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>

struct nsMsgUpdateLocalBounds;
struct nsMsgExtractRenderData;
struct nsMsgTransformChanged;
class nsAbstractObjectNode;

/// \brief Base class for all reflection probes.
class NS_RENDERERCORE_DLL nsReflectionProbeComponentBase : public nsComponent
{
  NS_ADD_DYNAMIC_REFLECTION(nsReflectionProbeComponentBase, nsComponent);

public:
  nsReflectionProbeComponentBase();
  ~nsReflectionProbeComponentBase();

  void SetReflectionProbeMode(nsEnum<nsReflectionProbeMode> mode);           // [ property ]
  nsEnum<nsReflectionProbeMode> GetReflectionProbeMode() const;              // [ property ]

  const nsTagSet& GetIncludeTags() const;                                    // [ property ]
  void InsertIncludeTag(const char* szTag);                                  // [ property ]
  void RemoveIncludeTag(const char* szTag);                                  // [ property ]

  const nsTagSet& GetExcludeTags() const;                                    // [ property ]
  void InsertExcludeTag(const char* szTag);                                  // [ property ]
  void RemoveExcludeTag(const char* szTag);                                  // [ property ]

  float GetNearPlane() const { return m_Desc.m_fNearPlane; }                 // [ property ]
  void SetNearPlane(float fNearPlane);                                       // [ property ]

  float GetFarPlane() const { return m_Desc.m_fFarPlane; }                   // [ property ]
  void SetFarPlane(float fFarPlane);                                         // [ property ]

  const nsVec3& GetCaptureOffset() const { return m_Desc.m_vCaptureOffset; } // [ property ]
  void SetCaptureOffset(const nsVec3& vOffset);                              // [ property ]

  void SetShowDebugInfo(bool bShowDebugInfo);                                // [ property ]
  bool GetShowDebugInfo() const;                                             // [ property ]

  void SetShowMipMaps(bool bShowMipMaps);                                    // [ property ]
  bool GetShowMipMaps() const;                                               // [ property ]

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  float ComputePriority(nsMsgExtractRenderData& msg, nsReflectionProbeRenderData* pRenderData, float fVolume, const nsVec3& vScale) const;

protected:
  nsReflectionProbeDesc m_Desc;

  nsReflectionProbeId m_Id;
  // Set to true if a change was made that requires recomputing the cube map.
  mutable bool m_bStatesDirty = true;
};
