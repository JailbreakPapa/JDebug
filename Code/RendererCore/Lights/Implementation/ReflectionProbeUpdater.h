#pragma once

#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <RendererFoundation/RendererFoundationDLL.h>

NS_DECLARE_FLAGS(nsUInt8, nsReflectionProbeUpdaterFlags, SkyLight, HasCustomCubeMap);

/// \brief Renders reflection probes and stores filtered mipmap chains into an atlas texture as well as computing sky irradiance
/// Rendering sky irradiance is optional and only done if m_iIrradianceOutputIndex != -1.
class nsReflectionProbeUpdater
{
public:
  /// \brief Defines the target specular reflection probe atlas and index as well as the sky irradiance atlas and index in case the rendered cube map is a sky light.
  struct TargetSlot
  {
    nsGALTextureHandle m_hSpecularOutputTexture;   ///< Must be a valid cube map texture array handle.
    nsGALTextureHandle m_hIrradianceOutputTexture; ///< Optional. Must be set if m_iIrradianceOutputIndex != -1.
    nsInt32 m_iSpecularOutputIndex = -1;           ///< Must be a valid index into the atlas texture.
    nsInt32 m_iIrradianceOutputIndex = -1;         ///< If -1, no irradiance is computed.
  };

public:
  nsReflectionProbeUpdater();
  ~nsReflectionProbeUpdater();

  /// \brief Returns how many new probes can be started this frame.
  /// \param out_updatesFinished Contains the probes that finished last frame.
  /// \return The number of new probes can be started this frame.
  nsUInt32 GetFreeUpdateSlots(nsDynamicArray<nsReflectionProbeRef>& out_updatesFinished);

  /// \brief Starts rendering a new reflection probe.
  /// \param probe The world and probe index to be rendered. Used as an identifier.
  /// \param desc Probe render settings.
  /// \param globalTransform World position to be rendered.
  /// \param target Where the probe should be rendered into.
  /// \return Returns NS_FAILURE if no more free slots are available.
  nsResult StartDynamicUpdate(const nsReflectionProbeRef& probe, const nsReflectionProbeDesc& desc, const nsTransform& globalTransform, const TargetSlot& target);

  /// \brief Starts filtering an existing cube map into a new reflection probe.
  /// \param probe The world and probe index to be rendered. Used as an identifier.
  /// \param desc Probe render settings.
  /// \param sourceTexture Cube map that should be filtered into a reflection probe.
  /// \param target Where the probe should be rendered into.
  /// \return Returns NS_FAILURE if no more free slots are available.
  nsResult StartFilterUpdate(const nsReflectionProbeRef& probe, const nsReflectionProbeDesc& desc, nsTextureCubeResourceHandle hSourceTexture, const TargetSlot& target);

  /// \brief Cancel a previously started update.
  void CancelUpdate(const nsReflectionProbeRef& probe);

  /// \brief Generates update steps. Should be called in PreExtraction phase.
  void GenerateUpdateSteps();

  /// \brief Schedules probe rendering views. Should be called at some point during the extraction phase. Can be called multiple times. It will only do work on the first call after GenerateUpdateSteps.
  void ScheduleUpdateSteps();

private:
  struct ReflectionView
  {
    nsViewHandle m_hView;
    nsCamera m_Camera;
  };

  struct UpdateStep
  {
    using StorageType = nsUInt8;

    enum Enum
    {
      RenderFace0,
      RenderFace1,
      RenderFace2,
      RenderFace3,
      RenderFace4,
      RenderFace5,
      Filter,

      ENUM_COUNT,

      Default = Filter
    };

    static bool IsRenderStep(Enum value) { return value >= UpdateStep::RenderFace0 && value <= UpdateStep::RenderFace5; }
    static Enum NextStep(Enum value) { return static_cast<UpdateStep::Enum>((value + 1) % UpdateStep::ENUM_COUNT); }
  };

  struct ProbeUpdateInfo
  {
    ProbeUpdateInfo();
    ~ProbeUpdateInfo();

    nsBitflags<nsReflectionProbeUpdaterFlags> m_flags;
    nsReflectionProbeRef m_probe;
    nsReflectionProbeDesc m_desc;
    nsTransform m_globalTransform;
    nsTextureCubeResourceHandle m_sourceTexture;
    TargetSlot m_TargetSlot;

    struct Step
    {
      NS_DECLARE_POD_TYPE();

      nsUInt8 m_uiViewIndex;
      nsEnum<UpdateStep> m_UpdateStep;
    };

    bool m_bInUse = false;
    nsEnum<UpdateStep> m_LastUpdateStep;

    nsHybridArray<Step, 8> m_UpdateSteps;

    nsGALTextureHandle m_hCubemap;
    nsGALTextureHandle m_hCubemapProxies[6];
  };

private:
  static void CreateViews(
    nsDynamicArray<ReflectionView>& views, nsUInt32 uiMaxRenderViews, const char* szNameSuffix, const char* szRenderPipelineResource);
  void CreateReflectionViewsAndResources();

  void ResetProbeUpdateInfo(nsUInt32 uiInfo);
  void AddViewToRender(const ProbeUpdateInfo::Step& step, ProbeUpdateInfo& updateInfo);

  bool m_bUpdateStepsFlushed = true;

  nsDynamicArray<ReflectionView> m_RenderViews;
  nsDynamicArray<ReflectionView> m_FilterViews;

  // Active Dynamic Updates
  nsDynamicArray<nsUniquePtr<ProbeUpdateInfo>> m_DynamicUpdates;
  nsHybridArray<nsReflectionProbeRef, 4> m_FinishedLastFrame;
};
