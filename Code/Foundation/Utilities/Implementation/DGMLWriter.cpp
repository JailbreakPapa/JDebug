#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Utilities/DGMLWriter.h>

wdDGMLGraph::wdDGMLGraph(wdDGMLGraph::Direction graphDirection /*= LeftToRight*/, wdDGMLGraph::Layout graphLayout /*= Tree*/)
  : m_Direction(graphDirection)
  , m_Layout(graphLayout)
{
}

wdDGMLGraph::NodeId wdDGMLGraph::AddNode(const char* szTitle, const NodeDesc* pDesc)
{
  return AddGroup(szTitle, GroupType::None, pDesc);
}

wdDGMLGraph::NodeId wdDGMLGraph::AddGroup(const char* szTitle, GroupType type, const NodeDesc* pDesc /*= nullptr*/)
{
  wdDGMLGraph::Node& Node = m_Nodes.ExpandAndGetRef();

  Node.m_Title = szTitle;
  Node.m_GroupType = type;

  if (pDesc)
  {
    Node.m_Desc = *pDesc;
  }

  return m_Nodes.GetCount() - 1;
}

void wdDGMLGraph::AddNodeToGroup(NodeId node, NodeId group)
{
  WD_ASSERT_DEBUG(m_Nodes[group].m_GroupType != GroupType::None, "The given group node has not been created as a group node");

  m_Nodes[node].m_ParentGroup = group;
}

wdDGMLGraph::ConnectionId wdDGMLGraph::AddConnection(wdDGMLGraph::NodeId source, wdDGMLGraph::NodeId target, const char* szLabel)
{
  wdDGMLGraph::Connection& connection = m_Connections.ExpandAndGetRef();

  connection.m_Source = source;
  connection.m_Target = target;
  connection.m_sLabel = szLabel;

  return m_Connections.GetCount() - 1;
}

wdDGMLGraph::PropertyId wdDGMLGraph::AddPropertyType(const char* szName)
{
  auto& prop = m_PropertyTypes.ExpandAndGetRef();
  prop.m_Name = szName;
  return m_PropertyTypes.GetCount() - 1;
}

void wdDGMLGraph::AddNodeProperty(NodeId node, PropertyId property, const wdFormatString& fmt)
{
  wdStringBuilder tmp;

  auto& prop = m_Nodes[node].m_Properties.ExpandAndGetRef();
  prop.m_PropertyId = property;
  prop.m_sValue = fmt.GetText(tmp);
}

wdResult wdDGMLGraphWriter::WriteGraphToFile(wdStringView sFileName, const wdDGMLGraph& graph)
{
  wdStringBuilder sGraph;

  // Write to memory object and then to file
  if (WriteGraphToString(sGraph, graph).Succeeded())
  {
    wdStringBuilder sTemp;

    wdFileWriter fileWriter;
    if (!fileWriter.Open(sFileName.GetData(sTemp)).Succeeded())
      return WD_FAILURE;

    fileWriter.WriteBytes(sGraph.GetData(), sGraph.GetElementCount()).IgnoreResult();

    fileWriter.Close();

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdDGMLGraphWriter::WriteGraphToString(wdStringBuilder& ref_sStringBuilder, const wdDGMLGraph& graph)
{
  const char* szDirection = nullptr;
  const char* szLayout = nullptr;

  switch (graph.m_Direction)
  {
    case wdDGMLGraph::Direction::TopToBottom:
      szDirection = "TopToBottom";
      break;
    case wdDGMLGraph::Direction::BottomToTop:
      szDirection = "BottomToTop";
      break;
    case wdDGMLGraph::Direction::LeftToRight:
      szDirection = "LeftToRight";
      break;
    case wdDGMLGraph::Direction::RightToLeft:
      szDirection = "RightToLeft";
      break;
  }

  switch (graph.m_Layout)
  {
    case wdDGMLGraph::Layout::Free:
      szLayout = "None";
      break;
    case wdDGMLGraph::Layout::Tree:
      szLayout = "Sugiyama";
      break;
    case wdDGMLGraph::Layout::DependencyMatrix:
      szLayout = "DependencyMatrix";
      break;
  }

  ref_sStringBuilder.AppendFormat("<DirectedGraph xmlns=\"http://schemas.microsoft.com/vs/2009/dgml\" GraphDirection=\"{0}\" Layout=\"{1}\">\n", szDirection, szLayout);

  // Write out all the properties
  if (!graph.m_PropertyTypes.IsEmpty())
  {
    ref_sStringBuilder.Append("\t<Properties>\n");

    for (wdUInt32 i = 0; i < graph.m_PropertyTypes.GetCount(); ++i)
    {
      const auto& prop = graph.m_PropertyTypes[i];

      ref_sStringBuilder.AppendFormat("\t\t<Property Id=\"P_{0}\" Label=\"{1}\" DataType=\"String\"/>\n", i, prop.m_Name);
    }

    ref_sStringBuilder.Append("\t</Properties>\n");
  }

  // Write out all the nodes
  if (!graph.m_Nodes.IsEmpty())
  {
    wdStringBuilder ColorValue;
    wdStringBuilder PropertiesString;
    wdStringBuilder SanitizedName;
    const char* szGroupString;

    ref_sStringBuilder.Append("\t<Nodes>\n");
    for (wdUInt32 i = 0; i < graph.m_Nodes.GetCount(); ++i)
    {
      const wdDGMLGraph::Node& node = graph.m_Nodes[i];

      SanitizedName = node.m_Title;
      SanitizedName.ReplaceAll("&", "&#038;");
      SanitizedName.ReplaceAll("<", "&lt;");
      SanitizedName.ReplaceAll(">", "&gt;");
      SanitizedName.ReplaceAll("\"", "&quot;");
      SanitizedName.ReplaceAll("'", "&apos;");
      SanitizedName.ReplaceAll("\n", "&#xA;");

      ColorValue = "#FF";
      wdColorGammaUB RGBA(node.m_Desc.m_Color);
      ColorValue.AppendFormat("{0}{1}{2}", wdArgU(RGBA.r, 2, true, 16, true), wdArgU(RGBA.g, 2, true, 16, true), wdArgU(RGBA.b, 2, true, 16, true));

      wdStringBuilder StyleString;
      switch (node.m_Desc.m_Shape)
      {
        case wdDGMLGraph::NodeShape::None:
          StyleString = "Shape=\"None\"";
          break;
        case wdDGMLGraph::NodeShape::Rectangle:
          StyleString = "NodeRadius=\"0\"";
          break;
        case wdDGMLGraph::NodeShape::RoundedRectangle:
          StyleString = "NodeRadius=\"4\"";
          break;
        case wdDGMLGraph::NodeShape::Button:
          StyleString = "";
          break;
      }

      switch (node.m_GroupType)
      {

        case wdDGMLGraph::GroupType::Expanded:
          szGroupString = " Group=\"Expanded\"";
          break;

        case wdDGMLGraph::GroupType::Collapsed:
          szGroupString = " Group=\"Collapsed\"";
          break;

        case wdDGMLGraph::GroupType::None:
        default:
          szGroupString = nullptr;
          break;
      }

      PropertiesString.Clear();
      for (const auto& prop : node.m_Properties)
      {
        PropertiesString.AppendFormat(" {0}=\"{1}\"", graph.m_PropertyTypes[prop.m_PropertyId].m_Name, prop.m_sValue);
      }

      ref_sStringBuilder.AppendFormat("\t\t<Node Id=\"N_{0}\" Label=\"{1}\" Background=\"{2}\" {3}{4}{5}/>\n", i, SanitizedName, ColorValue, StyleString, szGroupString, PropertiesString);
    }
    ref_sStringBuilder.Append("\t</Nodes>\n");
  }

  // Write out the links
  if (!graph.m_Connections.IsEmpty())
  {
    ref_sStringBuilder.Append("\t<Links>\n");
    {
      for (wdUInt32 i = 0; i < graph.m_Connections.GetCount(); ++i)
      {
        ref_sStringBuilder.AppendFormat("\t\t<Link Source=\"N_{0}\" Target=\"N_{1}\" Label=\"{2}\" />\n", graph.m_Connections[i].m_Source, graph.m_Connections[i].m_Target, graph.m_Connections[i].m_sLabel);
      }

      for (wdUInt32 i = 0; i < graph.m_Nodes.GetCount(); ++i)
      {
        const wdDGMLGraph::Node& node = graph.m_Nodes[i];

        if (node.m_ParentGroup != 0xFFFFFFFF)
        {
          ref_sStringBuilder.AppendFormat("\t\t<Link Category=\"Contains\" Source=\"N_{0}\" Target=\"N_{1}\" />\n", node.m_ParentGroup, i);
        }
      }
    }
    ref_sStringBuilder.Append("\t</Links>\n");
  }

  ref_sStringBuilder.Append("</DirectedGraph>\n");

  return WD_SUCCESS;
}

WD_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_DGMLWriter);
