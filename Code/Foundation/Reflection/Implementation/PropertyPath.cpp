#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/PropertyPath.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Types/UniquePtr.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsPropertyPathStep, nsNoBase, 1, nsRTTIDefaultAllocator<nsPropertyPathStep>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Property", m_sProperty),
    NS_MEMBER_PROPERTY("Index", m_Index),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsPropertyPath::nsPropertyPath() = default;
nsPropertyPath::~nsPropertyPath() = default;

bool nsPropertyPath::IsValid() const
{
  return m_bIsValid;
}

nsResult nsPropertyPath::InitializeFromPath(const nsRTTI& rootObjectRtti, const char* szPath)
{
  m_bIsValid = false;

  const nsStringBuilder sPathParts = szPath;
  nsStringBuilder sIndex;
  nsStringBuilder sFieldName;

  nsHybridArray<nsStringView, 4> parts;
  sPathParts.Split(false, parts, "/");

  // an empty path is valid as well

  m_PathSteps.Clear();
  m_PathSteps.Reserve(parts.GetCount());

  const nsRTTI* pCurRtti = &rootObjectRtti;

  for (const nsStringView& part : parts)
  {
    if (part.EndsWith("]"))
    {
      const char* szBracket = part.FindSubString("[");

      sIndex.SetSubString_FromTo(szBracket + 1, part.GetEndPointer() - 1);

      sFieldName.SetSubString_FromTo(part.GetStartPointer(), szBracket);
    }
    else
    {
      sFieldName = part;
      sIndex.Clear();
    }

    const nsAbstractProperty* pAbsProp = pCurRtti->FindPropertyByName(sFieldName);

    if (pAbsProp == nullptr)
      return NS_FAILURE;

    auto& step = m_PathSteps.ExpandAndGetRef();
    step.m_pProperty = pAbsProp;

    if (pAbsProp->GetCategory() == nsPropertyCategory::Array)
    {
      if (sIndex.IsEmpty())
      {
        step.m_Index = nsVariant();
      }
      else
      {
        nsInt32 iIndex;
        NS_SUCCEED_OR_RETURN(nsConversionUtils::StringToInt(sIndex, iIndex));
        step.m_Index = iIndex;
      }
    }
    else if (pAbsProp->GetCategory() == nsPropertyCategory::Set)
    {
      if (sIndex.IsEmpty())
      {
        step.m_Index = nsVariant();
      }
      else
      {
        return NS_FAILURE;
      }
    }
    else if (pAbsProp->GetCategory() == nsPropertyCategory::Map)
    {
      step.m_Index = sIndex.IsEmpty() ? nsVariant() : nsVariant(sIndex.GetData());
    }

    pCurRtti = pAbsProp->GetSpecificType();
  }

  m_bIsValid = true;
  return NS_SUCCESS;
}

nsResult nsPropertyPath::InitializeFromPath(const nsRTTI* pRootObjectRtti, const nsArrayPtr<const nsPropertyPathStep> path)
{
  m_bIsValid = false;

  m_PathSteps.Clear();
  m_PathSteps.Reserve(path.GetCount());

  const nsRTTI* pCurRtti = pRootObjectRtti;
  for (const nsPropertyPathStep& pathStep : path)
  {
    const nsAbstractProperty* pAbsProp = pCurRtti->FindPropertyByName(pathStep.m_sProperty);
    if (pAbsProp == nullptr)
      return NS_FAILURE;

    auto& step = m_PathSteps.ExpandAndGetRef();
    step.m_pProperty = pAbsProp;
    step.m_Index = pathStep.m_Index;

    pCurRtti = pAbsProp->GetSpecificType();
  }

  m_bIsValid = true;
  return NS_SUCCESS;
}

nsResult nsPropertyPath::WriteToLeafObject(void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeaf, const nsRTTI& pType)> func) const
{
  NS_ASSERT_DEBUG(
    m_PathSteps.IsEmpty() || m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetTypeFlags().IsSet(nsTypeFlags::Class),
    "To resolve the leaf object the path needs to be empty or end in a class.");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr(), true, func);
}

nsResult nsPropertyPath::ReadFromLeafObject(void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeaf, const nsRTTI& pType)> func) const
{
  NS_ASSERT_DEBUG(
    m_PathSteps.IsEmpty() || m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetTypeFlags().IsSet(nsTypeFlags::Class),
    "To resolve the leaf object the path needs to be empty or end in a class.");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr(), false, func);
}

nsResult nsPropertyPath::WriteProperty(
  void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeafObject, const nsRTTI& pLeafType, const nsAbstractProperty* pProp, const nsVariant& index)> func) const
{
  NS_ASSERT_DEBUG(!m_PathSteps.IsEmpty(), "Call InitializeFromPath before WriteToObject");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr().GetSubArray(0, m_PathSteps.GetCount() - 1), true,
    [this, &func](void* pLeafObject, const nsRTTI& leafType)
    {
      auto& lastStep = m_PathSteps[m_PathSteps.GetCount() - 1];
      func(pLeafObject, leafType, lastStep.m_pProperty, lastStep.m_Index);
    });
}

nsResult nsPropertyPath::ReadProperty(
  void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeafObject, const nsRTTI& pLeafType, const nsAbstractProperty* pProp, const nsVariant& index)> func) const
{
  NS_ASSERT_DEBUG(m_bIsValid, "Call InitializeFromPath before WriteToObject");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr().GetSubArray(0, m_PathSteps.GetCount() - 1), false,
    [this, &func](void* pLeafObject, const nsRTTI& leafType)
    {
      auto& lastStep = m_PathSteps[m_PathSteps.GetCount() - 1];
      func(pLeafObject, leafType, lastStep.m_pProperty, lastStep.m_Index);
    });
}

void nsPropertyPath::SetValue(void* pRootObject, const nsRTTI& type, const nsVariant& value) const
{
  // NS_ASSERT_DEBUG(!m_PathSteps.IsEmpty() &&
  //                    value.CanConvertTo(m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetVariantType()),
  //                "The given value does not match the type at the given path.");

  WriteProperty(pRootObject, type, [&value](void* pLeaf, const nsRTTI& type, const nsAbstractProperty* pProp, const nsVariant& index)
    {
    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Member:
        nsReflectionUtils::SetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), pLeaf, value);
        break;
      case nsPropertyCategory::Array:
        nsReflectionUtils::SetArrayPropertyValue(static_cast<const nsAbstractArrayProperty*>(pProp), pLeaf, index.Get<nsInt32>(), value);
        break;
      case nsPropertyCategory::Map:
        nsReflectionUtils::SetMapPropertyValue(static_cast<const nsAbstractMapProperty*>(pProp), pLeaf, index.Get<nsString>(), value);
        break;
      default:
        NS_ASSERT_NOT_IMPLEMENTED;
        break;
    } })
    .IgnoreResult();
}

void nsPropertyPath::GetValue(void* pRootObject, const nsRTTI& type, nsVariant& out_value) const
{
  // NS_ASSERT_DEBUG(!m_PathSteps.IsEmpty() &&
  //                    m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetVariantType() != nsVariantType::Invalid,
  //                "The property path of value {} cannot be stored in an nsVariant.", m_PathSteps[m_PathSteps.GetCount() -
  //                1].m_pProperty->GetSpecificType()->GetTypeName());

  ReadProperty(pRootObject, type, [&out_value](void* pLeaf, const nsRTTI& type, const nsAbstractProperty* pProp, const nsVariant& index)
    {
    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Member:
        out_value = nsReflectionUtils::GetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), pLeaf);
        break;
      case nsPropertyCategory::Array:
        out_value = nsReflectionUtils::GetArrayPropertyValue(static_cast<const nsAbstractArrayProperty*>(pProp), pLeaf, index.Get<nsInt32>());
        break;
      case nsPropertyCategory::Map:
        out_value = nsReflectionUtils::GetMapPropertyValue(static_cast<const nsAbstractMapProperty*>(pProp), pLeaf, index.Get<nsString>());
        break;
      default:
        NS_ASSERT_NOT_IMPLEMENTED;
        break;
    } })
    .IgnoreResult();
}

nsResult nsPropertyPath::ResolvePath(void* pCurrentObject, const nsRTTI* pType, const nsArrayPtr<const ResolvedStep> path, bool bWriteToObject,
  const nsDelegate<void(void* pLeaf, const nsRTTI& pType)>& func)
{
  if (path.IsEmpty())
  {
    func(pCurrentObject, *pType);
    return NS_SUCCESS;
  }
  else // Recurse
  {
    const nsAbstractProperty* pProp = path[0].m_pProperty;
    const nsRTTI* pPropType = pProp->GetSpecificType();

    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Member:
      {
        auto pSpecific = static_cast<const nsAbstractMemberProperty*>(pProp);
        if (pPropType->GetProperties().GetCount() > 0)
        {
          void* pSubObject = pSpecific->GetPropertyPointer(pCurrentObject);
          // Do we have direct access to the property?
          if (pSubObject != nullptr)
          {
            return ResolvePath(pSubObject, pProp->GetSpecificType(), path.GetSubArray(1), bWriteToObject, func);
          }
          // If the property is behind an accessor, we need to retrieve it first.
          else if (pPropType->GetAllocator()->CanAllocate())
          {
            void* pRetrievedSubObject = pPropType->GetAllocator()->Allocate<void>();
            pSpecific->GetValuePtr(pCurrentObject, pRetrievedSubObject);

            nsResult res = ResolvePath(pRetrievedSubObject, pProp->GetSpecificType(), path.GetSubArray(1), bWriteToObject, func);

            if (bWriteToObject)
              pSpecific->SetValuePtr(pCurrentObject, pRetrievedSubObject);

            pPropType->GetAllocator()->Deallocate(pRetrievedSubObject);
            return res;
          }
          else
          {
            NS_REPORT_FAILURE("Non-allocatable property should not be part of an object chain!");
          }
        }
      }
      break;
      case nsPropertyCategory::Array:
      {
        auto pSpecific = static_cast<const nsAbstractArrayProperty*>(pProp);

        if (pPropType->GetAllocator()->CanAllocate())
        {
          const nsUInt32 uiIndex = path[0].m_Index.ConvertTo<nsUInt32>();
          if (uiIndex >= pSpecific->GetCount(pCurrentObject))
            return NS_FAILURE;

          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();
          pSpecific->GetValue(pCurrentObject, uiIndex, pSubObject);

          nsResult res = ResolvePath(pSubObject, pProp->GetSpecificType(), path.GetSubArray(1), bWriteToObject, func);

          if (bWriteToObject)
            pSpecific->SetValue(pCurrentObject, uiIndex, pSubObject);

          pPropType->GetAllocator()->Deallocate(pSubObject);
          return res;
        }
        else
        {
          NS_REPORT_FAILURE("Non-allocatable property should not be part of an object chain!");
        }
      }
      break;
      case nsPropertyCategory::Map:
      {
        auto pSpecific = static_cast<const nsAbstractMapProperty*>(pProp);
        const nsString& sKey = path[0].m_Index.Get<nsString>();
        if (!pSpecific->Contains(pCurrentObject, sKey))
          return NS_FAILURE;

        if (pPropType->GetAllocator()->CanAllocate())
        {
          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();

          pSpecific->GetValue(pCurrentObject, sKey, pSubObject);

          nsResult res = ResolvePath(pSubObject, pProp->GetSpecificType(), path.GetSubArray(1), bWriteToObject, func);

          if (bWriteToObject)
            pSpecific->Insert(pCurrentObject, sKey, pSubObject);

          pPropType->GetAllocator()->Deallocate(pSubObject);
          return res;
        }
        else
        {
          NS_REPORT_FAILURE("Non-allocatable property should not be part of an object chain!");
        }
      }
      break;
      case nsPropertyCategory::Set:
      default:
      {
        NS_REPORT_FAILURE("Property of type Set should not be part of an object chain!");
      }
      break;
    }
    return NS_FAILURE;
  }
}



NS_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_PropertyPath);
