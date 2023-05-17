#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/ActionViews/MenuActionMapView.moc.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>

wdQtMenuActionMapView::wdQtMenuActionMapView(QWidget* pParent)
{
  setToolTipsVisible(true);
}

wdQtMenuActionMapView::~wdQtMenuActionMapView()
{
  ClearView();
}

void wdQtMenuActionMapView::SetActionContext(const wdActionContext& context)
{
  auto pMap = wdActionMapManager::GetActionMap(context.m_sMapping);

  WD_ASSERT_DEV(pMap != nullptr, "The given mapping '{0}' does not exist", context.m_sMapping);

  m_pActionMap = pMap;
  m_Context = context;

  CreateView();
}

void wdQtMenuActionMapView::ClearView()
{
  m_Proxies.Clear();
}

void wdQtMenuActionMapView::AddDocumentObjectToMenu(wdHashTable<wdUuid, QSharedPointer<wdQtProxy>>& ref_proxies, wdActionContext& ref_context,
  wdActionMap* pActionMap, QMenu* pCurrentRoot, const wdActionMap::TreeNode* pObject)
{
  if (pObject == nullptr)
    return;

  for (auto pChild : pObject->GetChildren())
  {
    auto pDesc = pActionMap->GetDescriptor(pChild);
    QSharedPointer<wdQtProxy> pProxy = wdQtProxy::GetProxy(ref_context, pDesc->m_hAction);
    ref_proxies[pChild->GetGuid()] = pProxy;

    switch (pDesc->m_hAction.GetDescriptor()->m_Type)
    {
      case wdActionType::Action:
      {
        QAction* pQtAction = static_cast<wdQtActionProxy*>(pProxy.data())->GetQAction();
        pCurrentRoot->addAction(pQtAction);
      }
      break;

      case wdActionType::Category:
      {
        pCurrentRoot->addSeparator();

        AddDocumentObjectToMenu(ref_proxies, ref_context, pActionMap, pCurrentRoot, pChild);

        pCurrentRoot->addSeparator();
      }
      break;

      case wdActionType::Menu:
      {
        QMenu* pQtMenu = static_cast<wdQtMenuProxy*>(pProxy.data())->GetQMenu();
        pCurrentRoot->addMenu(pQtMenu);
        AddDocumentObjectToMenu(ref_proxies, ref_context, pActionMap, pQtMenu, pChild);
      }
      break;

      case wdActionType::ActionAndMenu:
      {
        QAction* pQtAction = static_cast<wdQtDynamicActionAndMenuProxy*>(pProxy.data())->GetQAction();
        QMenu* pQtMenu = static_cast<wdQtDynamicActionAndMenuProxy*>(pProxy.data())->GetQMenu();
        pCurrentRoot->addAction(pQtAction);
        pCurrentRoot->addMenu(pQtMenu);
        AddDocumentObjectToMenu(ref_proxies, ref_context, pActionMap, pQtMenu, pChild);
      }
      break;
    }
  }
}

void wdQtMenuActionMapView::CreateView()
{
  ClearView();

  auto pObject = m_pActionMap->GetRootObject();

  AddDocumentObjectToMenu(m_Proxies, m_Context, m_pActionMap, this, pObject);
}
