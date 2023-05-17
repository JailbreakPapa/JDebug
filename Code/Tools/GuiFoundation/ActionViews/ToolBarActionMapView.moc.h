#pragma once

#include <GuiFoundation/Action/ActionMap.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QSharedPointer>
#include <QToolBar>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

class QWidget;
class wdActionMap;
class QAction;
class wdQtProxy;
class QMenu;

class WD_GUIFOUNDATION_DLL wdQtToolBarActionMapView : public QToolBar
{
  Q_OBJECT
  WD_DISALLOW_COPY_AND_ASSIGN(wdQtToolBarActionMapView);

public:
  explicit wdQtToolBarActionMapView(QString sTitle, QWidget* pParent);
  ~wdQtToolBarActionMapView();

  void SetActionContext(const wdActionContext& context);

  virtual void setVisible(bool bVisible) override;

private:
  void TreeEventHandler(const wdDocumentObjectStructureEvent& e);
  void TreePropertyEventHandler(const wdDocumentObjectPropertyEvent& e);

  void ClearView();
  void CreateView();
  void CreateView(const wdActionMap::TreeNode* pRoot);

private:
  wdHashTable<wdUuid, QSharedPointer<wdQtProxy>> m_Proxies;

  wdActionContext m_Context;
  wdActionMap* m_pActionMap;
};

