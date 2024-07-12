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

class nsAbstractObjectGraph;

class NS_FOUNDATION_DLL nsAbstractObjectNode
{
public:
  struct Property
  {
    nsStringView m_sPropertyName;
    nsVariant m_Value;
  };

  nsAbstractObjectNode() = default;

  const nsHybridArray<Property, 16>& GetProperties() const { return m_Properties; }

  void AddProperty(nsStringView sName, const nsVariant& value);

  void RemoveProperty(nsStringView sName);

  void ChangeProperty(nsStringView sName, const nsVariant& value);

  void RenameProperty(nsStringView sOldName, nsStringView sNewName);

  void ClearProperties();

  // \brief Inlines a custom variant type. Use to patch properties that have been turned into custom variant type.
  // \sa NS_DEFINE_CUSTOM_VARIANT_TYPE, NS_DECLARE_CUSTOM_VARIANT_TYPE
  nsResult InlineProperty(nsStringView sName);

  const nsAbstractObjectGraph* GetOwner() const { return m_pOwner; }
  const nsUuid& GetGuid() const { return m_Guid; }
  nsUInt32 GetTypeVersion() const { return m_uiTypeVersion; }
  void SetTypeVersion(nsUInt32 uiTypeVersion) { m_uiTypeVersion = uiTypeVersion; }
  nsStringView GetType() const { return m_sType; }
  void SetType(nsStringView sType);

  const Property* FindProperty(nsStringView sName) const;
  Property* FindProperty(nsStringView sName);

  nsStringView GetNodeName() const { return m_sNodeName; }

private:
  friend class nsAbstractObjectGraph;

  nsAbstractObjectGraph* m_pOwner = nullptr;

  nsUuid m_Guid;
  nsUInt32 m_uiTypeVersion = 0;
  nsStringView m_sType;
  nsStringView m_sNodeName;

  nsHybridArray<Property, 16> m_Properties;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsAbstractObjectNode);

struct NS_FOUNDATION_DLL nsAbstractGraphDiffOperation
{
  enum class Op
  {
    NodeAdded,
    NodeRemoved,
    PropertyChanged
  };

  Op m_Operation;
  nsUuid m_Node;            // prop parent or added / deleted node
  nsString m_sProperty;     // prop name or type
  nsUInt32 m_uiTypeVersion; // only used for NodeAdded
  nsVariant m_Value;
};

struct NS_FOUNDATION_DLL nsObjectChangeType
{
  using StorageType = nsInt8;

  enum Enum : nsInt8
  {
    NodeAdded,
    NodeRemoved,
    PropertySet,
    PropertyInserted,
    PropertyRemoved,

    Default = NodeAdded
  };
};
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsObjectChangeType);


struct NS_FOUNDATION_DLL nsDiffOperation
{
  nsEnum<nsObjectChangeType> m_Operation;
  nsUuid m_Node;        // owner of m_sProperty
  nsString m_sProperty; // property
  nsVariant m_Index;
  nsVariant m_Value;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsDiffOperation);


class NS_FOUNDATION_DLL nsAbstractObjectGraph
{
public:
  nsAbstractObjectGraph() = default;
  ~nsAbstractObjectGraph();

  void Clear();

  using FilterFunction = nsDelegate<bool(const nsAbstractObjectNode*, const nsAbstractObjectNode::Property*)>;
  nsAbstractObjectNode* Clone(nsAbstractObjectGraph& ref_cloneTarget, const nsAbstractObjectNode* pRootNode = nullptr, FilterFunction filter = FilterFunction()) const;

  nsStringView RegisterString(nsStringView sString);

  const nsAbstractObjectNode* GetNode(const nsUuid& guid) const;
  nsAbstractObjectNode* GetNode(const nsUuid& guid);

  const nsAbstractObjectNode* GetNodeByName(nsStringView sName) const;
  nsAbstractObjectNode* GetNodeByName(nsStringView sName);

  nsAbstractObjectNode* AddNode(const nsUuid& guid, nsStringView sType, nsUInt32 uiTypeVersion, nsStringView sNodeName = {});
  void RemoveNode(const nsUuid& guid);

  const nsMap<nsUuid, nsAbstractObjectNode*>& GetAllNodes() const { return m_Nodes; }
  nsMap<nsUuid, nsAbstractObjectNode*>& GetAllNodes() { return m_Nodes; }

  /// \brief Remaps all node guids by adding the given seed, or if bRemapInverse is true, by subtracting it/
  ///   This is mostly used to remap prefab instance graphs to their prefab template graph.
  void ReMapNodeGuids(const nsUuid& seedGuid, bool bRemapInverse = false);

  /// \brief Tries to remap the guids of this graph to those in rhsGraph by walking in both down the hierarchy, starting at root and
  /// rhsRoot.
  ///
  ///  Note that in case of array properties the remapping assumes element indices to be equal
  ///  on both sides which will cause all moves inside the arrays to be lost as there is no way of recovering this information without an
  ///  equality criteria. This function is mostly used to remap a graph from a native object to a graph from nsDocumentObjects to allow
  ///  applying native side changes to the original nsDocumentObject hierarchy using diffs.
  void ReMapNodeGuidsToMatchGraph(nsAbstractObjectNode* pRoot, const nsAbstractObjectGraph& rhsGraph, const nsAbstractObjectNode* pRhsRoot);

  /// \brief Finds everything accessible by the given root node.
  void FindTransitiveHull(const nsUuid& rootGuid, nsSet<nsUuid>& out_reachableNodes) const;
  /// \brief Deletes everything not accessible by the given root node.
  void PruneGraph(const nsUuid& rootGuid);

  /// \brief Allows for a given node to be modified as a native object.
  /// Once the callback exits any changes to the sub-hierarchy of the given root node will be written back to the node objects.
  void ModifyNodeViaNativeCounterpart(nsAbstractObjectNode* pRootNode, nsDelegate<void(void*, const nsRTTI*)> callback);

  /// \brief Allows to copy a node from another graph into this graph.
  nsAbstractObjectNode* CopyNodeIntoGraph(const nsAbstractObjectNode* pNode);

  nsAbstractObjectNode* CopyNodeIntoGraph(const nsAbstractObjectNode* pNode, FilterFunction& ref_filter);

  void CreateDiffWithBaseGraph(const nsAbstractObjectGraph& base, nsDeque<nsAbstractGraphDiffOperation>& out_diffResult) const;

  void ApplyDiff(nsDeque<nsAbstractGraphDiffOperation>& ref_diff);

  void MergeDiffs(const nsDeque<nsAbstractGraphDiffOperation>& lhs, const nsDeque<nsAbstractGraphDiffOperation>& rhs, nsDeque<nsAbstractGraphDiffOperation>& ref_out) const;

private:
  NS_DISALLOW_COPY_AND_ASSIGN(nsAbstractObjectGraph);

  void RemapVariant(nsVariant& value, const nsHashTable<nsUuid, nsUuid>& guidMap);
  void MergeArrays(const nsVariantArray& baseArray, const nsVariantArray& leftArray, const nsVariantArray& rightArray, nsVariantArray& out) const;
  void ReMapNodeGuidsToMatchGraphRecursive(nsHashTable<nsUuid, nsUuid>& guidMap, nsAbstractObjectNode* lhs, const nsAbstractObjectGraph& rhsGraph, const nsAbstractObjectNode* rhs);

  nsSet<nsString> m_Strings;
  nsMap<nsUuid, nsAbstractObjectNode*> m_Nodes;
  nsMap<nsStringView, nsAbstractObjectNode*> m_NodesByName;
};
