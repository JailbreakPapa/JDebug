#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/ObjectMetaData.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocumentObject;

class WD_TOOLSFOUNDATION_DLL wdPrefabUtils
{
public:
  /// \brief
  static void LoadGraph(wdAbstractObjectGraph& out_graph, const char* szGraph);

  static wdAbstractObjectNode* GetFirstRootNode(wdAbstractObjectGraph& ref_graph);

  static void GetRootNodes(wdAbstractObjectGraph& ref_graph, wdHybridArray<wdAbstractObjectNode*, 4>& out_nodes);

  static wdUuid GetPrefabRoot(const wdDocumentObject* pObject, const wdObjectMetaData<wdUuid, wdDocumentObjectMetaData>& documentObjectMetaData, wdInt32* pDepth = nullptr);

  static wdVariant GetDefaultValue(
    const wdAbstractObjectGraph& graph, const wdUuid& objectGuid, const char* szProperty, wdVariant index = wdVariant(), bool* pValueFound = nullptr);

  static void WriteDiff(const wdDeque<wdAbstractGraphDiffOperation>& mergedDiff, wdStringBuilder& out_sText);

  /// \brief Merges diffs of left and right graphs relative to their base graph. Conflicts prefer the right graph.
  static void Merge(const wdAbstractObjectGraph& baseGraph, const wdAbstractObjectGraph& leftGraph, const wdAbstractObjectGraph& rightGraph,
    wdDeque<wdAbstractGraphDiffOperation>& out_mergedDiff);

  /// \brief Merges diffs of left and right graphs relative to their base graph. Conflicts prefer the right graph. Base and left are provided as
  /// serialized DDL graphs and the right graph is build directly from pRight and its PrefabSeed.
  static void Merge(const char* szBase, const char* szLeft, wdDocumentObject* pRight, bool bRightIsNotPartOfPrefab, const wdUuid& prefabSeed,
    wdStringBuilder& out_sNewGraph);

  static wdString ReadDocumentAsString(const char* szFile);
};
