#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/PropertyPath.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Types/UniquePtr.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdPropertyPathStep, wdNoBase, 1, wdRTTIDefaultAllocator<wdPropertyPathStep>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Property", m_sProperty),
    WD_MEMBER_PROPERTY("Index", m_Index),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdPropertyPath::wdPropertyPath() = default;
wdPropertyPath::~wdPropertyPath() = default;


bool wdPropertyPath::IsValid() const
{
  return m_bIsValid;
}

wdResult wdPropertyPath::InitializeFromPath(const wdRTTI& rootObjectRtti, const char* szPath)
{
  m_bIsValid = false;

  const wdStringBuilder sPathParts = szPath;
  wdStringBuilder sIndex;
  wdStringBuilder sFieldName;

  wdHybridArray<wdStringView, 4> parts;
  sPathParts.Split(false, parts, "/");

  // an empty path is valid as well

  m_PathSteps.Clear();
  m_PathSteps.Reserve(parts.GetCount());

  const wdRTTI* pCurRtti = &rootObjectRtti;

  for (const wdStringView& part : parts)
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

    wdAbstractProperty* pAbsProp = pCurRtti->FindPropertyByName(sFieldName);

    if (pAbsProp == nullptr)
      return WD_FAILURE;

    auto& step = m_PathSteps.ExpandAndGetRef();
    step.m_pProperty = pAbsProp;

    if (pAbsProp->GetCategory() == wdPropertyCategory::Array)
    {
      if (sIndex.IsEmpty())
      {
        step.m_Index = wdVariant();
      }
      else
      {
        wdInt32 iIndex;
        WD_SUCCEED_OR_RETURN(wdConversionUtils::StringToInt(sIndex, iIndex));
        step.m_Index = iIndex;
      }
    }
    else if (pAbsProp->GetCategory() == wdPropertyCategory::Set)
    {
      if (sIndex.IsEmpty())
      {
        step.m_Index = wdVariant();
      }
      else
      {
        return WD_FAILURE;
      }
    }
    else if (pAbsProp->GetCategory() == wdPropertyCategory::Map)
    {
      step.m_Index = sIndex.IsEmpty() ? wdVariant() : wdVariant(sIndex.GetData());
    }

    pCurRtti = pAbsProp->GetSpecificType();
  }

  m_bIsValid = true;
  return WD_SUCCESS;
}

wdResult wdPropertyPath::InitializeFromPath(const wdRTTI& rootObjectRtti, const wdArrayPtr<const wdPropertyPathStep> path)
{
  m_bIsValid = false;

  m_PathSteps.Clear();
  m_PathSteps.Reserve(path.GetCount());

  const wdRTTI* pCurRtti = &rootObjectRtti;
  for (const wdPropertyPathStep& pathStep : path)
  {
    wdAbstractProperty* pAbsProp = pCurRtti->FindPropertyByName(pathStep.m_sProperty);
    if (pAbsProp == nullptr)
      return WD_FAILURE;

    auto& step = m_PathSteps.ExpandAndGetRef();
    step.m_pProperty = pAbsProp;
    step.m_Index = pathStep.m_Index;

    pCurRtti = pAbsProp->GetSpecificType();
  }

  m_bIsValid = true;
  return WD_SUCCESS;
}

wdResult wdPropertyPath::WriteToLeafObject(void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeaf, const wdRTTI& pType)> func) const
{
  WD_ASSERT_DEBUG(
    m_PathSteps.IsEmpty() || m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetTypeFlags().IsSet(wdTypeFlags::Class),
    "To resolve the leaf object the path needs to be empty or end in a class.");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr(), true, func);
}

wdResult wdPropertyPath::ReadFromLeafObject(void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeaf, const wdRTTI& pType)> func) const
{
  WD_ASSERT_DEBUG(
    m_PathSteps.IsEmpty() || m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetTypeFlags().IsSet(wdTypeFlags::Class),
    "To resolve the leaf object the path needs to be empty or end in a class.");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr(), false, func);
}

wdResult wdPropertyPath::WriteProperty(
  void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeafObject, const wdRTTI& pLeafType, wdAbstractProperty* pProp, const wdVariant& index)> func) const
{
  WD_ASSERT_DEBUG(!m_PathSteps.IsEmpty(), "Call InitializeFromPath before WriteToObject");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr().GetSubArray(0, m_PathSteps.GetCount() - 1), true,
    [this, &func](void* pLeafObject, const wdRTTI& leafType) {
      auto& lastStep = m_PathSteps[m_PathSteps.GetCount() - 1];
      func(pLeafObject, leafType, lastStep.m_pProperty, lastStep.m_Index);
    });
}

wdResult wdPropertyPath::ReadProperty(
  void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeafObject, const wdRTTI& pLeafType, const wdAbstractProperty* pProp, const wdVariant& index)> func) const
{
  WD_ASSERT_DEBUG(m_bIsValid, "Call InitializeFromPath before WriteToObject");
  return ResolvePath(pRootObject, &type, m_PathSteps.GetArrayPtr().GetSubArray(0, m_PathSteps.GetCount() - 1), false,
    [this, &func](void* pLeafObject, const wdRTTI& leafType) {
      auto& lastStep = m_PathSteps[m_PathSteps.GetCount() - 1];
      func(pLeafObject, leafType, lastStep.m_pProperty, lastStep.m_Index);
    });
}

void wdPropertyPath::SetValue(void* pRootObject, const wdRTTI& type, const wdVariant& value) const
{
  // WD_ASSERT_DEBUG(!m_PathSteps.IsEmpty() &&
  //                    value.CanConvertTo(m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetVariantType()),
  //                "The given value does not match the type at the given path.");

  WriteProperty(pRootObject, type, [&value](void* pLeaf, const wdRTTI& type, wdAbstractProperty* pProp, const wdVariant& index) {
    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Member:
        wdReflectionUtils::SetMemberPropertyValue(static_cast<wdAbstractMemberProperty*>(pProp), pLeaf, value);
        break;
      case wdPropertyCategory::Array:
        wdReflectionUtils::SetArrayPropertyValue(static_cast<wdAbstractArrayProperty*>(pProp), pLeaf, index.Get<wdInt32>(), value);
        break;
      case wdPropertyCategory::Map:
        wdReflectionUtils::SetMapPropertyValue(static_cast<wdAbstractMapProperty*>(pProp), pLeaf, index.Get<wdString>(), value);
        break;
      default:
        WD_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }).IgnoreResult();
}

void wdPropertyPath::GetValue(void* pRootObject, const wdRTTI& type, wdVariant& out_value) const
{
  // WD_ASSERT_DEBUG(!m_PathSteps.IsEmpty() &&
  //                    m_PathSteps[m_PathSteps.GetCount() - 1].m_pProperty->GetSpecificType()->GetVariantType() != wdVariantType::Invalid,
  //                "The property path of value {} cannot be stored in an wdVariant.", m_PathSteps[m_PathSteps.GetCount() -
  //                1].m_pProperty->GetSpecificType()->GetTypeName());

  ReadProperty(pRootObject, type, [&out_value](void* pLeaf, const wdRTTI& type, const wdAbstractProperty* pProp, const wdVariant& index) {
    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Member:
        out_value = wdReflectionUtils::GetMemberPropertyValue(static_cast<const wdAbstractMemberProperty*>(pProp), pLeaf);
        break;
      case wdPropertyCategory::Array:
        out_value = wdReflectionUtils::GetArrayPropertyValue(static_cast<const wdAbstractArrayProperty*>(pProp), pLeaf, index.Get<wdInt32>());
        break;
      case wdPropertyCategory::Map:
        out_value = wdReflectionUtils::GetMapPropertyValue(static_cast<const wdAbstractMapProperty*>(pProp), pLeaf, index.Get<wdString>());
        break;
      default:
        WD_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }).IgnoreResult();
}

wdResult wdPropertyPath::ResolvePath(void* pCurrentObject, const wdRTTI* pType, const wdArrayPtr<const ResolvedStep> path, bool bWriteToObject,
  const wdDelegate<void(void* pLeaf, const wdRTTI& pType)>& func)
{
  if (path.IsEmpty())
  {
    func(pCurrentObject, *pType);
    return WD_SUCCESS;
  }
  else // Recurse
  {
    wdAbstractProperty* pProp = path[0].m_pProperty;
    const wdRTTI* pPropType = pProp->GetSpecificType();

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Member:
      {
        wdAbstractMemberProperty* pSpecific = static_cast<wdAbstractMemberProperty*>(pProp);
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

            wdResult res = ResolvePath(pRetrievedSubObject, pProp->GetSpecificType(), path.GetSubArray(1), bWriteToObject, func);

            if (bWriteToObject)
              pSpecific->SetValuePtr(pCurrentObject, pRetrievedSubObject);

            pPropType->GetAllocator()->Deallocate(pRetrievedSubObject);
            return res;
          }
          else
          {
            WD_REPORT_FAILURE("Non-allocatable property should not be part of an object chain!");
          }
        }
      }
      break;
      case wdPropertyCategory::Array:
      {
        wdAbstractArrayProperty* pSpecific = static_cast<wdAbstractArrayProperty*>(pProp);

        if (pPropType->GetAllocator()->CanAllocate())
        {
          const wdUInt32 uiIndex = path[0].m_Index.ConvertTo<wdUInt32>();
          if (uiIndex >= pSpecific->GetCount(pCurrentObject))
            return WD_FAILURE;

          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();
          pSpecific->GetValue(pCurrentObject, uiIndex, pSubObject);

          wdResult res = ResolvePath(pSubObject, pProp->GetSpecificType(), path.GetSubArray(1), bWriteToObject, func);

          if (bWriteToObject)
            pSpecific->SetValue(pCurrentObject, uiIndex, pSubObject);

          pPropType->GetAllocator()->Deallocate(pSubObject);
          return res;
        }
        else
        {
          WD_REPORT_FAILURE("Non-allocatable property should not be part of an object chain!");
        }
      }
      break;
      case wdPropertyCategory::Map:
      {
        wdAbstractMapProperty* pSpecific = static_cast<wdAbstractMapProperty*>(pProp);
        const wdString& sKey = path[0].m_Index.Get<wdString>();
        if (!pSpecific->Contains(pCurrentObject, sKey))
          return WD_FAILURE;

        if (pPropType->GetAllocator()->CanAllocate())
        {
          void* pSubObject = pPropType->GetAllocator()->Allocate<void>();

          pSpecific->GetValue(pCurrentObject, sKey, pSubObject);

          wdResult res = ResolvePath(pSubObject, pProp->GetSpecificType(), path.GetSubArray(1), bWriteToObject, func);

          if (bWriteToObject)
            pSpecific->Insert(pCurrentObject, sKey, pSubObject);

          pPropType->GetAllocator()->Deallocate(pSubObject);
          return res;
        }
        else
        {
          WD_REPORT_FAILURE("Non-allocatable property should not be part of an object chain!");
        }
      }
      break;
      case wdPropertyCategory::Set:
      default:
      {
        WD_REPORT_FAILURE("Property of type Set should not be part of an object chain!");
      }
      break;
    }
    return WD_FAILURE;
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_PropertyPath);
