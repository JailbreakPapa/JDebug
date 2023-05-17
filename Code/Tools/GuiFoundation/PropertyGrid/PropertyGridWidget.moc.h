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

class wdQtGroupBoxBase;
class wdDocument;
class wdDocumentObjectManager;
class wdCommandHistory;
class wdObjectAccessorBase;
struct wdDocumentObjectPropertyEvent;
struct wdPropertyMetaStateEvent;
struct wdObjectAccessorChangeEvent;
struct wdPropertyDefaultEvent;
struct wdContainerElementMetaStateEvent;

class WD_GUIFOUNDATION_DLL wdQtPropertyGridWidget : public QWidget
{
  Q_OBJECT
public:
  wdQtPropertyGridWidget(QWidget* pParent, wdDocument* pDocument = nullptr, bool bBindToSelectionManager = true);
  ~wdQtPropertyGridWidget();

  void SetDocument(wdDocument* pDocument, bool bBindToSelectionManager = true);

  void ClearSelection();
  void SetSelectionIncludeExcludeProperties(const char* szIncludeProperties = nullptr, const char* szExcludeProperties = nullptr);
  void SetSelection(const wdDeque<const wdDocumentObject*>& selection);
  const wdDocument* GetDocument() const;
  const wdDocumentObjectManager* GetObjectManager() const;
  wdCommandHistory* GetCommandHistory() const;
  wdObjectAccessorBase* GetObjectAccessor() const;

  static wdRttiMappedObjectFactory<wdQtPropertyWidget>& GetFactory();
  static wdQtPropertyWidget* CreateMemberPropertyWidget(const wdAbstractProperty* pProp);
  static wdQtPropertyWidget* CreatePropertyWidget(const wdAbstractProperty* pProp);

  void SetCollapseState(wdQtGroupBoxBase* pBox);

Q_SIGNALS:
  void ExtendContextMenu(QMenu& ref_menu, const wdHybridArray<wdPropertySelection, 8>& items, const wdAbstractProperty* pProp);

public Q_SLOTS:
  void OnCollapseStateChanged(bool bCollapsed);

private:
  static wdRttiMappedObjectFactory<wdQtPropertyWidget> s_Factory;
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, PropertyGrid);

private:
  void ObjectAccessorChangeEventHandler(const wdObjectAccessorChangeEvent& e);
  void SelectionEventHandler(const wdSelectionManagerEvent& e);
  void FactoryEventHandler(const wdRttiMappedObjectFactory<wdQtPropertyWidget>::Event& e);
  void TypeEventHandler(const wdPhantomRttiManagerEvent& e);
  wdUInt32 GetGroupBoxHash(wdQtGroupBoxBase* pBox) const;

private:
  wdDocument* m_pDocument;
  bool m_bBindToSelectionManager = false;
  wdDeque<const wdDocumentObject*> m_Selection;
  wdMap<wdUInt32, bool> m_CollapseState;
  wdString m_sSelectionIncludeProperties;
  wdString m_sSelectionExcludeProperties;

  QVBoxLayout* m_pLayout;
  QScrollArea* m_pScroll;
  QWidget* m_pContent;
  QVBoxLayout* m_pContentLayout;

  wdQtTypeWidget* m_pTypeWidget;
  QSpacerItem* m_pSpacer;
};

