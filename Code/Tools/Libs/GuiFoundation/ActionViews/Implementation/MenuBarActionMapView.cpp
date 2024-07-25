#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/ActionViews/MenuActionMapView.moc.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>

nsQtMenuBarActionMapView::nsQtMenuBarActionMapView(QWidget* pParent)
  : QMenuBar(pParent)
{
}

nsQtMenuBarActionMapView::~nsQtMenuBarActionMapView()
{
  ClearView();
}

void nsQtMenuBarActionMapView::SetActionContext(const nsActionContext& context)
{
  auto pMap = nsActionMapManager::GetActionMap(context.m_sMapping);

  NS_ASSERT_DEV(pMap != nullptr, "The given mapping '{0}' does not exist", context.m_sMapping);

  m_pActionMap = pMap;
  m_Context = context;

  CreateView();
}

void nsQtMenuBarActionMapView::ClearView()
{
  m_Proxies.Clear();
}

void nsQtMenuBarActionMapView::CreateView()
{
  ClearView();

  auto pObject = m_pActionMap->GetRootObject();

  for (auto pChild : pObject->GetChildren())
  {
    auto pDesc = m_pActionMap->GetDescriptor(pChild);

    QSharedPointer<nsQtProxy> pProxy = nsQtProxy::GetProxy(m_Context, pDesc->m_hAction);
    m_Proxies[pChild->GetGuid()] = pProxy;

    switch (pDesc->m_hAction.GetDescriptor()->m_Type)
    {
      case nsActionType::Action:
      {
        NS_REPORT_FAILURE("Cannot map actions in a menubar view!");
      }
      break;

      case nsActionType::Category:
      {
        NS_REPORT_FAILURE("Cannot map category in a menubar view!");
      }
      break;

      case nsActionType::Menu:
      {
        QMenu* pQtMenu = static_cast<nsQtMenuProxy*>(pProxy.data())->GetQMenu();
        addMenu(pQtMenu);
        nsQtMenuActionMapView::AddDocumentObjectToMenu(m_Proxies, m_Context, m_pActionMap, pQtMenu, pChild);
      }
      break;

      case nsActionType::ActionAndMenu:
      {
        NS_REPORT_FAILURE("Cannot map ActionAndMenu in a menubar view!");
      }
      break;
    }
  }
}
