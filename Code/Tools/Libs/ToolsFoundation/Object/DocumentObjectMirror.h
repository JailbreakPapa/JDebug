#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Reflection/PropertyPath.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>


/// \brief An object change starts at the heap object m_Root (because we can only safely store pointers to those).
///  From this object we follow m_Steps (member arrays, structs) to execute m_Change at the end target.
///
/// In case of an NodeAdded operation, m_GraphData contains the entire subgraph of this node.
class NS_TOOLSFOUNDATION_DLL nsObjectChange
{
public:
  nsObjectChange() = default;
  nsObjectChange(const nsObjectChange&);
  nsObjectChange(nsObjectChange&& rhs);
  void operator=(nsObjectChange&& rhs);
  void operator=(nsObjectChange& rhs);
  void GetGraph(nsAbstractObjectGraph& ref_graph) const;
  void SetGraph(nsAbstractObjectGraph& ref_graph);

  nsUuid m_Root;                                //< The object that is the parent of the op, namely the parent heap object we can store a pointer to.
  nsHybridArray<nsPropertyPathStep, 2> m_Steps; //< Path from root to target of change.
  nsDiffOperation m_Change;                     //< Change at the target.
  nsDataBuffer m_GraphData;                     //< In case of ObjectAdded, this holds the binary serialized object graph.
};
NS_DECLARE_REFLECTABLE_TYPE(NS_TOOLSFOUNDATION_DLL, nsObjectChange);


class NS_TOOLSFOUNDATION_DLL nsDocumentObjectMirror
{
public:
  nsDocumentObjectMirror();
  virtual ~nsDocumentObjectMirror();

  void InitSender(const nsDocumentObjectManager* pManager);
  void InitReceiver(nsRttiConverterContext* pContext);
  void DeInit();

  using FilterFunction = nsDelegate<bool(const nsDocumentObject*, nsStringView)>;
  /// \brief
  ///
  /// \param filter
  ///   Filter that defines whether an object property should be mirrored or not.
  void SetFilterFunction(FilterFunction filter);

  void SendDocument();
  void Clear();

  void TreeStructureEventHandler(const nsDocumentObjectStructureEvent& e);
  void TreePropertyEventHandler(const nsDocumentObjectPropertyEvent& e);

  void* GetNativeObjectPointer(const nsDocumentObject* pObject);
  const void* GetNativeObjectPointer(const nsDocumentObject* pObject) const;

protected:
  bool IsRootObject(const nsDocumentObject* pParent);
  bool IsHeapAllocated(const nsDocumentObject* pParent, nsStringView sParentProperty);
  bool IsDiscardedByFilter(const nsDocumentObject* pObject, nsStringView sProperty) const;
  static void CreatePath(nsObjectChange& out_change, const nsDocumentObject* pRoot, nsStringView sProperty);
  static nsUuid FindRootOpObject(const nsDocumentObject* pObject, nsHybridArray<const nsDocumentObject*, 8>& path);
  static void FlattenSteps(const nsArrayPtr<const nsDocumentObject* const> path, nsHybridArray<nsPropertyPathStep, 2>& out_steps);

  virtual void ApplyOp(nsObjectChange& change);
  void ApplyOp(nsRttiConverterObject object, const nsObjectChange& change);

protected:
  nsRttiConverterContext* m_pContext;
  const nsDocumentObjectManager* m_pManager;
  FilterFunction m_Filter;
};
