#include <Foundation/FoundationPCH.h>

#include <Foundation/Serialization/BinarySerializer.h>
#include <Foundation/Serialization/GraphVersioning.h>

enum nsBinarySerializerVersion : nsUInt32
{
  InvalidVersion = 0,
  Version1,
  // << insert new versions here >>

  ENUM_COUNT,
  CurrentVersion = ENUM_COUNT - 1 // automatically the highest version number
};

static void WriteGraph(const nsAbstractObjectGraph* pGraph, nsStreamWriter& inout_stream)
{
  const auto& Nodes = pGraph->GetAllNodes();

  nsUInt32 uiNodes = Nodes.GetCount();
  inout_stream << uiNodes;
  for (auto itNode = Nodes.GetIterator(); itNode.IsValid(); ++itNode)
  {
    const auto& node = *itNode.Value();
    inout_stream << node.GetGuid();
    inout_stream << node.GetType();
    inout_stream << node.GetTypeVersion();
    inout_stream << node.GetNodeName();

    const nsHybridArray<nsAbstractObjectNode::Property, 16>& properties = node.GetProperties();
    nsUInt32 uiProps = properties.GetCount();
    inout_stream << uiProps;
    for (const nsAbstractObjectNode::Property& prop : properties)
    {
      inout_stream << prop.m_sPropertyName;
      inout_stream << prop.m_Value;
    }
  }
}

void nsAbstractGraphBinarySerializer::Write(nsStreamWriter& inout_stream, const nsAbstractObjectGraph* pGraph, const nsAbstractObjectGraph* pTypesGraph)
{
  nsUInt32 uiVersion = nsBinarySerializerVersion::CurrentVersion;
  inout_stream << uiVersion;

  WriteGraph(pGraph, inout_stream);
  if (pTypesGraph)
  {
    WriteGraph(pTypesGraph, inout_stream);
  }
}

static void ReadGraph(nsStreamReader& inout_stream, nsAbstractObjectGraph* pGraph)
{
  nsUInt32 uiNodes = 0;
  inout_stream >> uiNodes;
  for (nsUInt32 uiNodeIdx = 0; uiNodeIdx < uiNodes; uiNodeIdx++)
  {
    nsUuid guid;
    nsUInt32 uiTypeVersion;
    nsStringBuilder sType;
    nsStringBuilder sNodeName;
    inout_stream >> guid;
    inout_stream >> sType;
    inout_stream >> uiTypeVersion;
    inout_stream >> sNodeName;
    nsAbstractObjectNode* pNode = pGraph->AddNode(guid, sType, uiTypeVersion, sNodeName);
    nsUInt32 uiProps = 0;
    inout_stream >> uiProps;
    for (nsUInt32 propIdx = 0; propIdx < uiProps; ++propIdx)
    {
      nsStringBuilder sPropName;
      nsVariant value;
      inout_stream >> sPropName;
      inout_stream >> value;
      pNode->AddProperty(sPropName, value);
    }
  }
}

void nsAbstractGraphBinarySerializer::Read(
  nsStreamReader& inout_stream, nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph, bool bApplyPatches)
{
  nsUInt32 uiVersion = 0;
  inout_stream >> uiVersion;
  if (uiVersion != nsBinarySerializerVersion::CurrentVersion)
  {
    NS_REPORT_FAILURE(
      "Binary serializer version {0} does not match expected version {1}, re-export file.", uiVersion, nsBinarySerializerVersion::CurrentVersion);
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
      nsGraphVersioning::GetSingleton()->PatchGraph(pTypesGraph);
    nsGraphVersioning::GetSingleton()->PatchGraph(pGraph, pTypesGraph);
  }
}
