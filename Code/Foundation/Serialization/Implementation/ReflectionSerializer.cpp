#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/BinarySerializer.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Types/VariantTypeRegistry.h>

////////////////////////////////////////////////////////////////////////
// nsReflectionSerializer public static functions
////////////////////////////////////////////////////////////////////////

void nsReflectionSerializer::WriteObjectToDDL(nsStreamWriter& inout_stream, const nsRTTI* pRtti, const void* pObject, bool bCompactMmode /*= true*/, nsOpenDdlWriter::TypeStringMode typeMode /*= nsOpenDdlWriter::TypeStringMode::Shortest*/)
{
  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;
  nsRttiConverterWriter conv(&graph, &context, false, true);

  context.RegisterObject(nsUuid::MakeUuid(), pRtti, const_cast<void*>(pObject));
  conv.AddObjectToGraph(pRtti, const_cast<void*>(pObject), "root");

  nsAbstractGraphDdlSerializer::Write(inout_stream, &graph, nullptr, bCompactMmode, typeMode);
}

void nsReflectionSerializer::WriteObjectToDDL(nsOpenDdlWriter& ref_ddl, const nsRTTI* pRtti, const void* pObject, nsUuid guid /*= nsUuid()*/)
{
  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;
  nsRttiConverterWriter conv(&graph, &context, false, true);

  if (!guid.IsValid())
  {
    guid = nsUuid::MakeUuid();
  }

  context.RegisterObject(guid, pRtti, const_cast<void*>(pObject));
  conv.AddObjectToGraph(pRtti, const_cast<void*>(pObject), "root");

  nsAbstractGraphDdlSerializer::Write(ref_ddl, &graph, nullptr);
}

void nsReflectionSerializer::WriteObjectToBinary(nsStreamWriter& inout_stream, const nsRTTI* pRtti, const void* pObject)
{
  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;
  nsRttiConverterWriter conv(&graph, &context, false, true);

  context.RegisterObject(nsUuid::MakeUuid(), pRtti, const_cast<void*>(pObject));
  conv.AddObjectToGraph(pRtti, const_cast<void*>(pObject), "root");

  nsAbstractGraphBinarySerializer::Write(inout_stream, &graph);
}

void* nsReflectionSerializer::ReadObjectFromDDL(nsStreamReader& inout_stream, const nsRTTI*& ref_pRtti)
{
  nsOpenDdlReader reader;
  if (reader.ParseDocument(inout_stream, 0, nsLog::GetThreadLocalLogSystem()).Failed())
  {
    nsLog::Error("Failed to parse DDL graph");
    return nullptr;
  }

  return ReadObjectFromDDL(reader.GetRootElement(), ref_pRtti);
}

void* nsReflectionSerializer::ReadObjectFromDDL(const nsOpenDdlReaderElement* pRootElement, const nsRTTI*& ref_pRtti)
{
  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;

  nsAbstractGraphDdlSerializer::Read(pRootElement, &graph).IgnoreResult();

  nsRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  NS_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  ref_pRtti = nsRTTI::FindTypeByName(pRootNode->GetType());

  void* pTarget = context.CreateObject(pRootNode->GetGuid(), ref_pRtti);

  convRead.ApplyPropertiesToObject(pRootNode, ref_pRtti, pTarget);

  return pTarget;
}

void* nsReflectionSerializer::ReadObjectFromBinary(nsStreamReader& inout_stream, const nsRTTI*& ref_pRtti)
{
  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;

  nsAbstractGraphBinarySerializer::Read(inout_stream, &graph);

  nsRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  NS_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  ref_pRtti = nsRTTI::FindTypeByName(pRootNode->GetType());

  void* pTarget = context.CreateObject(pRootNode->GetGuid(), ref_pRtti);

  convRead.ApplyPropertiesToObject(pRootNode, ref_pRtti, pTarget);

  return pTarget;
}

void nsReflectionSerializer::ReadObjectPropertiesFromDDL(nsStreamReader& inout_stream, const nsRTTI& rtti, void* pObject)
{
  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;

  nsAbstractGraphDdlSerializer::Read(inout_stream, &graph).IgnoreResult();

  nsRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  NS_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  if (pRootNode == nullptr)
    return;

  convRead.ApplyPropertiesToObject(pRootNode, &rtti, pObject);
}

void nsReflectionSerializer::ReadObjectPropertiesFromBinary(nsStreamReader& inout_stream, const nsRTTI& rtti, void* pObject)
{
  nsAbstractObjectGraph graph;
  nsRttiConverterContext context;

  nsAbstractGraphBinarySerializer::Read(inout_stream, &graph);

  nsRttiConverterReader convRead(&graph, &context);
  auto* pRootNode = graph.GetNodeByName("root");

  NS_ASSERT_DEV(pRootNode != nullptr, "invalid document");

  convRead.ApplyPropertiesToObject(pRootNode, &rtti, pObject);
}


namespace
{
  static void CloneProperty(const void* pObject, void* pClone, const nsAbstractProperty* pProp)
  {
    if (pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
      return;

    const nsRTTI* pPropType = pProp->GetSpecificType();

    const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

    nsVariant vTemp;
    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Member:
      {
        auto pSpecific = static_cast<const nsAbstractMemberProperty*>(pProp);

        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          vTemp = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);

          void* pRefrencedObject = vTemp.ConvertTo<void*>();
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner) && pRefrencedObject)
          {
            pRefrencedObject = nsReflectionSerializer::Clone(pRefrencedObject, pPropType);
            vTemp = nsVariant(pRefrencedObject, pPropType);
          }

          nsVariant vOldValue = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pClone);
          nsReflectionUtils::SetMemberPropertyValue(pSpecific, pClone, vTemp);
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
            nsReflectionUtils::DeleteObject(vOldValue.ConvertTo<void*>(), pProp);
        }
        else
        {
          if (bIsValueType || pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
          {
            vTemp = nsReflectionUtils::GetMemberPropertyValue(pSpecific, pObject);
            nsReflectionUtils::SetMemberPropertyValue(pSpecific, pClone, vTemp);
          }
          else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
          {
            void* pSubObject = pSpecific->GetPropertyPointer(pObject);
            // Do we have direct access to the property?
            if (pSubObject != nullptr)
            {
              void* pSubClone = pSpecific->GetPropertyPointer(pClone);
              nsReflectionSerializer::Clone(pSubObject, pSubClone, pPropType);
            }
            // If the property is behind an accessor, we need to retrieve it first.
            else if (pPropType->GetAllocator()->CanAllocate())
            {
              pSubObject = pPropType->GetAllocator()->Allocate<void>();
              pSpecific->GetValuePtr(pObject, pSubObject);
              pSpecific->SetValuePtr(pClone, pSubObject);
              pPropType->GetAllocator()->Deallocate(pSubObject);
            }
          }
        }
      }
      break;
      case nsPropertyCategory::Array:
      {
        auto pSpecific = static_cast<const nsAbstractArrayProperty*>(pProp);
        // Delete old values
        if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
        {
          const nsInt32 iCloneCount = (nsInt32)pSpecific->GetCount(pClone);
          for (nsInt32 i = iCloneCount - 1; i >= 0; --i)
          {
            void* pOldSubClone = nullptr;
            pSpecific->GetValue(pClone, i, &pOldSubClone);
            pSpecific->Remove(pClone, i);
            if (pOldSubClone)
              nsReflectionUtils::DeleteObject(pOldSubClone, pProp);
          }
        }

        const nsUInt32 uiCount = pSpecific->GetCount(pObject);
        pSpecific->SetCount(pClone, uiCount);
        if (pSpecific->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          for (nsUInt32 i = 0; i < uiCount; ++i)
          {
            vTemp = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
            void* pRefrencedObject = vTemp.ConvertTo<void*>();
            if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner) && pRefrencedObject)
            {
              pRefrencedObject = nsReflectionSerializer::Clone(pRefrencedObject, pPropType);
              vTemp = nsVariant(pRefrencedObject, pPropType);
            }
            nsReflectionUtils::SetArrayPropertyValue(pSpecific, pClone, i, vTemp);
          }
        }
        else
        {
          if (bIsValueType)
          {
            for (nsUInt32 i = 0; i < uiCount; ++i)
            {
              vTemp = nsReflectionUtils::GetArrayPropertyValue(pSpecific, pObject, i);
              nsReflectionUtils::SetArrayPropertyValue(pSpecific, pClone, i, vTemp);
            }
          }
          else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class) && pPropType->GetAllocator()->CanAllocate())
          {
            void* pSubObject = pPropType->GetAllocator()->Allocate<void>();

            for (nsUInt32 i = 0; i < uiCount; ++i)
            {
              pSpecific->GetValue(pObject, i, pSubObject);
              pSpecific->SetValue(pClone, i, pSubObject);
            }

            pPropType->GetAllocator()->Deallocate(pSubObject);
          }
        }
      }
      break;
      case nsPropertyCategory::Set:
      {
        auto pSpecific = static_cast<const nsAbstractSetProperty*>(pProp);

        // Delete old values
        if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
        {
          nsHybridArray<nsVariant, 16> keys;
          pSpecific->GetValues(pClone, keys);
          pSpecific->Clear(pClone);
          for (nsVariant& value : keys)
          {
            void* pOldClone = value.ConvertTo<void*>();
            if (pOldClone)
              nsReflectionUtils::DeleteObject(pOldClone, pProp);
          }
        }
        pSpecific->Clear(pClone);

        nsHybridArray<nsVariant, 16> values;
        pSpecific->GetValues(pObject, values);


        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          for (nsUInt32 i = 0; i < values.GetCount(); ++i)
          {
            void* pRefrencedObject = values[i].ConvertTo<void*>();
            if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner) && pRefrencedObject)
            {
              pRefrencedObject = nsReflectionSerializer::Clone(pRefrencedObject, pPropType);
            }
            vTemp = nsVariant(pRefrencedObject, pPropType);
            nsReflectionUtils::InsertSetPropertyValue(pSpecific, pClone, vTemp);
          }
        }
        else if (bIsValueType)
        {
          for (nsUInt32 i = 0; i < values.GetCount(); ++i)
          {
            nsReflectionUtils::InsertSetPropertyValue(pSpecific, pClone, values[i]);
          }
        }
      }
      break;
      case nsPropertyCategory::Map:
      {
        auto pSpecific = static_cast<const nsAbstractMapProperty*>(pProp);

        // Delete old values
        if (pProp->GetFlags().AreAllSet(nsPropertyFlags::Pointer | nsPropertyFlags::PointerOwner))
        {
          nsHybridArray<nsString, 16> keys;
          pSpecific->GetKeys(pClone, keys);
          for (const nsString& sKey : keys)
          {
            nsVariant value = nsReflectionUtils::GetMapPropertyValue(pSpecific, pClone, sKey);
            void* pOldClone = value.ConvertTo<void*>();
            pSpecific->Remove(pClone, sKey);
            if (pOldClone)
              nsReflectionUtils::DeleteObject(pOldClone, pProp);
          }
        }
        pSpecific->Clear(pClone);

        nsHybridArray<nsString, 16> keys;
        pSpecific->GetKeys(pObject, keys);

        for (nsUInt32 i = 0; i < keys.GetCount(); ++i)
        {
          if (bIsValueType ||
              (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)))
          {
            nsVariant value = nsReflectionUtils::GetMapPropertyValue(pSpecific, pObject, keys[i]);
            nsReflectionUtils::SetMapPropertyValue(pSpecific, pClone, keys[i], value);
          }
          else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
          {
            if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
            {
              void* pValue = nullptr;
              pSpecific->GetValue(pObject, keys[i], &pValue);
              pValue = nsReflectionSerializer::Clone(pValue, pPropType);
              pSpecific->Insert(pClone, keys[i], &pValue);
            }
            else
            {
              if (pPropType->GetAllocator()->CanAllocate())
              {
                void* pValue = pPropType->GetAllocator()->Allocate<void>();
                NS_SCOPE_EXIT(pPropType->GetAllocator()->Deallocate(pValue););
                NS_VERIFY(pSpecific->GetValue(pObject, keys[i], pValue), "Previously retrieved key does not exist.");
                pSpecific->Insert(pClone, keys[i], pValue);
              }
              else
              {
                nsLog::Error("The property '{0}' can not be cloned as the type '{1}' cannot be allocated.", pProp->GetPropertyName(), pPropType->GetTypeName());
              }
            }
          }
        }
      }
      break;
      default:
        break;
    }
  }

  static void CloneProperties(const void* pObject, void* pClone, const nsRTTI* pType)
  {
    if (pType->GetParentType())
      CloneProperties(pObject, pClone, pType->GetParentType());

    for (auto* pProp : pType->GetProperties())
    {
      CloneProperty(pObject, pClone, pProp);
    }
  }
} // namespace

void* nsReflectionSerializer::Clone(const void* pObject, const nsRTTI* pType)
{
  if (!pObject)
    return nullptr;

  NS_ASSERT_DEV(pType != nullptr, "invalid type.");
  if (pType->IsDerivedFrom<nsReflectedClass>())
  {
    const nsReflectedClass* pRefObject = static_cast<const nsReflectedClass*>(pObject);
    pType = pRefObject->GetDynamicRTTI();
  }

  NS_ASSERT_DEV(pType->GetAllocator()->CanAllocate(), "The type '{0}' can't be cloned!", pType->GetTypeName());
  void* pClone = pType->GetAllocator()->Allocate<void>();
  CloneProperties(pObject, pClone, pType);
  return pClone;
}


void nsReflectionSerializer::Clone(const void* pObject, void* pClone, const nsRTTI* pType)
{
  NS_ASSERT_DEV(pObject && pClone && pType, "invalid type.");
  if (pType->IsDerivedFrom<nsReflectedClass>())
  {
    const nsReflectedClass* pRefObject = static_cast<const nsReflectedClass*>(pObject);
    pType = pRefObject->GetDynamicRTTI();
    NS_ASSERT_DEV(pType == static_cast<nsReflectedClass*>(pClone)->GetDynamicRTTI(), "Object '{0}' and clone '{1}' have mismatching types!", pType->GetTypeName(), static_cast<nsReflectedClass*>(pClone)->GetDynamicRTTI()->GetTypeName());
  }

  CloneProperties(pObject, pClone, pType);
}
