#pragma once

#include <Core/World/World.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Textures/Texture2DResource.h>

struct nsMsgExtractRenderData;

using nsRenderTargetComponentManager = nsComponentManager<class nsRenderTargetActivatorComponent, nsBlockStorageType::Compact>;

/// \brief Attach this component to an object that uses a render target for reading, to ensure that the render target gets written to.
///
/// If you build a monitor that displays the output of a security camera in your level, the engine needs to know when it should
/// update the render target that displays the security camera footage, and when it can skip that part to not waste performance.
/// Thus, by default, the engine will not update the render target, as long as this isn't requested.
/// This component implements this request functionality.
///
/// It is a render component, which means that it tracks when it is visible and when visible, it will 'activate' the desired
/// render target, so that it will be updated.
/// By attaching it to an object, like the monitor, it activates the render target whenever the monitor object itself gets rendered.
class NS_RENDERERCORE_DLL nsRenderTargetActivatorComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsRenderTargetActivatorComponent, nsRenderComponent, nsRenderTargetComponentManager);

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
  // nsRenderTargetActivatorComponent

public:
  nsRenderTargetActivatorComponent();
  ~nsRenderTargetActivatorComponent();

  /// \brief Sets the resource file for the nsRenderToTexture2DResource
  void SetRenderTargetFile(const char* szFile); // [ property ]
  const char* GetRenderTargetFile() const;      // [ property ]

  /// \brief Sets the nsRenderToTexture2DResource to render activate.
  void SetRenderTarget(const nsRenderToTexture2DResourceHandle& hResource);
  nsRenderToTexture2DResourceHandle GetRenderTarget() const { return m_hRenderTarget; }

private:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  nsRenderToTexture2DResourceHandle m_hRenderTarget;
};
