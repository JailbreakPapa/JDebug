#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/ActionViews/MenuActionMapView.moc.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>

wdQtMenuBarActionMapView::wdQtMenuBarActionMapView(QWidget* pParent)
  : QMenuBar(pParent)
{
}

wdQtMenuBarActionMapView::~wdQtMenuBarActionMapView()
{
  ClearView();
}

void wdQtMenuBarActionMapView::SetActionContext(const wdActionContext& context)
{
  auto pMap = wdActionMapManager::GetActionMap(context.m_sMapping);

  WD_ASSERT_DEV(pMap != nullptr, "The given mapping '{0}' does not exist", context.m_sMapping);

  m_pActionMap = pMap;
  m_Context = context;

  CreateView();
}

void wdQtMenuBarActionMapView::ClearView()
{
  m_Proxies.Clear();
}

void wdQtMenuBarActionMapView::CreateView()
{
  ClearView();

  auto pObject = m_pActionMap->GetRootObject();

  for (auto pChild : pObject->GetChildren())
  {
    auto pDesc = m_pActionMap->GetDescriptor(pChild);

    QSharedPointer<wdQtProxy> pProxy = wdQtProxy::GetProxy(m_Context, pDesc->m_hAction);
    m_Proxies[pChild->GetGuid()] = pProxy;

    switch (pDesc->m_hAction.GetDescriptor()->m_Type)
    {
      case wdActionType::Action:
      {
        WD_REPORT_FAILURE("Cannot map actions in a menubar view!");
      }
      break;

      case wdActionType::Category:
      {
        WD_REPORT_FAILURE("Cannot map category in a menubar view!");
      }
      break;

      case wdActionType::Menu:
      {
        QMenu* pQtMenu = static_cast<wdQtMenuProxy*>(pProxy.data())->GetQMenu();
        addMenu(pQtMenu);
        wdQtMenuActionMapView::AddDocumentObjectToMenu(m_Proxies, m_Context, m_pActionMap, pQtMenu, pChild);
      }
      break;

      case wdActionType::ActionAndMenu:
      {
        WD_REPORT_FAILURE("Cannot map ActionAndMenu in a menubar view!");
      }
      break;
    }
  }
}
