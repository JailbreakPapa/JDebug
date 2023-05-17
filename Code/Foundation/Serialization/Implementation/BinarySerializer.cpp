#include <Foundation/FoundationPCH.h>

#include <Foundation/Serialization/BinarySerializer.h>
#include <Foundation/Serialization/GraphVersioning.h>

enum wdBinarySerializerVersion : wdUInt32
{
  InvalidVersion = 0,
  Version1,
  // << insert new versions here >>

  ENUM_COUNT,
  CurrentVersion = ENUM_COUNT - 1 // automatically the highest version number
};

static void WriteGraph(const wdAbstractObjectGraph* pGraph, wdStreamWriter& inout_stream)
{
  const auto& Nodes = pGraph->GetAllNodes();

  wdUInt32 uiNodes = Nodes.GetCount();
  inout_stream << uiNodes;
  for (auto itNode = Nodes.GetIterator(); itNode.IsValid(); ++itNode)
  {
    const auto& node = *itNode.Value();
    inout_stream << node.GetGuid();
    inout_stream << node.GetType();
    inout_stream << node.GetTypeVersion();
    inout_stream << node.GetNodeName();

    const wdHybridArray<wdAbstractObjectNode::Property, 16>& properties = node.GetProperties();
    wdUInt32 uiProps = properties.GetCount();
    inout_stream << uiProps;
    for (const wdAbstractObjectNode::Property& prop : properties)
    {
      inout_stream << prop.m_szPropertyName;
      inout_stream << prop.m_Value;
    }
  }
}

void wdAbstractGraphBinarySerializer::Write(wdStreamWriter& inout_stream, const wdAbstractObjectGraph* pGraph, const wdAbstractObjectGraph* pTypesGraph)
{
  wdUInt32 uiVersion = wdBinarySerializerVersion::CurrentVersion;
  inout_stream << uiVersion;

  WriteGraph(pGraph, inout_stream);
  if (pTypesGraph)
  {
    WriteGraph(pTypesGraph, inout_stream);
  }
}

static void ReadGraph(wdStreamReader& inout_stream, wdAbstractObjectGraph* pGraph)
{
  wdUInt32 uiNodes = 0;
  inout_stream >> uiNodes;
  for (wdUInt32 uiNodeIdx = 0; uiNodeIdx < uiNodes; uiNodeIdx++)
  {
    wdUuid guid;
    wdUInt32 uiTypeVersion;
    wdStringBuilder sType;
    wdStringBuilder sNodeName;
    inout_stream >> guid;
    inout_stream >> sType;
    inout_stream >> uiTypeVersion;
    inout_stream >> sNodeName;
    wdAbstractObjectNode* pNode = pGraph->AddNode(guid, sType, uiTypeVersion, sNodeName);
    wdUInt32 uiProps = 0;
    inout_stream >> uiProps;
    for (wdUInt32 propIdx = 0; propIdx < uiProps; ++propIdx)
    {
      wdStringBuilder sPropName;
      wdVariant value;
      inout_stream >> sPropName;
      inout_stream >> value;
      pNode->AddProperty(sPropName, value);
    }
  }
}

void wdAbstractGraphBinarySerializer::Read(
  wdStreamReader& inout_stream, wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph, bool bApplyPatches)
{
  wdUInt32 uiVersion = 0;
  inout_stream >> uiVersion;
  if (uiVersion != wdBinarySerializerVersion::CurrentVersion)
  {
    WD_REPORT_FAILURE(
      "Binary serializer version {0} does not match expected version {1}, re-export file.", uiVersion, wdBinarySerializerVersion::CurrentVersion);
    return;
  }
  ReadGraph(inout_stream, pGraph);
  if (pTypesGraph)
  {
    ReadGraph(inout_stream, pTypesGraph);
  }

  if (bApplyPatches)
  {
    if (pTypesGraph)
      wdGraphVersioning::GetSingleton()->PatchGraph(pTypesGraph);
    wdGraphVersioning::GetSingleton()->PatchGraph(pGraph, pTypesGraph);
  }
}

WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_BinarySerializer);
