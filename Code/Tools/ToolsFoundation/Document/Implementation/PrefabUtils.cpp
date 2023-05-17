#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

#define PREFAB_DEBUG false

wdString ToBinary(const wdUuid& guid)
{
  wdStringBuilder s, sResult;

  wdUInt8* pBytes = (wdUInt8*)&guid;

  for (wdUInt32 i = 0; i < sizeof(wdUuid); ++i)
  {
    s.Format("{0}", wdArgU((wdUInt32)*pBytes, 2, true, 16, true));
    ++pBytes;

    sResult.Append(s.GetData());
  }

  return sResult;
}

void wdPrefabUtils::LoadGraph(wdAbstractObjectGraph& out_graph, const char* szGraph)
{
  wdPrefabCache::GetSingleton()->LoadGraph(out_graph, wdStringView(szGraph));
}


wdAbstractObjectNode* wdPrefabUtils::GetFirstRootNode(wdAbstractObjectGraph& ref_graph)
{
  auto& nodes = ref_graph.GetAllNodes();
  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    auto* pNode = it.Value();
    if (wdStringUtils::IsEqual(pNode->GetNodeName(), "ObjectTree"))
    {
      for (const auto& ObjectTreeProp : pNode->GetProperties())
      {
        if (wdStringUtils::IsEqual(ObjectTreeProp.m_szPropertyName, "Children") && ObjectTreeProp.m_Value.IsA<wdVariantArray>())
        {
          const wdVariantArray& RootChildren = ObjectTreeProp.m_Value.Get<wdVariantArray>();

          for (const wdVariant& childGuid : RootChildren)
          {
            if (!childGuid.IsA<wdUuid>())
              continue;

            const wdUuid& rootObjectGuid = childGuid.Get<wdUuid>();

            return ref_graph.GetNode(rootObjectGuid);
          }
        }
      }
    }
  }
  return nullptr;
}

void wdPrefabUtils::GetRootNodes(wdAbstractObjectGraph& ref_graph, wdHybridArray<wdAbstractObjectNode*, 4>& out_nodes)
{
  auto& nodes = ref_graph.GetAllNodes();
  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    auto* pNode = it.Value();
    if (wdStringUtils::IsEqual(pNode->GetNodeName(), "ObjectTree"))
    {
      for (const auto& ObjectTreeProp : pNode->GetProperties())
      {
        if (wdStringUtils::IsEqual(ObjectTreeProp.m_szPropertyName, "Children") && ObjectTreeProp.m_Value.IsA<wdVariantArray>())
        {
          const wdVariantArray& RootChildren = ObjectTreeProp.m_Value.Get<wdVariantArray>();

          for (const wdVariant& childGuid : RootChildren)
          {
            if (!childGuid.IsA<wdUuid>())
              continue;

            const wdUuid& rootObjectGuid = childGuid.Get<wdUuid>();

            out_nodes.PushBack(ref_graph.GetNode(rootObjectGuid));
          }

          return;
        }
      }

      return;
    }
  }
}

wdUuid wdPrefabUtils::GetPrefabRoot(const wdDocumentObject* pObject, const wdObjectMetaData<wdUuid, wdDocumentObjectMetaData>& documentObjectMetaData, wdInt32* pDepth)
{
  auto pMeta = documentObjectMetaData.BeginReadMetaData(pObject->GetGuid());
  wdUuid source = pMeta->m_CreateFromPrefab;
  documentObjectMetaData.EndReadMetaData();

  if (source.IsValid())
  {
    return pObject->GetGuid();
  }

  if (pObject->GetParent() != nullptr)
  {
    if (pDepth)
      *pDepth += 1;
    return GetPrefabRoot(pObject->GetParent(), documentObjectMetaData);
  }
  return wdUuid();
}


wdVariant wdPrefabUtils::GetDefaultValue(const wdAbstractObjectGraph& graph, const wdUuid& objectGuid, const char* szProperty, wdVariant index, bool* pValueFound)
{
  if (pValueFound)
    *pValueFound = false;

  const wdAbstractObjectNode* pNode = graph.GetNode(objectGuid);
  if (!pNode)
    return wdVariant();

  const wdAbstractObjectNode::Property* pProp = pNode->FindProperty(szProperty);
  if (pProp)
  {
    const wdVariant& value = pProp->m_Value;

    if (value.IsA<wdVariantArray>() && index.CanConvertTo<wdUInt32>())
    {
      wdUInt32 uiIndex = index.ConvertTo<wdUInt32>();
      const wdVariantArray& valueArray = value.Get<wdVariantArray>();
      if (uiIndex < valueArray.GetCount())
      {
        if (pValueFound)
          *pValueFound = true;
        return valueArray[uiIndex];
      }
      return wdVariant();
    }
    else if (value.IsA<wdVariantDictionary>() && index.CanConvertTo<wdString>())
    {
      wdString sKey = index.ConvertTo<wdString>();
      const wdVariantDictionary& valueDict = value.Get<wdVariantDictionary>();
      auto it = valueDict.Find(sKey);
      if (it.IsValid())
      {
        if (pValueFound)
          *pValueFound = true;
        return it.Value();
      }
      return wdVariant();
    }
    if (pValueFound)
      *pValueFound = true;
    return value;
  }

  return wdVariant();
}

void wdPrefabUtils::WriteDiff(const wdDeque<wdAbstractGraphDiffOperation>& mergedDiff, wdStringBuilder& out_sText)
{
  for (const auto& diff : mergedDiff)
  {
    wdStringBuilder Data = ToBinary(diff.m_Node);

    switch (diff.m_Operation)
    {
      case wdAbstractGraphDiffOperation::Op::NodeAdded:
      {
        out_sText.AppendFormat("<add> - {{0}} ({1})\n", Data, diff.m_sProperty);
      }
      break;

      case wdAbstractGraphDiffOperation::Op::NodeRemoved:
      {
        out_sText.AppendFormat("<del> - {{0}}\n", Data);
      }
      break;

      case wdAbstractGraphDiffOperation::Op::PropertyChanged:
        if (diff.m_Value.CanConvertTo<wdString>())
          out_sText.AppendFormat("<set> - {{0}} - \"{1}\" = {2}\n", Data, diff.m_sProperty, diff.m_Value.ConvertTo<wdString>());
        else
          out_sText.AppendFormat("<set> - {{0}} - \"{1}\" = xxx\n", Data, diff.m_sProperty);
        break;
    }
  }
}

void wdPrefabUtils::Merge(const wdAbstractObjectGraph& baseGraph, const wdAbstractObjectGraph& leftGraph, const wdAbstractObjectGraph& rightGraph, wdDeque<wdAbstractGraphDiffOperation>& out_mergedDiff)
{
  // debug output
  if (PREFAB_DEBUG)
  {
    {
      wdFileWriter file;
      file.Open("C:\\temp\\Prefab - base.txt").IgnoreResult();
      wdAbstractGraphDdlSerializer::Write(file, &baseGraph, nullptr, false, wdOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }

    {
      wdFileWriter file;
      file.Open("C:\\temp\\Prefab - template.txt").IgnoreResult();
      wdAbstractGraphDdlSerializer::Write(file, &leftGraph, nullptr, false, wdOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }

    {
      wdFileWriter file;
      file.Open("C:\\temp\\Prefab - instance.txt").IgnoreResult();
      wdAbstractGraphDdlSerializer::Write(file, &rightGraph, nullptr, false, wdOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }
  }

  wdDeque<wdAbstractGraphDiffOperation> LeftToBase;
  leftGraph.CreateDiffWithBaseGraph(baseGraph, LeftToBase);
  wdDeque<wdAbstractGraphDiffOperation> RightToBase;
  rightGraph.CreateDiffWithBaseGraph(baseGraph, RightToBase);

  baseGraph.MergeDiffs(LeftToBase, RightToBase, out_mergedDiff);

  // debug output
  if (PREFAB_DEBUG)
  {
    wdFileWriter file;
    file.Open("C:\\temp\\Prefab - diff.txt").IgnoreResult();

    wdStringBuilder sDiff;
    sDiff.Append("######## Template To Base #######\n");
    wdPrefabUtils::WriteDiff(LeftToBase, sDiff);
    sDiff.Append("\n\n######## Instance To Base #######\n");
    wdPrefabUtils::WriteDiff(RightToBase, sDiff);
    sDiff.Append("\n\n######## Merged Diff #######\n");
    wdPrefabUtils::WriteDiff(out_mergedDiff, sDiff);


    file.WriteBytes(sDiff.GetData(), sDiff.GetElementCount()).IgnoreResult();
  }
}

void wdPrefabUtils::Merge(const char* szBase, const char* szLeft, wdDocumentObject* pRight, bool bRightIsNotPartOfPrefab, const wdUuid& prefabSeed, wdStringBuilder& out_sNewGraph)
{
  // prepare the original prefab as a graph
  wdAbstractObjectGraph baseGraph;
  wdPrefabUtils::LoadGraph(baseGraph, szBase);
  if (auto pHeader = baseGraph.GetNodeByName("Header"))
  {
    baseGraph.RemoveNode(pHeader->GetGuid());
  }

  {
    // read the new template as a graph
    wdAbstractObjectGraph leftGraph;
    wdPrefabUtils::LoadGraph(leftGraph, szLeft);
    if (auto pHeader = leftGraph.GetNodeByName("Header"))
    {
      leftGraph.RemoveNode(pHeader->GetGuid());
    }

    // prepare the current state as a graph
    wdAbstractObjectGraph rightGraph;
    {
      wdDocumentObjectConverterWriter writer(&rightGraph, pRight->GetDocumentObjectManager());

      wdVariantArray children;
      if (bRightIsNotPartOfPrefab)
      {
        for (wdDocumentObject* pChild : pRight->GetChildren())
        {
          writer.AddObjectToGraph(pChild);
          children.PushBack(pChild->GetGuid());
        }
      }
      else
      {
        writer.AddObjectToGraph(pRight);
        children.PushBack(pRight->GetGuid());
      }

      rightGraph.ReMapNodeGuids(prefabSeed, true);
      // just take the entire ObjectTree node as is TODO: this may cause a crash if the root object is replaced
      wdAbstractObjectNode* pRightObjectTree = rightGraph.CopyNodeIntoGraph(leftGraph.GetNodeByName("ObjectTree"));
      // The root node should always have a property 'children' where all the root objects are attached to. We need to replace that property's value as the prefab instance graph can have less or more objects than the template.
      wdAbstractObjectNode::Property* pChildrenProp = pRightObjectTree->FindProperty("Children");
      pChildrenProp->m_Value = children;
    }

    // Merge diffs relative to base
    wdDeque<wdAbstractGraphDiffOperation> mergedDiff;
    wdPrefabUtils::Merge(baseGraph, leftGraph, rightGraph, mergedDiff);


    {
      // Apply merged diff to base.
      baseGraph.ApplyDiff(mergedDiff);

      wdContiguousMemoryStreamStorage stor;
      wdMemoryStreamWriter sw(&stor);

      wdAbstractGraphDdlSerializer::Write(sw, &baseGraph, nullptr, true, wdOpenDdlWriter::TypeStringMode::Shortest);

      out_sNewGraph.SetSubString_ElementCount((const char*)stor.GetData(), stor.GetStorageSize32());
    }

    // debug output
    if (PREFAB_DEBUG)
    {
      wdFileWriter file;
      file.Open("C:\\temp\\Prefab - result.txt").IgnoreResult();
      wdAbstractGraphDdlSerializer::Write(file, &baseGraph, nullptr, false, wdOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }
  }
}

wdString wdPrefabUtils::ReadDocumentAsString(const char* szFile)
{
  wdFileReader file;
  if (file.Open(szFile) == WD_FAILURE)
  {
    wdLog::Error("Failed to open document file '{0}'", szFile);
    return wdString();
  }

  wdStringBuilder sGraph;
  sGraph.ReadAll(file);

  return sGraph;
}
