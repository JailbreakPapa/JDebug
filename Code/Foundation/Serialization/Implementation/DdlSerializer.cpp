#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/GraphVersioning.h>

namespace
{
  wdSerializedBlock* FindBlock(wdHybridArray<wdSerializedBlock, 3>& ref_blocks, const char* szName)
  {
    for (auto& block : ref_blocks)
    {
      if (block.m_Name == szName)
      {
        return &block;
      }
    }
    return nullptr;
  }

  wdSerializedBlock* FindHeaderBlock(wdHybridArray<wdSerializedBlock, 3>& ref_blocks, wdInt32& out_iVersion)
  {
    wdStringBuilder sHeaderName = "HeaderV";
    out_iVersion = 0;
    for (auto& block : ref_blocks)
    {
      if (block.m_Name.StartsWith(sHeaderName))
      {
        wdResult res = wdConversionUtils::StringToInt(block.m_Name.GetData() + sHeaderName.GetElementCount(), out_iVersion);
        if (res.Failed())
        {
          wdLog::Error("Failed to parse version from header name '{0}'", block.m_Name);
        }
        return &block;
      }
    }
    return nullptr;
  }

  wdSerializedBlock* GetOrCreateBlock(wdHybridArray<wdSerializedBlock, 3>& ref_blocks, const char* szName)
  {
    wdSerializedBlock* pBlock = FindBlock(ref_blocks, szName);
    if (!pBlock)
    {
      pBlock = &ref_blocks.ExpandAndGetRef();
      pBlock->m_Name = szName;
    }
    if (!pBlock->m_Graph)
    {
      pBlock->m_Graph = WD_DEFAULT_NEW(wdAbstractObjectGraph);
    }
    return pBlock;
  }
} // namespace

static void WriteGraph(wdOpenDdlWriter& ref_writer, const wdAbstractObjectGraph* pGraph, const char* szName)
{
  wdMap<const char*, const wdVariant*, CompareConstChar> SortedProperties;

  ref_writer.BeginObject(szName);

  const auto& Nodes = pGraph->GetAllNodes();
  for (auto itNode = Nodes.GetIterator(); itNode.IsValid(); ++itNode)
  {
    const auto& node = *itNode.Value();

    ref_writer.BeginObject("o");

    {

      wdOpenDdlUtils::StoreUuid(ref_writer, node.GetGuid(), "id");
      wdOpenDdlUtils::StoreString(ref_writer, node.GetType(), "t");
      wdOpenDdlUtils::StoreUInt32(ref_writer, node.GetTypeVersion(), "v");

      if (!wdStringUtils::IsNullOrEmpty(node.GetNodeName()))
        wdOpenDdlUtils::StoreString(ref_writer, node.GetNodeName(), "n");

      ref_writer.BeginObject("p");
      {
        for (const auto& prop : node.GetProperties())
          SortedProperties[prop.m_szPropertyName] = &prop.m_Value;

        for (auto it = SortedProperties.GetIterator(); it.IsValid(); ++it)
        {
          wdOpenDdlUtils::StoreVariant(ref_writer, *it.Value(), it.Key());
        }

        SortedProperties.Clear();
      }
      ref_writer.EndObject();
    }
    ref_writer.EndObject();
  }

  ref_writer.EndObject();
}

void wdAbstractGraphDdlSerializer::Write(wdStreamWriter& inout_stream, const wdAbstractObjectGraph* pGraph, const wdAbstractObjectGraph* pTypesGraph,
  bool bCompactMmode, wdOpenDdlWriter::TypeStringMode typeMode)
{
  wdOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(bCompactMmode);
  writer.SetFloatPrecisionMode(wdOpenDdlWriter::FloatPrecisionMode::Exact);
  writer.SetPrimitiveTypeStringMode(typeMode);

  if (typeMode != wdOpenDdlWriter::TypeStringMode::Compliant)
    writer.SetIndentation(-1);

  Write(writer, pGraph, pTypesGraph);
}


void wdAbstractGraphDdlSerializer::Write(
  wdOpenDdlWriter& ref_writer, const wdAbstractObjectGraph* pGraph, const wdAbstractObjectGraph* pTypesGraph /*= nullptr*/)
{
  WriteGraph(ref_writer, pGraph, "Objects");
  if (pTypesGraph)
  {
    WriteGraph(ref_writer, pTypesGraph, "Types");
  }
}

static void ReadGraph(wdAbstractObjectGraph* pGraph, const wdOpenDdlReaderElement* pRoot)
{
  wdStringBuilder tmp, tmp2;
  wdVariant varTmp;

  for (const wdOpenDdlReaderElement* pObject = pRoot->GetFirstChild(); pObject != nullptr; pObject = pObject->GetSibling())
  {
    const wdOpenDdlReaderElement* pGuid = pObject->FindChildOfType(wdOpenDdlPrimitiveType::Custom, "id");
    const wdOpenDdlReaderElement* pType = pObject->FindChildOfType(wdOpenDdlPrimitiveType::String, "t");
    const wdOpenDdlReaderElement* pTypeVersion = pObject->FindChildOfType(wdOpenDdlPrimitiveType::UInt32, "v");
    const wdOpenDdlReaderElement* pName = pObject->FindChildOfType(wdOpenDdlPrimitiveType::String, "n");
    const wdOpenDdlReaderElement* pProps = pObject->FindChildOfType("p");

    if (pGuid == nullptr || pType == nullptr || pProps == nullptr)
    {
      WD_REPORT_FAILURE("Object contains invalid elements");
      continue;
    }

    wdUuid guid;
    if (wdOpenDdlUtils::ConvertToUuid(pGuid, guid).Failed())
    {
      WD_REPORT_FAILURE("Object has an invalid guid");
      continue;
    }

    tmp = pType->GetPrimitivesString()[0];

    if (pName)
      tmp2 = pName->GetPrimitivesString()[0];
    else
      tmp2.Clear();

    wdUInt32 uiTypeVersion = 0;
    if (pTypeVersion)
    {
      uiTypeVersion = pTypeVersion->GetPrimitivesUInt32()[0];
    }

    auto* pNode = pGraph->AddNode(guid, tmp, uiTypeVersion, tmp2);

    for (const wdOpenDdlReaderElement* pProp = pProps->GetFirstChild(); pProp != nullptr; pProp = pProp->GetSibling())
    {
      if (!pProp->HasName())
        continue;

      if (wdOpenDdlUtils::ConvertToVariant(pProp, varTmp).Failed())
        continue;

      pNode->AddProperty(pProp->GetName(), varTmp);
    }
  }
}

wdResult wdAbstractGraphDdlSerializer::Read(
  wdStreamReader& inout_stream, wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph, bool bApplyPatches)
{
  wdOpenDdlReader reader;
  if (reader.ParseDocument(inout_stream, 0, wdLog::GetThreadLocalLogSystem()).Failed())
  {
    wdLog::Error("Failed to parse DDL graph");
    return WD_FAILURE;
  }

  return Read(reader.GetRootElement(), pGraph, pTypesGraph, bApplyPatches);
}


wdResult wdAbstractGraphDdlSerializer::Read(const wdOpenDdlReaderElement* pRootElement, wdAbstractObjectGraph* pGraph,
  wdAbstractObjectGraph* pTypesGraph /*= nullptr*/, bool bApplyPatches /*= true*/)
{
  const wdOpenDdlReaderElement* pObjects = pRootElement->FindChildOfType("Objects");
  if (pObjects != nullptr)
  {
    ReadGraph(pGraph, pObjects);
  }
  else
  {
    wdLog::Error("DDL graph does not contain an 'Objects' root object");
    return WD_FAILURE;
  }

  wdAbstractObjectGraph* pTempTypesGraph = pTypesGraph;
  if (pTempTypesGraph == nullptr)
  {
    pTempTypesGraph = WD_DEFAULT_NEW(wdAbstractObjectGraph);
  }
  const wdOpenDdlReaderElement* pTypes = pRootElement->FindChildOfType("Types");
  if (pTypes != nullptr)
  {
    ReadGraph(pTempTypesGraph, pTypes);
  }

  if (bApplyPatches)
  {
    if (pTempTypesGraph)
      wdGraphVersioning::GetSingleton()->PatchGraph(pTempTypesGraph);
    wdGraphVersioning::GetSingleton()->PatchGraph(pGraph, pTempTypesGraph);
  }

  if (pTypesGraph == nullptr)
    WD_DEFAULT_DELETE(pTempTypesGraph);

  return WD_SUCCESS;
}

wdResult wdAbstractGraphDdlSerializer::ReadBlocks(wdStreamReader& stream, wdHybridArray<wdSerializedBlock, 3>& blocks)
{
  wdOpenDdlReader reader;
  if (reader.ParseDocument(stream, 0, wdLog::GetThreadLocalLogSystem()).Failed())
  {
    wdLog::Error("Failed to parse DDL graph");
    return WD_FAILURE;
  }

  const wdOpenDdlReaderElement* pRoot = reader.GetRootElement();
  for (const wdOpenDdlReaderElement* pChild = pRoot->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
  {
    wdSerializedBlock* pBlock = GetOrCreateBlock(blocks, pChild->GetCustomType());
    ReadGraph(pBlock->m_Graph.Borrow(), pChild);
  }
  return WD_SUCCESS;
}

#define WD_DOCUMENT_VERSION 2

void wdAbstractGraphDdlSerializer::WriteDocument(wdStreamWriter& inout_stream, const wdAbstractObjectGraph* pHeader, const wdAbstractObjectGraph* pGraph,
  const wdAbstractObjectGraph* pTypes, bool bCompactMode, wdOpenDdlWriter::TypeStringMode typeMode)
{
  wdOpenDdlWriter writer;
  writer.SetOutputStream(&inout_stream);
  writer.SetCompactMode(bCompactMode);
  writer.SetFloatPrecisionMode(wdOpenDdlWriter::FloatPrecisionMode::Exact);
  writer.SetPrimitiveTypeStringMode(typeMode);

  if (typeMode != wdOpenDdlWriter::TypeStringMode::Compliant)
    writer.SetIndentation(-1);

  wdStringBuilder sHeaderVersion;
  sHeaderVersion.Format("HeaderV{0}", (int)WD_DOCUMENT_VERSION);
  WriteGraph(writer, pHeader, sHeaderVersion);
  WriteGraph(writer, pGraph, "Objects");
  WriteGraph(writer, pTypes, "Types");
}

wdResult wdAbstractGraphDdlSerializer::ReadDocument(wdStreamReader& inout_stream, wdUniquePtr<wdAbstractObjectGraph>& ref_pHeader,
  wdUniquePtr<wdAbstractObjectGraph>& ref_pGraph, wdUniquePtr<wdAbstractObjectGraph>& ref_pTypes, bool bApplyPatches)
{
  wdHybridArray<wdSerializedBlock, 3> blocks;
  if (ReadBlocks(inout_stream, blocks).Failed())
  {
    return WD_FAILURE;
  }

  wdInt32 iVersion = 2;
  wdSerializedBlock* pHB = FindHeaderBlock(blocks, iVersion);
  wdSerializedBlock* pOB = FindBlock(blocks, "Objects");
  wdSerializedBlock* pTB = FindBlock(blocks, "Types");
  if (!pOB)
  {
    wdLog::Error("No 'Objects' block in document");
    return WD_FAILURE;
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
    wdStringBuilder sHeaderVersion;
    sHeaderVersion.Format("HeaderV{0}", iVersion);
    pHB = GetOrCreateBlock(blocks, sHeaderVersion);
    wdAbstractObjectGraph& graph = *pOB->m_Graph.Borrow();
    if (auto* pHeaderNode = graph.GetNodeByName("Header"))
    {
      wdAbstractObjectGraph& headerGraph = *pHB->m_Graph.Borrow();
      /*auto* pNewHeaderNode =*/headerGraph.CopyNodeIntoGraph(pHeaderNode);
      // pNewHeaderNode->AddProperty("DocVersion", iVersion);
      graph.RemoveNode(pHeaderNode->GetGuid());
    }
  }

  if (bApplyPatches && pTB)
  {
    wdGraphVersioning::GetSingleton()->PatchGraph(pTB->m_Graph.Borrow());
    wdGraphVersioning::GetSingleton()->PatchGraph(pHB->m_Graph.Borrow(), pTB->m_Graph.Borrow());
    wdGraphVersioning::GetSingleton()->PatchGraph(pOB->m_Graph.Borrow(), pTB->m_Graph.Borrow());
  }

  ref_pHeader = std::move(pHB->m_Graph);
  ref_pGraph = std::move(pOB->m_Graph);
  if (pTB)
  {
    ref_pTypes = std::move(pTB->m_Graph);
  }

  return WD_SUCCESS;
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
class HeaderReader : public wdOpenDdlReader
{
public:
  HeaderReader() {}

  bool m_bHasHeader = false;
  wdInt32 m_iDepth = 0;

  virtual void OnBeginObject(const char* szType, const char* szName, bool bGlobalName) override
  {
    //////////////////////////////////////////////////////////////////////////
    // New document format has header block.
    if (m_iDepth == 0 && wdStringUtils::StartsWith(szType, "HeaderV"))
    {
      m_bHasHeader = true;
    }
    if (m_bHasHeader)
    {
      ++m_iDepth;
      wdOpenDdlReader::OnBeginObject(szType, szName, bGlobalName);
      return;
    }

    //////////////////////////////////////////////////////////////////////////
    // Old header is stored in the object block.
    // not yet entered the "Objects" group
    if (m_iDepth == 0 && wdStringUtils::IsEqual(szType, "Objects"))
    {
      ++m_iDepth;

      wdOpenDdlReader::OnBeginObject(szType, szName, bGlobalName);
      return;
    }

    // not yet entered the "AssetInfo" group, but inside "Objects"
    if (m_iDepth == 1 && wdStringUtils::IsEqual(szType, "AssetInfo"))
    {
      ++m_iDepth;

      wdOpenDdlReader::OnBeginObject(szType, szName, bGlobalName);
      return;
    }

    // inside "AssetInfo"
    if (m_iDepth > 1)
    {
      ++m_iDepth;
      wdOpenDdlReader::OnBeginObject(szType, szName, bGlobalName);
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
    wdOpenDdlReader::OnEndObject();
  }
};

wdResult wdAbstractGraphDdlSerializer::ReadHeader(wdStreamReader& inout_stream, wdAbstractObjectGraph* pGraph)
{
  HeaderReader reader;
  if (reader.ParseDocument(inout_stream, 0, wdLog::GetThreadLocalLogSystem()).Failed())
  {
    WD_REPORT_FAILURE("Failed to parse DDL graph");
    return WD_FAILURE;
  }

  const wdOpenDdlReaderElement* pObjects = nullptr;
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
    WD_REPORT_FAILURE("DDL graph does not contain an 'Objects' root object");
    return WD_FAILURE;
  }
  return WD_SUCCESS;
}



WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_DdlSerializer);
