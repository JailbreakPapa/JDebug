#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/ActionViews/MenuActionMapView.moc.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QMenu>
#include <QToolButton>

wdQtToolBarActionMapView::wdQtToolBarActionMapView(QString sTitle, QWidget* pParent)
  : QToolBar(sTitle, pParent)
{
  setIconSize(QSize(16, 16));
  setFloatable(false);

  toggleViewAction()->setEnabled(false);
}

wdQtToolBarActionMapView::~wdQtToolBarActionMapView()
{
  ClearView();
}

void wdQtToolBarActionMapView::SetActionContext(const wdActionContext& context)
{
  auto pMap = wdActionMapManager::GetActionMap(context.m_sMapping);

  WD_ASSERT_DEV(pMap != nullptr, "The given mapping '{0}' does not exist", context.m_sMapping);

  m_pActionMap = pMap;
  m_Context = context;

  CreateView();
}

void wdQtToolBarActionMapView::setVisible(bool bVisible)
{
  QToolBar::setVisible(true);
}

void wdQtToolBarActionMapView::ClearView()
{
  m_Proxies.Clear();
}

void wdQtToolBarActionMapView::CreateView()
{
  ClearView();

  auto pObject = m_pActionMap->GetRootObject();

  CreateView(pObject);

  if (!actions().isEmpty() && actions().back()->isSeparator())
  {
    QAction* pAction = actions().back();
    removeAction(pAction);
    pAction->deleteLater();
  }
}

void wdQtToolBarActionMapView::CreateView(const wdActionMap::TreeNode* pObject)
{
  for (auto pChild : pObject->GetChildren())
  {
    auto pDesc = m_pActionMap->GetDescriptor(pChild);
    QSharedPointer<wdQtProxy> pProxy = wdQtProxy::GetProxy(m_Context, pDesc->m_hAction);
    m_Proxies[pChild->GetGuid()] = pProxy;

    switch (pDesc->m_hAction.GetDescriptor()->m_Type)
    {
      case wdActionType::Action:
      {
        QAction* pQtAction = static_cast<wdQtActionProxy*>(pProxy.data())->GetQAction();
        addAction(pQtAction);
      }
      break;

      case wdActionType::Category:
      {
        if (!actions().isEmpty() && !actions().back()->isSeparator())
          addSeparator()->setParent(pProxy.data());

        CreateView(pChild);

        if (!actions().isEmpty() && !actions().back()->isSeparator())
          addSeparator()->setParent(pProxy.data());
      }
      break;

      case wdActionType::Menu:
      {
        wdNamedAction* pNamed = static_cast<wdNamedAction*>(pProxy->GetAction());

        QMenu* pQtMenu = static_cast<wdQtMenuProxy*>(pProxy.data())->GetQMenu();
        // TODO pButton leaks!
        QToolButton* pButton = new QToolButton(this);
        pButton->setMenu(pQtMenu);
        pButton->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
        pButton->setText(pQtMenu->title());
        pButton->setIcon(wdQtUiServices::GetCachedIconResource(pNamed->GetIconPath()));
        pButton->setToolTip(pQtMenu->title().toUtf8().data());

        // TODO addWidget return value of QAction leaks!
        QAction* pToolButtonAction = addWidget(pButton);
        pToolButtonAction->setParent(pQtMenu);

        wdQtMenuActionMapView::AddDocumentObjectToMenu(m_Proxies, m_Context, m_pActionMap, pQtMenu, pChild);
      }
      break;

      case wdActionType::ActionAndMenu:
      {
        wdNamedAction* pNamed = static_cast<wdNamedAction*>(pProxy->GetAction());

        QMenu* pQtMenu = static_cast<wdQtDynamicActionAndMenuProxy*>(pProxy.data())->GetQMenu();
        QAction* pQtAction = static_cast<wdQtDynamicActionAndMenuProxy*>(pProxy.data())->GetQAction();
        // TODO pButton leaks!
        QToolButton* pButton = new QToolButton(this);
        pButton->setDefaultAction(pQtAction);
        pButton->setMenu(pQtMenu);
        pButton->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);

        // TODO addWidget return value of QAction leaks!
        QAction* pToolButtonAction = addWidget(pButton);
        pToolButtonAction->setParent(pQtMenu);

        wdQtMenuActionMapView::AddDocumentObjectToMenu(m_Proxies, m_Context, m_pActionMap, pQtMenu, pChild);
      }
      break;
    }
  }
}
