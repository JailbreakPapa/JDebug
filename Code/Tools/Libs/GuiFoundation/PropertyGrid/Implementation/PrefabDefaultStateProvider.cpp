#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/PrefabDefaultStateProvider.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

nsSharedPtr<nsDefaultStateProvider> nsPrefabDefaultStateProvider::CreateProvider(nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp)
{
  const auto* pMetaData = pObject->GetDocumentObjectManager()->GetDocument()->m_DocumentObjectMetaData.Borrow();
  nsInt32 iRootDepth = 0;
  nsUuid rootObjectGuid = nsPrefabUtils::GetPrefabRoot(pObject, *pMetaData, &iRootDepth);
  // The root depth is taken x2 because GetPrefabRoot counts the number of parent objects while nsDefaultStateProvider expects to count the properties as well.
  iRootDepth *= 2;
  // If we construct this from a property scope, the root is an additional hop away as GetPrefabRoot counts from the parent object.
  if (pProp)
    iRootDepth += 1;

  if (rootObjectGuid.IsValid())
  {
    auto pMeta = pMetaData->BeginReadMetaData(rootObjectGuid);
    NS_SCOPE_EXIT(pMetaData->EndReadMetaData(););
    nsUuid objectPrefabGuid = pObject->GetGuid();
    objectPrefabGuid.RevertCombinationWithSeed(pMeta->m_PrefabSeedGuid);
    const nsAbstractObjectGraph* pGraph = nsPrefabCache::GetSingleton()->GetCachedPrefabGraph(pMeta->m_CreateFromPrefab);
    if (pGraph)
    {
      if (pGraph->GetNode(objectPrefabGuid) != nullptr)
      {
        // The object was found in the prefab, we can thus use its prefab counterpart to provide a default state.
        return NS_DEFAULT_NEW(nsPrefabDefaultStateProvider, rootObjectGuid, pMeta->m_CreateFromPrefab, pMeta->m_PrefabSeedGuid, iRootDepth);
      }
    }
  }
  return nullptr;
}

nsPrefabDefaultStateProvider::nsPrefabDefaultStateProvider(const nsUuid& rootObjectGuid, const nsUuid& createFromPrefab, const nsUuid& prefabSeedGuid, nsInt32 iRootDepth)
  : m_RootObjectGuid(rootObjectGuid)
  , m_CreateFromPrefab(createFromPrefab)
  , m_PrefabSeedGuid(prefabSeedGuid)
  , m_iRootDepth(iRootDepth)
{
}

nsInt32 nsPrefabDefaultStateProvider::GetRootDepth() const
{
  return m_iRootDepth;
}

nsColorGammaUB nsPrefabDefaultStateProvider::GetBackgroundColor() const
{
  return nsColorScheme::DarkUI(nsColorScheme::Blue).WithAlpha(0.25f);
}

nsVariant nsPrefabDefaultStateProvider::GetDefaultValue(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index)
{
  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags);

  const nsAbstractObjectGraph* pGraph = nsPrefabCache::GetSingleton()->GetCachedPrefabGraph(m_CreateFromPrefab);
  nsUuid objectPrefabGuid = pObject->GetGuid();
  objectPrefabGuid.RevertCombinationWithSeed(m_PrefabSeedGuid);
  if (pGraph)
  {
    bool bValueFound = true;
    nsVariant defaultValue = nsPrefabUtils::GetDefaultValue(*pGraph, objectPrefabGuid, pProp->GetPropertyName(), index, &bValueFound);
    if (!bValueFound)
    {
      return superPtr[0]->GetDefaultValue(superPtr.GetSubArray(1), pAccessor, pObject, pProp, index);
    }

    if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags) && defaultValue.IsA<nsString>())
    {
      nsInt64 iValue = 0;
      if (nsReflectionUtils::StringToEnumeration(pProp->GetSpecificType(), defaultValue.Get<nsString>(), iValue))
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
        case nsPropertyCategory::Member:
        {
          nsUuid& targetGuid = defaultValue.GetWritable<nsUuid>();
          targetGuid.CombineWithSeed(m_PrefabSeedGuid);
        }
        break;
        case nsPropertyCategory::Array:
        case nsPropertyCategory::Set:
        {
          if (index.IsValid())
          {
            nsUuid& targetGuid = defaultValue.GetWritable<nsUuid>();
            targetGuid.CombineWithSeed(m_PrefabSeedGuid);
          }
          else
          {
            nsVariantArray& defaultValueArray = defaultValue.GetWritable<nsVariantArray>();
            for (nsVariant& value : defaultValueArray)
            {
              nsUuid& targetGuid = value.GetWritable<nsUuid>();
              targetGuid.CombineWithSeed(m_PrefabSeedGuid);
            }
          }
        }
        break;
        case nsPropertyCategory::Map:
        {
          if (index.IsValid())
          {
            nsUuid& targetGuid = defaultValue.GetWritable<nsUuid>();
            targetGuid.CombineWithSeed(m_PrefabSeedGuid);
          }
          else
          {
            nsVariantDictionary& defaultValueDict = defaultValue.GetWritable<nsVariantDictionary>();
            for (auto it : defaultValueDict)
            {
              nsUuid& targetGuid = it.Value().GetWritable<nsUuid>();
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
      if (defaultValue.IsString() && pProp->GetAttributeByType<nsGameObjectReferenceAttribute>())
      {
        // While pretty expensive this restores the default state of game object references which are stored as strings.
        nsStringView sValue = defaultValue.GetType() == nsVariantType::StringView ? defaultValue.Get<nsStringView>() : nsStringView(defaultValue.Get<nsString>().GetData());
        if (nsConversionUtils::IsStringUuid(sValue))
        {
          nsUuid guid = nsConversionUtils::ConvertStringToUuid(sValue);
          guid.CombineWithSeed(m_PrefabSeedGuid);
          nsStringBuilder sTemp;
          defaultValue = nsConversionUtils::ToString(guid, sTemp).GetData();
        }
      }

      return defaultValue;
    }
  }
  return superPtr[0]->GetDefaultValue(superPtr.GetSubArray(1), pAccessor, pObject, pProp);
}

nsStatus nsPrefabDefaultStateProvider::CreateRevertContainerDiff(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDeque<nsAbstractGraphDiffOperation>& out_diff)
{
  nsVariant defaultValue = GetDefaultValue(superPtr, pAccessor, pObject, pProp);
  nsVariant currentValue;
  NS_SUCCEED_OR_RETURN(pAccessor->GetValue(pObject, pProp, currentValue));

  const nsAbstractObjectGraph* pGraph = nsPrefabCache::GetSingleton()->GetCachedPrefabGraph(m_CreateFromPrefab);
  nsUuid objectPrefabGuid = pObject->GetGuid();
  objectPrefabGuid.RevertCombinationWithSeed(m_PrefabSeedGuid);
  if (pGraph)
  {
    // We create a sub-graph of only the parent node in both re-mapped prefab as well as from the actually object. We limit the graph to only the container property.
    auto pNode = pGraph->GetNode(objectPrefabGuid);
    nsAbstractObjectGraph prefabSubGraph;
    pGraph->Clone(prefabSubGraph, pNode, [pRootNode = pNode, pRootProp = pProp](const nsAbstractObjectNode* pNode, const nsAbstractObjectNode::Property* pProp)
      {
        if (pNode == pRootNode && pProp->m_sPropertyName != pRootProp->GetPropertyName())
          return false;

        return true; //
      });

    prefabSubGraph.ReMapNodeGuids(m_PrefabSeedGuid);

    nsAbstractObjectGraph instanceSubGraph;
    nsDocumentObjectConverterWriter writer(&instanceSubGraph, pObject->GetDocumentObjectManager(), [pRootObject = pObject, pRootProp = pProp](const nsDocumentObject* pObject, const nsAbstractProperty* pProp)
      {
        if (pObject == pRootObject && pProp != pRootProp)
          return false;

        return true; //
      });

    writer.AddObjectToGraph(pObject);

    prefabSubGraph.CreateDiffWithBaseGraph(instanceSubGraph, out_diff);

    return nsStatus(NS_SUCCESS);
  }

  return nsStatus(nsFmt("The object was not found in the base prefab graph."));
}
