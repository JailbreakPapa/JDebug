#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

#define PREFAB_DEBUG false

nsString ToBinary(const nsUuid& guid)
{
  nsStringBuilder s, sResult;

  nsUInt8* pBytes = (nsUInt8*)&guid;

  for (nsUInt32 i = 0; i < sizeof(nsUuid); ++i)
  {
    s.SetFormat("{0}", nsArgU((nsUInt32)*pBytes, 2, true, 16, true));
    ++pBytes;

    sResult.Append(s.GetData());
  }

  return sResult;
}

void nsPrefabUtils::LoadGraph(nsAbstractObjectGraph& out_graph, nsStringView sGraph)
{
  nsPrefabCache::GetSingleton()->LoadGraph(out_graph, nsStringView(sGraph));
}


nsAbstractObjectNode* nsPrefabUtils::GetFirstRootNode(nsAbstractObjectGraph& ref_graph)
{
  auto& nodes = ref_graph.GetAllNodes();
  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    auto* pNode = it.Value();
    if (pNode->GetNodeName() == "ObjectTree")
    {
      for (const auto& ObjectTreeProp : pNode->GetProperties())
      {
        if (ObjectTreeProp.m_sPropertyName == "Children" && ObjectTreeProp.m_Value.IsA<nsVariantArray>())
        {
          const nsVariantArray& RootChildren = ObjectTreeProp.m_Value.Get<nsVariantArray>();

          for (const nsVariant& childGuid : RootChildren)
          {
            if (!childGuid.IsA<nsUuid>())
              continue;

            const nsUuid& rootObjectGuid = childGuid.Get<nsUuid>();

            return ref_graph.GetNode(rootObjectGuid);
          }
        }
      }
    }
  }
  return nullptr;
}

void nsPrefabUtils::GetRootNodes(nsAbstractObjectGraph& ref_graph, nsHybridArray<nsAbstractObjectNode*, 4>& out_nodes)
{
  auto& nodes = ref_graph.GetAllNodes();
  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    auto* pNode = it.Value();
    if (pNode->GetNodeName() == "ObjectTree")
    {
      for (const auto& ObjectTreeProp : pNode->GetProperties())
      {
        if (ObjectTreeProp.m_sPropertyName == "Children" && ObjectTreeProp.m_Value.IsA<nsVariantArray>())
        {
          const nsVariantArray& RootChildren = ObjectTreeProp.m_Value.Get<nsVariantArray>();

          for (const nsVariant& childGuid : RootChildren)
          {
            if (!childGuid.IsA<nsUuid>())
              continue;

            const nsUuid& rootObjectGuid = childGuid.Get<nsUuid>();

            out_nodes.PushBack(ref_graph.GetNode(rootObjectGuid));
          }

          return;
        }
      }

      return;
    }
  }
}

nsUuid nsPrefabUtils::GetPrefabRoot(const nsDocumentObject* pObject, const nsObjectMetaData<nsUuid, nsDocumentObjectMetaData>& documentObjectMetaData, nsInt32* pDepth)
{
  auto pMeta = documentObjectMetaData.BeginReadMetaData(pObject->GetGuid());
  nsUuid source = pMeta->m_CreateFromPrefab;
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
  return nsUuid();
}


nsVariant nsPrefabUtils::GetDefaultValue(const nsAbstractObjectGraph& graph, const nsUuid& objectGuid, nsStringView sProperty, nsVariant index, bool* pValueFound)
{
  if (pValueFound)
    *pValueFound = false;

  const nsAbstractObjectNode* pNode = graph.GetNode(objectGuid);
  if (!pNode)
    return nsVariant();

  const nsAbstractObjectNode::Property* pProp = pNode->FindProperty(sProperty);
  if (pProp)
  {
    const nsVariant& value = pProp->m_Value;

    if (value.IsA<nsVariantArray>() && index.CanConvertTo<nsUInt32>())
    {
      nsUInt32 uiIndex = index.ConvertTo<nsUInt32>();
      const nsVariantArray& valueArray = value.Get<nsVariantArray>();
      if (uiIndex < valueArray.GetCount())
      {
        if (pValueFound)
          *pValueFound = true;
        return valueArray[uiIndex];
      }
      return nsVariant();
    }
    else if (value.IsA<nsVariantDictionary>() && index.CanConvertTo<nsString>())
    {
      nsString sKey = index.ConvertTo<nsString>();
      const nsVariantDictionary& valueDict = value.Get<nsVariantDictionary>();
      auto it = valueDict.Find(sKey);
      if (it.IsValid())
      {
        if (pValueFound)
          *pValueFound = true;
        return it.Value();
      }
      return nsVariant();
    }
    if (pValueFound)
      *pValueFound = true;
    return value;
  }

  return nsVariant();
}

void nsPrefabUtils::WriteDiff(const nsDeque<nsAbstractGraphDiffOperation>& mergedDiff, nsStringBuilder& out_sText)
{
  for (const auto& diff : mergedDiff)
  {
    nsStringBuilder Data = ToBinary(diff.m_Node);

    switch (diff.m_Operation)
    {
      case nsAbstractGraphDiffOperation::Op::NodeAdded:
      {
        out_sText.AppendFormat("<add> - {{0}} ({1})\n", Data, diff.m_sProperty);
      }
      break;

      case nsAbstractGraphDiffOperation::Op::NodeRemoved:
      {
        out_sText.AppendFormat("<del> - {{0}}\n", Data);
      }
      break;

      case nsAbstractGraphDiffOperation::Op::PropertyChanged:
        if (diff.m_Value.CanConvertTo<nsString>())
          out_sText.AppendFormat("<set> - {{0}} - \"{1}\" = {2}\n", Data, diff.m_sProperty, diff.m_Value.ConvertTo<nsString>());
        else
          out_sText.AppendFormat("<set> - {{0}} - \"{1}\" = xxx\n", Data, diff.m_sProperty);
        break;
    }
  }
}

void nsPrefabUtils::Merge(const nsAbstractObjectGraph& baseGraph, const nsAbstractObjectGraph& leftGraph, const nsAbstractObjectGraph& rightGraph, nsDeque<nsAbstractGraphDiffOperation>& out_mergedDiff)
{
  // debug output
  if (PREFAB_DEBUG)
  {
    {
      nsFileWriter file;
      file.Open("C:\\temp\\Prefab - base.txt").IgnoreResult();
      nsAbstractGraphDdlSerializer::Write(file, &baseGraph, nullptr, false, nsOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }

    {
      nsFileWriter file;
      file.Open("C:\\temp\\Prefab - template.txt").IgnoreResult();
      nsAbstractGraphDdlSerializer::Write(file, &leftGraph, nullptr, false, nsOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }

    {
      nsFileWriter file;
      file.Open("C:\\temp\\Prefab - instance.txt").IgnoreResult();
      nsAbstractGraphDdlSerializer::Write(file, &rightGraph, nullptr, false, nsOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }
  }

  nsDeque<nsAbstractGraphDiffOperation> LeftToBase;
  leftGraph.CreateDiffWithBaseGraph(baseGraph, LeftToBase);
  nsDeque<nsAbstractGraphDiffOperation> RightToBase;
  rightGraph.CreateDiffWithBaseGraph(baseGraph, RightToBase);

  baseGraph.MergeDiffs(LeftToBase, RightToBase, out_mergedDiff);

  // debug output
  if (PREFAB_DEBUG)
  {
    nsFileWriter file;
    file.Open("C:\\temp\\Prefab - diff.txt").IgnoreResult();

    nsStringBuilder sDiff;
    sDiff.Append("######## Template To Base #######\n");
    nsPrefabUtils::WriteDiff(LeftToBase, sDiff);
    sDiff.Append("\n\n######## Instance To Base #######\n");
    nsPrefabUtils::WriteDiff(RightToBase, sDiff);
    sDiff.Append("\n\n######## Merged Diff #######\n");
    nsPrefabUtils::WriteDiff(out_mergedDiff, sDiff);


    file.WriteBytes(sDiff.GetData(), sDiff.GetElementCount()).IgnoreResult();
  }
}

void nsPrefabUtils::Merge(nsStringView sBase, nsStringView sLeft, nsDocumentObject* pRight, bool bRightIsNotPartOfPrefab, const nsUuid& prefabSeed, nsStringBuilder& out_sNewGraph)
{
  // prepare the original prefab as a graph
  nsAbstractObjectGraph baseGraph;
  nsPrefabUtils::LoadGraph(baseGraph, sBase);
  if (auto pHeader = baseGraph.GetNodeByName("Header"))
  {
    baseGraph.RemoveNode(pHeader->GetGuid());
  }

  {
    // read the new template as a graph
    nsAbstractObjectGraph leftGraph;
    nsPrefabUtils::LoadGraph(leftGraph, sLeft);
    if (auto pHeader = leftGraph.GetNodeByName("Header"))
    {
      leftGraph.RemoveNode(pHeader->GetGuid());
    }

    // prepare the current state as a graph
    nsAbstractObjectGraph rightGraph;
    {
      nsDocumentObjectConverterWriter writer(&rightGraph, pRight->GetDocumentObjectManager());

      nsVariantArray children;
      if (bRightIsNotPartOfPrefab)
      {
        for (nsDocumentObject* pChild : pRight->GetChildren())
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
      nsAbstractObjectNode* pRightObjectTree = rightGraph.CopyNodeIntoGraph(leftGraph.GetNodeByName("ObjectTree"));
      // The root node should always have a property 'children' where all the root objects are attached to. We need to replace that property's value as the prefab instance graph can have less or more objects than the template.
      nsAbstractObjectNode::Property* pChildrenProp = pRightObjectTree->FindProperty("Children");
      pChildrenProp->m_Value = children;
    }

    // Merge diffs relative to base
    nsDeque<nsAbstractGraphDiffOperation> mergedDiff;
    nsPrefabUtils::Merge(baseGraph, leftGraph, rightGraph, mergedDiff);


    {
      // Apply merged diff to base.
      baseGraph.ApplyDiff(mergedDiff);

      nsContiguousMemoryStreamStorage stor;
      nsMemoryStreamWriter sw(&stor);

      nsAbstractGraphDdlSerializer::Write(sw, &baseGraph, nullptr, true, nsOpenDdlWriter::TypeStringMode::Shortest);

      out_sNewGraph.SetSubString_ElementCount((const char*)stor.GetData(), stor.GetStorageSize32());
    }

    // debug output
    if (PREFAB_DEBUG)
    {
      nsFileWriter file;
      file.Open("C:\\temp\\Prefab - result.txt").IgnoreResult();
      nsAbstractGraphDdlSerializer::Write(file, &baseGraph, nullptr, false, nsOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);
    }
  }
}

nsString nsPrefabUtils::ReadDocumentAsString(nsStringView sFile)
{
  nsFileReader file;
  if (file.Open(sFile) == NS_FAILURE)
  {
    nsLog::Error("Failed to open document file '{0}'", sFile);
    return nsString();
  }

  nsStringBuilder sGraph;
  sGraph.ReadAll(file);

  return sGraph;
}
