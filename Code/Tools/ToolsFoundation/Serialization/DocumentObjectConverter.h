#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class wdObjectAccessorBase;

/// \brief Writes the state of an wdDocumentObject to an abstract graph.
///
/// This information can then be applied to another wdDocument object through wdDocumentObjectConverterReader,
/// or to entirely different class using wdRttiConverterReader.
class WD_TOOLSFOUNDATION_DLL wdDocumentObjectConverterWriter
{
public:
  using FilterFunction = wdDelegate<bool(const wdDocumentObject*, const wdAbstractProperty*)>;
  wdDocumentObjectConverterWriter(wdAbstractObjectGraph* pGraph, const wdDocumentObjectManager* pManager, FilterFunction filter = FilterFunction())
  {
    m_pGraph = pGraph;
    m_pManager = pManager;
    m_Filter = filter;
  }

  wdAbstractObjectNode* AddObjectToGraph(const wdDocumentObject* pObject, const char* szNodeName = nullptr);

private:
  void AddProperty(wdAbstractObjectNode* pNode, const wdAbstractProperty* pProp, const wdDocumentObject* pObject);
  void AddProperties(wdAbstractObjectNode* pNode, const wdDocumentObject* pObject);

  wdAbstractObjectNode* AddSubObjectToGraph(const wdDocumentObject* pObject, const char* szNodeName);

  const wdDocumentObjectManager* m_pManager;
  wdAbstractObjectGraph* m_pGraph;
  FilterFunction m_Filter;
  wdSet<const wdDocumentObject*> m_QueuedObjects;
};


class WD_TOOLSFOUNDATION_DLL wdDocumentObjectConverterReader
{
public:
  enum class Mode
  {
    CreateOnly,
    CreateAndAddToDocument,
  };
  wdDocumentObjectConverterReader(const wdAbstractObjectGraph* pGraph, wdDocumentObjectManager* pManager, Mode mode);

  wdDocumentObject* CreateObjectFromNode(const wdAbstractObjectNode* pNode);
  void ApplyPropertiesToObject(const wdAbstractObjectNode* pNode, wdDocumentObject* pObject);

  wdUInt32 GetNumUnknownObjectCreations() const { return m_uiUnknownTypeInstances; }
  const wdSet<wdString>& GetUnknownObjectTypes() const { return m_UnknownTypes; }

  static void ApplyDiffToObject(wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pObject, wdDeque<wdAbstractGraphDiffOperation>& ref_diff);

private:
  void AddObject(wdDocumentObject* pObject, wdDocumentObject* pParent, const char* szParentProperty, wdVariant index);
  void ApplyProperty(wdDocumentObject* pObject, wdAbstractProperty* pProp, const wdAbstractObjectNode::Property* pSource);
  static void ApplyDiff(wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pObject, wdAbstractProperty* pProp,
    wdAbstractGraphDiffOperation& op, wdDeque<wdAbstractGraphDiffOperation>& diff);

  Mode m_Mode;
  wdDocumentObjectManager* m_pManager;
  const wdAbstractObjectGraph* m_pGraph;
  wdSet<wdString> m_UnknownTypes;
  wdUInt32 m_uiUnknownTypeInstances;
};
