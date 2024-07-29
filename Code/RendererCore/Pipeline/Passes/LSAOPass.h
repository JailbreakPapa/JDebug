#pragma once

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/LSAOConstants.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// \brief Defines the depth compare function to be used to decide sample weights.
struct NS_RENDERERCORE_DLL nsLSAODepthCompareFunction
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Depth,                   ///< A hard cutoff function between the linear depth values. Samples with an absolute distance greater than
                             ///< nsLSAOPass::SetDepthCutoffDistance are ignored.
    Normal,                  ///< Samples that are on the same plane as constructed by the center position and normal will be weighted higher than those samples that
                             ///< are above or below the plane.
    NormalAndSampleDistance, ///< Same as Normal, but if two samples are tested, their distance to the center position is is inversely multiplied as
                             ///< well, giving closer matches a higher weight.
    Default = NormalAndSampleDistance
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsLSAODepthCompareFunction);

/// Screen space ambient occlusion using "line sweep ambient occlusion" by Ville Timonen
///
/// Resources:
/// Use in Quantum Break: http://wili.cc/research/quantum_break/SIGGRAPH_2015_Remedy_Notes.pdf
/// Presentation slides EGSR: http://wili.cc/research/lsao/EGSR13_LSAO.pdf
/// Paper: http://wili.cc/research/lsao/lsao.pdf
///
/// There are a few adjustments and own ideas worked into this implementation.
/// The biggest change probably is that pixels in the gather pass compute their target linesample arithmetically instead of relying on lookups.
class NS_RENDERERCORE_DLL nsLSAOPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsLSAOPass, nsRenderPipelinePass);

public:
  nsLSAOPass();
  ~nsLSAOPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void InitRenderPipelinePass(const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

  nsUInt32 GetLineToLinePixelOffset() const { return m_iLineToLinePixelOffset; }
  void SetLineToLinePixelOffset(nsUInt32 uiPixelOffset);
  nsUInt32 GetLineSamplePixelOffset() const { return m_iLineSamplePixelOffsetFactor; }
  void SetLineSamplePixelOffset(nsUInt32 uiPixelOffset);

  // Factor used for depth cutoffs (determines when a depth difference is too large to be considered)
  float GetDepthCutoffDistance() const;
  void SetDepthCutoffDistance(float fDepthCutoffDistance);

  // Determines how quickly the occlusion falls of.
  float GetOcclusionFalloff() const;
  void SetOcclusionFalloff(float fFalloff);


protected:
  /// Destroys all GPU data that might have been created in in SetupLineSweepData
  void DestroyLineSweepData();
  void SetupLineSweepData(const nsVec3I32& imageResolution);


  void AddLinesForDirection(const nsVec3I32& imageResolution, const nsVec2I32& sampleDir, nsUInt32 lineIndex, nsDynamicArray<LineInstruction>& outinLineInstructions, nsUInt32& outinTotalNumberOfSamples);

  nsRenderPipelineNodeInputPin m_PinDepthInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsConstantBufferStorageHandle m_hLineSweepCB;

  bool m_bSweepDataDirty = true;
  bool m_bConstantsDirty = true;

  /// Output of the line sweep pass.
  nsGALBufferHandle m_hLineSweepOutputBuffer;
  nsGALBufferUnorderedAccessViewHandle m_hLineSweepOutputUAV;
  nsGALBufferResourceViewHandle m_hLineSweepOutputSRV;

  /// Structured buffer containing instructions for every single line to trace.
  nsGALBufferHandle m_hLineInfoBuffer;
  nsGALBufferResourceViewHandle m_hLineSweepInfoSRV;

  /// Total number of lines to be traced.
  nsUInt32 m_uiNumSweepLines = 0;

  nsInt32 m_iLineToLinePixelOffset = 2;
  nsInt32 m_iLineSamplePixelOffsetFactor = 1;
  float m_fOcclusionFalloff = 0.2f;
  float m_fDepthCutoffDistance = 4.0f;

  nsEnum<nsLSAODepthCompareFunction> m_DepthCompareFunction;
  bool m_bDistributedGathering = true;

  nsShaderResourceHandle m_hShaderLineSweep;
  nsShaderResourceHandle m_hShaderGather;
  nsShaderResourceHandle m_hShaderAverage;
};