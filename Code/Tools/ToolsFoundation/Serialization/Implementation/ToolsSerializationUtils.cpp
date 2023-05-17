#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>
#include <ToolsFoundation/Serialization/ToolsSerializationUtils.h>

void wdToolsSerializationUtils::SerializeTypes(const wdSet<const wdRTTI*>& types, wdAbstractObjectGraph& ref_typesGraph)
{
  wdRttiConverterContext context;
  wdRttiConverterWriter rttiConverter(&ref_typesGraph, &context, true, true);
  for (const wdRTTI* pType : types)
  {
    wdReflectedTypeDescriptor desc;
    if (pType->GetTypeFlags().IsSet(wdTypeFlags::Phantom))
    {
      wdToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(pType, desc);
    }
    else
    {
      wdToolsReflectionUtils::GetMinimalReflectedTypeDescriptorFromRtti(pType, desc);
    }

    context.RegisterObject(wdUuid::StableUuidForString(pType->GetTypeName()), wdGetStaticRTTI<wdReflectedTypeDescriptor>(), &desc);
    rttiConverter.AddObjectToGraph(wdGetStaticRTTI<wdReflectedTypeDescriptor>(), &desc);
  }
}

void wdToolsSerializationUtils::CopyProperties(const wdDocumentObject* pSource, const wdDocumentObjectManager* pSourceManager, void* pTarget, const wdRTTI* pTargetType, FilterFunction propertFilter)
{
  wdAbstractObjectGraph graph;
  wdDocumentObjectConverterWriter writer(&graph, pSourceManager, [](const wdDocumentObject*, const wdAbstractProperty* p) { return p->GetAttributeByType<wdHiddenAttribute>() == nullptr; });
  wdAbstractObjectNode* pAbstractObj = writer.AddObjectToGraph(pSource);

  wdRttiConverterContext context;
  wdRttiConverterReader reader(&graph, &context);

  reader.ApplyPropertiesToObject(pAbstractObj, pTargetType, pTarget);
}
