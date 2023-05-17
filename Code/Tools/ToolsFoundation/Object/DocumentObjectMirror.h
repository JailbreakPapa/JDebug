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
class WD_TOOLSFOUNDATION_DLL wdObjectChange
{
public:
  wdObjectChange() {}
  wdObjectChange(const wdObjectChange&);
  wdObjectChange(wdObjectChange&& rhs);
  void operator=(wdObjectChange&& rhs);
  void operator=(wdObjectChange& rhs);
  void GetGraph(wdAbstractObjectGraph& ref_graph) const;
  void SetGraph(wdAbstractObjectGraph& ref_graph);

  wdUuid m_Root;                                //< The object that is the parent of the op, namely the parent heap object we can store a pointer to.
  wdHybridArray<wdPropertyPathStep, 2> m_Steps; //< Path from root to target of change.
  wdDiffOperation m_Change;                     //< Change at the target.
  wdDataBuffer m_GraphData;                     //< In case of ObjectAdded, this holds the binary serialized object graph.
};
WD_DECLARE_REFLECTABLE_TYPE(WD_TOOLSFOUNDATION_DLL, wdObjectChange);


class WD_TOOLSFOUNDATION_DLL wdDocumentObjectMirror
{
public:
  wdDocumentObjectMirror();
  virtual ~wdDocumentObjectMirror();

  void InitSender(const wdDocumentObjectManager* pManager);
  void InitReceiver(wdRttiConverterContext* pContext);
  void DeInit();

  typedef wdDelegate<bool(const wdDocumentObject* pObject, const char* szProperty)> FilterFunction;
  /// \brief
  ///
  /// \param filter
  ///   Filter that defines whether an object property should be mirrored or not.
  void SetFilterFunction(FilterFunction filter);

  void SendDocument();
  void Clear();

  void TreeStructureEventHandler(const wdDocumentObjectStructureEvent& e);
  void TreePropertyEventHandler(const wdDocumentObjectPropertyEvent& e);

  void* GetNativeObjectPointer(const wdDocumentObject* pObject);
  const void* GetNativeObjectPointer(const wdDocumentObject* pObject) const;

protected:
  bool IsRootObject(const wdDocumentObject* pParent);
  bool IsHeapAllocated(const wdDocumentObject* pParent, const char* szParentProperty);
  bool IsDiscardedByFilter(const wdDocumentObject* pObject, const char* szProperty) const;
  static void CreatePath(wdObjectChange& out_change, const wdDocumentObject* pRoot, const char* szProperty);
  static wdUuid FindRootOpObject(const wdDocumentObject* pObject, wdHybridArray<const wdDocumentObject*, 8>& path);
  static void FlattenSteps(const wdArrayPtr<const wdDocumentObject* const> path, wdHybridArray<wdPropertyPathStep, 2>& out_steps);

  virtual void ApplyOp(wdObjectChange& change);
  void ApplyOp(wdRttiConverterObject object, const wdObjectChange& change);

protected:
  wdRttiConverterContext* m_pContext;
  const wdDocumentObjectManager* m_pManager;
  FilterFunction m_Filter;
};
