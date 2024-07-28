#pragma once

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>

class nsProfilingId;
class nsView;
class nsRenderPipelinePass;
class nsFrameDataProviderBase;
struct nsPermutationVar;
class nsDGMLGraph;
class nsFrustum;
class nsRasterizerView;

class NS_RENDERERCORE_DLL nsRenderPipeline : public nsRefCounted
{
public:
  enum class PipelineState
  {
    Uninitialized,
    RebuildError,
    Initialized
  };

  nsRenderPipeline();
  ~nsRenderPipeline();

  void AddPass(nsUniquePtr<nsRenderPipelinePass>&& pPass);
  void RemovePass(nsRenderPipelinePass* pPass);
  void GetPasses(nsDynamicArray<const nsRenderPipelinePass*>& ref_passes) const;
  void GetPasses(nsDynamicArray<nsRenderPipelinePass*>& ref_passes);
  nsRenderPipelinePass* GetPassByName(const nsStringView& sPassName);
  nsHashedString GetViewName() const;

  bool Connect(nsRenderPipelinePass* pOutputNode, const char* szOutputPinName, nsRenderPipelinePass* pInputNode, const char* szInputPinName);
  bool Connect(nsRenderPipelinePass* pOutputNode, nsHashedString sOutputPinName, nsRenderPipelinePass* pInputNode, nsHashedString sInputPinName);
  bool Disconnect(nsRenderPipelinePass* pOutputNode, nsHashedString sOutputPinName, nsRenderPipelinePass* pInputNode, nsHashedString sInputPinName);

  const nsRenderPipelinePassConnection* GetInputConnection(const nsRenderPipelinePass* pPass, nsHashedString sInputPinName) const;
  const nsRenderPipelinePassConnection* GetOutputConnection(const nsRenderPipelinePass* pPass, nsHashedString sOutputPinName) const;

  void AddExtractor(nsUniquePtr<nsExtractor>&& pExtractor);
  void RemoveExtractor(nsExtractor* pExtractor);
  void GetExtractors(nsDynamicArray<const nsExtractor*>& ref_extractors) const;
  void GetExtractors(nsDynamicArray<nsExtractor*>& ref_extractors);
  nsExtractor* GetExtractorByName(const nsStringView& sExtractorName);

  template <typename T>
  NS_ALWAYS_INLINE T* GetFrameDataProvider() const
  {
    return static_cast<T*>(GetFrameDataProvider(nsGetStaticRTTI<T>()));
  }

  const nsExtractedRenderData& GetRenderData() const;
  nsRenderDataBatchList GetRenderDataBatchesWithCategory(
    nsRenderData::Category category, nsRenderDataBatch::Filter filter = nsRenderDataBatch::Filter()) const;

  /// \brief Creates a DGML graph of all passes and textures. Can be used to verify that no accidental temp textures are created due to poorly constructed pipelines or errors in code.
  void CreateDgmlGraph(nsDGMLGraph& ref_graph);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  static nsCVarBool cvar_SpatialCullingVis;
#endif

  NS_DISALLOW_COPY_AND_ASSIGN(nsRenderPipeline);

private:
  friend class nsRenderWorld;
  friend class nsView;

  // \brief Rebuilds the render pipeline, e.g. sorting passes via dependencies and creating render targets.
  PipelineState Rebuild(const nsView& view);
  bool RebuildInternal(const nsView& view);
  bool SortPasses();
  bool InitRenderTargetDescriptions(const nsView& view);
  bool CreateRenderTargetUsage(const nsView& view);
  bool InitRenderPipelinePasses();
  void SortExtractors();
  void UpdateViewData(const nsView& view, nsUInt32 uiDataIndex);

  void RemoveConnections(nsRenderPipelinePass* pPass);
  void ClearRenderPassGraphTextures();
  bool AreInputDescriptionsAvailable(const nsRenderPipelinePass* pPass, const nsHybridArray<nsRenderPipelinePass*, 32>& done) const;
  bool ArePassThroughInputsDone(const nsRenderPipelinePass* pPass, const nsHybridArray<nsRenderPipelinePass*, 32>& done) const;

  nsFrameDataProviderBase* GetFrameDataProvider(const nsRTTI* pRtti) const;

  void ExtractData(const nsView& view);
  void FindVisibleObjects(const nsView& view);

  void Render(nsRenderContext* pRenderer);

  nsRasterizerView* PrepareOcclusionCulling(const nsFrustum& frustum, const nsView& view);
  void PreviewOcclusionBuffer(const nsRasterizerView& rasterizer, const nsView& view);

private: // Member data
  // Thread data
  nsThreadID m_CurrentExtractThread;
  nsThreadID m_CurrentRenderThread;

  // Pipeline render data
  nsExtractedRenderData m_Data[2];
  nsDynamicArray<const nsGameObject*> m_VisibleObjects;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  nsTime m_AverageCullingTime;
#endif

  nsHashedString m_sName;
  nsUInt64 m_uiLastExtractionFrame;
  nsUInt64 m_uiLastRenderFrame;

  // Render pass graph data
  PipelineState m_PipelineState = PipelineState::Uninitialized;

  struct ConnectionData
  {
    // Inputs / outputs match the node pin indices. Value at index is nullptr if not connected.
    nsDynamicArray<nsRenderPipelinePassConnection*> m_Inputs;
    nsDynamicArray<nsRenderPipelinePassConnection*> m_Outputs;
  };
  nsDynamicArray<nsUniquePtr<nsRenderPipelinePass>> m_Passes;
  nsMap<const nsRenderPipelinePass*, ConnectionData> m_Connections;

  /// \brief Contains all connections that share the same path-through texture and their first and last usage pass index.
  struct TextureUsageData
  {
    nsHybridArray<nsRenderPipelinePassConnection*, 4> m_UsedBy;
    nsUInt16 m_uiFirstUsageIdx;
    nsUInt16 m_uiLastUsageIdx;
    nsInt32 m_iTargetTextureIndex = -1;
  };
  nsDynamicArray<TextureUsageData> m_TextureUsage;
  nsDynamicArray<nsUInt16> m_TextureUsageIdxSortedByFirstUsage; ///< Indices map into m_TextureUsage
  nsDynamicArray<nsUInt16> m_TextureUsageIdxSortedByLastUsage;  ///< Indices map into m_TextureUsage

  nsHashTable<nsRenderPipelinePassConnection*, nsUInt32> m_ConnectionToTextureIndex;

  // Extractors
  nsDynamicArray<nsUniquePtr<nsExtractor>> m_Extractors;
  nsDynamicArray<nsUniquePtr<nsExtractor>> m_SortedExtractors;

  // Data Providers
  mutable nsDynamicArray<nsUniquePtr<nsFrameDataProviderBase>> m_DataProviders;
  mutable nsHashTable<const nsRTTI*, nsUInt32> m_TypeToDataProviderIndex;

  nsDynamicArray<nsPermutationVar> m_PermutationVars;

  // Occlusion Culling
  nsGALTextureHandle m_hOcclusionDebugViewTexture;
};
