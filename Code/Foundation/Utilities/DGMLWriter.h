#pragma once

#include <Foundation/Math/Color.h>
#include <Foundation/Strings/String.h>

class nsFormatString;

/// \brief This class encapsulates building a DGML compatible graph.
class NS_FOUNDATION_DLL nsDGMLGraph
{
public:
  enum class Direction : nsUInt8
  {
    TopToBottom,
    BottomToTop,
    LeftToRight,
    RightToLeft
  };

  enum class Layout : nsUInt8
  {
    Free,
    Tree,
    DependencyMatrix
  };

  enum class NodeShape : nsUInt8
  {
    None,
    Rectangle,
    RoundedRectangle,
    Button
  };

  enum class GroupType : nsUInt8
  {
    None,
    Expanded,
    Collapsed,
  };

  using NodeId = nsUInt32;
  using PropertyId = nsUInt32;
  using ConnectionId = nsUInt32;

  struct NodeDesc
  {
    nsColor m_Color = nsColor::White;
    NodeShape m_Shape = NodeShape::Rectangle;
  };

  /// \brief Constructor for the graph.
  nsDGMLGraph(Direction graphDirection = Direction::LeftToRight, Layout graphLayout = Layout::Tree);

  /// \brief Adds a node to the graph.
  /// Adds a node to the graph and returns the node id which can be used to reference the node later to add connections etc.
  NodeId AddNode(nsStringView sTitle, const NodeDesc* pDesc = nullptr);

  /// \brief Adds a DGML node that can act as a group for other nodes
  NodeId AddGroup(nsStringView sTitle, GroupType type, const NodeDesc* pDesc = nullptr);

  /// \brief Inserts a node into an existing group node.
  void AddNodeToGroup(NodeId node, NodeId group);

  /// \brief Adds a directed connection to the graph (an arrow pointing from source to target node).
  ConnectionId AddConnection(NodeId source, NodeId target, nsStringView sLabel = {});

  /// \brief Adds a property type. All properties currently use the data type 'string'
  PropertyId AddPropertyType(nsStringView sName);

  /// \brief Adds a property of the specified type with the given value to a node
  void AddNodeProperty(NodeId node, PropertyId property, const nsFormatString& fmt);

protected:
  friend class nsDGMLGraphWriter;

  struct Connection
  {
    NodeId m_Source;
    NodeId m_Target;
    nsString m_sLabel;
  };

  struct PropertyType
  {
    nsString m_Name;
  };

  struct PropertyValue
  {
    PropertyId m_PropertyId;
    nsString m_sValue;
  };

  struct Node
  {
    nsString m_Title;
    GroupType m_GroupType = GroupType::None;
    NodeId m_ParentGroup = 0xFFFFFFFF;
    NodeDesc m_Desc;
    nsDynamicArray<PropertyValue> m_Properties;
  };

  nsHybridArray<Node, 16> m_Nodes;

  nsHybridArray<Connection, 32> m_Connections;

  nsHybridArray<PropertyType, 16> m_PropertyTypes;

  Direction m_Direction;

  Layout m_Layout;
};

/// \brief This class encapsulates the output of DGML compatible graphs to files and streams.
class NS_FOUNDATION_DLL nsDGMLGraphWriter
{
public:
  /// \brief Helper method to write the graph to a file.
  static nsResult WriteGraphToFile(nsStringView sFileName, const nsDGMLGraph& graph);

  /// \brief Writes the graph as a DGML formatted document to the given string builder.
  static nsResult WriteGraphToString(nsStringBuilder& ref_sStringBuilder, const nsDGMLGraph& graph);
};
