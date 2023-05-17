#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

wdAbstractObjectNode* wdDocumentObjectConverterWriter::AddObjectToGraph(const wdDocumentObject* pObject, const char* szNodeName)
{
  wdAbstractObjectNode* pNode = AddSubObjectToGraph(pObject, szNodeName);

  while (!m_QueuedObjects.IsEmpty())
  {
    auto itCur = m_QueuedObjects.GetIterator();

    AddSubObjectToGraph(itCur.Key(), nullptr);

    m_QueuedObjects.Remove(itCur);
  }

  return pNode;
}

void wdDocumentObjectConverterWriter::AddProperty(wdAbstractObjectNode* pNode, const wdAbstractProperty* pProp, const wdDocumentObject* pObject)
{
  if (m_Filter.IsValid() && !m_Filter(pObject, pProp))
    return;

  const wdRTTI* pPropType = pProp->GetSpecificType();
  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
        {
          const wdUuid guid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).Get<wdUuid>();

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
        if (pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
        {
          wdStringBuilder sTemp;
          wdReflectionUtils::EnumerationToString(
            pPropType, pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).ConvertTo<wdInt64>(), sTemp);
          pNode->AddProperty(pProp->GetPropertyName(), sTemp.GetData());
        }
        else if (bIsValueType)
        {
          pNode->AddProperty(pProp->GetPropertyName(), pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()));
        }
        else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
        {
          const wdUuid guid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).Get<wdUuid>();
          WD_ASSERT_DEV(guid.IsValid(), "Embedded class cannot be null.");
          pNode->AddProperty(pProp->GetPropertyName(), guid);
          m_QueuedObjects.Insert(m_pManager->GetObject(guid));
        }
      }
    }

    break;

    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
    {
      const wdInt32 iCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      WD_ASSERT_DEV(iCount >= 0, "Invalid array property size {0}", iCount);

      wdVariantArray values;
      values.SetCount(iCount);

      for (wdInt32 i = 0; i < iCount; ++i)
      {
        values[i] = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), i);
        if (!bIsValueType)
        {
          m_QueuedObjects.Insert(m_pManager->GetObject(values[i].Get<wdUuid>()));
        }
      }
      pNode->AddProperty(pProp->GetPropertyName(), values);
    }
    break;
    case wdPropertyCategory::Map:
    {
      const wdInt32 iCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      WD_ASSERT_DEV(iCount >= 0, "Invalid map property size {0}", iCount);

      wdVariantDictionary values;
      values.Reserve(iCount);
      wdHybridArray<wdVariant, 16> keys;
      pObject->GetTypeAccessor().GetKeys(pProp->GetPropertyName(), keys);

      for (const wdVariant& key : keys)
      {
        wdVariant value = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), key);
        values.Insert(key.Get<wdString>(), value);
        if (!bIsValueType)
        {
          m_QueuedObjects.Insert(m_pManager->GetObject(value.Get<wdUuid>()));
        }
      }
      pNode->AddProperty(pProp->GetPropertyName(), values);
    }
    break;
    case wdPropertyCategory::Constant:
      // Nothing to do here.
      break;
    default:
      WD_ASSERT_NOT_IMPLEMENTED
  }
}

void wdDocumentObjectConverterWriter::AddProperties(wdAbstractObjectNode* pNode, const wdDocumentObject* pObject)
{
  wdHybridArray<wdAbstractProperty*, 32> Properties;
  pObject->GetTypeAccessor().GetType()->GetAllProperties(Properties);

  for (const auto* pProp : Properties)
  {
    AddProperty(pNode, pProp, pObject);
  }
}

wdAbstractObjectNode* wdDocumentObjectConverterWriter::AddSubObjectToGraph(const wdDocumentObject* pObject, const char* szNodeName)
{
  wdAbstractObjectNode* pNode = m_pGraph->AddNode(pObject->GetGuid(), pObject->GetType()->GetTypeName(), pObject->GetType()->GetTypeVersion(), szNodeName);
  AddProperties(pNode, pObject);
  return pNode;
}

wdDocumentObjectConverterReader::wdDocumentObjectConverterReader(const wdAbstractObjectGraph* pGraph, wdDocumentObjectManager* pManager, Mode mode)
{
  m_pManager = pManager;
  m_pGraph = pGraph;
  m_Mode = mode;
  m_uiUnknownTypeInstances = 0;
}

wdDocumentObject* wdDocumentObjectConverterReader::CreateObjectFromNode(const wdAbstractObjectNode* pNode)
{
  wdDocumentObject* pObject = nullptr;
  wdRTTI* pType = wdRTTI::FindTypeByName(pNode->GetType());
  if (pType)
  {
    pObject = m_pManager->CreateObject(pType, pNode->GetGuid());
  }
  else
  {
    if (!m_UnknownTypes.Contains(pNode->GetType()))
    {
      wdLog::Error("Cannot create node of unknown type '{0}'.", pNode->GetType());
      m_UnknownTypes.Insert(pNode->GetType());
    }
    m_uiUnknownTypeInstances++;
  }
  return pObject;
}

void wdDocumentObjectConverterReader::AddObject(wdDocumentObject* pObject, wdDocumentObject* pParent, const char* szParentProperty, wdVariant index)
{
  WD_ASSERT_DEV(pObject && pParent, "Need to have valid objects to add them to the document");
  if (m_Mode == wdDocumentObjectConverterReader::Mode::CreateAndAddToDocument && pParent->GetDocumentObjectManager()->GetObject(pParent->GetGuid()))
  {
    m_pManager->AddObject(pObject, pParent, szParentProperty, index);
  }
  else
  {
    pParent->InsertSubObject(pObject, szParentProperty, index);
  }
}

void wdDocumentObjectConverterReader::ApplyPropertiesToObject(const wdAbstractObjectNode* pNode, wdDocumentObject* pObject)
{
  // WD_ASSERT_DEV(pObject->GetChildren().GetCount() == 0, "Can only apply properties to empty objects!");
  wdHybridArray<wdAbstractProperty*, 32> Properties;
  pObject->GetTypeAccessor().GetType()->GetAllProperties(Properties);

  for (auto* pProp : Properties)
  {
    auto* pOtherProp = pNode->FindProperty(pProp->GetPropertyName());
    if (pOtherProp == nullptr)
      continue;

    ApplyProperty(pObject, pProp, pOtherProp);
  }
}

void wdDocumentObjectConverterReader::ApplyDiffToObject(
  wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pObject, wdDeque<wdAbstractGraphDiffOperation>& ref_diff)
{
  wdHybridArray<wdAbstractGraphDiffOperation*, 4> change;

  for (auto& op : ref_diff)
  {
    if (op.m_Operation == wdAbstractGraphDiffOperation::Op::PropertyChanged && pObject->GetGuid() == op.m_Node)
      change.PushBack(&op);
  }

  for (auto* op : change)
  {
    wdAbstractProperty* pProp = pObject->GetTypeAccessor().GetType()->FindPropertyByName(op->m_sProperty);
    if (!pProp)
      continue;

    ApplyDiff(pObjectAccessor, pObject, pProp, *op, ref_diff);
  }

  // Recurse into owned sub objects (old or new)
  for (const wdDocumentObject* pSubObject : pObject->GetChildren())
  {
    ApplyDiffToObject(pObjectAccessor, pSubObject, ref_diff);
  }
}

void wdDocumentObjectConverterReader::ApplyDiff(wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pObject, wdAbstractProperty* pProp,
  wdAbstractGraphDiffOperation& op, wdDeque<wdAbstractGraphDiffOperation>& diff)
{
  wdStringBuilder sTemp;

  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

  auto NeedsToBeDeleted = [&diff](const wdUuid& guid) -> bool
  {
    for (auto& op : diff)
    {
      if (op.m_Operation == wdAbstractGraphDiffOperation::Op::NodeRemoved && guid == op.m_Node)
        return true;
    }
    return false;
  };
  auto NeedsToBeCreated = [&diff](const wdUuid& guid) -> wdAbstractGraphDiffOperation*
  {
    for (auto& op : diff)
    {
      if (op.m_Operation == wdAbstractGraphDiffOperation::Op::NodeAdded && guid == op.m_Node)
        return &op;
    }
    return nullptr;
  };

  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      if (pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags) || bIsValueType)
      {
        pObjectAccessor->SetValue(pObject, pProp, op.m_Value);
      }
      else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
      {
        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
        {
          if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          {
            const wdUuid oldGuid = pObjectAccessor->Get<wdUuid>(pObject, pProp);
            const wdUuid newGuid = op.m_Value.Get<wdUuid>();
            if (oldGuid.IsValid())
            {
              if (NeedsToBeDeleted(oldGuid))
              {
                pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(oldGuid));
              }
            }

            if (newGuid.IsValid())
            {
              if (wdAbstractGraphDiffOperation* pCreate = NeedsToBeCreated(newGuid))
              {
                pObjectAccessor->AddObject(pObject, pProp, wdVariant(), wdRTTI::FindTypeByName(pCreate->m_sProperty), pCreate->m_Node);
              }

              const wdDocumentObject* pChild = pObject->GetChild(newGuid);
              WD_ASSERT_DEV(pChild != nullptr, "References child object does not exist!");
            }
          }
          else
          {
            pObjectAccessor->SetValue(pObject, pProp, op.m_Value);
          }
        }
        else
        {
          // Noting to do here, value cannot change
        }
      }
      break;
    }
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
    {
      const wdVariantArray& values = op.m_Value.Get<wdVariantArray>();
      wdInt32 iCurrentCount = pObjectAccessor->GetCount(pObject, pProp);
      if (bIsValueType || (pProp->GetFlags().IsAnySet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)))
      {
        for (wdUInt32 i = 0; i < values.GetCount(); ++i)
        {
          if (i < (wdUInt32)iCurrentCount)
            pObjectAccessor->SetValue(pObject, pProp, values[i], i);
          else
            pObjectAccessor->InsertValue(pObject, pProp, values[i], i);
        }
        for (wdInt32 i = iCurrentCount - 1; i >= (wdInt32)values.GetCount(); --i)
        {
          pObjectAccessor->RemoveValue(pObject, pProp, i);
        }
      }
      else // Class
      {
        const wdInt32 iCurrentCount2 = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());

        wdHybridArray<wdVariant, 16> currentValues;
        pObject->GetTypeAccessor().GetValues(pProp->GetPropertyName(), currentValues);
        for (wdInt32 i = iCurrentCount2 - 1; i >= 0; --i)
        {
          if (NeedsToBeDeleted(currentValues[i].Get<wdUuid>()))
          {
            pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(currentValues[i].Get<wdUuid>()));
          }
        }

        for (wdUInt32 i = 0; i < values.GetCount(); ++i)
        {
          if (wdAbstractGraphDiffOperation* pCreate = NeedsToBeCreated(values[i].Get<wdUuid>()))
          {
            pObjectAccessor->AddObject(pObject, pProp, i, wdRTTI::FindTypeByName(pCreate->m_sProperty), pCreate->m_Node);
          }
          else
          {
            pObjectAccessor->MoveObject(pObjectAccessor->GetObject(values[i].Get<wdUuid>()), pObject, pProp, i);
          }
        }
      }
      break;
    }
    case wdPropertyCategory::Map:
    {
      const wdVariantDictionary& values = op.m_Value.Get<wdVariantDictionary>();
      wdHybridArray<wdVariant, 16> keys;
      WD_VERIFY(pObjectAccessor->GetKeys(pObject, pProp, keys).Succeeded(), "Property is not a map, getting keys failed.");

      if (bIsValueType || (pProp->GetFlags().IsAnySet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)))
      {
        for (const wdVariant& key : keys)
        {
          const wdString& sKey = key.Get<wdString>();
          if (!values.Contains(sKey))
          {
            WD_VERIFY(pObjectAccessor->RemoveValue(pObject, pProp, key).Succeeded(), "RemoveValue failed.");
          }
        }
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          wdVariant variantKey(it.Key());
          if (keys.Contains(variantKey))
            pObjectAccessor->SetValue(pObject, pProp, it.Value(), variantKey);
          else
            pObjectAccessor->InsertValue(pObject, pProp, it.Value(), variantKey);
        }
      }
      else // Class
      {
        for (const wdVariant& key : keys)
        {
          wdVariant value;
          WD_VERIFY(pObjectAccessor->GetValue(pObject, pProp, value, key).Succeeded(), "");
          if (NeedsToBeDeleted(value.Get<wdUuid>()))
          {
            pObjectAccessor->RemoveObject(pObjectAccessor->GetObject(value.Get<wdUuid>()));
          }
        }
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          const wdVariant& value = it.Value();
          wdVariant variantKey(it.Key());
          if (wdAbstractGraphDiffOperation* pCreate = NeedsToBeCreated(value.Get<wdUuid>()))
          {
            pObjectAccessor->AddObject(pObject, pProp, variantKey, wdRTTI::FindTypeByName(pCreate->m_sProperty), pCreate->m_Node);
          }
          else
          {
            pObjectAccessor->MoveObject(pObjectAccessor->GetObject(value.Get<wdUuid>()), pObject, pProp, variantKey);
          }
        }
      }
      break;
    }

    case wdPropertyCategory::Function:
    case wdPropertyCategory::Constant:
      break; // nothing to do
  }
}

void wdDocumentObjectConverterReader::ApplyProperty(
  wdDocumentObject* pObject, wdAbstractProperty* pProp, const wdAbstractObjectNode::Property* pSource)
{
  wdStringBuilder sTemp;

  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);

  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
        {
          const wdUuid guid = pSource->m_Value.Get<wdUuid>();
          if (guid.IsValid())
          {
            auto* pSubNode = m_pGraph->GetNode(guid);
            WD_ASSERT_DEV(pSubNode != nullptr, "invalid document");

            if (auto* pSubObject = CreateObjectFromNode(pSubNode))
            {
              ApplyPropertiesToObject(pSubNode, pSubObject);
              AddObject(pSubObject, pObject, pProp->GetPropertyName(), wdVariant());
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
        if (pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags) || bIsValueType)
        {
          pObject->GetTypeAccessor().SetValue(pProp->GetPropertyName(), pSource->m_Value);
        }
        else // wdPropertyFlags::Class
        {
          const wdUuid& nodeGuid = pSource->m_Value.Get<wdUuid>();

          const wdUuid subObjectGuid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName()).Get<wdUuid>();
          wdDocumentObject* pEmbeddedClassObject = pObject->GetChild(subObjectGuid);
          WD_ASSERT_DEV(pEmbeddedClassObject != nullptr, "CreateObject should have created all embedded classes!");
          auto* pSubNode = m_pGraph->GetNode(nodeGuid);
          WD_ASSERT_DEV(pSubNode != nullptr, "invalid document");

          ApplyPropertiesToObject(pSubNode, pEmbeddedClassObject);
        }
      }
      break;
    }
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
    {
      const wdVariantArray& array = pSource->m_Value.Get<wdVariantArray>();
      const wdInt32 iCurrentCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
      if (bIsValueType || (pProp->GetFlags().IsAnySet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)))
      {
        for (wdUInt32 i = 0; i < array.GetCount(); ++i)
        {
          if (i < (wdUInt32)iCurrentCount)
          {
            pObject->GetTypeAccessor().SetValue(pProp->GetPropertyName(), array[i], i);
          }
          else
          {
            pObject->GetTypeAccessor().InsertValue(pProp->GetPropertyName(), i, array[i]);
          }
        }
        for (wdInt32 i = iCurrentCount - 1; i >= (wdInt32)array.GetCount(); i--)
        {
          pObject->GetTypeAccessor().RemoveValue(pProp->GetPropertyName(), i);
        }
      }
      else
      {
        for (wdUInt32 i = 0; i < array.GetCount(); ++i)
        {
          const wdUuid guid = array[i].Get<wdUuid>();
          if (guid.IsValid())
          {
            auto* pSubNode = m_pGraph->GetNode(guid);
            WD_ASSERT_DEV(pSubNode != nullptr, "invalid document");

            if (i < (wdUInt32)iCurrentCount)
            {
              // Overwrite existing object
              wdUuid childGuid = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), i).ConvertTo<wdUuid>();
              if (wdDocumentObject* pSubObject = m_pManager->GetObject(childGuid))
              {
                ApplyPropertiesToObject(pSubNode, pSubObject);
              }
            }
            else
            {
              if (wdDocumentObject* pSubObject = CreateObjectFromNode(pSubNode))
              {
                ApplyPropertiesToObject(pSubNode, pSubObject);
                AddObject(pSubObject, pObject, pProp->GetPropertyName(), -1);
              }
            }
          }
        }
        for (wdInt32 i = iCurrentCount - 1; i >= (wdInt32)array.GetCount(); i--)
        {
          WD_REPORT_FAILURE("Not implemented");
        }
      }
      break;
    }
    case wdPropertyCategory::Map:
    {
      const wdVariantDictionary& values = pSource->m_Value.Get<wdVariantDictionary>();
      wdHybridArray<wdVariant, 16> keys;
      pObject->GetTypeAccessor().GetKeys(pProp->GetPropertyName(), keys);

      if (bIsValueType || (pProp->GetFlags().IsAnySet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner)))
      {
        for (const wdVariant& key : keys)
        {
          pObject->GetTypeAccessor().RemoveValue(pProp->GetPropertyName(), key);
        }
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          pObject->GetTypeAccessor().InsertValue(pProp->GetPropertyName(), wdVariant(it.Key()), it.Value());
        }
      }
      else
      {
        for (auto it = values.GetIterator(); it.IsValid(); ++it)
        {
          const wdVariant& value = it.Value();
          const wdUuid guid = value.Get<wdUuid>();

          const wdVariant variantKey(it.Key());

          if (guid.IsValid())
          {
            auto* pSubNode = m_pGraph->GetNode(guid);
            WD_ASSERT_DEV(pSubNode != nullptr, "invalid document");
            if (wdDocumentObject* pSubObject = CreateObjectFromNode(pSubNode))
            {
              ApplyPropertiesToObject(pSubNode, pSubObject);
              AddObject(pSubObject, pObject, pProp->GetPropertyName(), variantKey);
            }
          }
        }
      }
      break;
    }

    case wdPropertyCategory::Function:
    case wdPropertyCategory::Constant:
      break; // nothing to do
  }
}
