#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/ObjectMetaData.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsDocumentObject;

class NS_TOOLSFOUNDATION_DLL nsPrefabUtils
{
public:
  /// \brief
  static void LoadGraph(nsAbstractObjectGraph& out_graph, nsStringView sGraph);

  static nsAbstractObjectNode* GetFirstRootNode(nsAbstractObjectGraph& ref_graph);

  static void GetRootNodes(nsAbstractObjectGraph& ref_graph, nsHybridArray<nsAbstractObjectNode*, 4>& out_nodes);

  static nsUuid GetPrefabRoot(const nsDocumentObject* pObject, const nsObjectMetaData<nsUuid, nsDocumentObjectMetaData>& documentObjectMetaData, nsInt32* pDepth = nullptr);

  static nsVariant GetDefaultValue(
    const nsAbstractObjectGraph& graph, const nsUuid& objectGuid, nsStringView sProperty, nsVariant index = nsVariant(), bool* pValueFound = nullptr);

  static void WriteDiff(const nsDeque<nsAbstractGraphDiffOperation>& mergedDiff, nsStringBuilder& out_sText);

  /// \brief Merges diffs of left and right graphs relative to their base graph. Conflicts prefer the right graph.
  static void Merge(const nsAbstractObjectGraph& baseGraph, const nsAbstractObjectGraph& leftGraph, const nsAbstractObjectGraph& rightGraph,
    nsDeque<nsAbstractGraphDiffOperation>& out_mergedDiff);

  /// \brief Merges diffs of left and right graphs relative to their base graph. Conflicts prefer the right graph. Base and left are provided as
  /// serialized DDL graphs and the right graph is build directly from pRight and its PrefabSeed.
  static void Merge(nsStringView sBase, nsStringView sLeft, nsDocumentObject* pRight, bool bRightIsNotPartOfPrefab, const nsUuid& prefabSeed,
    nsStringBuilder& out_sNewGraph);

  static nsString ReadDocumentAsString(nsStringView sFile);
};
