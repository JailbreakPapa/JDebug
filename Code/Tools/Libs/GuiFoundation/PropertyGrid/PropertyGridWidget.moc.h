#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/Implementation/TypeWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <QWidget>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>
#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Selection/SelectionManager.h>

class QSpacerItem;
class QVBoxLayout;
class QScrollArea;

class nsQtGroupBoxBase;
class nsDocument;
class nsDocumentObjectManager;
class nsCommandHistory;
class nsObjectAccessorBase;
struct nsDocumentObjectPropertyEvent;
struct nsPropertyMetaStateEvent;
struct nsObjectAccessorChangeEvent;
struct nsPropertyDefaultEvent;
struct nsContainerElementMetaStateEvent;

class NS_GUIFOUNDATION_DLL nsQtPropertyGridWidget : public QWidget
{
  Q_OBJECT
public:
  nsQtPropertyGridWidget(QWidget* pParent, nsDocument* pDocument = nullptr, bool bBindToSelectionManager = true);
  ~nsQtPropertyGridWidget();

  void SetDocument(nsDocument* pDocument, bool bBindToSelectionManager = true);

  void ClearSelection();
  void SetSelectionIncludeExcludeProperties(const char* szIncludeProperties = nullptr, const char* szExcludeProperties = nullptr);
  void SetSelection(const nsDeque<const nsDocumentObject*>& selection);
  const nsDocument* GetDocument() const;
  const nsDocumentObjectManager* GetObjectManager() const;
  nsCommandHistory* GetCommandHistory() const;
  nsObjectAccessorBase* GetObjectAccessor() const;

  static nsRttiMappedObjectFactory<nsQtPropertyWidget>& GetFactory();
  static nsQtPropertyWidget* CreateMemberPropertyWidget(const nsAbstractProperty* pProp);
  static nsQtPropertyWidget* CreatePropertyWidget(const nsAbstractProperty* pProp);

  void SetCollapseState(nsQtGroupBoxBase* pBox);

Q_SIGNALS:
  void ExtendContextMenu(QMenu& ref_menu, const nsHybridArray<nsPropertySelection, 8>& items, const nsAbstractProperty* pProp);

public Q_SLOTS:
  void OnCollapseStateChanged(bool bCollapsed);

private:
  static nsRttiMappedObjectFactory<nsQtPropertyWidget> s_Factory;
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, PropertyGrid);

private:
  void ObjectAccessorChangeEventHandler(const nsObjectAccessorChangeEvent& e);
  void SelectionEventHandler(const nsSelectionManagerEvent& e);
  void FactoryEventHandler(const nsRttiMappedObjectFactory<nsQtPropertyWidget>::Event& e);
  void TypeEventHandler(const nsPhantomRttiManagerEvent& e);
  nsUInt32 GetGroupBoxHash(nsQtGroupBoxBase* pBox) const;

private:
  nsDocument* m_pDocument;
  bool m_bBindToSelectionManager = false;
  nsDeque<const nsDocumentObject*> m_Selection;
  nsMap<nsUInt32, bool> m_CollapseState;
  nsString m_sSelectionIncludeProperties;
  nsString m_sSelectionExcludeProperties;

  QVBoxLayout* m_pLayout;
  QScrollArea* m_pScroll;
  QWidget* m_pContent;
  QVBoxLayout* m_pContentLayout;

  nsQtTypeWidget* m_pTypeWidget;
  QSpacerItem* m_pSpacer;
};
