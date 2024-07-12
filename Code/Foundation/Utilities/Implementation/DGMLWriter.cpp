#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Utilities/DGMLWriter.h>

nsDGMLGraph::nsDGMLGraph(nsDGMLGraph::Direction graphDirection /*= LeftToRight*/, nsDGMLGraph::Layout graphLayout /*= Tree*/)
  : m_Direction(graphDirection)
  , m_Layout(graphLayout)
{
}

nsDGMLGraph::NodeId nsDGMLGraph::AddNode(nsStringView sTitle, const NodeDesc* pDesc)
{
  return AddGroup(sTitle, GroupType::None, pDesc);
}

nsDGMLGraph::NodeId nsDGMLGraph::AddGroup(nsStringView sTitle, GroupType type, const NodeDesc* pDesc /*= nullptr*/)
{
  nsDGMLGraph::Node& Node = m_Nodes.ExpandAndGetRef();

  Node.m_Title = sTitle;
  Node.m_GroupType = type;

  if (pDesc)
  {
    Node.m_Desc = *pDesc;
  }

  return m_Nodes.GetCount() - 1;
}

void nsDGMLGraph::AddNodeToGroup(NodeId node, NodeId group)
{
  NS_ASSERT_DEBUG(m_Nodes[group].m_GroupType != GroupType::None, "The given group node has not been created as a group node");

  m_Nodes[node].m_ParentGroup = group;
}

nsDGMLGraph::ConnectionId nsDGMLGraph::AddConnection(nsDGMLGraph::NodeId source, nsDGMLGraph::NodeId target, nsStringView sLabel)
{
  nsDGMLGraph::Connection& connection = m_Connections.ExpandAndGetRef();

  connection.m_Source = source;
  connection.m_Target = target;
  connection.m_sLabel = sLabel;

  return m_Connections.GetCount() - 1;
}

nsDGMLGraph::PropertyId nsDGMLGraph::AddPropertyType(nsStringView sName)
{
  auto& prop = m_PropertyTypes.ExpandAndGetRef();
  prop.m_Name = sName;
  return m_PropertyTypes.GetCount() - 1;
}

void nsDGMLGraph::AddNodeProperty(NodeId node, PropertyId property, const nsFormatString& fmt)
{
  nsStringBuilder tmp;

  auto& prop = m_Nodes[node].m_Properties.ExpandAndGetRef();
  prop.m_PropertyId = property;
  prop.m_sValue = fmt.GetText(tmp);
}

nsResult nsDGMLGraphWriter::WriteGraphToFile(nsStringView sFileName, const nsDGMLGraph& graph)
{
  nsStringBuilder sGraph;

  // Write to memory object and then to file
  if (WriteGraphToString(sGraph, graph).Succeeded())
  {
    nsStringBuilder sTemp;

    nsFileWriter fileWriter;
    if (!fileWriter.Open(sFileName.GetData(sTemp)).Succeeded())
      return NS_FAILURE;

    fileWriter.WriteBytes(sGraph.GetData(), sGraph.GetElementCount()).IgnoreResult();

    fileWriter.Close();

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsDGMLGraphWriter::WriteGraphToString(nsStringBuilder& ref_sStringBuilder, const nsDGMLGraph& graph)
{
  const char* szDirection = nullptr;
  const char* szLayout = nullptr;

  switch (graph.m_Direction)
  {
    case nsDGMLGraph::Direction::TopToBottom:
      szDirection = "TopToBottom";
      break;
    case nsDGMLGraph::Direction::BottomToTop:
      szDirection = "BottomToTop";
      break;
    case nsDGMLGraph::Direction::LeftToRight:
      szDirection = "LeftToRight";
      break;
    case nsDGMLGraph::Direction::RightToLeft:
      szDirection = "RightToLeft";
      break;
  }

  switch (graph.m_Layout)
  {
    case nsDGMLGraph::Layout::Free:
      szLayout = "None";
      break;
    case nsDGMLGraph::Layout::Tree:
      szLayout = "Sugiyama";
      break;
    case nsDGMLGraph::Layout::DependencyMatrix:
      szLayout = "DependencyMatrix";
      break;
  }

  ref_sStringBuilder.AppendFormat("<DirectedGraph xmlns=\"http://schemas.microsoft.com/vs/2009/dgml\" GraphDirection=\"{0}\" Layout=\"{1}\">\n", szDirection, szLayout);

  // Write out all the properties
  if (!graph.m_PropertyTypes.IsEmpty())
  {
    ref_sStringBuilder.Append("\t<Properties>\n");

    for (nsUInt32 i = 0; i < graph.m_PropertyTypes.GetCount(); ++i)
    {
      const auto& prop = graph.m_PropertyTypes[i];

      ref_sStringBuilder.AppendFormat("\t\t<Property Id=\"P_{0}\" Label=\"{1}\" DataType=\"String\"/>\n", i, prop.m_Name);
    }

    ref_sStringBuilder.Append("\t</Properties>\n");
  }

  // Write out all the nodes
  if (!graph.m_Nodes.IsEmpty())
  {
    nsStringBuilder ColorValue;
    nsStringBuilder PropertiesString;
    nsStringBuilder SanitizedName;
    const char* szGroupString;

    ref_sStringBuilder.Append("\t<Nodes>\n");
    for (nsUInt32 i = 0; i < graph.m_Nodes.GetCount(); ++i)
    {
      const nsDGMLGraph::Node& node = graph.m_Nodes[i];

      SanitizedName = node.m_Title;
      SanitizedName.ReplaceAll("&", "&#038;");
      SanitizedName.ReplaceAll("<", "&lt;");
      SanitizedName.ReplaceAll(">", "&gt;");
      SanitizedName.ReplaceAll("\"", "&quot;");
      SanitizedName.ReplaceAll("'", "&apos;");
      SanitizedName.ReplaceAll("\n", "&#xA;");

      ColorValue = "#FF";
      nsColorGammaUB RGBA(node.m_Desc.m_Color);
      ColorValue.AppendFormat("{0}{1}{2}", nsArgU(RGBA.r, 2, true, 16, true), nsArgU(RGBA.g, 2, true, 16, true), nsArgU(RGBA.b, 2, true, 16, true));

      nsStringBuilder StyleString;
      switch (node.m_Desc.m_Shape)
      {
        case nsDGMLGraph::NodeShape::None:
          StyleString = "Shape=\"None\"";
          break;
        case nsDGMLGraph::NodeShape::Rectangle:
          StyleString = "NodeRadius=\"0\"";
          break;
        case nsDGMLGraph::NodeShape::RoundedRectangle:
          StyleString = "NodeRadius=\"4\"";
          break;
        case nsDGMLGraph::NodeShape::Button:
          StyleString = "";
          break;
      }

      switch (node.m_GroupType)
      {

        case nsDGMLGraph::GroupType::Expanded:
          szGroupString = " Group=\"Expanded\"";
          break;

        case nsDGMLGraph::GroupType::Collapsed:
          szGroupString = " Group=\"Collapsed\"";
          break;

        case nsDGMLGraph::GroupType::None:
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
      for (nsUInt32 i = 0; i < graph.m_Connections.GetCount(); ++i)
      {
        ref_sStringBuilder.AppendFormat("\t\t<Link Source=\"N_{0}\" Target=\"N_{1}\" Label=\"{2}\" />\n", graph.m_Connections[i].m_Source, graph.m_Connections[i].m_Target, graph.m_Connections[i].m_sLabel);
      }

      for (nsUInt32 i = 0; i < graph.m_Nodes.GetCount(); ++i)
      {
        const nsDGMLGraph::Node& node = graph.m_Nodes[i];

        if (node.m_ParentGroup != 0xFFFFFFFF)
        {
          ref_sStringBuilder.AppendFormat("\t\t<Link Category=\"Contains\" Source=\"N_{0}\" Target=\"N_{1}\" />\n", node.m_ParentGroup, i);
        }
      }
    }
    ref_sStringBuilder.Append("\t</Links>\n");
  }

  ref_sStringBuilder.Append("</DirectedGraph>\n");

  return NS_SUCCESS;
}
