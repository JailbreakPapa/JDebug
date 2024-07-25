#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

void nsDocument::UpdatePrefabs()
{
  GetCommandHistory()->StartTransaction("Update Prefabs");

  UpdatePrefabsRecursive(GetObjectManager()->GetRootObject());

  GetCommandHistory()->FinishTransaction();

  ShowDocumentStatus("Prefabs have been updated");
  SetModified(true);
}

void nsDocument::RevertPrefabs(nsArrayPtr<const nsDocumentObject*> selection)
{
  if (selection.IsEmpty())
    return;

  auto pHistory = GetCommandHistory();

  pHistory->StartTransaction("Revert Prefab");

  for (auto pItem : selection)
  {
    RevertPrefab(pItem);
  }

  pHistory->FinishTransaction();
}

void nsDocument::UnlinkPrefabs(nsArrayPtr<const nsDocumentObject*> selection)
{
  if (selection.IsEmpty())
    return;

  auto pHistory = GetCommandHistory();
  pHistory->StartTransaction("Unlink Prefab");

  for (auto pObject : selection)
  {
    nsUnlinkPrefabCommand cmd;
    cmd.m_Object = pObject->GetGuid();

    pHistory->AddCommand(cmd).AssertSuccess();
  }

  pHistory->FinishTransaction();
}

nsStatus nsDocument::CreatePrefabDocumentFromSelection(nsStringView sFile, const nsRTTI* pRootType, nsDelegate<void(nsAbstractObjectNode*)> adjustGraphNodeCB, nsDelegate<void(nsDocumentObject*)> adjustNewNodesCB, nsDelegate<void(nsAbstractObjectGraph& graph, nsDynamicArray<nsAbstractObjectNode*>& graphRootNodes)> finalizeGraphCB)
{
  nsHybridArray<nsSelectionEntry, 64> selection;
  GetSelectionManager()->GetTopLevelSelectionOfType(pRootType, selection);

  if (selection.IsEmpty())
    return nsStatus("To create a prefab, the selection must not be empty");

  nsHybridArray<const nsDocumentObject*, 32> nodes;
  nodes.Reserve(selection.GetCount());
  for (const auto& e : selection)
  {
    nodes.PushBack(e.m_pObject);
  }

  nsUuid PrefabGuid, SeedGuid;
  SeedGuid = nsUuid::MakeUuid();
  nsStatus res = CreatePrefabDocument(sFile, nodes, SeedGuid, PrefabGuid, adjustGraphNodeCB, true, finalizeGraphCB);

  if (res.m_Result.Succeeded())
  {
    GetCommandHistory()->StartTransaction("Replace all by Prefab");

    // this replaces ONE object by the new prefab (we pick the last one in the selection)
    nsUuid newObj = ReplaceByPrefab(nodes.PeekBack(), sFile, PrefabGuid, SeedGuid, true);

    // if we had more than one selected objects, remove the others as well
    if (nodes.GetCount() > 1)
    {
      nodes.PopBack();

      for (auto pNode : nodes)
      {
        nsRemoveObjectCommand remCmd;
        remCmd.m_Object = pNode->GetGuid();

        GetCommandHistory()->AddCommand(remCmd).AssertSuccess();
      }
    }

    auto pObject = GetObjectManager()->GetObject(newObj);

    if (adjustNewNodesCB.IsValid())
    {
      adjustNewNodesCB(pObject);
    }

    GetCommandHistory()->FinishTransaction();
    GetSelectionManager()->SetSelection(pObject);
  }

  return res;
}

nsStatus nsDocument::CreatePrefabDocument(nsStringView sFile, nsArrayPtr<const nsDocumentObject*> rootObjects, const nsUuid& invPrefabSeed,
  nsUuid& out_newDocumentGuid, nsDelegate<void(nsAbstractObjectNode*)> adjustGraphNodeCB, bool bKeepOpen, nsDelegate<void(nsAbstractObjectGraph& graph, nsDynamicArray<nsAbstractObjectNode*>& graphRootNodes)> finalizeGraphCB)
{
  const nsDocumentTypeDescriptor* pTypeDesc = nullptr;
  if (nsDocumentManager::FindDocumentTypeFromPath(sFile, true, pTypeDesc).Failed())
    return nsStatus(nsFmt("Document type is unknown: '{0}'", sFile));

  pTypeDesc->m_pManager->EnsureDocumentIsClosed(sFile);

  // prepare the current state as a graph
  nsAbstractObjectGraph PrefabGraph;
  nsDocumentObjectConverterWriter writer(&PrefabGraph, GetObjectManager());

  nsHybridArray<nsAbstractObjectNode*, 32> graphRootNodes;
  graphRootNodes.Reserve(rootObjects.GetCount() + 1);

  for (nsUInt32 i = 0; i < rootObjects.GetCount(); ++i)
  {
    auto pSaveAsPrefab = rootObjects[i];

    NS_ASSERT_DEV(pSaveAsPrefab != nullptr, "CreatePrefabDocument: pSaveAsPrefab must be a valid object!");

    auto pPrefabGraphMainNode = writer.AddObjectToGraph(pSaveAsPrefab);
    graphRootNodes.PushBack(pPrefabGraphMainNode);

    // allow external adjustments
    if (adjustGraphNodeCB.IsValid())
    {
      adjustGraphNodeCB(pPrefabGraphMainNode);
    }
  }

  if (finalizeGraphCB.IsValid())
  {
    finalizeGraphCB(PrefabGraph, graphRootNodes);
  }

  PrefabGraph.ReMapNodeGuids(invPrefabSeed, true);

  nsDocument* pSceneDocument = nullptr;

  NS_SUCCEED_OR_RETURN(pTypeDesc->m_pManager->CreateDocument("Prefab", sFile, pSceneDocument, nsDocumentFlags::RequestWindow | nsDocumentFlags::AddToRecentFilesList | nsDocumentFlags::EmptyDocument));

  out_newDocumentGuid = pSceneDocument->GetGuid();
  auto pPrefabSceneRoot = pSceneDocument->GetObjectManager()->GetRootObject();

  nsDocumentObjectConverterReader reader(&PrefabGraph, pSceneDocument->GetObjectManager(), nsDocumentObjectConverterReader::Mode::CreateAndAddToDocument);

  for (nsUInt32 i = 0; i < graphRootNodes.GetCount(); ++i)
  {
    const nsRTTI* pRootType = nsRTTI::FindTypeByName(graphRootNodes[i]->GetType());

    nsUuid rootGuid = graphRootNodes[i]->GetGuid();
    rootGuid.RevertCombinationWithSeed(invPrefabSeed);

    nsDocumentObject* pPrefabSceneMainObject = pSceneDocument->GetObjectManager()->CreateObject(pRootType, rootGuid);
    pSceneDocument->GetObjectManager()->AddObject(pPrefabSceneMainObject, pPrefabSceneRoot, "Children", -1);

    reader.ApplyPropertiesToObject(graphRootNodes[i], pPrefabSceneMainObject);
  }

  pSceneDocument->SetModified(true);
  auto res = pSceneDocument->SaveDocument();

  if (!bKeepOpen)
  {
    pTypeDesc->m_pManager->CloseDocument(pSceneDocument);
  }

  return res;
}


nsUuid nsDocument::ReplaceByPrefab(const nsDocumentObject* pRootObject, nsStringView sPrefabFile, const nsUuid& prefabAsset, const nsUuid& prefabSeed, bool bEnginePrefab)
{
  GetCommandHistory()->StartTransaction("Replace by Prefab");

  nsUuid instantiatedRoot;

  if (!bEnginePrefab) // create editor prefab
  {
    nsInstantiatePrefabCommand instCmd;
    instCmd.m_Index = pRootObject->GetPropertyIndex().ConvertTo<nsInt32>();
    instCmd.m_bAllowPickedPosition = false;
    instCmd.m_CreateFromPrefab = prefabAsset;
    instCmd.m_Parent = pRootObject->GetParent() == GetObjectManager()->GetRootObject() ? nsUuid() : pRootObject->GetParent()->GetGuid();
    instCmd.m_sBasePrefabGraph = nsPrefabUtils::ReadDocumentAsString(
      sPrefabFile); // since the prefab might have been created just now, going through the cache (via GUID) will most likely fail
    instCmd.m_RemapGuid = prefabSeed;

    GetCommandHistory()->AddCommand(instCmd).AssertSuccess();

    instantiatedRoot = instCmd.m_CreatedRootObject;
  }
  else // create an object with the reference prefab component
  {
    auto pHistory = GetCommandHistory();

    nsStringBuilder tmp;
    nsUuid CmpGuid = nsUuid::MakeUuid();
    instantiatedRoot = nsUuid::MakeUuid();

    nsAddObjectCommand cmd;
    cmd.m_Parent = (pRootObject->GetParent() == GetObjectManager()->GetRootObject()) ? nsUuid() : pRootObject->GetParent()->GetGuid();
    cmd.m_Index = pRootObject->GetPropertyIndex();
    cmd.SetType("nsGameObject");
    cmd.m_NewObjectGuid = instantiatedRoot;
    cmd.m_sParentProperty = "Children";

    NS_VERIFY(pHistory->AddCommand(cmd).m_Result.Succeeded(), "AddCommand failed");

    cmd.SetType("nsPrefabReferenceComponent");
    cmd.m_sParentProperty = "Components";
    cmd.m_Index = -1;
    cmd.m_NewObjectGuid = CmpGuid;
    cmd.m_Parent = instantiatedRoot;
    NS_VERIFY(pHistory->AddCommand(cmd).m_Result.Succeeded(), "AddCommand failed");

    nsSetObjectPropertyCommand cmd2;
    cmd2.m_Object = CmpGuid;
    cmd2.m_sProperty = "Prefab";
    cmd2.m_NewValue = nsConversionUtils::ToString(prefabAsset, tmp).GetData();
    NS_VERIFY(pHistory->AddCommand(cmd2).m_Result.Succeeded(), "AddCommand failed");
  }

  {
    nsRemoveObjectCommand remCmd;
    remCmd.m_Object = pRootObject->GetGuid();

    GetCommandHistory()->AddCommand(remCmd).AssertSuccess();
  }

  GetCommandHistory()->FinishTransaction();

  return instantiatedRoot;
}

nsUuid nsDocument::RevertPrefab(const nsDocumentObject* pObject)
{
  auto pHistory = GetCommandHistory();
  auto pMeta = m_DocumentObjectMetaData->BeginReadMetaData(pObject->GetGuid());

  const nsUuid PrefabAsset = pMeta->m_CreateFromPrefab;

  if (!PrefabAsset.IsValid())
  {
    m_DocumentObjectMetaData->EndReadMetaData();
    return nsUuid();
  }

  nsRemoveObjectCommand remCmd;
  remCmd.m_Object = pObject->GetGuid();

  nsInstantiatePrefabCommand instCmd;
  instCmd.m_Index = pObject->GetPropertyIndex().ConvertTo<nsInt32>();
  instCmd.m_bAllowPickedPosition = false;
  instCmd.m_CreateFromPrefab = PrefabAsset;
  instCmd.m_Parent = pObject->GetParent() == GetObjectManager()->GetRootObject() ? nsUuid() : pObject->GetParent()->GetGuid();
  instCmd.m_RemapGuid = pMeta->m_PrefabSeedGuid;
  instCmd.m_sBasePrefabGraph = nsPrefabCache::GetSingleton()->GetCachedPrefabDocument(pMeta->m_CreateFromPrefab);

  m_DocumentObjectMetaData->EndReadMetaData();

  pHistory->AddCommand(remCmd).AssertSuccess();
  pHistory->AddCommand(instCmd).AssertSuccess();

  return instCmd.m_CreatedRootObject;
}


void nsDocument::UpdatePrefabsRecursive(nsDocumentObject* pObject)
{
  // Deliberately copy the array as the UpdatePrefabObject function will add / remove elements from the array.
  auto ChildArray = pObject->GetChildren();

  nsStringBuilder sPrefabBase;

  for (auto pChild : ChildArray)
  {
    auto pMeta = m_DocumentObjectMetaData->BeginReadMetaData(pChild->GetGuid());
    const nsUuid PrefabAsset = pMeta->m_CreateFromPrefab;
    const nsUuid PrefabSeed = pMeta->m_PrefabSeedGuid;
    sPrefabBase = pMeta->m_sBasePrefab;

    m_DocumentObjectMetaData->EndReadMetaData();

    // if this is a prefab instance, update it
    if (PrefabAsset.IsValid())
    {
      UpdatePrefabObject(pChild, PrefabAsset, PrefabSeed, sPrefabBase);
    }
    else
    {
      // only recurse if no prefab was found
      // nested prefabs are not allowed
      UpdatePrefabsRecursive(pChild);
    }
  }
}

void nsDocument::UpdatePrefabObject(nsDocumentObject* pObject, const nsUuid& PrefabAsset, const nsUuid& PrefabSeed, nsStringView sBasePrefab)
{
  const nsStringBuilder& sNewBasePrefab = nsPrefabCache::GetSingleton()->GetCachedPrefabDocument(PrefabAsset);

  nsStringBuilder sNewMergedGraph;
  nsPrefabUtils::Merge(sBasePrefab, sNewBasePrefab, pObject, true, PrefabSeed, sNewMergedGraph);

  // remove current object
  nsRemoveObjectCommand rm;
  rm.m_Object = pObject->GetGuid();

  // instantiate prefab again
  nsInstantiatePrefabCommand inst;
  inst.m_Index = pObject->GetPropertyIndex().ConvertTo<nsInt32>();
  inst.m_bAllowPickedPosition = false;
  inst.m_CreateFromPrefab = PrefabAsset;
  inst.m_Parent = pObject->GetParent() == GetObjectManager()->GetRootObject() ? nsUuid() : pObject->GetParent()->GetGuid();
  inst.m_RemapGuid = PrefabSeed;
  inst.m_sBasePrefabGraph = sNewBasePrefab;
  inst.m_sObjectGraph = sNewMergedGraph;

  GetCommandHistory()->AddCommand(rm).AssertSuccess();
  GetCommandHistory()->AddCommand(inst).AssertSuccess();
}
