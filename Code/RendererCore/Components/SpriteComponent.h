#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <Core/World/World.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Pipeline/RenderData.h>

struct nsMsgSetColor;
using nsTexture2DResourceHandle = nsTypedResourceHandle<class nsTexture2DResource>;

struct nsSpriteBlendMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Masked,
    Transparent,
    Additive,
    ShapeIcon,

    Default = Masked
  };

  static nsTempHashedString GetPermutationValue(Enum blendMode);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsSpriteBlendMode);

class NS_RENDERERCORE_DLL nsSpriteRenderData : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsSpriteRenderData, nsRenderData);

public:
  void FillBatchIdAndSortingKey();

  nsTexture2DResourceHandle m_hTexture;

  float m_fSize;
  float m_fMaxScreenSize;
  float m_fAspectRatio;
  nsEnum<nsSpriteBlendMode> m_BlendMode;

  nsColor m_color;

  nsVec2 m_texCoordScale;
  nsVec2 m_texCoordOffset;

  nsUInt32 m_uiUniqueID;
};

using nsSpriteComponentManager = nsComponentManager<class nsSpriteComponent, nsBlockStorageType::Compact>;

/// \brief Renders a screen-oriented quad (billboard) with a maximum screen size.
///
/// This component is typically used to attach an icon to a game object.
/// The sprite becomes smaller the farther away it is, but when you come closer, its screen size gets clamped to a fixed maximum.
///
/// It can also be used to render simple projectiles.
///
/// If you want to render a glow effect for a lightsource, use the nsLensFlareComponent instead.
class NS_RENDERERCORE_DLL nsSpriteComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsSpriteComponent, nsRenderComponent, nsSpriteComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) override;

  //////////////////////////////////////////////////////////////////////////
  // nsSpriteComponent

public:
  nsSpriteComponent();
  ~nsSpriteComponent();

  void SetTexture(const nsTexture2DResourceHandle& hTexture);
  const nsTexture2DResourceHandle& GetTexture() const;

  void SetTextureFile(const char* szFile); // [ property ]
  const char* GetTextureFile() const;      // [ property ]

  void SetColor(nsColor color);            // [ property ]
  nsColor GetColor() const;                // [ property ]

  /// \brief Sets the size of the sprite in world-space units. This determines how large the sprite will be at certain distances.
  void SetSize(float fSize); // [ property ]
  float GetSize() const;     // [ property ]

  /// \brief Sets the maximum screen-space size in pixels. Once a sprite is close enough to have reached this size, it will not grow larger.
  void SetMaxScreenSize(float fSize);         // [ property ]
  float GetMaxScreenSize() const;             // [ property ]

  void OnMsgSetColor(nsMsgSetColor& ref_msg); // [ property ]

private:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  nsTexture2DResourceHandle m_hTexture;
  nsEnum<nsSpriteBlendMode> m_BlendMode;
  nsColor m_Color = nsColor::White;

  float m_fSize = 1.0f;
  float m_fMaxScreenSize = 64.0f;
  float m_fAspectRatio = 1.0f;
};
