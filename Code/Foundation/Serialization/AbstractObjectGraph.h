#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Enum.h>
#include <Foundation/Types/Uuid.h>
#include <Foundation/Types/Variant.h>

class wdAbstractObjectGraph;

class WD_FOUNDATION_DLL wdAbstractObjectNode
{
public:
  struct Property
  {
    const char* m_szPropertyName;
    wdVariant m_Value;
  };

  wdAbstractObjectNode()
    : m_pOwner(nullptr)
    , m_uiTypeVersion(0)
    , m_szType(nullptr)
    , m_szNodeName(nullptr)
  {
  }

  const wdHybridArray<Property, 16>& GetProperties() const { return m_Properties; }

  void AddProperty(const char* szName, const wdVariant& value);

  void RemoveProperty(const char* szName);

  void ChangeProperty(const char* szName, const wdVariant& value);

  void RenameProperty(const char* szOldName, const char* szNewName);

  void ClearProperties();

  // \brief Inlines a custom variant type. Use to patch properties that have been turned into custom variant type.
  // \sa WD_DEFINE_CUSTOM_VARIANT_TYPE, WD_DECLARE_CUSTOM_VARIANT_TYPE
  wdResult InlineProperty(const char* szName);

  const wdAbstractObjectGraph* GetOwner() const { return m_pOwner; }
  const wdUuid& GetGuid() const { return m_Guid; }
  wdUInt32 GetTypeVersion() const { return m_uiTypeVersion; }
  void SetTypeVersion(wdUInt32 uiTypeVersion) { m_uiTypeVersion = uiTypeVersion; }
  const char* GetType() const { return m_szType; }
  void SetType(const char* szType);

  const Property* FindProperty(const char* szName) const;
  Property* FindProperty(const char* szName);

  const char* GetNodeName() const { return m_szNodeName; }

private:
  friend class wdAbstractObjectGraph;

  wdAbstractObjectGraph* m_pOwner;

  wdUuid m_Guid;
  wdUInt32 m_uiTypeVersion;
  const char* m_szType;
  const char* m_szNodeName;

  wdHybridArray<Property, 16> m_Properties;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdAbstractObjectNode);

struct WD_FOUNDATION_DLL wdAbstractGraphDiffOperation
{
  enum class Op
  {
    NodeAdded,
    NodeRemoved,
    PropertyChanged
  };

  Op m_Operation;
  wdUuid m_Node;            // prop parent or added / deleted node
  wdString m_sProperty;     // prop name or type
  wdUInt32 m_uiTypeVersion; // only used for NodeAdded
  wdVariant m_Value;
};

struct WD_FOUNDATION_DLL wdObjectChangeType
{
  typedef wdInt8 StorageType;

  enum Enum : wdInt8
  {
    NodeAdded,
    NodeRemoved,
    PropertySet,
    PropertyInserted,
    PropertyRemoved,

    Default = NodeAdded
  };
};
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdObjectChangeType);


struct WD_FOUNDATION_DLL wdDiffOperation
{
  wdEnum<wdObjectChangeType> m_Operation;
  wdUuid m_Node;        // owner of m_sProperty
  wdString m_sProperty; // property
  wdVariant m_Index;
  wdVariant m_Value;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdDiffOperation);


class WD_FOUNDATION_DLL wdAbstractObjectGraph
{
public:
  wdAbstractObjectGraph() {}
  ~wdAbstractObjectGraph();

  void Clear();

  using FilterFunction = wdDelegate<bool(const wdAbstractObjectNode*, const wdAbstractObjectNode::Property*)>;
  wdAbstractObjectNode* Clone(wdAbstractObjectGraph& ref_cloneTarget, const wdAbstractObjectNode* pRootNode = nullptr, FilterFunction filter = FilterFunction()) const;

  const char* RegisterString(const char* szString);

  const wdAbstractObjectNode* GetNode(const wdUuid& guid) const;
  wdAbstractObjectNode* GetNode(const wdUuid& guid);

  const wdAbstractObjectNode* GetNodeByName(const char* szName) const;
  wdAbstractObjectNode* GetNodeByName(const char* szName);

  wdAbstractObjectNode* AddNode(const wdUuid& guid, const char* szType, wdUInt32 uiTypeVersion, const char* szNodeName = nullptr);
  void RemoveNode(const wdUuid& guid);

  const wdMap<wdUuid, wdAbstractObjectNode*>& GetAllNodes() const { return m_Nodes; }
  wdMap<wdUuid, wdAbstractObjectNode*>& GetAllNodes() { return m_Nodes; }

  /// \brief Remaps all node guids by adding the given seed, or if bRemapInverse is true, by subtracting it/
  ///   This is mostly used to remap prefab instance graphs to their prefab template graph.
  void ReMapNodeGuids(const wdUuid& seedGuid, bool bRemapInverse = false);

  /// \brief Tries to remap the guids of this graph to those in rhsGraph by walking in both down the hierarchy, starting at root and
  /// rhsRoot.
  ///
  ///  Note that in case of array properties the remapping assumes element indices to be equal
  ///  on both sides which will cause all moves inside the arrays to be lost as there is no way of recovering this information without an
  ///  equality criteria. This function is mostly used to remap a graph from a native object to a graph from wdDocumentObjects to allow
  ///  applying native side changes to the original wdDocumentObject hierarchy using diffs.
  void ReMapNodeGuidsToMatchGraph(wdAbstractObjectNode* pRoot, const wdAbstractObjectGraph& rhsGraph, const wdAbstractObjectNode* pRhsRoot);

  /// \brief Finds everything accessible by the given root node.
  void FindTransitiveHull(const wdUuid& rootGuid, wdSet<wdUuid>& out_reachableNodes) const;
  /// \brief Deletes everything not accessible by the given root node.
  void PruneGraph(const wdUuid& rootGuid);

  /// \brief Allows for a given node to be modified as a native object.
  /// Once the callback exits any changes to the sub-hierarchy of the given root node will be written back to the node objects.
  void ModifyNodeViaNativeCounterpart(wdAbstractObjectNode* pRootNode, wdDelegate<void(void*, const wdRTTI*)> callback);

  /// \brief Allows to copy a node from another graph into this graph.
  wdAbstractObjectNode* CopyNodeIntoGraph(const wdAbstractObjectNode* pNode);

  wdAbstractObjectNode* CopyNodeIntoGraph(const wdAbstractObjectNode* pNode, FilterFunction& ref_filter);

  void CreateDiffWithBaseGraph(const wdAbstractObjectGraph& base, wdDeque<wdAbstractGraphDiffOperation>& out_diffResult) const;

  void ApplyDiff(wdDeque<wdAbstractGraphDiffOperation>& ref_diff);

  void MergeDiffs(const wdDeque<wdAbstractGraphDiffOperation>& lhs, const wdDeque<wdAbstractGraphDiffOperation>& rhs, wdDeque<wdAbstractGraphDiffOperation>& ref_out) const;

private:
  WD_DISALLOW_COPY_AND_ASSIGN(wdAbstractObjectGraph);

  void RemapVariant(wdVariant& value, const wdHashTable<wdUuid, wdUuid>& guidMap);
  void MergeArrays(const wdVariantArray& baseArray, const wdVariantArray& leftArray, const wdVariantArray& rightArray, wdVariantArray& out) const;
  void ReMapNodeGuidsToMatchGraphRecursive(wdHashTable<wdUuid, wdUuid>& guidMap, wdAbstractObjectNode* lhs, const wdAbstractObjectGraph& rhsGraph, const wdAbstractObjectNode* rhs);

  wdSet<wdString> m_Strings;
  wdMap<wdUuid, wdAbstractObjectNode*> m_Nodes;
  wdMap<const char*, wdAbstractObjectNode*, CompareConstChar> m_NodesByName;
};
