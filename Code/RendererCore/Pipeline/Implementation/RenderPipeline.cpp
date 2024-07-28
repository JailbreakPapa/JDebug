#include <RendererCore/RendererCorePCH.h>

#include <Core/ResourceManager/ResourceManager.h>
#include <Core/World/World.h>
#include <Foundation/Application/Application.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/ColorScheme.h>
#include <Foundation/Math/Frustum.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/SimdMath/SimdBBox.h>
#include <Foundation/Time/Clock.h>
#include <Foundation/Utilities/DGMLWriter.h>
#include <RendererCore/Components/AlwaysVisibleComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Extractor.h>
#include <RendererCore/Pipeline/FrameDataProvider.h>
#include <RendererCore/Pipeline/Passes/TargetPass.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/Rasterizer/RasterizerView.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
nsCVarBool nsRenderPipeline::cvar_SpatialCullingVis("Spatial.Culling.Vis", false, nsCVarFlags::Default, "Enables debug visualization of visibility culling");
nsCVarBool cvar_SpatialCullingShowStats("Spatial.Culling.ShowStats", false, nsCVarFlags::Default, "Display some stats of the visibility culling");
#endif

nsCVarBool cvar_SpatialCullingOcclusionEnable("Spatial.Occlusion.Enable", true, nsCVarFlags::Default, "Use software rasterization for occlusion culling.");
nsCVarBool cvar_SpatialCullingOcclusionVisView("Spatial.Occlusion.VisView", false, nsCVarFlags::Default, "Render the occlusion framebuffer as an overlay.");
nsCVarFloat cvar_SpatialCullingOcclusionBoundsInlation("Spatial.Occlusion.BoundsInflation", 0.5f, nsCVarFlags::Default, "How much to inflate bounds during occlusion check.");
nsCVarFloat cvar_SpatialCullingOcclusionFarPlane("Spatial.Occlusion.FarPlane", 50.0f, nsCVarFlags::Default, "Far plane distance for finding occluders.");

nsRenderPipeline::nsRenderPipeline()

{
  m_CurrentExtractThread = (nsThreadID)0;
  m_CurrentRenderThread = (nsThreadID)0;
  m_uiLastExtractionFrame = -1;
  m_uiLastRenderFrame = -1;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  m_AverageCullingTime = nsTime::MakeFromSeconds(0.1f);
#endif
}

nsRenderPipeline::~nsRenderPipeline()
{
  if (!m_hOcclusionDebugViewTexture.IsInvalidated())
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
    pDevice->DestroyTexture(m_hOcclusionDebugViewTexture);
    m_hOcclusionDebugViewTexture.Invalidate();
  }

  m_Data[0].Clear();
  m_Data[1].Clear();

  ClearRenderPassGraphTextures();
  while (!m_Passes.IsEmpty())
  {
    RemovePass(m_Passes.PeekBack().Borrow());
  }
}

void nsRenderPipeline::AddPass(nsUniquePtr<nsRenderPipelinePass>&& pPass)
{
  m_PipelineState = PipelineState::Uninitialized;
  pPass->m_pPipeline = this;
  pPass->InitializePins();

  auto it = m_Connections.Insert(pPass.Borrow(), ConnectionData());
  it.Value().m_Inputs.SetCount(pPass->GetInputPins().GetCount());
  it.Value().m_Outputs.SetCount(pPass->GetOutputPins().GetCount());
  m_Passes.PushBack(std::move(pPass));
}

void nsRenderPipeline::RemovePass(nsRenderPipelinePass* pPass)
{
  for (nsUInt32 i = 0; i < m_Passes.GetCount(); ++i)
  {
    if (m_Passes[i].Borrow() == pPass)
    {
      m_PipelineState = PipelineState::Uninitialized;
      RemoveConnections(pPass);
      m_Connections.Remove(pPass);
      pPass->m_pPipeline = nullptr;
      m_Passes.RemoveAtAndCopy(i);
      break;
    }
  }
}

void nsRenderPipeline::GetPasses(nsDynamicArray<const nsRenderPipelinePass*>& ref_passes) const
{
  ref_passes.Reserve(m_Passes.GetCount());

  for (auto& pPass : m_Passes)
  {
    ref_passes.PushBack(pPass.Borrow());
  }
}

void nsRenderPipeline::GetPasses(nsDynamicArray<nsRenderPipelinePass*>& ref_passes)
{
  ref_passes.Reserve(m_Passes.GetCount());

  for (auto& pPass : m_Passes)
  {
    ref_passes.PushBack(pPass.Borrow());
  }
}

nsRenderPipelinePass* nsRenderPipeline::GetPassByName(const nsStringView& sPassName)
{
  for (auto& pPass : m_Passes)
  {
    if (sPassName.IsEqual(pPass->GetName()))
    {
      return pPass.Borrow();
    }
  }

  return nullptr;
}

nsHashedString nsRenderPipeline::GetViewName() const
{
  return m_sName;
}

bool nsRenderPipeline::Connect(nsRenderPipelinePass* pOutputNode, const char* szOutputPinName, nsRenderPipelinePass* pInputNode, const char* szInputPinName)
{
  nsHashedString sOutputPinName;
  sOutputPinName.Assign(szOutputPinName);
  nsHashedString sInputPinName;
  sInputPinName.Assign(szInputPinName);
  return Connect(pOutputNode, sOutputPinName, pInputNode, sInputPinName);
}

bool nsRenderPipeline::Connect(nsRenderPipelinePass* pOutputNode, nsHashedString sOutputPinName, nsRenderPipelinePass* pInputNode, nsHashedString sInputPinName)
{
  nsLogBlock b("nsRenderPipeline::Connect");

  auto itOut = m_Connections.Find(pOutputNode);
  if (!itOut.IsValid())
  {
    nsLog::Error("Output node '{0}' not added to pipeline!", pOutputNode->GetName());
    return false;
  }
  auto itIn = m_Connections.Find(pInputNode);
  if (!itIn.IsValid())
  {
    nsLog::Error("Input node '{0}' not added to pipeline!", pInputNode->GetName());
    return false;
  }
  const nsRenderPipelineNodePin* pPinSource = pOutputNode->GetPinByName(sOutputPinName);
  if (!pPinSource)
  {
    nsLog::Error("Source pin '{0}::{1}' does not exist!", pOutputNode->GetName(), sOutputPinName);
    return false;
  }
  const nsRenderPipelineNodePin* pPinTarget = pInputNode->GetPinByName(sInputPinName);
  if (!pPinTarget)
  {
    nsLog::Error("Target pin '{0}::{1}' does not exist!", pInputNode->GetName(), sInputPinName);
    return false;
  }
  if (itIn.Value().m_Inputs[pPinTarget->m_uiInputIndex] != nullptr)
  {
    nsLog::Error("Pins already connected: '{0}::{1}' -> '{2}::{3}'!", pOutputNode->GetName(), sOutputPinName, pInputNode->GetName(), sInputPinName);
    return false;
  }

  // Add at output
  nsRenderPipelinePassConnection* pConnection = itOut.Value().m_Outputs[pPinSource->m_uiOutputIndex];
  if (pConnection == nullptr)
  {
    pConnection = NS_DEFAULT_NEW(nsRenderPipelinePassConnection);
    pConnection->m_pOutput = pPinSource;
    itOut.Value().m_Outputs[pPinSource->m_uiOutputIndex] = pConnection;
  }
  else
  {
    // Check that only one passthrough is connected
    if (pPinTarget->m_Type == nsRenderPipelineNodePin::Type::PassThrough)
    {
      for (const nsRenderPipelineNodePin* pPin : pConnection->m_Inputs)
      {
        if (pPin->m_Type == nsRenderPipelineNodePin::Type::PassThrough)
        {
          nsLog::Error("A pass through pin is already connected to the '{0}' pin!", sOutputPinName);
          return false;
        }
      }
    }
  }

  // Add at input
  pConnection->m_Inputs.PushBack(pPinTarget);
  itIn.Value().m_Inputs[pPinTarget->m_uiInputIndex] = pConnection;
  m_PipelineState = PipelineState::Uninitialized;
  return true;
}

bool nsRenderPipeline::Disconnect(nsRenderPipelinePass* pOutputNode, nsHashedString sOutputPinName, nsRenderPipelinePass* pInputNode, nsHashedString sInputPinName)
{
  nsLogBlock b("nsRenderPipeline::Connect");

  auto itOut = m_Connections.Find(pOutputNode);
  if (!itOut.IsValid())
  {
    nsLog::Error("Output node '{0}' not added to pipeline!", pOutputNode->GetName());
    return false;
  }
  auto itIn = m_Connections.Find(pInputNode);
  if (!itIn.IsValid())
  {
    nsLog::Error("Input node '{0}' not added to pipeline!", pInputNode->GetName());
    return false;
  }
  const nsRenderPipelineNodePin* pPinSource = pOutputNode->GetPinByName(sOutputPinName);
  if (!pPinSource)
  {
    nsLog::Error("Source pin '{0}::{1}' does not exist!", pOutputNode->GetName(), sOutputPinName);
    return false;
  }
  const nsRenderPipelineNodePin* pPinTarget = pInputNode->GetPinByName(sInputPinName);
  if (!pPinTarget)
  {
    nsLog::Error("Target pin '{0}::{1}' does not exist!", pInputNode->GetName(), sInputPinName);
    return false;
  }
  if (itIn.Value().m_Inputs[pPinTarget->m_uiInputIndex] == nullptr || itIn.Value().m_Inputs[pPinTarget->m_uiInputIndex] != itOut.Value().m_Outputs[pPinSource->m_uiOutputIndex])
  {
    nsLog::Error("Pins not connected: '{0}::{1}' -> '{2}::{3}'!", pOutputNode->GetName(), sOutputPinName, pInputNode->GetName(), sInputPinName);
    return false;
  }

  // Remove at input
  nsRenderPipelinePassConnection* pConnection = itOut.Value().m_Outputs[pPinSource->m_uiOutputIndex];
  pConnection->m_Inputs.RemoveAndCopy(pPinTarget);
  itIn.Value().m_Inputs[pPinTarget->m_uiInputIndex] = nullptr;

  if (pConnection->m_Inputs.IsEmpty())
  {
    // Remove at output
    itOut.Value().m_Outputs[pPinSource->m_uiOutputIndex] = nullptr;
    NS_DEFAULT_DELETE(pConnection);
  }

  m_PipelineState = PipelineState::Uninitialized;
  return true;
}

const nsRenderPipelinePassConnection* nsRenderPipeline::GetInputConnection(const nsRenderPipelinePass* pPass, nsHashedString sInputPinName) const
{
  auto it = m_Connections.Find(pPass);
  if (!it.IsValid())
    return nullptr;

  auto& data = it.Value();
  const nsRenderPipelineNodePin* pPin = pPass->GetPinByName(sInputPinName);
  if (!pPin || pPin->m_uiInputIndex == 0xFF)
    return nullptr;

  return data.m_Inputs[pPin->m_uiInputIndex];
}

const nsRenderPipelinePassConnection* nsRenderPipeline::GetOutputConnection(const nsRenderPipelinePass* pPass, nsHashedString sOutputPinName) const
{
  auto it = m_Connections.Find(pPass);
  if (!it.IsValid())
    return nullptr;

  auto& data = it.Value();
  const nsRenderPipelineNodePin* pPin = pPass->GetPinByName(sOutputPinName);
  if (!pPin)
    return nullptr;

  return data.m_Outputs[pPin->m_uiOutputIndex];
}

nsRenderPipeline::PipelineState nsRenderPipeline::Rebuild(const nsView& view)
{
  nsLogBlock b("nsRenderPipeline::Rebuild");

  ClearRenderPassGraphTextures();

  bool bRes = RebuildInternal(view);
  if (!bRes)
  {
    ClearRenderPassGraphTextures();
  }
  else
  {
    // make sure the renderdata stores the updated view data
    UpdateViewData(view, nsRenderWorld::GetDataIndexForRendering());
  }

  m_PipelineState = bRes ? PipelineState::Initialized : PipelineState::RebuildError;
  return m_PipelineState;
}

bool nsRenderPipeline::RebuildInternal(const nsView& view)
{
  if (!SortPasses())
    return false;
  if (!InitRenderTargetDescriptions(view))
    return false;
  if (!CreateRenderTargetUsage(view))
    return false;
  if (!InitRenderPipelinePasses())
    return false;

  SortExtractors();

  return true;
}

bool nsRenderPipeline::SortPasses()
{
  nsLogBlock b("Sort Passes");
  nsHybridArray<nsRenderPipelinePass*, 32> done;
  done.Reserve(m_Passes.GetCount());

  nsHybridArray<nsRenderPipelinePass*, 8> usable;     // Stack of passes with all connections setup, they can be asked for descriptions.
  nsHybridArray<nsRenderPipelinePass*, 8> candidates; // Not usable yet, but all input connections are available

  // Find all source passes from which we can start the output description propagation.
  for (auto& pPass : m_Passes)
  {
    // if (std::all_of(cbegin(it.Value().m_Inputs), cend(it.Value().m_Inputs), [](nsRenderPipelinePassConnection* pConn){return pConn ==
    // nullptr; }))
    if (AreInputDescriptionsAvailable(pPass.Borrow(), done))
    {
      usable.PushBack(pPass.Borrow());
    }
  }

  // Via a depth first traversal, order the passes
  while (!usable.IsEmpty())
  {
    nsRenderPipelinePass* pPass = usable.PeekBack();
    nsLogBlock b2("Traverse", pPass->GetName());

    usable.PopBack();
    ConnectionData& data = m_Connections[pPass];

    NS_ASSERT_DEBUG(data.m_Inputs.GetCount() == pPass->GetInputPins().GetCount(), "Input pin count missmatch!");
    NS_ASSERT_DEBUG(data.m_Outputs.GetCount() == pPass->GetOutputPins().GetCount(), "Output pin count missmatch!");

    // Check for new candidate passes. Can't be done in the previous loop as multiple connections may be required by a node.
    for (nsUInt32 i = 0; i < data.m_Outputs.GetCount(); i++)
    {
      if (data.m_Outputs[i] != nullptr)
      {
        // Go through all inputs this connection is connected to and test the corresponding node for availability
        for (const nsRenderPipelineNodePin* pPin : data.m_Outputs[i]->m_Inputs)
        {
          NS_ASSERT_DEBUG(pPin->m_pParent != nullptr, "Pass was not initialized!");
          nsRenderPipelinePass* pTargetPass = static_cast<nsRenderPipelinePass*>(pPin->m_pParent);
          if (done.Contains(pTargetPass))
          {
            nsLog::Error("Loop detected, graph not supported!");
            return false;
          }

          if (!usable.Contains(pTargetPass) && !candidates.Contains(pTargetPass))
          {
            candidates.PushBack(pTargetPass);
          }
        }
      }
    }

    done.PushBack(pPass);

    // Check for usable candidates. Reverse order for depth first traversal.
    for (nsInt32 i = (nsInt32)candidates.GetCount() - 1; i >= 0; i--)
    {
      nsRenderPipelinePass* pCandidatePass = candidates[i];
      if (AreInputDescriptionsAvailable(pCandidatePass, done) && ArePassThroughInputsDone(pCandidatePass, done))
      {
        usable.PushBack(pCandidatePass);
        candidates.RemoveAtAndCopy(i);
      }
    }
  }

  if (done.GetCount() < m_Passes.GetCount())
  {
    nsLog::Error("Pipeline: Not all nodes could be initialized");
    for (auto& pass : m_Passes)
    {
      if (!done.Contains(pass.Borrow()))
      {
        nsLog::Error("Failed to initialize node: {} - {}", pass->GetName(), pass->GetDynamicRTTI()->GetTypeName());
      }
    }
    return false;
  }

  struct nsPipelineSorter
  {
    /// \brief Returns true if a is less than b
    NS_FORCE_INLINE bool Less(const nsUniquePtr<nsRenderPipelinePass>& a, const nsUniquePtr<nsRenderPipelinePass>& b) const { return m_pDone->IndexOf(a.Borrow()) < m_pDone->IndexOf(b.Borrow()); }

    /// \brief Returns true if a is equal to b
    NS_ALWAYS_INLINE bool Equal(const nsUniquePtr<nsRenderPipelinePass>& a, const nsUniquePtr<nsRenderPipelinePass>& b) const { return a.Borrow() == b.Borrow(); }

    nsHybridArray<nsRenderPipelinePass*, 32>* m_pDone;
  };

  nsPipelineSorter sorter;
  sorter.m_pDone = &done;
  m_Passes.Sort(sorter);
  return true;
}

bool nsRenderPipeline::InitRenderTargetDescriptions(const nsView& view)
{
  nsLogBlock b("Init Render Target Descriptions");
  nsHybridArray<nsGALTextureCreationDescription*, 10> inputs;
  nsHybridArray<nsGALTextureCreationDescription, 10> outputs;

  for (auto& pPass : m_Passes)
  {
    nsLogBlock b2("InitPass", pPass->GetName());

    if (view.GetCamera()->IsStereoscopic() && !pPass->IsStereoAware())
    {
      nsLog::Error("View '{0}' uses a stereoscopic camera, but the render pass '{1}' does not support stereo rendering!", view.GetName(), pPass->GetName());
    }

    ConnectionData& data = m_Connections[pPass.Borrow()];

    NS_ASSERT_DEBUG(data.m_Inputs.GetCount() == pPass->GetInputPins().GetCount(), "Input pin count missmatch!");
    NS_ASSERT_DEBUG(data.m_Outputs.GetCount() == pPass->GetOutputPins().GetCount(), "Output pin count missmatch!");

    inputs.SetCount(data.m_Inputs.GetCount());
    outputs.Clear();
    outputs.SetCount(data.m_Outputs.GetCount());
    // Fill inputs array
    for (nsUInt32 i = 0; i < data.m_Inputs.GetCount(); i++)
    {
      if (data.m_Inputs[i] != nullptr)
      {
        inputs[i] = &data.m_Inputs[i]->m_Desc;
      }
      else
      {
        inputs[i] = nullptr;
      }
    }

    bool bRes = pPass->GetRenderTargetDescriptions(view, inputs, outputs);
    if (!bRes)
    {
      nsLog::Error("The pass could not be successfully queried for render target descriptions.");
      return false;
    }

    // Copy queried outputs into the output connections.
    for (nsUInt32 i = 0; i < data.m_Outputs.GetCount(); i++)
    {
      if (data.m_Outputs[i] != nullptr)
      {
        data.m_Outputs[i]->m_Desc = outputs[i];
      }
    }

    // Check pass-through consistency of input / output target desc.
    auto inputPins = pPass->GetInputPins();
    for (const nsRenderPipelineNodePin* pPin : inputPins)
    {
      if (pPin->m_Type == nsRenderPipelineNodePin::Type::PassThrough)
      {
        if (data.m_Outputs[pPin->m_uiOutputIndex] != nullptr)
        {
          if (data.m_Inputs[pPin->m_uiInputIndex] == nullptr)
          {
            // nsLog::Error("The pass of type '{0}' has a pass through pin '{1}' that has an output but no input!",
            // pPass->GetDynamicRTTI()->GetTypeName(), pPass->GetPinName(pPin));  return false;
          }
          else if (data.m_Outputs[pPin->m_uiOutputIndex]->m_Desc.CalculateHash() != data.m_Inputs[pPin->m_uiInputIndex]->m_Desc.CalculateHash())
          {
            nsLog::Error("The pass has a pass through pin '{0}' that has different descriptors for input and output!", pPass->GetPinName(pPin));
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool nsRenderPipeline::CreateRenderTargetUsage(const nsView& view)
{
  nsLogBlock b("Create Render Target Usage Data");
  NS_ASSERT_DEBUG(m_TextureUsage.IsEmpty(), "Need to call ClearRenderPassGraphTextures before re-creating the pipeline.");

  m_ConnectionToTextureIndex.Clear();

  // Gather all connections that share the same path-through texture and their first and last usage pass index.
  for (nsUInt16 i = 0; i < static_cast<nsUInt16>(m_Passes.GetCount()); i++)
  {
    const auto& pPass = m_Passes[i].Borrow();
    ConnectionData& data = m_Connections[pPass];
    for (nsRenderPipelinePassConnection* pConn : data.m_Inputs)
    {
      if (pConn != nullptr)
      {
        nsUInt32 uiDataIdx = m_ConnectionToTextureIndex[pConn];
        m_TextureUsage[uiDataIdx].m_uiLastUsageIdx = i;
      }
    }

    for (nsRenderPipelinePassConnection* pConn : data.m_Outputs)
    {
      if (pConn != nullptr)
      {
        if (pConn->m_pOutput->m_Type == nsRenderPipelineNodePin::Type::PassThrough && data.m_Inputs[pConn->m_pOutput->m_uiInputIndex] != nullptr)
        {
          nsRenderPipelinePassConnection* pCorrespondingInputConn = data.m_Inputs[pConn->m_pOutput->m_uiInputIndex];
          NS_ASSERT_DEV(m_ConnectionToTextureIndex.Contains(pCorrespondingInputConn), "");
          nsUInt32 uiDataIdx = m_ConnectionToTextureIndex[pCorrespondingInputConn];
          m_TextureUsage[uiDataIdx].m_UsedBy.PushBack(pConn);
          m_TextureUsage[uiDataIdx].m_uiLastUsageIdx = i;

          NS_ASSERT_DEV(!m_ConnectionToTextureIndex.Contains(pConn), "");
          m_ConnectionToTextureIndex[pConn] = uiDataIdx;
        }
        else
        {
          m_ConnectionToTextureIndex[pConn] = m_TextureUsage.GetCount();
          TextureUsageData& texData = m_TextureUsage.ExpandAndGetRef();

          texData.m_iTargetTextureIndex = -1;
          texData.m_uiFirstUsageIdx = i;
          texData.m_uiLastUsageIdx = i;
          texData.m_UsedBy.PushBack(pConn);
        }
      }
    }
  }

  static nsUInt32 defaultTextureDescHash = nsGALTextureCreationDescription().CalculateHash();
  // Set view's render target textures to target pass connections.
  for (nsUInt32 i = 0; i < m_Passes.GetCount(); i++)
  {
    const auto& pPass = m_Passes[i].Borrow();
    if (pPass->IsInstanceOf<nsTargetPass>())
    {
      const nsGALRenderTargets& renderTargets = view.GetActiveRenderTargets();

      nsTargetPass* pTargetPass = static_cast<nsTargetPass*>(pPass);
      ConnectionData& data = m_Connections[pPass];
      for (nsUInt32 j = 0; j < data.m_Inputs.GetCount(); j++)
      {
        nsRenderPipelinePassConnection* pConn = data.m_Inputs[j];
        if (pConn != nullptr)
        {
          const nsGALTextureHandle* hTexture = pTargetPass->GetTextureHandle(renderTargets, pPass->GetInputPins()[j]);
          NS_ASSERT_DEV(m_ConnectionToTextureIndex.Contains(pConn), "");

          nsUInt32 uiDataIdx = m_ConnectionToTextureIndex[pConn];
          if (!hTexture)
          {
            m_TextureUsage[uiDataIdx].m_iTargetTextureIndex = -1;
            for (auto pUsedByConn : m_TextureUsage[uiDataIdx].m_UsedBy)
            {
              pUsedByConn->m_TextureHandle.Invalidate();
            }
          }
          else if (!hTexture->IsInvalidated() || pConn->m_Desc.CalculateHash() == defaultTextureDescHash)
          {
            m_TextureUsage[uiDataIdx].m_iTargetTextureIndex = static_cast<nsInt32>(hTexture - reinterpret_cast<const nsGALTextureHandle*>(&renderTargets));
            NS_ASSERT_DEV(reinterpret_cast<const nsGALTextureHandle*>(&renderTargets)[m_TextureUsage[uiDataIdx].m_iTargetTextureIndex] == *hTexture, "Offset computation broken.");

            for (auto pUsedByConn : m_TextureUsage[uiDataIdx].m_UsedBy)
            {
              pUsedByConn->m_TextureHandle = *hTexture;
            }
          }
          else
          {
            // In this case, the nsTargetPass does not provide a render target for the connection but the descriptor is set so we can instead use the pool to supplement the missing texture.
          }
        }
      }
    }
  }

  // Stupid loop to gather all TextureUsageData indices that are not view render target textures.
  for (nsUInt32 i = 0; i < m_TextureUsage.GetCount(); i++)
  {
    TextureUsageData& data = m_TextureUsage[i];
    if (data.m_iTargetTextureIndex != -1)
      continue;

    m_TextureUsageIdxSortedByFirstUsage.PushBack((nsUInt16)i);
    m_TextureUsageIdxSortedByLastUsage.PushBack((nsUInt16)i);
  }

  // Sort first and last usage arrays, these will determine the lifetime of the pool textures.
  struct FirstUsageComparer
  {
    FirstUsageComparer(nsDynamicArray<TextureUsageData>& ref_textureUsage)
      : m_TextureUsage(ref_textureUsage)
    {
    }

    NS_ALWAYS_INLINE bool Less(nsUInt16 a, nsUInt16 b) const { return m_TextureUsage[a].m_uiFirstUsageIdx < m_TextureUsage[b].m_uiFirstUsageIdx; }

    nsDynamicArray<TextureUsageData>& m_TextureUsage;
  };

  struct LastUsageComparer
  {
    LastUsageComparer(nsDynamicArray<TextureUsageData>& ref_textureUsage)
      : m_TextureUsage(ref_textureUsage)
    {
    }

    NS_ALWAYS_INLINE bool Less(nsUInt16 a, nsUInt16 b) const { return m_TextureUsage[a].m_uiLastUsageIdx < m_TextureUsage[b].m_uiLastUsageIdx; }

    nsDynamicArray<TextureUsageData>& m_TextureUsage;
  };

  m_TextureUsageIdxSortedByFirstUsage.Sort(FirstUsageComparer(m_TextureUsage));
  m_TextureUsageIdxSortedByLastUsage.Sort(LastUsageComparer(m_TextureUsage));

  return true;
}

bool nsRenderPipeline::InitRenderPipelinePasses()
{
  nsLogBlock b("Init Render Pipeline Passes");
  // Init every pass now.
  for (auto& pPass : m_Passes)
  {
    ConnectionData& data = m_Connections[pPass.Borrow()];
    pPass->InitRenderPipelinePass(data.m_Inputs, data.m_Outputs);
  }

  return true;
}

void nsRenderPipeline::SortExtractors()
{
  struct Helper
  {
    static bool FindDependency(const nsHashedString& sDependency, nsArrayPtr<nsUniquePtr<nsExtractor>> container)
    {
      for (auto& extractor : container)
      {
        if (sDependency == nsTempHashedString(extractor->GetDynamicRTTI()->GetTypeNameHash()))
        {
          return true;
        }
      }

      return false;
    }
  };

  m_SortedExtractors.Clear();
  m_SortedExtractors.Reserve(m_Extractors.GetCount());

  nsUInt32 uiIndex = 0;
  while (!m_Extractors.IsEmpty())
  {
    nsUniquePtr<nsExtractor>& extractor = m_Extractors[uiIndex];

    bool allDependenciesFound = true;
    for (auto& sDependency : extractor->m_DependsOn)
    {
      if (!Helper::FindDependency(sDependency, m_SortedExtractors))
      {
        allDependenciesFound = false;
        break;
      }
    }

    if (allDependenciesFound)
    {
      m_SortedExtractors.PushBack(std::move(extractor));
      m_Extractors.RemoveAtAndCopy(uiIndex);
    }
    else
    {
      ++uiIndex;
    }

    if (uiIndex >= m_Extractors.GetCount())
    {
      uiIndex = 0;
    }
  }

  m_Extractors.Swap(m_SortedExtractors);
}

void nsRenderPipeline::UpdateViewData(const nsView& view, nsUInt32 uiDataIndex)
{
  if (!view.IsValid())
    return;

  if (uiDataIndex == nsRenderWorld::GetDataIndexForExtraction() && m_CurrentExtractThread != (nsThreadID)0)
    return;

  NS_ASSERT_DEV(uiDataIndex <= 1, "Data index must be 0 or 1");
  auto& data = m_Data[uiDataIndex];

  data.SetCamera(*view.GetCamera());
  data.SetViewData(view.GetData());
}

void nsRenderPipeline::AddExtractor(nsUniquePtr<nsExtractor>&& pExtractor)
{
  m_Extractors.PushBack(std::move(pExtractor));
}

void nsRenderPipeline::RemoveExtractor(nsExtractor* pExtractor)
{
  for (nsUInt32 i = 0; i < m_Extractors.GetCount(); ++i)
  {
    if (m_Extractors[i].Borrow() == pExtractor)
    {
      m_Extractors.RemoveAtAndCopy(i);
      break;
    }
  }
}

void nsRenderPipeline::GetExtractors(nsDynamicArray<const nsExtractor*>& ref_extractors) const
{
  ref_extractors.Reserve(m_Extractors.GetCount());

  for (auto& pExtractor : m_Extractors)
  {
    ref_extractors.PushBack(pExtractor.Borrow());
  }
}

void nsRenderPipeline::GetExtractors(nsDynamicArray<nsExtractor*>& ref_extractors)
{
  ref_extractors.Reserve(m_Extractors.GetCount());

  for (auto& pExtractor : m_Extractors)
  {
    ref_extractors.PushBack(pExtractor.Borrow());
  }
}


nsExtractor* nsRenderPipeline::GetExtractorByName(const nsStringView& sExtractorName)
{
  for (auto& pExtractor : m_Extractors)
  {
    if (sExtractorName.IsEqual(pExtractor->GetName()))
    {
      return pExtractor.Borrow();
    }
  }

  return nullptr;
}

void nsRenderPipeline::RemoveConnections(nsRenderPipelinePass* pPass)
{
  auto it = m_Connections.Find(pPass);
  if (!it.IsValid())
    return;

  ConnectionData& data = it.Value();
  for (nsUInt32 i = 0; i < data.m_Inputs.GetCount(); i++)
  {
    nsRenderPipelinePassConnection* pConn = data.m_Inputs[i];
    if (pConn != nullptr)
    {
      nsRenderPipelinePass* pSource = static_cast<nsRenderPipelinePass*>(pConn->m_pOutput->m_pParent);
      bool bRes = Disconnect(pSource, pSource->GetPinName(pConn->m_pOutput), pPass, pPass->GetPinName(pPass->GetInputPins()[i]));
      NS_IGNORE_UNUSED(bRes);
      NS_ASSERT_DEBUG(bRes, "nsRenderPipeline::RemoveConnections should not fail to disconnect pins!");
    }
  }
  for (nsUInt32 i = 0; i < data.m_Outputs.GetCount(); i++)
  {
    nsRenderPipelinePassConnection* pConn = data.m_Outputs[i];
    while (pConn != nullptr)
    {
      nsRenderPipelinePass* pTarget = static_cast<nsRenderPipelinePass*>(pConn->m_Inputs[0]->m_pParent);
      bool bRes = Disconnect(pPass, pPass->GetPinName(pConn->m_pOutput), pTarget, pTarget->GetPinName(pConn->m_Inputs[0]));
      NS_IGNORE_UNUSED(bRes);
      NS_ASSERT_DEBUG(bRes, "nsRenderPipeline::RemoveConnections should not fail to disconnect pins!");

      pConn = data.m_Outputs[i];
    }
  }
}

void nsRenderPipeline::ClearRenderPassGraphTextures()
{
  m_TextureUsage.Clear();
  m_TextureUsageIdxSortedByFirstUsage.Clear();
  m_TextureUsageIdxSortedByLastUsage.Clear();

  // nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  for (auto it = m_Connections.GetIterator(); it.IsValid(); ++it)
  {
    auto& conn = it.Value();
    for (auto pConn : conn.m_Outputs)
    {
      if (pConn)
      {
        pConn->m_Desc = nsGALTextureCreationDescription();
        if (!pConn->m_TextureHandle.IsInvalidated())
        {
          pConn->m_TextureHandle.Invalidate();
        }
      }
    }
  }
}

bool nsRenderPipeline::AreInputDescriptionsAvailable(const nsRenderPipelinePass* pPass, const nsHybridArray<nsRenderPipelinePass*, 32>& done) const
{
  auto it = m_Connections.Find(pPass);
  const ConnectionData& data = it.Value();
  for (nsUInt32 i = 0; i < data.m_Inputs.GetCount(); i++)
  {
    const nsRenderPipelinePassConnection* pConn = data.m_Inputs[i];
    if (pConn != nullptr)
    {
      // If the connections source is not done yet, the connections output is undefined yet and the inputs can't be processed yet.
      if (!done.Contains(static_cast<nsRenderPipelinePass*>(pConn->m_pOutput->m_pParent)))
      {
        return false;
      }
    }
  }

  return true;
}

bool nsRenderPipeline::ArePassThroughInputsDone(const nsRenderPipelinePass* pPass, const nsHybridArray<nsRenderPipelinePass*, 32>& done) const
{
  auto it = m_Connections.Find(pPass);
  const ConnectionData& data = it.Value();
  auto inputs = pPass->GetInputPins();
  for (nsUInt32 i = 0; i < inputs.GetCount(); i++)
  {
    const nsRenderPipelineNodePin* pPin = inputs[i];
    if (pPin->m_Type == nsRenderPipelineNodePin::Type::PassThrough)
    {
      const nsRenderPipelinePassConnection* pConn = data.m_Inputs[pPin->m_uiInputIndex];
      if (pConn != nullptr)
      {
        for (const nsRenderPipelineNodePin* pInputPin : pConn->m_Inputs)
        {
          // Any input that is also connected to the source of pPin must be done before we can use the pass through input
          if (pInputPin != pPin && !done.Contains(static_cast<nsRenderPipelinePass*>(pInputPin->m_pParent)))
          {
            return false;
          }
        }
      }
    }
  }
  return true;
}

nsFrameDataProviderBase* nsRenderPipeline::GetFrameDataProvider(const nsRTTI* pRtti) const
{
  nsUInt32 uiIndex = 0;
  if (m_TypeToDataProviderIndex.TryGetValue(pRtti, uiIndex))
  {
    return m_DataProviders[uiIndex].Borrow();
  }

  nsUniquePtr<nsFrameDataProviderBase> pNewDataProvider = pRtti->GetAllocator()->Allocate<nsFrameDataProviderBase>();
  nsFrameDataProviderBase* pResult = pNewDataProvider.Borrow();
  pResult->m_pOwnerPipeline = this;

  m_TypeToDataProviderIndex.Insert(pRtti, m_DataProviders.GetCount());
  m_DataProviders.PushBack(std::move(pNewDataProvider));

  return pResult;
}

void nsRenderPipeline::ExtractData(const nsView& view)
{
  NS_ASSERT_DEV(m_CurrentExtractThread == (nsThreadID)0, "Extract must not be called from multiple threads.");
  m_CurrentExtractThread = nsThreadUtils::GetCurrentThreadID();

  // Is this view already extracted?
  if (m_uiLastExtractionFrame == nsRenderWorld::GetFrameCounter())
  {
    NS_REPORT_FAILURE("View '{0}' is extracted multiple times", view.GetName());
    return;
  }

  m_uiLastExtractionFrame = nsRenderWorld::GetFrameCounter();

  // Determine visible objects
  FindVisibleObjects(view);

  // Extract and sort data
  auto& data = m_Data[nsRenderWorld::GetDataIndexForExtraction()];

  // Usually clear is not needed, only if the multithreading flag is switched during runtime.
  data.Clear();

  // Store camera and viewdata
  data.SetCamera(*view.GetCamera());
  data.SetLodCamera(*view.GetLodCamera());
  data.SetViewData(view.GetData());
  data.SetWorldTime(view.GetWorld()->GetClock().GetAccumulatedTime());
  data.SetWorldDebugContext(view.GetWorld());
  data.SetViewDebugContext(view.GetHandle());

  // Extract object render data
  for (auto& pExtractor : m_Extractors)
  {
    if (pExtractor->m_bActive)
    {
      NS_PROFILE_SCOPE(pExtractor->m_sName.GetData());

      pExtractor->Extract(view, m_VisibleObjects, data);
    }
  }

  data.SortAndBatch();

  for (auto& pExtractor : m_Extractors)
  {
    if (pExtractor->m_bActive)
    {
      NS_PROFILE_SCOPE(pExtractor->m_sName.GetData());

      pExtractor->PostSortAndBatch(view, m_VisibleObjects, data);
    }
  }

  m_CurrentExtractThread = (nsThreadID)0;
}

nsUniquePtr<nsRasterizerViewPool> g_pRasterizerViewPool;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, SwRasterizer)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    g_pRasterizerViewPool = NS_DEFAULT_NEW(nsRasterizerViewPool);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    g_pRasterizerViewPool.Clear();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

void nsRenderPipeline::FindVisibleObjects(const nsView& view)
{
  NS_PROFILE_SCOPE("Visibility Culling");

  nsFrustum frustum;
  view.ComputeCullingFrustum(frustum);

  NS_LOCK(view.GetWorld()->GetReadMarker());

  const bool bIsMainView = (view.GetCameraUsageHint() == nsCameraUsageHint::MainView || view.GetCameraUsageHint() == nsCameraUsageHint::EditorView);
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  const bool bRecordStats = cvar_SpatialCullingShowStats && bIsMainView;
  nsSpatialSystem::QueryStats stats;
#endif

  nsSpatialSystem::QueryParams queryParams;
  queryParams.m_uiCategoryBitmask = nsDefaultSpatialDataCategories::RenderStatic.GetBitmask() | nsDefaultSpatialDataCategories::RenderDynamic.GetBitmask();
  queryParams.m_pIncludeTags = &view.m_IncludeTags;
  queryParams.m_pExcludeTags = &view.m_ExcludeTags;
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  queryParams.m_pStats = bRecordStats ? &stats : nullptr;
#endif

  nsFrustum limitedFrustum = frustum;
  const nsPlane farPlane = limitedFrustum.GetPlane(nsFrustum::PlaneType::FarPlane);
  limitedFrustum.AccessPlane(nsFrustum::PlaneType::FarPlane) = nsPlane::MakeFromNormalAndPoint(farPlane.m_vNormal, view.GetCullingCamera()->GetCenterPosition() + farPlane.m_vNormal * cvar_SpatialCullingOcclusionFarPlane.GetValue()); // only use occluders closer than this

  nsRasterizerView* pRasterizer = PrepareOcclusionCulling(limitedFrustum, view);
  NS_SCOPE_EXIT(g_pRasterizerViewPool->ReturnRasterizerView(pRasterizer));

  const nsVisibilityState visType = bIsMainView ? nsVisibilityState::Direct : nsVisibilityState::Indirect;

  if (pRasterizer != nullptr && pRasterizer->HasRasterizedAnyOccluders())
  {
    NS_PROFILE_SCOPE("Occlusion::FindVisibleObjects");

    auto IsOccluded = [=](const nsSimdBBox& aabb)
    {
      // grow the bbox by some percent to counter the lower precision of the occlusion buffer

      const nsSimdVec4f c = aabb.GetCenter();
      const nsSimdVec4f e = aabb.GetHalfExtents();
      const nsSimdBBox aabb2 = nsSimdBBox::MakeFromCenterAndHalfExtents(c, e.CompMul(nsSimdVec4f(1.0f + cvar_SpatialCullingOcclusionBoundsInlation)));

      return !pRasterizer->IsVisible(aabb2);
    };

    m_VisibleObjects.Clear();
    view.GetWorld()->GetSpatialSystem()->FindVisibleObjects(frustum, queryParams, m_VisibleObjects, IsOccluded, visType);
  }
  else
  {
    m_VisibleObjects.Clear();
    view.GetWorld()->GetSpatialSystem()->FindVisibleObjects(frustum, queryParams, m_VisibleObjects, {}, visType);
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (pRasterizer)
  {
    if (view.GetCameraUsageHint() == nsCameraUsageHint::EditorView || view.GetCameraUsageHint() == nsCameraUsageHint::MainView)
    {
      PreviewOcclusionBuffer(*pRasterizer, view);
    }
  }
#endif

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  nsViewHandle hView = view.GetHandle();

  if (cvar_SpatialCullingVis && bIsMainView)
  {
    nsDebugRenderer::DrawLineFrustum(view.GetWorld(), frustum, nsColor::LimeGreen, false);
  }

  if (bRecordStats)
  {
    nsStringBuilder sb;

    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "VisCulling", "Visibility Culling Stats", nsColor::LimeGreen);

    sb.SetFormat("Total Num Objects: {0}", stats.m_uiTotalNumObjects);
    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "VisCulling", sb, nsColor::LimeGreen);

    sb.SetFormat("Num Objects Tested: {0}", stats.m_uiNumObjectsTested);
    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "VisCulling", sb, nsColor::LimeGreen);

    sb.SetFormat("Num Objects Passed: {0}", stats.m_uiNumObjectsPassed);
    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "VisCulling", sb, nsColor::LimeGreen);

    // Exponential moving average for better readability.
    m_AverageCullingTime = nsMath::Lerp(m_AverageCullingTime, stats.m_TimeTaken, 0.05f);

    sb.SetFormat("Time Taken: {0}ms", m_AverageCullingTime.GetMilliseconds());
    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "VisCulling", sb, nsColor::LimeGreen);

    view.GetWorld()->GetSpatialSystem()->GetInternalStats(sb);
    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "VisCulling", sb, nsColor::AntiqueWhite);
  }
#endif
}

void nsRenderPipeline::Render(nsRenderContext* pRenderContext)
{
  // NS_PROFILE_AND_MARKER(pRenderContext->GetGALContext(), m_sName.GetData());
  NS_PROFILE_SCOPE(m_sName.GetData());

  NS_ASSERT_DEV(m_PipelineState != PipelineState::Uninitialized, "Pipeline must be rebuild before rendering.");
  if (m_PipelineState == PipelineState::RebuildError)
  {
    return;
  }

  NS_ASSERT_DEV(m_CurrentRenderThread == (nsThreadID)0, "Render must not be called from multiple threads.");
  m_CurrentRenderThread = nsThreadUtils::GetCurrentThreadID();

  NS_ASSERT_DEV(m_uiLastRenderFrame != nsRenderWorld::GetFrameCounter(), "Render must not be called multiple times per frame.");
  m_uiLastRenderFrame = nsRenderWorld::GetFrameCounter();


  auto& data = m_Data[nsRenderWorld::GetDataIndexForRendering()];
  const nsCamera* pCamera = &data.GetCamera();
  const nsCamera* pLodCamera = &data.GetLodCamera();
  const nsViewData* pViewData = &data.GetViewData();

  auto& gc = pRenderContext->WriteGlobalConstants();
  for (int i = 0; i < 2; ++i)
  {
    gc.CameraToScreenMatrix[i] = pViewData->m_ProjectionMatrix[i];
    gc.ScreenToCameraMatrix[i] = pViewData->m_InverseProjectionMatrix[i];
    gc.WorldToCameraMatrix[i] = pViewData->m_ViewMatrix[i];
    gc.CameraToWorldMatrix[i] = pViewData->m_InverseViewMatrix[i];
    gc.WorldToScreenMatrix[i] = pViewData->m_ViewProjectionMatrix[i];
    gc.ScreenToWorldMatrix[i] = pViewData->m_InverseViewProjectionMatrix[i];
  }

  const nsRectFloat& viewport = pViewData->m_ViewPortRect;
  gc.ViewportSize = nsVec4(viewport.width, viewport.height, 1.0f / viewport.width, 1.0f / viewport.height);

  float fNear = pCamera->GetNearPlane();
  float fFar = pCamera->GetFarPlane();
  gc.ClipPlanes = nsVec4(fNear, fFar, 1.0f / fFar, 0.0f);

  const bool bIsDirectionalLightShadow = pViewData->m_CameraUsageHint == nsCameraUsageHint::Shadow && pCamera->IsOrthographic();
  gc.MaxZValue = bIsDirectionalLightShadow ? 0.0f : nsMath::MinValue<float>();

  // Wrap around to prevent floating point issues. Wrap around is dividable by all whole numbers up to 11.
  gc.DeltaTime = (float)nsClock::GetGlobalClock()->GetTimeDiff().GetSeconds();
  gc.GlobalTime = (float)nsMath::Mod(nsClock::GetGlobalClock()->GetAccumulatedTime().GetSeconds(), 20790.0);
  gc.WorldTime = (float)nsMath::Mod(data.GetWorldTime().GetSeconds(), 20790.0);

  gc.Exposure = pCamera->GetExposure();
  gc.RenderPass = nsViewRenderMode::GetRenderPassForShader(pViewData->m_ViewRenderMode);

  nsRenderViewContext renderViewContext;
  renderViewContext.m_pCamera = pCamera;
  renderViewContext.m_pLodCamera = pLodCamera;
  renderViewContext.m_pViewData = pViewData;
  renderViewContext.m_pRenderContext = pRenderContext;
  renderViewContext.m_pWorldDebugContext = &data.GetWorldDebugContext();
  renderViewContext.m_pViewDebugContext = &data.GetViewDebugContext();

  // Set camera mode permutation variable here since it doesn't change throughout the frame
  static nsHashedString sCameraMode = nsMakeHashedString("CAMERA_MODE");
  static nsHashedString sOrtho = nsMakeHashedString("CAMERA_MODE_ORTHO");
  static nsHashedString sPerspective = nsMakeHashedString("CAMERA_MODE_PERSPECTIVE");
  static nsHashedString sStereo = nsMakeHashedString("CAMERA_MODE_STEREO");

  static nsHashedString sVSRTAI = nsMakeHashedString("VERTEX_SHADER_RENDER_TARGET_ARRAY_INDEX");
  static nsHashedString sClipSpaceFlipped = nsMakeHashedString("CLIP_SPACE_FLIPPED");
  static nsHashedString sTrue = nsMakeHashedString("TRUE");
  static nsHashedString sFalse = nsMakeHashedString("FALSE");

  if (pCamera->IsOrthographic())
    pRenderContext->SetShaderPermutationVariable(sCameraMode, sOrtho);
  else if (pCamera->IsStereoscopic())
    pRenderContext->SetShaderPermutationVariable(sCameraMode, sStereo);
  else
    pRenderContext->SetShaderPermutationVariable(sCameraMode, sPerspective);

  if (nsGALDevice::GetDefaultDevice()->GetCapabilities().m_bVertexShaderRenderTargetArrayIndex)
    pRenderContext->SetShaderPermutationVariable(sVSRTAI, sTrue);
  else
    pRenderContext->SetShaderPermutationVariable(sVSRTAI, sFalse);

  pRenderContext->SetShaderPermutationVariable(sClipSpaceFlipped, nsClipSpaceYMode::RenderToTextureDefault == nsClipSpaceYMode::Flipped ? sTrue : sFalse);

  // Also set pipeline specific permutation vars
  for (auto& var : m_PermutationVars)
  {
    pRenderContext->SetShaderPermutationVariable(var.m_sName, var.m_sValue);
  }

  nsRenderWorldRenderEvent renderEvent;
  renderEvent.m_Type = nsRenderWorldRenderEvent::Type::BeforePipelineExecution;
  renderEvent.m_pPipeline = this;
  renderEvent.m_pRenderViewContext = &renderViewContext;
  renderEvent.m_uiFrameCounter = nsRenderWorld::GetFrameCounter();
  {
    NS_PROFILE_SCOPE("BeforePipelineExecution");
    nsRenderWorld::s_RenderEvent.Broadcast(renderEvent);
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  pDevice->BeginPipeline(m_sName, renderViewContext.m_pViewData->m_hSwapChain);

  if (const nsGALSwapChain* pSwapChain = pDevice->GetSwapChain(renderViewContext.m_pViewData->m_hSwapChain))
  {
    const nsGALRenderTargets& renderTargets = pSwapChain->GetRenderTargets();
    // Update target textures after the swap chain acquired new textures.
    for (nsUInt32 i = 0; i < m_TextureUsage.GetCount(); i++)
    {
      TextureUsageData& textureUsageData = m_TextureUsage[i];
      if (textureUsageData.m_iTargetTextureIndex != -1)
      {
        nsGALTextureHandle hTexture = reinterpret_cast<const nsGALTextureHandle*>(&renderTargets)[textureUsageData.m_iTargetTextureIndex];
        for (auto pUsedByConn : textureUsageData.m_UsedBy)
        {
          pUsedByConn->m_TextureHandle = hTexture;
        }
      }
    }
  }

  nsUInt32 uiCurrentFirstUsageIdx = 0;
  nsUInt32 uiCurrentLastUsageIdx = 0;
  for (nsUInt32 i = 0; i < m_Passes.GetCount(); ++i)
  {
    auto& pPass = m_Passes[i];
    NS_PROFILE_SCOPE(pPass->GetName());
    nsLogBlock passBlock("Render Pass", pPass->GetName());

    // Create pool textures
    for (; uiCurrentFirstUsageIdx < m_TextureUsageIdxSortedByFirstUsage.GetCount();)
    {
      nsUInt16 uiCurrentUsageData = m_TextureUsageIdxSortedByFirstUsage[uiCurrentFirstUsageIdx];
      TextureUsageData& usageData = m_TextureUsage[uiCurrentUsageData];
      if (usageData.m_uiFirstUsageIdx == i)
      {
        nsGALTextureHandle hTexture = nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(usageData.m_UsedBy[0]->m_Desc);
        NS_ASSERT_DEV(!hTexture.IsInvalidated(), "GPU pool returned an invalidated texture!");
        for (nsRenderPipelinePassConnection* pConn : usageData.m_UsedBy)
        {
          pConn->m_TextureHandle = hTexture;
        }
        ++uiCurrentFirstUsageIdx;
      }
      else
      {
        // The current usage data blocks m_uiFirstUsageIdx isn't reached yet so wait.
        break;
      }
    }

    // Execute pass block
    {
      ConnectionData& connectionData = m_Connections[pPass.Borrow()];
      if (pPass->m_bActive)
      {
        pPass->Execute(renderViewContext, connectionData.m_Inputs, connectionData.m_Outputs);
      }
      else
      {
        pPass->ExecuteInactive(renderViewContext, connectionData.m_Inputs, connectionData.m_Outputs);
      }
    }

    // Release pool textures
    for (; uiCurrentLastUsageIdx < m_TextureUsageIdxSortedByLastUsage.GetCount();)
    {
      nsUInt16 uiCurrentUsageData = m_TextureUsageIdxSortedByLastUsage[uiCurrentLastUsageIdx];
      TextureUsageData& usageData = m_TextureUsage[uiCurrentUsageData];
      if (usageData.m_uiLastUsageIdx == i)
      {
        nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(usageData.m_UsedBy[0]->m_TextureHandle);
        for (nsRenderPipelinePassConnection* pConn : usageData.m_UsedBy)
        {
          pConn->m_TextureHandle.Invalidate();
        }
        ++uiCurrentLastUsageIdx;
      }
      else
      {
        // The current usage data blocks m_uiLastUsageIdx isn't reached yet so wait.
        break;
      }
    }
  }
  NS_ASSERT_DEV(uiCurrentFirstUsageIdx == m_TextureUsageIdxSortedByFirstUsage.GetCount(), "Rendering all passes should have moved us through all texture usage blocks!");
  NS_ASSERT_DEV(uiCurrentLastUsageIdx == m_TextureUsageIdxSortedByLastUsage.GetCount(), "Rendering all passes should have moved us through all texture usage blocks!");

  pDevice->EndPipeline(renderViewContext.m_pViewData->m_hSwapChain);

  renderEvent.m_Type = nsRenderWorldRenderEvent::Type::AfterPipelineExecution;
  {
    NS_PROFILE_SCOPE("AfterPipelineExecution");
    nsRenderWorld::s_RenderEvent.Broadcast(renderEvent);
  }

  pRenderContext->ResetContextState();

  data.Clear();

  m_CurrentRenderThread = (nsThreadID)0;
}

const nsExtractedRenderData& nsRenderPipeline::GetRenderData() const
{
  return m_Data[nsRenderWorld::GetDataIndexForRendering()];
}

nsRenderDataBatchList nsRenderPipeline::GetRenderDataBatchesWithCategory(nsRenderData::Category category, nsRenderDataBatch::Filter filter) const
{
  auto& data = m_Data[nsRenderWorld::GetDataIndexForRendering()];
  return data.GetRenderDataBatchesWithCategory(category, filter);
}

void nsRenderPipeline::CreateDgmlGraph(nsDGMLGraph& ref_graph)
{
  nsStringBuilder sTmp;
  nsHashTable<const nsRenderPipelineNode*, nsUInt32> nodeMap;
  nodeMap.Reserve(m_Passes.GetCount() + m_TextureUsage.GetCount() * 3);
  for (nsUInt32 p = 0; p < m_Passes.GetCount(); ++p)
  {
    const auto& pPass = m_Passes[p];
    sTmp.SetFormat("#{}: {}", p, nsStringUtils::IsNullOrEmpty(pPass->GetName()) ? pPass->GetDynamicRTTI()->GetTypeName() : pPass->GetName());

    nsDGMLGraph::NodeDesc nd;
    nd.m_Color = nsColor::Gray;
    nd.m_Shape = nsDGMLGraph::NodeShape::Rectangle;
    nsUInt32 uiGraphNode = ref_graph.AddNode(sTmp, &nd);
    nodeMap.Insert(pPass.Borrow(), uiGraphNode);
  }

  for (nsUInt32 i = 0; i < m_TextureUsage.GetCount(); ++i)
  {
    const TextureUsageData& data = m_TextureUsage[i];

    for (const nsRenderPipelinePassConnection* pCon : data.m_UsedBy)
    {
      nsDGMLGraph::NodeDesc nd;
      nd.m_Color = data.m_iTargetTextureIndex != -1 ? nsColor::Black : nsColorScheme::GetColor(static_cast<nsColorScheme::Enum>(i % nsColorScheme::Count), 4);
      nd.m_Shape = nsDGMLGraph::NodeShape::RoundedRectangle;

      nsStringBuilder sFormat;
      if (!nsReflectionUtils::EnumerationToString(nsGetStaticRTTI<nsGALResourceFormat>(), pCon->m_Desc.m_Format, sFormat, nsReflectionUtils::EnumConversionMode::ValueNameOnly))
      {
        sFormat.SetFormat("Unknown Format {}", (int)pCon->m_Desc.m_Format);
      }
      sTmp.SetFormat("{} #{}: {}x{}:{}, MSAA:{}, {}Format: {}", data.m_iTargetTextureIndex != -1 ? "RenderTarget" : "PoolTexture", i, pCon->m_Desc.m_uiWidth, pCon->m_Desc.m_uiHeight, pCon->m_Desc.m_uiArraySize, (int)pCon->m_Desc.m_SampleCount, nsGALResourceFormat::IsDepthFormat(pCon->m_Desc.m_Format) ? "Depth" : "Color", sFormat);
      nsUInt32 uiTextureNode = ref_graph.AddNode(sTmp, &nd);

      nsUInt32 uiOutputNode = *nodeMap.GetValue(pCon->m_pOutput->m_pParent);
      ref_graph.AddConnection(uiOutputNode, uiTextureNode, pCon->m_pOutput->m_pParent->GetPinName(pCon->m_pOutput));
      for (const nsRenderPipelineNodePin* pInput : pCon->m_Inputs)
      {
        nsUInt32 uiInputNode = *nodeMap.GetValue(pInput->m_pParent);
        ref_graph.AddConnection(uiTextureNode, uiInputNode, pInput->m_pParent->GetPinName(pInput));
      }
    }
  }
}

nsRasterizerView* nsRenderPipeline::PrepareOcclusionCulling(const nsFrustum& frustum, const nsView& view)
{
#if NS_ENABLED(NS_PLATFORM_ARCH_X86)
  if (!cvar_SpatialCullingOcclusionEnable)
    return nullptr;

  if (!nsSystemInformation::Get().GetCpuFeatures().IsAvx1Available())
    return nullptr;

  nsRasterizerView* pRasterizer = nullptr;

  // extract all occlusion geometry from the scene
  NS_PROFILE_SCOPE("Occlusion::RasterizeView");

  pRasterizer = g_pRasterizerViewPool->GetRasterizerView(static_cast<nsUInt32>(view.GetViewport().width / 2), static_cast<nsUInt32>(view.GetViewport().height / 2), (float)view.GetViewport().width / (float)view.GetViewport().height);
  pRasterizer->SetCamera(view.GetCullingCamera());

  {
    NS_PROFILE_SCOPE("Occlusion::FindOccluders");

    nsSpatialSystem::QueryParams queryParams;
    queryParams.m_uiCategoryBitmask = nsDefaultSpatialDataCategories::OcclusionStatic.GetBitmask() | nsDefaultSpatialDataCategories::OcclusionDynamic.GetBitmask();
    queryParams.m_pIncludeTags = &view.m_IncludeTags;
    queryParams.m_pExcludeTags = &view.m_ExcludeTags;

    m_VisibleObjects.Clear();
    view.GetWorld()->GetSpatialSystem()->FindVisibleObjects(frustum, queryParams, m_VisibleObjects, {}, nsVisibilityState::Indirect);
  }

  pRasterizer->BeginScene();

  for (const nsGameObject* pObj : m_VisibleObjects)
  {
    nsMsgExtractOccluderData msg;
    pObj->SendMessage(msg);

    for (const auto& ed : msg.m_ExtractedOccluderData)
    {
      pRasterizer->AddObject(ed.m_pObject, ed.m_Transform);
    }
  }

  pRasterizer->EndScene();

  return pRasterizer;
#else
  return nullptr;
#endif
}

void nsRenderPipeline::PreviewOcclusionBuffer(const nsRasterizerView& rasterizer, const nsView& view)
{
  if (!cvar_SpatialCullingOcclusionVisView || !rasterizer.HasRasterizedAnyOccluders())
    return;

  NS_PROFILE_SCOPE("Occlusion::DebugPreview");

  const nsUInt32 uiImgWidth = rasterizer.GetResolutionX();
  const nsUInt32 uiImgHeight = rasterizer.GetResolutionY();

  // get the debug image from the rasterizer
  nsDynamicArray<nsColorLinearUB> fb;
  fb.SetCountUninitialized(uiImgWidth * uiImgHeight);
  rasterizer.ReadBackFrame(fb);

  const float w = (float)uiImgWidth;
  const float h = (float)uiImgHeight;
  nsRectFloat rectInPixel1 = nsRectFloat(5.0f, 5.0f, w + 10, h + 10);
  nsRectFloat rectInPixel2 = nsRectFloat(10.0f, 10.0f, w, h);

  nsDebugRenderer::Draw2DRectangle(view.GetHandle(), rectInPixel1, 0.0f, nsColor::MediumPurple);

  // TODO: it would be better to update a single texture every frame, however since this is a render pass,
  // we currently can't create nested passes
  // so either this has to be done elsewhere, or nested passes have to be allowed
  if (false)
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

    // check whether we need to re-create the texture
    if (!m_hOcclusionDebugViewTexture.IsInvalidated())
    {
      const nsGALTexture* pTexture = pDevice->GetTexture(m_hOcclusionDebugViewTexture);

      if (pTexture->GetDescription().m_uiWidth != uiImgWidth ||
          pTexture->GetDescription().m_uiHeight != uiImgHeight)
      {
        pDevice->DestroyTexture(m_hOcclusionDebugViewTexture);
        m_hOcclusionDebugViewTexture.Invalidate();
      }
    }

    // create the texture
    if (m_hOcclusionDebugViewTexture.IsInvalidated())
    {
      nsGALTextureCreationDescription desc;
      desc.m_uiWidth = uiImgWidth;
      desc.m_uiHeight = uiImgHeight;
      desc.m_Format = nsGALResourceFormat::RGBAUByteNormalized;
      desc.m_ResourceAccess.m_bImmutable = false;

      m_hOcclusionDebugViewTexture = pDevice->CreateTexture(desc);
    }

    // upload the image to the texture
    {
      nsGALPass* pGALPass = pDevice->BeginPass("RasterizerDebugViewUpdate");
      auto pCommandEncoder = pGALPass->BeginCompute();

      nsBoundingBoxu32 destBox;
      destBox.m_vMin.SetZero();
      destBox.m_vMax = nsVec3U32(uiImgWidth, uiImgHeight, 1);

      nsGALSystemMemoryDescription sourceData;
      sourceData.m_pData = fb.GetData();
      sourceData.m_uiRowPitch = uiImgWidth * sizeof(nsColorLinearUB);

      pCommandEncoder->UpdateTexture(m_hOcclusionDebugViewTexture, nsGALTextureSubresource(), destBox, sourceData);

      pGALPass->EndCompute(pCommandEncoder);
      pDevice->EndPass(pGALPass);
    }

    nsDebugRenderer::Draw2DRectangle(view.GetHandle(), rectInPixel2, 0.0f, nsColor::White, pDevice->GetDefaultResourceView(m_hOcclusionDebugViewTexture), nsVec2(1, -1));
  }
  else
  {
    nsTexture2DResourceDescriptor d;
    d.m_DescGAL.m_uiWidth = rasterizer.GetResolutionX();
    d.m_DescGAL.m_uiHeight = rasterizer.GetResolutionY();
    d.m_DescGAL.m_Format = nsGALResourceFormat::RGBAByteNormalized;

    nsGALSystemMemoryDescription content[1];
    content[0].m_pData = fb.GetData();
    content[0].m_uiRowPitch = sizeof(nsColorLinearUB) * d.m_DescGAL.m_uiWidth;
    content[0].m_uiSlicePitch = content[0].m_uiRowPitch * d.m_DescGAL.m_uiHeight;
    d.m_InitialContent = content;

    static nsAtomicInteger32 name = 0;
    name.Increment();

    nsStringBuilder sName;
    sName.SetFormat("RasterizerPreview-{}", name);

    nsTexture2DResourceHandle hDebug = nsResourceManager::CreateResource<nsTexture2DResource>(sName, std::move(d));

    nsDebugRenderer::Draw2DRectangle(view.GetHandle(), rectInPixel2, 0.0f, nsColor::White, hDebug, nsVec2(1, -1));
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_RenderPipeline);
