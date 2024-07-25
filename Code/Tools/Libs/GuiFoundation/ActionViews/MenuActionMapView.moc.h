#pragma once

#include <GuiFoundation/Action/ActionMap.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QMenu>
#include <QSharedPointer>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

class QWidget;
class nsActionMap;
class QAction;
class nsQtProxy;


class NS_GUIFOUNDATION_DLL nsQtMenuActionMapView : public QMenu
{
  Q_OBJECT
  NS_DISALLOW_COPY_AND_ASSIGN(nsQtMenuActionMapView);

public:
  explicit nsQtMenuActionMapView(QWidget* pParent);
  ~nsQtMenuActionMapView();

  void SetActionContext(const nsActionContext& context);

  static void AddDocumentObjectToMenu(nsHashTable<nsUuid, QSharedPointer<nsQtProxy>>& ref_proxies, nsActionContext& ref_context, nsActionMap* pActionMap,
    QMenu* pCurrentRoot, const nsActionMap::TreeNode* pObject);

private:
  void ClearView();
  void CreateView();

private:
  nsHashTable<nsUuid, QSharedPointer<nsQtProxy>> m_Proxies;

  nsActionContext m_Context;
  nsActionMap* m_pActionMap;
};
