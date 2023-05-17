#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/PrefabDefaultStateProvider.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

wdSharedPtr<wdDefaultStateProvider> wdPrefabDefaultStateProvider::CreateProvider(wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp)
{
  const auto* pMetaData = pObject->GetDocumentObjectManager()->GetDocument()->m_DocumentObjectMetaData.Borrow();
  wdInt32 iRootDepth = 0;
  wdUuid rootObjectGuid = wdPrefabUtils::GetPrefabRoot(pObject, *pMetaData, &iRootDepth);
  // The root depth is taken x2 because GetPrefabRoot counts the number of parent objects while wdDefaultStateProvider expects to count the properties as well.
  iRootDepth *= 2;
  // If we construct this from a property scope, the root is an additional hop away as GetPrefabRoot counts from the parent object.
  if (pProp)
    iRootDepth += 1;

  if (rootObjectGuid.IsValid())
  {
    auto pMeta = pMetaData->BeginReadMetaData(rootObjectGuid);
    WD_SCOPE_EXIT(pMetaData->EndReadMetaData(););
    wdUuid objectPrefabGuid = pObject->GetGuid();
    objectPrefabGuid.RevertCombinationWithSeed(pMeta->m_PrefabSeedGuid);
    const wdAbstractObjectGraph* pGraph = wdPrefabCache::GetSingleton()->GetCachedPrefabGraph(pMeta->m_CreateFromPrefab);
    if (pGraph)
    {
      if (pGraph->GetNode(objectPrefabGuid) != nullptr)
      {
        // The object was found in the prefab, we can thus use its prefab counterpart to provide a default state.
        return WD_DEFAULT_NEW(wdPrefabDefaultStateProvider, rootObjectGuid, pMeta->m_CreateFromPrefab, pMeta->m_PrefabSeedGuid, iRootDepth);
      }
    }
  }
  return nullptr;
}

wdPrefabDefaultStateProvider::wdPrefabDefaultStateProvider(const wdUuid& rootObjectGuid, const wdUuid& createFromPrefab, const wdUuid& prefabSeedGuid, wdInt32 iRootDepth)
  : m_RootObjectGuid(rootObjectGuid)
  , m_CreateFromPrefab(createFromPrefab)
  , m_PrefabSeedGuid(prefabSeedGuid)
  , m_iRootDepth(iRootDepth)
{
}

wdInt32 wdPrefabDefaultStateProvider::GetRootDepth() const
{
  return m_iRootDepth;
}

wdColorGammaUB wdPrefabDefaultStateProvider::GetBackgroundColor() const
{
  return wdColorScheme::DarkUI(wdColorScheme::Blue).WithAlpha(0.25f);
}

wdVariant wdPrefabDefaultStateProvider::GetDefaultValue(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index)
{
  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags);

  const wdAbstractObjectGraph* pGraph = wdPrefabCache::GetSingleton()->GetCachedPrefabGraph(m_CreateFromPrefab);
  wdUuid objectPrefabGuid = pObject->GetGuid();
  objectPrefabGuid.RevertCombinationWithSeed(m_PrefabSeedGuid);
  if (pGraph)
  {
    bool bValueFound = true;
    wdVariant defaultValue = wdPrefabUtils::GetDefaultValue(*pGraph, objectPrefabGuid, pProp->GetPropertyName(), index, &bValueFound);
    if (!bValueFound)
    {
      return superPtr[0]->GetDefaultValue(superPtr.GetSubArray(1), pAccessor, pObject, pProp, index);
    }

    if (pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags) && defaultValue.IsA<wdString>())
    {
      wdInt64 iValue = 0;
      if (wdReflectionUtils::StringToEnumeration(pProp->GetSpecificType(), defaultValue.Get<wdString>(), iValue))
      {
        defaultValue = iValue;
      }
      else
      {
        defaultValue = superPtr[0]->GetDefaultValue(superPtr.GetSubArray(1), pAccessor, pObject, pProp, index);
      }
    }
    else if (!bIsValueType)
    {
      // For object references we need to reverse the object GUID mapping from prefab -> instance.
      switch (pProp->GetCategory())
      {
        case wdPropertyCategory::Member:
        {
          wdUuid& targetGuid = defaultValue.GetWritable<wdUuid>();
          targetGuid.CombineWithSeed(m_PrefabSeedGuid);
        }
        break;
        case wdPropertyCategory::Array:
        case wdPropertyCategory::Set:
        {
          if (index.IsValid())
          {
            wdUuid& targetGuid = defaultValue.GetWritable<wdUuid>();
            targetGuid.CombineWithSeed(m_PrefabSeedGuid);
          }
          else
          {
            wdVariantArray& defaultValueArray = defaultValue.GetWritable<wdVariantArray>();
            for (wdVariant& value : defaultValueArray)
            {
              wdUuid& targetGuid = value.GetWritable<wdUuid>();
              targetGuid.CombineWithSeed(m_PrefabSeedGuid);
            }
          }
        }
        break;
        case wdPropertyCategory::Map:
        {
          if (index.IsValid())
          {
            wdUuid& targetGuid = defaultValue.GetWritable<wdUuid>();
            targetGuid.CombineWithSeed(m_PrefabSeedGuid);
          }
          else
          {
            wdVariantDictionary& defaultValueDict = defaultValue.GetWritable<wdVariantDictionary>();
            for (auto it : defaultValueDict)
            {
              wdUuid& targetGuid = it.Value().GetWritable<wdUuid>();
              targetGuid.CombineWithSeed(m_PrefabSeedGuid);
            }
          }
        }
        break;
        default:
          break;
      }
    }

    if (defaultValue.IsValid())
    {
      if (defaultValue.IsString() && pProp->GetAttributeByType<wdGameObjectReferenceAttribute>())
      {
        // While pretty expensive this restores the default state of game object references which are stored as strings.
        wdStringView sValue = defaultValue.GetType() == wdVariantType::StringView ? defaultValue.Get<wdStringView>() : wdStringView(defaultValue.Get<wdString>().GetData());
        if (wdConversionUtils::IsStringUuid(sValue))
        {
          wdUuid guid = wdConversionUtils::ConvertStringToUuid(sValue);
          guid.CombineWithSeed(m_PrefabSeedGuid);
          wdStringBuilder sTemp;
          defaultValue = wdConversionUtils::ToString(guid, sTemp).GetData();
        }
      }

      return defaultValue;
    }
  }
  return superPtr[0]->GetDefaultValue(superPtr.GetSubArray(1), pAccessor, pObject, pProp);
}

wdStatus wdPrefabDefaultStateProvider::CreateRevertContainerDiff(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDeque<wdAbstractGraphDiffOperation>& out_diff)
{
  wdVariant defaultValue = GetDefaultValue(superPtr, pAccessor, pObject, pProp);
  wdVariant currentValue;
  WD_SUCCEED_OR_RETURN(pAccessor->GetValue(pObject, pProp, currentValue));

  const wdAbstractObjectGraph* pGraph = wdPrefabCache::GetSingleton()->GetCachedPrefabGraph(m_CreateFromPrefab);
  wdUuid objectPrefabGuid = pObject->GetGuid();
  objectPrefabGuid.RevertCombinationWithSeed(m_PrefabSeedGuid);
  if (pGraph)
  {
    // We create a sub-graph of only the parent node in both re-mapped prefab as well as from the actually object. We limit the graph to only the container property.
    auto pNode = pGraph->GetNode(objectPrefabGuid);
    wdAbstractObjectGraph prefabSubGraph;
    wdAbstractObjectNode* pPrefabSubRoot = pGraph->Clone(prefabSubGraph, pNode, [pRootNode = pNode, pRootProp = pProp](const wdAbstractObjectNode* pNode, const wdAbstractObjectNode::Property* pProp) {
      if (pNode == pRootNode && !wdStringUtils::IsEqual(pProp->m_szPropertyName, pRootProp->GetPropertyName()))
        return false;
      return true;
    });
    prefabSubGraph.ReMapNodeGuids(m_PrefabSeedGuid);

    wdAbstractObjectGraph instanceSubGraph;
    wdDocumentObjectConverterWriter writer(&instanceSubGraph, pObject->GetDocumentObjectManager(), [pRootObject = pObject, pRootProp = pProp](const wdDocumentObject* pObject, const wdAbstractProperty* pProp) {
      if (pObject == pRootObject && pProp != pRootProp)
        return false;
      return true;
    });
    wdAbstractObjectNode* pInstanceSubRoot = writer.AddObjectToGraph(pObject);

    prefabSubGraph.CreateDiffWithBaseGraph(instanceSubGraph, out_diff);

    return wdStatus(WD_SUCCESS);
  }

  return wdStatus(wdFmt("The object was not found in the base prefab graph."));
}
