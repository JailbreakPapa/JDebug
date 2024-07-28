#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/RenderPipelineNode.h>

// NS_CHECK_AT_COMPILETIME(sizeof(nsRenderPipelineNodePin) == 4);

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRenderPipelineNode, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsRenderPipelineNodePin, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_ATTRIBUTES
  {
   new nsHiddenAttribute(),
  }
  NS_END_ATTRIBUTES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsRenderPipelineNodeInputPin, nsRenderPipelineNodePin, 1, nsRTTINoAllocator)
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsRenderPipelineNodeOutputPin, nsRenderPipelineNodePin, 1, nsRTTINoAllocator)
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsRenderPipelineNodePassThrougPin, nsRenderPipelineNodePin, 1, nsRTTINoAllocator)
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

void nsRenderPipelineNode::InitializePins()
{
  m_InputPins.Clear();
  m_OutputPins.Clear();
  m_NameToPin.Clear();

  const nsRTTI* pType = GetDynamicRTTI();

  nsHybridArray<const nsAbstractProperty*, 32> properties;
  pType->GetAllProperties(properties);

  for (auto pProp : properties)
  {
    if (pProp->GetCategory() != nsPropertyCategory::Member || !pProp->GetSpecificType()->IsDerivedFrom(nsGetStaticRTTI<nsRenderPipelineNodePin>()))
      continue;

    auto pPinProp = static_cast<const nsAbstractMemberProperty*>(pProp);
    nsRenderPipelineNodePin* pPin = static_cast<nsRenderPipelineNodePin*>(pPinProp->GetPropertyPointer(this));

    pPin->m_pParent = this;
    if (pPin->m_Type == nsRenderPipelineNodePin::Type::Unknown)
    {
      NS_REPORT_FAILURE("Pin '{0}' has an invalid type. Do not use nsRenderPipelineNodePin directly as member but one of its derived types", pProp->GetPropertyName());
      continue;
    }

    if (pPin->m_Type == nsRenderPipelineNodePin::Type::Input || pPin->m_Type == nsRenderPipelineNodePin::Type::PassThrough)
    {
      pPin->m_uiInputIndex = static_cast<nsUInt8>(m_InputPins.GetCount());
      m_InputPins.PushBack(pPin);
    }
    if (pPin->m_Type == nsRenderPipelineNodePin::Type::Output || pPin->m_Type == nsRenderPipelineNodePin::Type::PassThrough)
    {
      pPin->m_uiOutputIndex = static_cast<nsUInt8>(m_OutputPins.GetCount());
      m_OutputPins.PushBack(pPin);
    }

    nsHashedString sHashedName;
    sHashedName.Assign(pProp->GetPropertyName());
    m_NameToPin.Insert(sHashedName, pPin);
  }
}

nsHashedString nsRenderPipelineNode::GetPinName(const nsRenderPipelineNodePin* pPin) const
{
  for (auto it = m_NameToPin.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value() == pPin)
    {
      return it.Key();
    }
  }
  return nsHashedString();
}

const nsRenderPipelineNodePin* nsRenderPipelineNode::GetPinByName(const char* szName) const
{
  nsHashedString sHashedName;
  sHashedName.Assign(szName);
  return GetPinByName(sHashedName);
}

const nsRenderPipelineNodePin* nsRenderPipelineNode::GetPinByName(nsHashedString sName) const
{
  const nsRenderPipelineNodePin* pin;
  if (m_NameToPin.TryGetValue(sName, pin))
  {
    return pin;
  }

  return nullptr;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_RenderPipelineNode);
