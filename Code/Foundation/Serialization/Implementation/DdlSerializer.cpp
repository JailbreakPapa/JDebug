#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/GraphVersioning.h>

namespace
{
  nsSerializedBlock* FindBlock(nsHybridArray<nsSerializedBlock, 3>& ref_blocks, nsStringView sName)
  {
    for (auto& block : ref_blocks)
    {
      if (block.m_Name == sName)
      {
        return &block;
      }
    }
    return nullptr;
  }

  nsSerializedBlock* FindHeaderBlock(nsHybridArray<nsSerializedBlock, 3>& ref_blocks, nsInt32& out_iVersion)
  {
    nsStringBuilder sHeaderName = "HeaderV";
    out_iVersion = 0;
    for (auto& block : ref_blocks)
    {
      if (block.m_Name.StartsWith(sHeaderName))
      {
        nsResult res = nsConversionUtils::StringToInt(block.m_Name.GetData() + sHeaderName.GetElementCount(), out_iVersion);
        if (res.Failed())
        {
          nsLog::Error("Failed to parse version from header name '{0}'", block.m_Name);
        }
        return &block;
      }
    }
    return nullptr;
  }

  nsSerializedBlock* GetOrCreateBlock(nsHybridArray<nsSerializedBlock, 3>& ref_blocks, nsStringView sName)
  {
    nsSerializedBlock* pBlock = FindBlock(ref_blocks, sName);
    if (!pBlock)
    {
      pBlock = &ref_blocks.ExpandAndGetRef();
      pBlock->m_Name = sName;
    }
    if (!pBlock->m_Graph)
    {
      pBlock->m_Graph = NS_DEFAULT_NEW(nsAbstractObjectGraph);
    }
    return pBlock;
  }
} // namespace

static void WriteGraph(nsOpenDdlWriter& ref_writer, const nsAbstractObjectGraph* pGraph, const char* szName)
{
  nsMap<nsStringView, const nsVariant*> SortedProperties;

  ref_writer.BeginObject(szName);

  const auto& Nodes = pGraph->GetAllNodes();
  for (auto itNode = Nodes.GetIterator(); itNode.IsValid(); ++itNode)
  {
    const auto& node = *itNode.Value();

    ref_writer.BeginObject("o");

    {

      nsOpenDdlUtils::StoreUuid(ref_writer, node.GetGuid(), "id");
      nsOpenDdlUtils::StoreString(ref_writer, node.GetType(), "t");
      nsOpenDdlUtils::StoreUInt32(ref_writer, node.GetTypeVersion(), "v");

      if (!node.GetNodeName().IsEmpty())
        nsOpenDdlUtils::StoreString(ref_writer, node.GetNodeName(), "n");

      ref_writer.BeginObject("p");
      {
        for (const auto& prop : node.GetProperties())
          SortedProperties[prop.m_sPropertyName] = &prop.m_Value;

        for (auto it = SortedProperties.GetIterator(); it.IsValid(); ++it)
        {
          nsOpenDdlUtils::StoreVariant(ref_writer, *it.Value(), it.Key());
        }

        SortedProperties.Clear();
      }
      ref_writer.EndObject();
    }
    ref_writer.EndObject();
  }

  ref_writer.EndObject();
}

void nsAbstractGraphDdlSerializer::Write(nsStreamWriter& inout_stream, const nsAbstractObjectGraph* pGraph, const nsAbstractObjectGraph* pTypesGraph,
  bool bCompactMmode, nsOpenDdlWriter::TypeStringMode typeMode)
{
  nsOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(bCompactMmode);
  writer.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Exact);
  writer.SetPrimitiveTypeStringMode(typeMode);

  if (typeMode != nsOpenDdlWriter::TypeStringMode::Compliant)
    writer.SetIndentation(-1);

  Write(writer, pGraph, pTypesGraph);
}


void nsAbstractGraphDdlSerializer::Write(
  nsOpenDdlWriter& ref_writer, const nsAbstractObjectGraph* pGraph, const nsAbstractObjectGraph* pTypesGraph /*= nullptr*/)
{
  WriteGraph(ref_writer, pGraph, "Objects");
  if (pTypesGraph)
  {
    WriteGraph(ref_writer, pTypesGraph, "Types");
  }
}

static void ReadGraph(nsAbstractObjectGraph* pGraph, const nsOpenDdlReaderElement* pRoot)
{
  nsStringBuilder tmp, tmp2;
  nsVariant varTmp;

  for (const nsOpenDdlReaderElement* pObject = pRoot->GetFirstChild(); pObject != nullptr; pObject = pObject->GetSibling())
  {
    const nsOpenDdlReaderElement* pGuid = pObject->FindChildOfType(nsOpenDdlPrimitiveType::Custom, "id");
    const nsOpenDdlReaderElement* pType = pObject->FindChildOfType(nsOpenDdlPrimitiveType::String, "t");
    const nsOpenDdlReaderElement* pTypeVersion = pObject->FindChildOfType(nsOpenDdlPrimitiveType::UInt32, "v");
    const nsOpenDdlReaderElement* pName = pObject->FindChildOfType(nsOpenDdlPrimitiveType::String, "n");
    const nsOpenDdlReaderElement* pProps = pObject->FindChildOfType("p");

    if (pGuid == nullptr || pType == nullptr || pProps == nullptr)
    {
      NS_REPORT_FAILURE("Object contains invalid elements");
      continue;
    }

    nsUuid guid;
    if (nsOpenDdlUtils::ConvertToUuid(pGuid, guid).Failed())
    {
      NS_REPORT_FAILURE("Object has an invalid guid");
      continue;
    }

    tmp = pType->GetPrimitivesString()[0];

    if (pName)
      tmp2 = pName->GetPrimitivesString()[0];
    else
      tmp2.Clear();

    nsUInt32 uiTypeVersion = 0;
    if (pTypeVersion)
    {
      uiTypeVersion = pTypeVersion->GetPrimitivesUInt32()[0];
    }

    auto* pNode = pGraph->AddNode(guid, tmp, uiTypeVersion, tmp2);

    for (const nsOpenDdlReaderElement* pProp = pProps->GetFirstChild(); pProp != nullptr; pProp = pProp->GetSibling())
    {
      if (!pProp->HasName())
        continue;

      if (nsOpenDdlUtils::ConvertToVariant(pProp, varTmp).Failed())
        continue;

      pNode->AddProperty(pProp->GetName(), varTmp);
    }
  }
}

nsResult nsAbstractGraphDdlSerializer::Read(
  nsStreamReader& inout_stream, nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph, bool bApplyPatches)
{
  nsOpenDdlReader reader;
  if (reader.ParseDocument(inout_stream, 0, nsLog::GetThreadLocalLogSystem()).Failed())
  {
    nsLog::Error("Failed to parse DDL graph");
    return NS_FAILURE;
  }

  return Read(reader.GetRootElement(), pGraph, pTypesGraph, bApplyPatches);
}


nsResult nsAbstractGraphDdlSerializer::Read(const nsOpenDdlReaderElement* pRootElement, nsAbstractObjectGraph* pGraph,
  nsAbstractObjectGraph* pTypesGraph /*= nullptr*/, bool bApplyPatches /*= true*/)
{
  const nsOpenDdlReaderElement* pObjects = pRootElement->FindChildOfType("Objects");
  if (pObjects != nullptr)
  {
    ReadGraph(pGraph, pObjects);
  }
  else
  {
    nsLog::Error("DDL graph does not contain an 'Objects' root object");
    return NS_FAILURE;
  }

  nsAbstractObjectGraph* pTempTypesGraph = pTypesGraph;
  if (pTempTypesGraph == nullptr)
  {
    pTempTypesGraph = NS_DEFAULT_NEW(nsAbstractObjectGraph);
  }
  const nsOpenDdlReaderElement* pTypes = pRootElement->FindChildOfType("Types");
  if (pTypes != nullptr)
  {
    ReadGraph(pTempTypesGraph, pTypes);
  }

  if (bApplyPatches)
  {
    if (pTempTypesGraph)
      nsGraphVersioning::GetSingleton()->PatchGraph(pTempTypesGraph);
    nsGraphVersioning::GetSingleton()->PatchGraph(pGraph, pTempTypesGraph);
  }

  if (pTypesGraph == nullptr)
    NS_DEFAULT_DELETE(pTempTypesGraph);

  return NS_SUCCESS;
}

nsResult nsAbstractGraphDdlSerializer::ReadBlocks(nsStreamReader& stream, nsHybridArray<nsSerializedBlock, 3>& blocks)
{
  nsOpenDdlReader reader;
  if (reader.ParseDocument(stream, 0, nsLog::GetThreadLocalLogSystem()).Failed())
  {
    nsLog::Error("Failed to parse DDL graph");
    return NS_FAILURE;
  }

  const nsOpenDdlReaderElement* pRoot = reader.GetRootElement();
  for (const nsOpenDdlReaderElement* pChild = pRoot->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
  {
    nsSerializedBlock* pBlock = GetOrCreateBlock(blocks, pChild->GetCustomType());
    ReadGraph(pBlock->m_Graph.Borrow(), pChild);
  }
  return NS_SUCCESS;
}

#define NS_DOCUMENT_VERSION 2

void nsAbstractGraphDdlSerializer::WriteDocument(nsStreamWriter& inout_stream, const nsAbstractObjectGraph* pHeader, const nsAbstractObjectGraph* pGraph,
  const nsAbstractObjectGraph* pTypes, bool bCompactMode, nsOpenDdlWriter::TypeStringMode typeMode)
{
  nsOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(bCompactMode);
  writer.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Exact);
  writer.SetPrimitiveTypeStringMode(typeMode);

  if (typeMode != nsOpenDdlWriter::TypeStringMode::Compliant)
    writer.SetIndentation(-1);

  nsStringBuilder sHeaderVersion;
  sHeaderVersion.SetFormat("HeaderV{0}", (int)NS_DOCUMENT_VERSION);
  WriteGraph(writer, pHeader, sHeaderVersion);
  WriteGraph(writer, pGraph, "Objects");
  WriteGraph(writer, pTypes, "Types");
}

nsResult nsAbstractGraphDdlSerializer::ReadDocument(nsStreamReader& inout_stream, nsUniquePtr<nsAbstractObjectGraph>& ref_pHeader,
  nsUniquePtr<nsAbstractObjectGraph>& ref_pGraph, nsUniquePtr<nsAbstractObjectGraph>& ref_pTypes, bool bApplyPatches)
{
  nsHybridArray<nsSerializedBlock, 3> blocks;
  if (ReadBlocks(inout_stream, blocks).Failed())
  {
    return NS_FAILURE;
  }

  nsInt32 iVersion = 2;
  nsSerializedBlock* pHB = FindHeaderBlock(blocks, iVersion);
  nsSerializedBlock* pOB = FindBlock(blocks, "Objects");
  nsSerializedBlock* pTB = FindBlock(blocks, "Types");
  if (!pOB)
  {
    nsLog::Error("No 'Objects' block in document");
    return NS_FAILURE;
  }
  if (!pTB && !pHB)
  {
    iVersion = 0;
  }
  else if (!pHB)
  {
    iVersion = 1;
  }
  if (iVersion < 2)
  {
    // Move header into its own graph.
    nsStringBuilder sHeaderVersion;
    sHeaderVersion.SetFormat("HeaderV{0}", iVersion);
    pHB = GetOrCreateBlock(blocks, sHeaderVersion);
    nsAbstractObjectGraph& graph = *pOB->m_Graph.Borrow();
    if (auto* pHeaderNode = graph.GetNodeByName("Header"))
    {
      nsAbstractObjectGraph& headerGraph = *pHB->m_Graph.Borrow();
      /*auto* pNewHeaderNode =*/headerGraph.CopyNodeIntoGraph(pHeaderNode);
      // pNewHeaderNode->AddProperty("DocVersion", iVersion);
      graph.RemoveNode(pHeaderNode->GetGuid());
    }
  }

  if (bApplyPatches && pTB)
  {
    nsGraphVersioning::GetSingleton()->PatchGraph(pTB->m_Graph.Borrow());
    nsGraphVersioning::GetSingleton()->PatchGraph(pHB->m_Graph.Borrow(), pTB->m_Graph.Borrow());
    nsGraphVersioning::GetSingleton()->PatchGraph(pOB->m_Graph.Borrow(), pTB->m_Graph.Borrow());
  }

  ref_pHeader = std::move(pHB->m_Graph);
  ref_pGraph = std::move(pOB->m_Graph);
  if (pTB)
  {
    ref_pTypes = std::move(pTB->m_Graph);
  }

  return NS_SUCCESS;
}

// This is a handcrafted DDL reader that ignores everything that is not an 'AssetInfo' object
// The purpose is to speed up reading asset information by skipping everything else
//
// Version 0 and 1:
// The reader 'knows' the file format details and uses them.
// Top-level (ie. depth 0) there is an "Objects" object -> we need to enter that
// Inside that (depth 1) there is the "AssetInfo" object -> need to enter that as well
// All objects inside that must be stored
// Once the AssetInfo object is left everything else can be skipped
//
// Version 2:
// The very first top level object is "Header" and only that is read and parsing is stopped afterwards.
class HeaderReader : public nsOpenDdlReader
{
public:
  HeaderReader() = default;

  bool m_bHasHeader = false;
  nsInt32 m_iDepth = 0;

  virtual void OnBeginObject(nsStringView sType, nsStringView sName, bool bGlobalName) override
  {
    //////////////////////////////////////////////////////////////////////////
    // New document format has header block.
    if (m_iDepth == 0 && sType.StartsWith("HeaderV"))
    {
      m_bHasHeader = true;
    }
    if (m_bHasHeader)
    {
      ++m_iDepth;
      nsOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    //////////////////////////////////////////////////////////////////////////
    // Old header is stored in the object block.
    // not yet entered the "Objects" group
    if (m_iDepth == 0 && sType == "Objects")
    {
      ++m_iDepth;

      nsOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    // not yet entered the "AssetInfo" group, but inside "Objects"
    if (m_iDepth == 1 && sType == "AssetInfo")
    {
      ++m_iDepth;

      nsOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    // inside "AssetInfo"
    if (m_iDepth > 1)
    {
      ++m_iDepth;
      nsOpenDdlReader::OnBeginObject(sType, sName, bGlobalName);
      return;
    }

    // ignore everything else
    SkipRestOfObject();
  }


  virtual void OnEndObject() override
  {
    --m_iDepth;
    if (m_bHasHeader)
    {
      if (m_iDepth == 0)
      {
        m_iDepth = -1;
        StopParsing();
      }
    }
    else
    {
      if (m_iDepth <= 1)
      {
        // we were inside "AssetInfo" or "Objects" and returned from it, so now skip the rest
        m_iDepth = -1;
        StopParsing();
      }
    }
    nsOpenDdlReader::OnEndObject();
  }
};

nsResult nsAbstractGraphDdlSerializer::ReadHeader(nsStreamReader& inout_stream, nsAbstractObjectGraph* pGraph)
{
  HeaderReader reader;
  if (reader.ParseDocument(inout_stream, 0, nsLog::GetThreadLocalLogSystem()).Failed())
  {
    NS_REPORT_FAILURE("Failed to parse DDL graph");
    return NS_FAILURE;
  }

  const nsOpenDdlReaderElement* pObjects = nullptr;
  if (reader.m_bHasHeader)
  {
    pObjects = reader.GetRootElement()->GetFirstChild();
  }
  else
  {
    pObjects = reader.GetRootElement()->FindChildOfType("Objects");
  }

  if (pObjects != nullptr)
  {
    ReadGraph(pGraph, pObjects);
  }
  else
  {
    NS_REPORT_FAILURE("DDL graph does not contain an 'Objects' root object");
    return NS_FAILURE;
  }
  return NS_SUCCESS;
}
