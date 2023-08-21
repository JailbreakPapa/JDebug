#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <CppProjectPlugin/CppProjectPluginDLL.h>

struct wdMsgSetColor;

using wdTexture2DResourceHandle = wdTypedResourceHandle<class wdTexture2DResource>;

// Bitmask to allow the user to select what debug rendering the component should do
struct SampleRenderComponentMask
{
  using StorageType = wdUInt8;

  // the enum names for the bits
  enum Enum
  {
    Box = WD_BIT(0),
    Sphere = WD_BIT(1),
    Cross = WD_BIT(2),
    Quad = WD_BIT(3),
    All = 0xFF,

    // required enum member; used by wdBitflags for default initialization
    Default = All
  };

  // this allows the debugger to show us names for a bitmask
  // just try this out by looking at an wdBitflags variable in a debugger
  struct Bits
  {
    wdUInt8 Box : 1;
    wdUInt8 Sphere : 1;
    wdUInt8 Cross : 1;
    wdUInt8 Quad : 1;
  };
};

WD_DECLARE_REFLECTABLE_TYPE(WD_CPPPROJECTPLUGIN_DLL, SampleRenderComponentMask);

// use wdComponentUpdateType::Always for this component to have 'Update' called even inside the editor when it is not simulating
// otherwise we would see the debug render output only when simulating the scene
using SampleRenderComponentManager = wdComponentManagerSimple<class SampleRenderComponent, wdComponentUpdateType::Always>;

class SampleRenderComponent : public wdComponent
{
  WD_DECLARE_COMPONENT_TYPE(SampleRenderComponent, wdComponent, SampleRenderComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // wdComponent

public:
  virtual void SerializeComponent(wdWorldWriter& stream) const override;
  virtual void DeserializeComponent(wdWorldReader& stream) override;

  //////////////////////////////////////////////////////////////////////////
  // SampleRenderComponent

public:
  SampleRenderComponent();
  ~SampleRenderComponent();

  float m_fSize = 1.0f;             // [ property ]
  wdColor m_Color = wdColor::White; // [ property ]

  void SetTexture(const wdTexture2DResourceHandle& hTexture);
  const wdTexture2DResourceHandle& GetTexture() const;

  void SetTextureFile(const char* szFile); // [ property ]
  const char* GetTextureFile(void) const;  // [ property ]

  wdBitflags<SampleRenderComponentMask> m_RenderTypes; // [ property ]

  void OnSetColor(wdMsgSetColor& msg); // [ msg handler ]

  void SetRandomColor(); // [ scriptable ]

private:
  void Update();

  wdTexture2DResourceHandle m_hTexture;
};
