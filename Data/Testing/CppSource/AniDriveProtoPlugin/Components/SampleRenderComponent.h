/*
 *   Copyright (c) 2023 WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <AniDriveProtoPlugin/AniDriveProtoPluginDLL.h>

struct nsMsgSetColor;

using nsTexture2DResourceHandle = nsTypedResourceHandle<class nsTexture2DResource>;

// Bitmask to allow the user to select what debug rendering the component should do
struct SampleRenderComponentMask
{
  using StorageType = nsUInt8;

  // the enum names for the bits
  enum Enum
  {
    Box = NS_BIT(0),
    Sphere = NS_BIT(1),
    Cross = NS_BIT(2),
    Quad = NS_BIT(3),
    All = 0xFF,

    // required enum member; used by nsBitflags for default initialization
    Default = All
  };

  // this allows the debugger to show us names for a bitmask
  // just try this out by looking at an nsBitflags variable in a debugger
  struct Bits
  {
    nsUInt8 Box : 1;
    nsUInt8 Sphere : 1;
    nsUInt8 Cross : 1;
    nsUInt8 Quad : 1;
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_ANIDRIVEPROTOPLUGIN_DLL, SampleRenderComponentMask);

// use nsComponentUpdateType::Always for this component to have 'Update' called even inside the editor when it is not simulating
// otherwise we would see the debug render output only when simulating the scene
using SampleRenderComponentManager = nsComponentManagerSimple<class SampleRenderComponent, nsComponentUpdateType::Always>;

class SampleRenderComponent : public nsComponent
{
  NS_DECLARE_COMPONENT_TYPE(SampleRenderComponent, nsComponent, SampleRenderComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& stream) const override;
  virtual void DeserializeComponent(nsWorldReader& stream) override;

  //////////////////////////////////////////////////////////////////////////
  // SampleRenderComponent

public:
  SampleRenderComponent();
  ~SampleRenderComponent();

  float m_fSize = 1.0f;             // [ property ]
  nsColor m_Color = nsColor::White; // [ property ]

  void SetTexture(const nsTexture2DResourceHandle& hTexture);
  const nsTexture2DResourceHandle& GetTexture() const;

  void SetTextureFile(const char* szFile); // [ property ]
  const char* GetTextureFile(void) const;  // [ property ]

  nsBitflags<SampleRenderComponentMask> m_RenderTypes; // [ property ]

  void OnSetColor(nsMsgSetColor& msg); // [ msg handler ]

  void SetRandomColor(); // [ scriptable ]

private:
  void Update();

  nsTexture2DResourceHandle m_hTexture;
};
