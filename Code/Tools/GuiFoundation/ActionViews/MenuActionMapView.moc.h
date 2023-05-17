#pragma once

#include <GuiFoundation/Action/ActionMap.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QMenu>
#include <QSharedPointer>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

class QWidget;
class wdActionMap;
class QAction;
class wdQtProxy;


class WD_GUIFOUNDATION_DLL wdQtMenuActionMapView : public QMenu
{
  Q_OBJECT
  WD_DISALLOW_COPY_AND_ASSIGN(wdQtMenuActionMapView);

public:
  explicit wdQtMenuActionMapView(QWidget* pParent);
  ~wdQtMenuActionMapView();

  void SetActionContext(const wdActionContext& context);

  static void AddDocumentObjectToMenu(wdHashTable<wdUuid, QSharedPointer<wdQtProxy>>& ref_proxies, wdActionContext& ref_context, wdActionMap* pActionMap,
    QMenu* pCurrentRoot, const wdActionMap::TreeNode* pObject);

private:
  void ClearView();
  void CreateView();

private:
  wdHashTable<wdUuid, QSharedPointer<wdQtProxy>> m_Proxies;

  wdActionContext m_Context;
  wdActionMap* m_pActionMap;
};

