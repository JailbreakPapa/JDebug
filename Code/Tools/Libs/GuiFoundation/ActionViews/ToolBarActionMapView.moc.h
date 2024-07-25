#pragma once

#include <GuiFoundation/Action/ActionMap.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QSharedPointer>
#include <QToolBar>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

class QWidget;
class nsActionMap;
class QAction;
class nsQtProxy;
class QMenu;

class NS_GUIFOUNDATION_DLL nsQtToolBarActionMapView : public QToolBar
{
  Q_OBJECT
  NS_DISALLOW_COPY_AND_ASSIGN(nsQtToolBarActionMapView);

public:
  explicit nsQtToolBarActionMapView(QString sTitle, QWidget* pParent);
  ~nsQtToolBarActionMapView();

  void SetActionContext(const nsActionContext& context);

  virtual void setVisible(bool bVisible) override;

private:
  void TreeEventHandler(const nsDocumentObjectStructureEvent& e);
  void TreePropertyEventHandler(const nsDocumentObjectPropertyEvent& e);

  void ClearView();
  void CreateView();
  void CreateView(const nsActionMap::TreeNode* pRoot);

private:
  nsHashTable<nsUuid, QSharedPointer<nsQtProxy>> m_Proxies;

  nsActionContext m_Context;
  nsActionMap* m_pActionMap;
};
