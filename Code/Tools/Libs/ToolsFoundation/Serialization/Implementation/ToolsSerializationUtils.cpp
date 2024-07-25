#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>
#include <ToolsFoundation/Serialization/ToolsSerializationUtils.h>

void nsToolsSerializationUtils::SerializeTypes(const nsSet<const nsRTTI*>& types, nsAbstractObjectGraph& ref_typesGraph)
{
  nsRttiConverterContext context;
  nsRttiConverterWriter rttiConverter(&ref_typesGraph, &context, true, true);
  for (const nsRTTI* pType : types)
  {
    nsReflectedTypeDescriptor desc;
    if (pType->GetTypeFlags().IsSet(nsTypeFlags::Phantom))
    {
      nsToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(pType, desc);
    }
    else
    {
      nsToolsReflectionUtils::GetMinimalReflectedTypeDescriptorFromRtti(pType, desc);
    }

    context.RegisterObject(nsUuid::MakeStableUuidFromString(pType->GetTypeName()), nsGetStaticRTTI<nsReflectedTypeDescriptor>(), &desc);
    rttiConverter.AddObjectToGraph(nsGetStaticRTTI<nsReflectedTypeDescriptor>(), &desc);
  }
}

void nsToolsSerializationUtils::CopyProperties(const nsDocumentObject* pSource, const nsDocumentObjectManager* pSourceManager, void* pTarget, const nsRTTI* pTargetType, FilterFunction propertFilter)
{
  nsAbstractObjectGraph graph;
  nsDocumentObjectConverterWriter writer(&graph, pSourceManager, [](const nsDocumentObject*, const nsAbstractProperty* p)
    { return p->GetAttributeByType<nsHiddenAttribute>() == nullptr; });
  nsAbstractObjectNode* pAbstractObj = writer.AddObjectToGraph(pSource);

  nsRttiConverterContext context;
  nsRttiConverterReader reader(&graph, &context);

  reader.ApplyPropertiesToObject(pAbstractObj, pTargetType, pTarget);
}
