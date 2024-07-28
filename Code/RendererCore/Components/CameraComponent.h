#pragma once

#include <Core/Graphics/Camera.h>
#include <Core/World/World.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/Declarations.h>

class nsView;
struct nsResourceEvent;

class NS_RENDERERCORE_DLL nsCameraComponentManager : public nsComponentManager<class nsCameraComponent, nsBlockStorageType::Compact>
{
public:
  nsCameraComponentManager(nsWorld* pWorld);
  ~nsCameraComponentManager();

  virtual void Initialize() override;
  virtual void Deinitialize() override;

  void Update(const nsWorldModule::UpdateContext& context);

  void ReinitializeAllRenderTargetCameras();

  const nsCameraComponent* GetCameraByUsageHint(nsCameraUsageHint::Enum usageHint) const;
  nsCameraComponent* GetCameraByUsageHint(nsCameraUsageHint::Enum usageHint);

private:
  friend class nsCameraComponent;

  void AddRenderTargetCamera(nsCameraComponent* pComponent);
  void RemoveRenderTargetCamera(nsCameraComponent* pComponent);

  void OnViewCreated(nsView* pView);
  void OnCameraConfigsChanged(void* dummy);

  nsDynamicArray<nsComponentHandle> m_ModifiedCameras;
  nsDynamicArray<nsComponentHandle> m_RenderTargetCameras;
};

/// \brief Adds a camera to the scene.
///
/// Cameras have different use cases which are selected through the nsCameraUsageHint property.
/// A game needs (exactly) one camera with the usage hint "MainView", since that is what the renderer uses to render the output.
/// Other cameras are optional or for specialized use cases.
///
/// The camera component defines the field-of-view, near and far clipping plane distances,
/// which render pipeline to use, which objects to include and exclude in the rendered image and various other options.
///
/// A camera object may be created and controlled through a player prefab, for example in a first person or third person game.
/// It may also be created by an nsGameState and controlled by its game logic, for example in top-down games that don't
/// really have a player object.
///
/// Ultimately camera components don't have functionality, they mostly exist and store some data.
/// It is the game state's decision how the game camera works. By default, the game state iterates over all camera components
/// and picks the best one (usually the "MainView") to place the renderer camera.
class NS_RENDERERCORE_DLL nsCameraComponent : public nsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsCameraComponent, nsComponent, nsCameraComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  //////////////////////////////////////////////////////////////////////////
  // nsCameraComponent

public:
  nsCameraComponent();
  ~nsCameraComponent();

  /// \brief Sets what the camera should be used for.
  void SetUsageHint(nsEnum<nsCameraUsageHint> val);                      // [ property ]
  nsEnum<nsCameraUsageHint> GetUsageHint() const { return m_UsageHint; } // [ property ]

  /// \brief Sets the asset name (or path) to a render target resource, in case this camera should render to texture.
  void SetRenderTargetFile(const char* szFile); // [ property ]
  const char* GetRenderTargetFile() const;      // [ property ]

  /// \brief An offset to render only to a part of a texture.
  void SetRenderTargetRectOffset(nsVec2 value);                                  // [ property ]
  nsVec2 GetRenderTargetRectOffset() const { return m_vRenderTargetRectOffset; } // [ property ]

  /// \brief A size to render only to a part of a texture.
  void SetRenderTargetRectSize(nsVec2 value);                                // [ property ]
  nsVec2 GetRenderTargetRectSize() const { return m_vRenderTargetRectSize; } // [ property ]

  /// \brief Specifies whether the camera should be perspective or orthogonal and how to use the aspect ratio.
  void SetCameraMode(nsEnum<nsCameraMode> val);                 // [ property ]
  nsEnum<nsCameraMode> GetCameraMode() const { return m_Mode; } // [ property ]

  /// \brief Configures the distance of the near plane. Objects in front of the near plane get culled and clipped.
  void SetNearPlane(float fVal);                      // [ property ]
  float GetNearPlane() const { return m_fNearPlane; } // [ property ]

  /// \brief Configures the distance of the far plane. Objects behin the far plane get culled and clipped.
  void SetFarPlane(float fVal);                     // [ property ]
  float GetFarPlane() const { return m_fFarPlane; } // [ property ]

  /// \brief Sets the opening angle of the perspective view frustum. Whether this means the horizontal or vertical angle is determined by the camera mode.
  void SetFieldOfView(float fVal);                                   // [ property ]
  float GetFieldOfView() const { return m_fPerspectiveFieldOfView; } // [ property ]

  /// \brief Sets the size of the orthogonal view frustum. Whether this means the horizontal or vertical size is determined by the camera mode.
  void SetOrthoDimension(float fVal);                           // [ property ]
  float GetOrthoDimension() const { return m_fOrthoDimension; } // [ property ]

  /// \brief Returns the handle to the render pipeline that is in use.
  nsRenderPipelineResourceHandle GetRenderPipeline() const;

  /// \brief Returns a handle to the view that the camera renders to.
  nsViewHandle GetRenderTargetView() const;

  /// \brief Sets the name of the render pipeline to use.
  void SetRenderPipelineEnum(const char* szFile);                           // [ property ]
  const char* GetRenderPipelineEnum() const;                                // [ property ]

  void SetAperture(float fAperture);                                        // [ property ]
  float GetAperture() const { return m_fAperture; }                         // [ property ]

  void SetShutterTime(nsTime shutterTime);                                  // [ property ]
  nsTime GetShutterTime() const { return m_ShutterTime; }                   // [ property ]

  void SetISO(float fISO);                                                  // [ property ]
  float GetISO() const { return m_fISO; }                                   // [ property ]

  void SetExposureCompensation(float fEC);                                  // [ property ]
  float GetExposureCompensation() const { return m_fExposureCompensation; } // [ property ]

  float GetEV100() const;                                                   // [ property ]
  float GetExposure() const;                                                // [ property ]

  /// \brief If non-empty, only objects with these tags will be included in this camera's output.
  nsTagSet m_IncludeTags; // [ property ]

  /// \brief If non-empty, objects with these tags will be excluded from this camera's output.
  nsTagSet m_ExcludeTags; // [ property ]

  void ApplySettingsToView(nsView* pView) const;

private:
  void UpdateRenderTargetCamera();
  void ShowStats(nsView* pView);

  void ResourceChangeEventHandler(const nsResourceEvent& e);

  nsEnum<nsCameraUsageHint> m_UsageHint;
  nsEnum<nsCameraMode> m_Mode;
  nsRenderToTexture2DResourceHandle m_hRenderTarget;
  float m_fNearPlane = 0.25f;
  float m_fFarPlane = 1000.0f;
  float m_fPerspectiveFieldOfView = 60.0f;
  float m_fOrthoDimension = 10.0f;
  nsRenderPipelineResourceHandle m_hCachedRenderPipeline;

  float m_fAperture = 1.0f;
  nsTime m_ShutterTime = nsTime::MakeFromSeconds(1.0f);
  float m_fISO = 100.0f;
  float m_fExposureCompensation = 0.0f;

  void MarkAsModified();
  void MarkAsModified(nsCameraComponentManager* pCameraManager);

  bool m_bIsModified = false;
  bool m_bShowStats = false;
  bool m_bRenderTargetInitialized = false;

  // -1 for none, 0 to 9 for ALT+Number
  nsInt8 m_iEditorShortcut = -1; // [ property ]

  void ActivateRenderToTexture();
  void DeactivateRenderToTexture();

  nsViewHandle m_hRenderTargetView;
  nsVec2 m_vRenderTargetRectOffset = nsVec2(0.0f);
  nsVec2 m_vRenderTargetRectSize = nsVec2(1.0f);
  nsCamera m_RenderTargetCamera;
  nsHashedString m_sRenderPipeline;
};
