#pragma once

#include <Foundation/Math/Color.h>
#include <Foundation/Strings/String.h>

class wdFormatString;

/// \brief This class encapsulates building a DGML compatible graph.
class WD_FOUNDATION_DLL wdDGMLGraph
{
public:
  enum class Direction : wdUInt8
  {
    TopToBottom,
    BottomToTop,
    LeftToRight,
    RightToLeft
  };

  enum class Layout : wdUInt8
  {
    Free,
    Tree,
    DependencyMatrix
  };

  enum class NodeShape : wdUInt8
  {
    None,
    Rectangle,
    RoundedRectangle,
    Button
  };

  enum class GroupType : wdUInt8
  {
    None,
    Expanded,
    Collapsed,
  };

  typedef wdUInt32 NodeId;
  typedef wdUInt32 PropertyId;
  typedef wdUInt32 ConnectionId;

  struct NodeDesc
  {
    wdColor m_Color = wdColor::White;
    NodeShape m_Shape = NodeShape::Rectangle;
  };

  /// \brief Constructor for the graph.
  wdDGMLGraph(Direction graphDirection = Direction::LeftToRight, Layout graphLayout = Layout::Tree);

  /// \brief Adds a node to the graph.
  /// Adds a node to the graph and returns the node id which can be used to reference the node later to add connections etc.
  NodeId AddNode(const char* szTitle, const NodeDesc* pDesc = nullptr);

  /// \brief Adds a DGML node that can act as a group for other nodes
  NodeId AddGroup(const char* szTitle, GroupType type, const NodeDesc* pDesc = nullptr);

  /// \brief Inserts a node into an existing group node.
  void AddNodeToGroup(NodeId node, NodeId group);

  /// \brief Adds a directed connection to the graph (an arrow pointing from source to target node).
  ConnectionId AddConnection(NodeId source, NodeId target, const char* szLabel = nullptr);

  /// \brief Adds a property type. All properties currently use the data type 'string'
  PropertyId AddPropertyType(const char* szName);

  /// \brief Adds a property of the specified type with the given value to a node
  void AddNodeProperty(NodeId node, PropertyId property, const wdFormatString& fmt);

protected:
  friend class wdDGMLGraphWriter;

  struct Connection
  {
    NodeId m_Source;
    NodeId m_Target;
    wdString m_sLabel;
  };

  struct PropertyType
  {
    wdString m_Name;
  };

  struct PropertyValue
  {
    PropertyId m_PropertyId;
    wdString m_sValue;
  };

  struct Node
  {
    wdString m_Title;
    GroupType m_GroupType = GroupType::None;
    NodeId m_ParentGroup = 0xFFFFFFFF;
    NodeDesc m_Desc;
    wdDynamicArray<PropertyValue> m_Properties;
  };

  wdHybridArray<Node, 16> m_Nodes;

  wdHybridArray<Connection, 32> m_Connections;

  wdHybridArray<PropertyType, 16> m_PropertyTypes;

  Direction m_Direction;

  Layout m_Layout;
};

/// \brief This class encapsulates the output of DGML compatible graphs to files and streams.
class WD_FOUNDATION_DLL wdDGMLGraphWriter
{
public:
  /// \brief Helper method to write the graph to a file.
  static wdResult WriteGraphToFile(wdStringView sFileName, const wdDGMLGraph& graph);

  /// \brief Writes the graph as a DGML formatted document to the given string builder.
  static wdResult WriteGraphToString(wdStringBuilder& ref_sStringBuilder, const wdDGMLGraph& graph);
};
