#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class nsObjectAccessorBase;

/// \brief Writes the state of an nsDocumentObject to an abstract graph.
///
/// This information can then be applied to another nsDocument object through nsDocumentObjectConverterReader,
/// or to entirely different class using nsRttiConverterReader.
class NS_TOOLSFOUNDATION_DLL nsDocumentObjectConverterWriter
{
public:
  using FilterFunction = nsDelegate<bool(const nsDocumentObject*, const nsAbstractProperty*)>;
  nsDocumentObjectConverterWriter(nsAbstractObjectGraph* pGraph, const nsDocumentObjectManager* pManager, FilterFunction filter = FilterFunction())
  {
    m_pGraph = pGraph;
    m_pManager = pManager;
    m_Filter = filter;
  }

  nsAbstractObjectNode* AddObjectToGraph(const nsDocumentObject* pObject, nsStringView sNodeName = nullptr);

private:
  void AddProperty(nsAbstractObjectNode* pNode, const nsAbstractProperty* pProp, const nsDocumentObject* pObject);
  void AddProperties(nsAbstractObjectNode* pNode, const nsDocumentObject* pObject);

  nsAbstractObjectNode* AddSubObjectToGraph(const nsDocumentObject* pObject, nsStringView sNodeName);

  const nsDocumentObjectManager* m_pManager;
  nsAbstractObjectGraph* m_pGraph;
  FilterFunction m_Filter;
  nsSet<const nsDocumentObject*> m_QueuedObjects;
};


class NS_TOOLSFOUNDATION_DLL nsDocumentObjectConverterReader
{
public:
  enum class Mode
  {
    CreateOnly,
    CreateAndAddToDocument,
  };
  nsDocumentObjectConverterReader(const nsAbstractObjectGraph* pGraph, nsDocumentObjectManager* pManager, Mode mode);

  nsDocumentObject* CreateObjectFromNode(const nsAbstractObjectNode* pNode);
  void ApplyPropertiesToObject(const nsAbstractObjectNode* pNode, nsDocumentObject* pObject);

  nsUInt32 GetNumUnknownObjectCreations() const { return m_uiUnknownTypeInstances; }
  const nsSet<nsString>& GetUnknownObjectTypes() const { return m_UnknownTypes; }

  static void ApplyDiffToObject(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pObject, nsDeque<nsAbstractGraphDiffOperation>& ref_diff);

private:
  void AddObject(nsDocumentObject* pObject, nsDocumentObject* pParent, nsStringView sParentProperty, nsVariant index);
  void ApplyProperty(nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsAbstractObjectNode::Property* pSource);
  static void ApplyDiff(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp,
    nsAbstractGraphDiffOperation& op, nsDeque<nsAbstractGraphDiffOperation>& diff);

  Mode m_Mode;
  nsDocumentObjectManager* m_pManager;
  const nsAbstractObjectGraph* m_pGraph;
  nsSet<nsString> m_UnknownTypes;
  nsUInt32 m_uiUnknownTypeInstances;
};
