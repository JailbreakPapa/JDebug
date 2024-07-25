#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/ActionViews/MenuActionMapView.moc.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>

nsQtMenuActionMapView::nsQtMenuActionMapView(QWidget* pParent)
{
  setToolTipsVisible(true);
}

nsQtMenuActionMapView::~nsQtMenuActionMapView()
{
  ClearView();
}

void nsQtMenuActionMapView::SetActionContext(const nsActionContext& context)
{
  auto pMap = nsActionMapManager::GetActionMap(context.m_sMapping);

  NS_ASSERT_DEV(pMap != nullptr, "The given mapping '{0}' does not exist", context.m_sMapping);

  m_pActionMap = pMap;
  m_Context = context;

  CreateView();
}

void nsQtMenuActionMapView::ClearView()
{
  m_Proxies.Clear();
}

void nsQtMenuActionMapView::AddDocumentObjectToMenu(nsHashTable<nsUuid, QSharedPointer<nsQtProxy>>& ref_proxies, nsActionContext& ref_context,
  nsActionMap* pActionMap, QMenu* pCurrentRoot, const nsActionMap::TreeNode* pObject)
{
  if (pObject == nullptr)
    return;

  for (auto pChild : pObject->GetChildren())
  {
    auto pDesc = pActionMap->GetDescriptor(pChild);
    QSharedPointer<nsQtProxy> pProxy = nsQtProxy::GetProxy(ref_context, pDesc->m_hAction);
    ref_proxies[pChild->GetGuid()] = pProxy;

    switch (pDesc->m_hAction.GetDescriptor()->m_Type)
    {
      case nsActionType::Action:
      {
        QAction* pQtAction = static_cast<nsQtActionProxy*>(pProxy.data())->GetQAction();
        pCurrentRoot->addAction(pQtAction);
      }
      break;

      case nsActionType::Category:
      {
        pCurrentRoot->addSeparator();

        AddDocumentObjectToMenu(ref_proxies, ref_context, pActionMap, pCurrentRoot, pChild);

        pCurrentRoot->addSeparator();
      }
      break;

      case nsActionType::Menu:
      {
        QMenu* pQtMenu = static_cast<nsQtMenuProxy*>(pProxy.data())->GetQMenu();
        pCurrentRoot->addMenu(pQtMenu);
        AddDocumentObjectToMenu(ref_proxies, ref_context, pActionMap, pQtMenu, pChild);
      }
      break;

      case nsActionType::ActionAndMenu:
      {
        QAction* pQtAction = static_cast<nsQtDynamicActionAndMenuProxy*>(pProxy.data())->GetQAction();
        QMenu* pQtMenu = static_cast<nsQtDynamicActionAndMenuProxy*>(pProxy.data())->GetQMenu();
        pCurrentRoot->addAction(pQtAction);
        pCurrentRoot->addMenu(pQtMenu);
        AddDocumentObjectToMenu(ref_proxies, ref_context, pActionMap, pQtMenu, pChild);
      }
      break;
    }
  }
}

void nsQtMenuActionMapView::CreateView()
{
  ClearView();

  auto pObject = m_pActionMap->GetRootObject();

  AddDocumentObjectToMenu(m_Proxies, m_Context, m_pActionMap, this, pObject);
}
