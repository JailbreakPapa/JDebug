#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

nsAbstractObjectNode* nsDocumentObjectConverterWriter::AddObjectToGraph(const nsDocumentObject* pObject, nsStringView sNodeName)
{
  nsAbstractObjectNode* pNode = AddSubObjectToGraph(pObject, sNodeName);

  while (!m_QueuedObjects.IsEmpty())
  {
    auto itCur = m_QueuedObjects.GetIterator();

    AddSubObjectToGraph(itCur.Key(), nullptr);

    m_QueuedObjects.Remove(itCur);
  }

  return pNode;
}

void nsDocumentObjectConverterWriter::AddProperty(nsAbstractObjectNode* pNode, const nsAbstractProperty* pProp, const nsDocumentObject* pObject)
{
  if (m_Filter.IsValid() && !m_Filter(pObject, pProp))
    return;

  const nsRTTI* pPropType = pProp->GetSpecificType();
  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
        {
          const nsUuid guid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).Get<nsUuid>();

          pNode->AddProperty(pProp->GetPropertyName(), guid);
          if (guid.IsValid())
            m_QueuedObjects.Insert(m_pManager->GetObject(guid));
        }
        else
        {
          pNode->AddProperty(pProp->GetPropertyName(), pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()));
        }
      }
      else
      {
        if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
        {
          nsStringBuilder sTemp;
          nsReflectionUtils::EnumerationToString(
            pPropType, pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).ConvertTo<nsInt64>(), sTemp);
          pNode->AddProperty(pProp->GetPropertyName(), sTemp.GetData());
        }
        else if (bIsValueType)
        {
          pNode->AddProperty(pProp->GetPropertyName(), pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()));
        }
        else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
        {
          const nsUuid guid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).Get<nsUuid>();
          NS_ASSERT_DEV(guid.IsValid(), "Embedded class cannot be null.");
          pNode->AddProperty(pProp->GetPropertyName(), guid);
          m_QueuedObjects.Insert(m_pManager->GetObject(guid));
        }
      }
    }

    break;

    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
    {
      const nsInt32 iCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      NS_ASSERT_DEV(iCount >= 0, "Invalid array property size {0}", iCount);

      nsVariantArray values;
      values.SetCount(iCount);

      for (nsInt32 i = 0; i < iCount; ++i)
      {
        values[i] = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), i);
        if (!bIsValueType)
        {
          m_QueuedObjects.Insert(m_pManager->GetObject(values[i].Get<nsUuid>()));
        }
      }
      pNode->AddProperty(pProp->GetPropertyName(), values);
    }
    break;
    case nsPropertyCategory::Map:
    {
      const nsInt32 iCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      NS_ASSERT_DEV(iCount >= 0, "Invalid map property size {0}", iCount);

      nsVariantDictionary values;
      values.Reserve(iCount);
      nsHybridArray<nsVariant, 16> keys;
      pObject->GetTypeAccessor().GetKeys(pProp->GetPropertyName(), keys);

      for (const nsVariant& key : keys)
      {
        nsVariant value = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), key);
        values.Insert(key.Get<nsString>(), value);
        if (!bIsValueType)
        {
          m_QueuedObjects.Insert(m_pManager->GetObject(value.Get<nsUuid>()));
        }
      }
      pNode->AddProperty(pProp->GetPropertyName(), values);
    }
    break;
    case nsPropertyCategory::Constant:
      // Nothing to do here.
      break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED
  }
}

void nsDocumentObjectConverterWriter::AddProperties(nsAbstractObjectNode* pNode, const nsDocumentObject* pObject)
{
  nsHybridArray<const nsAbstractProperty*, 32> properties;
  pObject->GetTypeAccessor().GetType()->GetAllProperties(properties);

  for (const auto* pProp : properties)
  {
    AddProperty(pNode, pProp, pObject);
  }
}

nsAbstractObjectNode* nsDocumentObjectConverterWriter::AddSubObjectToGraph(const nsDocumentObject* pObject, nsStringView sNodeName)
{
  nsAbstractObjectNode* pNode = m_pGraph->AddNode(pObject->GetGuid(), pObject->GetType()->GetTypeName(), pObject->GetType()->GetTypeVersion(), sNodeName);
  AddProperties(pNode, pObject);
  return pNode;
}

nsDocumentObjectConverterReader::nsDocumentObjectConverterReader(const nsAbstractObjectGraph* pGraph, nsDocumentObjectManager* pManager, Mode mode)
{
  m_pManager = pManager;
  m_pGraph = pGraph;
  m_Mode = mode;
  m_uiUnknownTypeInstances = 0;
}

nsDocumentObject* nsDocumentObjectConverterReader::CreateObjectFromNode(const nsAbstractObjectNode* pNode)
{
  nsDocumentObject* pObject = nullptr;
  const nsRTTI* pType = nsRTTI::FindTypeByName(pNode->GetType());
  if (pType)
  {
    pObject = m_pManager->CreateObject(pType, pNode->GetGuid());
  }
  else
  {
    if (!m_UnknownTypes.Contains(pNode->GetType()))
    {
      nsLog::Error("Cannot create node of unknown type '{0}'.", pNode->GetType());
      m_UnknownTypes.Insert(pNode->GetType());
    }
    m_uiUnknownTypeInstances++;
  }
  return pObject;
}

void nsDocumentObjectConverterReader::AddObject(nsDocumentObject* pObject, nsDocumentObject* pParent, nsStringView sParentProperty, nsVariant index)
{
  NS_ASSERT_DEV(pObject && pParent, "Need to have valid objects to add them to the document");
  if (m_Mode == nsDocumentObjectConverterReader::Mode::CreateAndAddToDocument && pParent->GetDocumentObjectManager()->GetObject(pParent->GetGuid()))
  {
    m_pManager->AddObject(pObject, pParent, sParentProperty, index);
  }
  else
  {
    pParent->InsertSubObject(pObject, sParentProperty, index);
  }
}

void nsDocumentObjectConverterReader::ApplyPropertiesToObject(const nsAbstractObjectNode* pNode, nsDocumentObject* pObject)
{
  // NS_ASSERT_DEV(pObject->GetChildren().GetCount() == 0, "Can only apply properties to empty objects!");
  nsHybridArray<const nsAbstractProperty*, 32> properties;
  pObject->GetTypeAccessor().GetType()->GetAllProperties(properties);

  for (auto* pProp : properties)
  {
    auto* pOtherProp = pNode->FindProperty(pProp->GetPropertyName());
    if (pOtherProp == nullptr)
      continue;

    ApplyProperty(pObject, pProp, pOtherProp);
  }
}

void nsDocumentObjectConverterReader::ApplyDiffToObject(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pObject, nsDeque<nsAbstractGraphDiffOperation>& ref_diff)
{
  nsHybridArray<nsAbstractGraphDiffOperation*, 4> change;

  for (auto& op : ref_diff)
  {
    if (op.m_Operation == nsAbstractGraphDiffOperation::Op::PropertyChanged && pObject->GetGuid() == op.m_Node)
      change.PushBack(&op);
  }

  for (auto* op : change)
  {
    const nsAbstractProperty* pProp = pObject->GetTypeAccessor().GetType()->FindPropertyByName(op->m_sProperty);
    if (!pProp)
      continue;

    ApplyDiff(pObjectAccessor, pObject, pProp, *op, ref_diff);
  }

  // Recurse into owned sub objects (old or new)
  for (const nsDocumentObject* pSubObject : pObject->GetChildren())
  {
    ApplyDiffToObject(pObjectAccessor, pSubObject, ref_diff);
  }
}

void nsDocumentObjectConverterReader::ApplyDiff(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsAbstractGraphDiffOperation& op, nsDeque<nsAbstractGraphDiffOperation>& diff)
{
  nsStringBuilder sTemp;

  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

  auto NeedsToBeDeleted = [&diff](const nsUuid& guid) -> bool
  {
    for (auto& op : diff)
    {
      if (op.m_Operation == nsAbstractGraphDiffOperation::Op::NodeRemoved && guid == op.m_Node)
        return true;
    }
    return false;
  };
  auto NeedsToBeCreated = [&diff](const nsUuid& guid) -> nsAbstractGraphDiffOperation*
  {
    for (auto& op : diff)
    {
      if (op.m_Operation == nsAbstractGraphDiffOperation::Op::NodeAdded && guid == op.m_Node)
        return &op;
    }
    return nullptr;
  };

  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags) || bIsValueType)
      {
        pObjectAccessor->SetValue(pObject, pProp, op.m_Value).IgnoreResult();
      }
      else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
      {
        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
        {
          if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          {
            const nsUuid oldGuid = pObjectAccessor->Get<nsUuid>(pObject, pProp);
            const nsUuid newGuid = op.m_Value.Get<nsUuid>();
            if (oldGuid.IsValid())
            {
              if (NeedsToBeDeleted(oldGuid))
              {
                pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(oldGuid)).IgnoreResult();
              }
            }

            if (newGuid.IsValid())
            {
              if (nsAbstractGraphDiffOperation* pCreate = NeedsToBeCreated(newGuid))
              {
                pObjectAccessor->AddObject(pObject, pProp, nsVariant(), nsRTTI::FindTypeByName(pCreate->m_sProperty), pCreate->m_Node).IgnoreResult();
              }

              const nsDocumentObject* pChild = pObject->GetChild(newGuid);
              NS_ASSERT_DEV(pChild != nullptr, "References child object does not exist!");
            }
          }
          else
          {
            pObjectAccessor->SetValue(pObject, pProp, op.m_Value).IgnoreResult();
          }
        }
        else
        {
          // Noting to do here, value cannot change
        }
      }
      break;
    }
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
    {
      const nsVariantArray& values = op.m_Value.Get<nsVariantArray>();
      nsInt32 iCurrentCount = pObjectAccessor->GetCount(pObject, pProp);
      if (bIsValueType || (pProp->GetFlags().IsAnySet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)))
      {
        for (nsUInt32 i = 0; i < values.GetCount(); ++i)
        {
          if (i < (nsUInt32)iCurrentCount)
            pObjectAccessor->SetValue(pObject, pProp, values[i], i).IgnoreResult();
          else
            pObjectAccessor->InsertValue(pObject, pProp, values[i], i).IgnoreResult();
        }
        for (nsInt32 i = iCurrentCount - 1; i >= (nsInt32)values.GetCount(); --i)
        {
          pObjectAccessor->RemoveValue(pObject, pProp, i).IgnoreResult();
        }
      }
      else // Class
      {
        const nsInt32 iCurrentCount2 = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());

        nsHybridArray<nsVariant, 16> currentValues;
        pObject->GetTypeAccessor().GetValues(pProp->GetPropertyName(), currentValues);
        for (nsInt32 i = iCurrentCount2 - 1; i >= 0; --i)
        {
          if (NeedsToBeDeleted(currentValues[i].Get<nsUuid>()))
          {
            pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(currentValues[i].Get<nsUuid>())).IgnoreResult();
          }
        }

        for (nsUInt32 i = 0; i < values.GetCount(); ++i)
        {
          if (nsAbstractGraphDiffOperation* pCreate = NeedsToBeCreated(values[i].Get<nsUuid>()))
          {
            pObjectAccessor->AddObject(pObject, pProp, i, nsRTTI::FindTypeByName(pCreate->m_sProperty), pCreate->m_Node).IgnoreResult();
          }
          else
          {
            pObjectAccessor->MoveObject(pObjectAccessor->GetObject(values[i].Get<nsUuid>()), pObject, pProp, i).IgnoreResult();
          }
        }
      }
      break;
    }
    case nsPropertyCategory::Map:
    {
      const nsVariantDictionary& values = op.m_Value.Get<nsVariantDictionary>();
      nsHybridArray<nsVariant, 16> keys;
      NS_VERIFY(pObjectAccessor->GetKeys(pObject, pProp, keys).Succeeded(), "Property is not a map, getting keys failed.");

      if (bIsValueType || (pProp->GetFlags().IsAnySet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)))
      {
        for (const nsVariant& key : keys)
        {
          const nsString& sKey = key.Get<nsString>();
          if (!values.Contains(sKey))
          {
            NS_VERIFY(pObjectAccessor->RemoveValue(pObject, pProp, key).Succeeded(), "RemoveValue failed.");
          }
        }
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          nsVariant variantKey(it.Key());
          if (keys.Contains(variantKey))
            pObjectAccessor->SetValue(pObject, pProp, it.Value(), variantKey).IgnoreResult();
          else
            pObjectAccessor->InsertValue(pObject, pProp, it.Value(), variantKey).IgnoreResult();
        }
      }
      else // Class
      {
        for (const nsVariant& key : keys)
        {
          nsVariant value;
          NS_VERIFY(pObjectAccessor->GetValue(pObject, pProp, value, key).Succeeded(), "");
          if (NeedsToBeDeleted(value.Get<nsUuid>()))
          {
            pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(value.Get<nsUuid>())).IgnoreResult();
          }
        }
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          const nsVariant& value = it.Value();
          nsVariant variantKey(it.Key());
          if (nsAbstractGraphDiffOperation* pCreate = NeedsToBeCreated(value.Get<nsUuid>()))
          {
            pObjectAccessor->AddObject(pObject, pProp, variantKey, nsRTTI::FindTypeByName(pCreate->m_sProperty), pCreate->m_Node).IgnoreResult();
          }
          else
          {
            pObjectAccessor->MoveObject(pObjectAccessor->GetObject(value.Get<nsUuid>()), pObject, pProp, variantKey).IgnoreResult();
          }
        }
      }
      break;
    }

    case nsPropertyCategory::Function:
    case nsPropertyCategory::Constant:
      break; // nothing to do
  }
}

void nsDocumentObjectConverterReader::ApplyProperty(nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsAbstractObjectNode::Property* pSource)
{
  nsStringBuilder sTemp;

  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
        {
          if (pSource->m_Value.IsA<nsUuid>())
          {
            const nsUuid guid = pSource->m_Value.Get<nsUuid>();
            if (guid.IsValid())
            {
              auto* pSubNode = m_pGraph->GetNode(guid);
              NS_ASSERT_DEV(pSubNode != nullptr, "invalid document");

              if (auto* pSubObject = CreateObjectFromNode(pSubNode))
              {
                ApplyPropertiesToObject(pSubNode, pSubObject);
                AddObject(pSubObject, pObject, pProp->GetPropertyName(), nsVariant());
              }
            }
          }
        }
        else
        {
          pObject->GetTypeAccessor().SetValue(pProp->GetPropertyName(), pSource->m_Value);
        }
      }
      else
      {
        if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags) || bIsValueType)
        {
          pObject->GetTypeAccessor().SetValue(pProp->GetPropertyName(), pSource->m_Value);
        }
        else if (pSource->m_Value.IsA<nsUuid>()) // nsPropertyFlags::Class
        {
          const nsUuid& nodeGuid = pSource->m_Value.Get<nsUuid>();
          if (nodeGuid.IsValid())
          {
            const nsUuid subObjectGuid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).Get<nsUuid>();
            nsDocumentObject* pEmbeddedClassObject = pObject->GetChild(subObjectGuid);
            NS_ASSERT_DEV(pEmbeddedClassObject != nullptr, "CreateObject should have created all embedded classes!");
            auto* pSubNode = m_pGraph->GetNode(nodeGuid);
            NS_ASSERT_DEV(pSubNode != nullptr, "invalid document");

            ApplyPropertiesToObject(pSubNode, pEmbeddedClassObject);
          }
        }
      }
      break;
    }
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
    {
      const nsVariantArray& array = pSource->m_Value.Get<nsVariantArray>();
      const nsInt32 iCurrentCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      if (bIsValueType || (pProp->GetFlags().IsAnySet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)))
      {
        for (nsUInt32 i = 0; i < array.GetCount(); ++i)
        {
          if (i < (nsUInt32)iCurrentCount)
          {
            pObject->GetTypeAccessor().SetValue(pProp->GetPropertyName(), array[i], i);
          }
          else
          {
            pObject->GetTypeAccessor().InsertValue(pProp->GetPropertyName(), i, array[i]);
          }
        }
        for (nsInt32 i = iCurrentCount - 1; i >= (nsInt32)array.GetCount(); i--)
        {
          pObject->GetTypeAccessor().RemoveValue(pProp->GetPropertyName(), i);
        }
      }
      else
      {
        for (nsUInt32 i = 0; i < array.GetCount(); ++i)
        {
          const nsUuid guid = array[i].Get<nsUuid>();
          if (guid.IsValid())
          {
            auto* pSubNode = m_pGraph->GetNode(guid);
            NS_ASSERT_DEV(pSubNode != nullptr, "invalid document");

            if (i < (nsUInt32)iCurrentCount)
            {
              // Overwrite existing object
              nsUuid childGuid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), i).ConvertTo<nsUuid>();
              if (nsDocumentObject* pSubObject = m_pManager->GetObject(childGuid))
              {
                ApplyPropertiesToObject(pSubNode, pSubObject);
              }
            }
            else
            {
              if (nsDocumentObject* pSubObject = CreateObjectFromNode(pSubNode))
              {
                ApplyPropertiesToObject(pSubNode, pSubObject);
                AddObject(pSubObject, pObject, pProp->GetPropertyName(), -1);
              }
            }
          }
        }
        for (nsInt32 i = iCurrentCount - 1; i >= (nsInt32)array.GetCount(); i--)
        {
          NS_REPORT_FAILURE("Not implemented");
        }
      }
      break;
    }
    case nsPropertyCategory::Map:
    {
      const nsVariantDictionary& values = pSource->m_Value.Get<nsVariantDictionary>();
      nsHybridArray<nsVariant, 16> keys;
      pObject->GetTypeAccessor().GetKeys(pProp->GetPropertyName(), keys);

      if (bIsValueType || (pProp->GetFlags().IsAnySet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner)))
      {
        for (const nsVariant& key : keys)
        {
          pObject->GetTypeAccessor().RemoveValue(pProp->GetPropertyName(), key);
        }
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          pObject->GetTypeAccessor().InsertValue(pProp->GetPropertyName(), nsVariant(it.Key()), it.Value());
        }
      }
      else
      {
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          const nsVariant& value = it.Value();
          const nsUuid guid = value.Get<nsUuid>();

          const nsVariant variantKey(it.Key());

          if (guid.IsValid())
          {
            auto* pSubNode = m_pGraph->GetNode(guid);
            NS_ASSERT_DEV(pSubNode != nullptr, "invalid document");
            if (nsDocumentObject* pSubObject = CreateObjectFromNode(pSubNode))
            {
              ApplyPropertiesToObject(pSubNode, pSubObject);
              AddObject(pSubObject, pObject, pProp->GetPropertyName(), variantKey);
            }
          }
        }
      }
      break;
    }

    case nsPropertyCategory::Function:
    case nsPropertyCategory::Constant:
      break; // nothing to do
  }
}
