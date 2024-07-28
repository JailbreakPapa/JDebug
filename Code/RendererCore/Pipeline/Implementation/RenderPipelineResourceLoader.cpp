#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/SerializationContext.h>
#include <Foundation/IO/StringDeduplicationContext.h>
#include <Foundation/IO/TypeVersionContext.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Serialization/BinarySerializer.h>
#include <RendererCore/Pipeline/Extractor.h>
#include <RendererCore/Pipeline/Implementation/RenderPipelineResourceLoader.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Pipeline/RenderPipelineResource.h>

////////////////////////////////////////////////////////////////////////
// nsDocumentNodeManager Internal
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsRenderPipelineResourceLoaderConnection, nsNoBase, 1, nsRTTIDefaultAllocator<nsRenderPipelineResourceLoaderConnection>)
{
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsResult nsRenderPipelineResourceLoaderConnection::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_uiSource;
  inout_stream << m_uiTarget;
  inout_stream << m_sSourcePin;
  inout_stream << m_sTargetPin;

  return NS_SUCCESS;
}

nsResult nsRenderPipelineResourceLoaderConnection::Deserialize(nsStreamReader& inout_stream)
{
  NS_VERIFY(nsTypeVersionReadContext::GetContext()->GetTypeVersion(nsGetStaticRTTI<nsRenderPipelineResourceLoaderConnection>()) == 1, "Unknown version");

  inout_stream >> m_uiSource;
  inout_stream >> m_uiTarget;
  inout_stream >> m_sSourcePin;
  inout_stream >> m_sTargetPin;

  return NS_SUCCESS;
}

constexpr nsTypeVersion s_RenderPipelineDescriptorVersion = 1;

// static
nsInternal::NewInstance<nsRenderPipeline> nsRenderPipelineResourceLoader::CreateRenderPipeline(const nsRenderPipelineResourceDescriptor& desc)
{
  auto pPipeline = NS_DEFAULT_NEW(nsRenderPipeline);

  nsRawMemoryStreamReader inout_stream(desc.m_SerializedPipeline);

  const auto uiVersion = inout_stream.ReadVersion(s_RenderPipelineDescriptorVersion);
  NS_IGNORE_UNUSED(uiVersion);

  nsStringDeduplicationReadContext stringDeduplicationReadContext(inout_stream);
  nsTypeVersionReadContext typeVersionReadContext(inout_stream);

  nsStringBuilder sTypeName;

  nsHybridArray<nsRenderPipelinePass*, 16> passes;

  // Passes
  {
    nsUInt32 uiNumPasses = 0;
    inout_stream >> uiNumPasses;

    for (nsUInt32 i = 0; i < uiNumPasses; ++i)
    {
      inout_stream >> sTypeName;
      if (const nsRTTI* pType = nsRTTI::FindTypeByName(sTypeName))
      {
        nsUniquePtr<nsRenderPipelinePass> pPass = pType->GetAllocator()->Allocate<nsRenderPipelinePass>();
        pPass->Deserialize(inout_stream).AssertSuccess("");
        passes.PushBack(pPass.Borrow());
        pPipeline->AddPass(std::move(pPass));
      }
      else
      {
        nsLog::Error("Unknown render pipeline pass type '{}'", sTypeName);
        return nullptr;
      }
    }
  }

  // Extractors
  {
    nsUInt32 uiNumExtractors = 0;
    inout_stream >> uiNumExtractors;

    for (nsUInt32 i = 0; i < uiNumExtractors; ++i)
    {
      inout_stream >> sTypeName;
      if (const nsRTTI* pType = nsRTTI::FindTypeByName(sTypeName))
      {
        nsUniquePtr<nsExtractor> pExtractor = pType->GetAllocator()->Allocate<nsExtractor>();
        pExtractor->Deserialize(inout_stream).AssertSuccess("");
        pPipeline->AddExtractor(std::move(pExtractor));
      }
      else
      {
        nsLog::Error("Unknown render pipeline extractor type '{}'", sTypeName);
        return nullptr;
      }
    }
  }

  // Connections
  {
    nsUInt32 uiNumConnections = 0;
    inout_stream >> uiNumConnections;

    for (nsUInt32 i = 0; i < uiNumConnections; ++i)
    {
      nsRenderPipelineResourceLoaderConnection data;
      data.Deserialize(inout_stream).AssertSuccess("Failed to deserialize render pipeline connection");

      nsRenderPipelinePass* pSource = passes[data.m_uiSource];
      nsRenderPipelinePass* pTarget = passes[data.m_uiTarget];

      if (!pPipeline->Connect(pSource, data.m_sSourcePin, pTarget, data.m_sTargetPin))
      {
        nsLog::Error("Failed to connect '{0}'::'{1}' to '{2}'::'{3}'!", pSource->GetName(), data.m_sSourcePin, pTarget->GetName(), data.m_sTargetPin);
      }
    }
  }
  return pPipeline;
}

// static
void nsRenderPipelineResourceLoader::CreateRenderPipelineResourceDescriptor(const nsRenderPipeline* pPipeline, nsRenderPipelineResourceDescriptor& ref_desc)
{
  nsHybridArray<const nsRenderPipelinePass*, 16> passes;
  nsHybridArray<const nsExtractor*, 16> extractors;
  nsHybridArray<nsRenderPipelineResourceLoaderConnection, 16> connections;

  nsHashTable<const nsRenderPipelineNode*, nsUInt32> passToIndex;
  pPipeline->GetPasses(passes);
  pPipeline->GetExtractors(extractors);

  passToIndex.Reserve(passes.GetCount());
  for (nsUInt32 i = 0; i < passes.GetCount(); i++)
  {
    passToIndex.Insert(passes[i], i);
  }


  for (nsUInt32 i = 0; i < passes.GetCount(); i++)
  {
    const nsRenderPipelinePass* pSource = passes[i];

    nsRenderPipelineResourceLoaderConnection data;
    data.m_uiSource = i;

    auto outputs = pSource->GetOutputPins();
    for (const nsRenderPipelineNodePin* pPinSource : outputs)
    {
      data.m_sSourcePin = pSource->GetPinName(pPinSource).GetView();

      const nsRenderPipelinePassConnection* pConnection = pPipeline->GetOutputConnection(pSource, pSource->GetPinName(pPinSource));
      if (!pConnection)
        continue;

      for (const nsRenderPipelineNodePin* pPinTarget : pConnection->m_Inputs)
      {
        NS_VERIFY(passToIndex.TryGetValue(pPinTarget->m_pParent, data.m_uiTarget), "Failed to resolve render pass to index");
        data.m_sTargetPin = pPinTarget->m_pParent->GetPinName(pPinTarget).GetView();

        connections.PushBack(data);
      }
    }
  }

  nsMemoryStreamContainerWrapperStorage<nsDynamicArray<nsUInt8>> storage(&ref_desc.m_SerializedPipeline);
  nsMemoryStreamWriter memoryWriter(&storage);
  ExportPipeline(passes.GetArrayPtr(), extractors.GetArrayPtr(), connections.GetArrayPtr(), memoryWriter).AssertSuccess("Failed to serialize pipeline");
}

nsResult nsRenderPipelineResourceLoader::ExportPipeline(nsArrayPtr<const nsRenderPipelinePass* const> passes, nsArrayPtr<const nsExtractor* const> extractors, nsArrayPtr<const nsRenderPipelineResourceLoaderConnection> connections, nsStreamWriter& ref_streamWriter)
{
  ref_streamWriter.WriteVersion(s_RenderPipelineDescriptorVersion);

  nsStringDeduplicationWriteContext stringDeduplicationWriteContext(ref_streamWriter);
  nsTypeVersionWriteContext typeVersionWriteContext;
  auto& stream = typeVersionWriteContext.Begin(stringDeduplicationWriteContext.Begin());

  // passes
  {
    const nsUInt32 uiNumPasses = passes.GetCount();
    stream << uiNumPasses;

    for (auto& pass : passes)
    {
      auto pPassType = pass->GetDynamicRTTI();
      typeVersionWriteContext.AddType(pPassType);

      stream << pPassType->GetTypeName();
      NS_SUCCEED_OR_RETURN(pass->Serialize(stream));
    }
  }

  // extractors
  {
    const nsUInt32 uiNumExtractors = extractors.GetCount();
    stream << uiNumExtractors;

    for (auto& extractor : extractors)
    {
      auto pExtractorType = extractor->GetDynamicRTTI();
      typeVersionWriteContext.AddType(pExtractorType);

      stream << pExtractorType->GetTypeName();
      NS_SUCCEED_OR_RETURN(extractor->Serialize(stream));
    }
  }

  // Connections
  {
    const nsUInt32 uiNumConnections = connections.GetCount();
    stream << uiNumConnections;

    typeVersionWriteContext.AddType(nsGetStaticRTTI<nsRenderPipelineResourceLoaderConnection>());

    for (auto& connection : connections)
    {
      NS_SUCCEED_OR_RETURN(connection.Serialize(stream));
    }
  }

  NS_SUCCEED_OR_RETURN(typeVersionWriteContext.End());
  NS_SUCCEED_OR_RETURN(stringDeduplicationWriteContext.End());

  return NS_SUCCESS;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_RenderPipelineResourceLoader);
