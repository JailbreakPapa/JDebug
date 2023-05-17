#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QAction>
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QSlider>
#include <QWidgetAction>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

wdRttiMappedObjectFactory<wdQtProxy> wdQtProxy::s_Factory;
wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>> wdQtProxy::s_GlobalActions;
wdMap<const wdDocument*, wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>>> wdQtProxy::s_DocumentActions;
wdMap<QWidget*, wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>>> wdQtProxy::s_WindowActions;
QObject* wdQtProxy::s_pSignalProxy = nullptr;

static wdQtProxy* QtMenuProxyCreator(const wdRTTI* pRtti)
{
  return new (wdQtMenuProxy);
}

static wdQtProxy* QtCategoryProxyCreator(const wdRTTI* pRtti)
{
  return new (wdQtCategoryProxy);
}

static wdQtProxy* QtButtonProxyCreator(const wdRTTI* pRtti)
{
  return new (wdQtButtonProxy);
}

static wdQtProxy* QtDynamicMenuProxyCreator(const wdRTTI* pRtti)
{
  return new (wdQtDynamicMenuProxy);
}

static wdQtProxy* QtDynamicActionAndMenuProxyCreator(const wdRTTI* pRtti)
{
  return new (wdQtDynamicActionAndMenuProxy);
}

static wdQtProxy* QtSliderProxyCreator(const wdRTTI* pRtti)
{
  return new (wdQtSliderProxy);
}

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, QtProxies)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "ToolsFoundation",
  "ActionManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdQtProxy::GetFactory().RegisterCreator(wdGetStaticRTTI<wdMenuAction>(), QtMenuProxyCreator);
    wdQtProxy::GetFactory().RegisterCreator(wdGetStaticRTTI<wdCategoryAction>(), QtCategoryProxyCreator);
    wdQtProxy::GetFactory().RegisterCreator(wdGetStaticRTTI<wdDynamicMenuAction>(), QtDynamicMenuProxyCreator);
    wdQtProxy::GetFactory().RegisterCreator(wdGetStaticRTTI<wdDynamicActionAndMenuAction>(), QtDynamicActionAndMenuProxyCreator);
    wdQtProxy::GetFactory().RegisterCreator(wdGetStaticRTTI<wdButtonAction>(), QtButtonProxyCreator);
    wdQtProxy::GetFactory().RegisterCreator(wdGetStaticRTTI<wdSliderAction>(), QtSliderProxyCreator);
    wdQtProxy::s_pSignalProxy = new QObject;
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdQtProxy::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdMenuAction>());
    wdQtProxy::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdCategoryAction>());
    wdQtProxy::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdDynamicMenuAction>());
    wdQtProxy::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdDynamicActionAndMenuAction>());
    wdQtProxy::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdButtonAction>());
    wdQtProxy::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdSliderAction>());
    wdQtProxy::s_GlobalActions.Clear();
    wdQtProxy::s_DocumentActions.Clear();
    wdQtProxy::s_WindowActions.Clear();
    delete wdQtProxy::s_pSignalProxy;
    wdQtProxy::s_pSignalProxy = nullptr;
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

bool wdQtProxy::TriggerDocumentAction(wdDocument* pDocument, QKeyEvent* pEvent)
{
  auto CheckActions = [](QKeyEvent* pEvent, wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>>& ref_actions) -> bool {
    for (auto weakActionProxy : ref_actions)
    {
      if (auto pProxy = weakActionProxy.Value().toStrongRef())
      {
        QAction* pQAction = nullptr;
        if (auto pActionProxy = qobject_cast<wdQtActionProxy*>(pProxy))
        {
          pQAction = pActionProxy->GetQAction();
        }
        else if (auto pActionProxy2 = qobject_cast<wdQtDynamicActionAndMenuProxy*>(pProxy))
        {
          pQAction = pActionProxy2->GetQAction();
        }

        if (pQAction)
        {
          QKeySequence ks = pQAction->shortcut();
          if (pQAction->isEnabled() && QKeySequence(pEvent->key() | pEvent->modifiers()) == ks)
          {
            pQAction->trigger();
            pEvent->accept();
            return true;
          }
        }
      }
    }
    return false;
  };

  if (pDocument)
  {
    wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>>& actions = s_DocumentActions[pDocument];
    if (CheckActions(pEvent, actions))
      return true;
  }
  return CheckActions(pEvent, s_GlobalActions);
}

wdRttiMappedObjectFactory<wdQtProxy>& wdQtProxy::GetFactory()
{
  return s_Factory;
}
QSharedPointer<wdQtProxy> wdQtProxy::GetProxy(wdActionContext& ref_context, wdActionDescriptorHandle hDesc)
{
  QSharedPointer<wdQtProxy> pProxy;
  const wdActionDescriptor* pDesc = hDesc.GetDescriptor();
  if (pDesc->m_Type != wdActionType::Action && pDesc->m_Type != wdActionType::ActionAndMenu)
  {
    auto pAction = pDesc->CreateAction(ref_context);
    pProxy = QSharedPointer<wdQtProxy>(wdQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
    WD_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
    pProxy->SetAction(pAction);
    WD_ASSERT_DEV(pProxy->GetAction()->GetContext().m_pDocument == ref_context.m_pDocument, "invalid document pointer");
    return pProxy;
  }

  // wdActionType::Action will be cached to ensure only one QAction exist in its scope to prevent shortcut collisions.
  switch (pDesc->m_Scope)
  {
    case wdActionScope::Global:
    {
      QWeakPointer<wdQtProxy> pTemp = s_GlobalActions[hDesc];
      if (pTemp.isNull())
      {
        auto pAction = pDesc->CreateAction(ref_context);
        pProxy = QSharedPointer<wdQtProxy>(wdQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
        WD_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
        pProxy->SetAction(pAction);
        s_GlobalActions[hDesc] = pProxy.toWeakRef();
      }
      else
      {
        pProxy = pTemp.toStrongRef();
      }

      break;
    }

    case wdActionScope::Document:
    {
      const wdDocument* pDocument = ref_context.m_pDocument; // may be null

      QWeakPointer<wdQtProxy> pTemp = s_DocumentActions[pDocument][hDesc];
      if (pTemp.isNull())
      {
        auto pAction = pDesc->CreateAction(ref_context);
        pProxy = QSharedPointer<wdQtProxy>(wdQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
        WD_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
        pProxy->SetAction(pAction);
        s_DocumentActions[pDocument][hDesc] = pProxy;
      }
      else
      {
        pProxy = pTemp.toStrongRef();
      }

      break;
    }

    case wdActionScope::Window:
    {
      bool bExisted = true;
      auto it = s_WindowActions.FindOrAdd(ref_context.m_pWindow, &bExisted);
      if (!bExisted)
      {
        s_pSignalProxy->connect(ref_context.m_pWindow, &QObject::destroyed, s_pSignalProxy, [ref_context]() { s_WindowActions.Remove(ref_context.m_pWindow); });
      }
      QWeakPointer<wdQtProxy> pTemp = it.Value()[hDesc];
      if (pTemp.isNull())
      {
        auto pAction = pDesc->CreateAction(ref_context);
        pProxy = QSharedPointer<wdQtProxy>(wdQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
        WD_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
        pProxy->SetAction(pAction);
        it.Value()[hDesc] = pProxy;
      }
      else
      {
        pProxy = pTemp.toStrongRef();
      }

      break;
    }
  }

  // make sure we don't use actions that are meant for a different document
  if (pProxy != nullptr && pProxy->GetAction()->GetContext().m_pDocument != nullptr)
  {
    // if this assert fires, you might have tried to map an action into multiple documents, which uses wdActionScope::Global
    wdAction* pAction = pProxy->GetAction();
    const wdActionContext& ctxt = pAction->GetContext();
    wdDocument* pDoc = ctxt.m_pDocument;
    WD_ASSERT_DEV(pDoc == ref_context.m_pDocument, "invalid document pointer");
  }
  return pProxy;
}

wdQtProxy::wdQtProxy()
{
  m_pAction = nullptr;
}

wdQtProxy::~wdQtProxy()
{
  if (m_pAction != nullptr)
    wdActionManager::GetActionDescriptor(m_pAction->GetDescriptorHandle())->DeleteAction(m_pAction);
}

void wdQtProxy::SetAction(wdAction* pAction)
{
  m_pAction = pAction;
}

//////////////////// wdQtMenuProxy /////////////////////

wdQtMenuProxy::wdQtMenuProxy()
{
  m_pMenu = nullptr;
}

wdQtMenuProxy::~wdQtMenuProxy()
{
  m_pMenu->deleteLater();
  delete m_pMenu;
}

void wdQtMenuProxy::Update()
{
  auto pMenu = static_cast<wdMenuAction*>(m_pAction);

  m_pMenu->setIcon(wdQtUiServices::GetCachedIconResource(pMenu->GetIconPath()));
  m_pMenu->setTitle(QString::fromUtf8(wdTranslate(pMenu->GetName())));
}

void wdQtMenuProxy::SetAction(wdAction* pAction)
{
  wdQtProxy::SetAction(pAction);

  m_pMenu = new QMenu();
  m_pMenu->setToolTipsVisible(true);
  Update();
}

QMenu* wdQtMenuProxy::GetQMenu()
{
  return m_pMenu;
}

//////////////////////////////////////////////////////////////////////////
//////////////////// wdQtButtonProxy /////////////////////
//////////////////////////////////////////////////////////////////////////

wdQtButtonProxy::wdQtButtonProxy()
{
  m_pQtAction = nullptr;
}

wdQtButtonProxy::~wdQtButtonProxy()
{
  m_pAction->m_StatusUpdateEvent.RemoveEventHandler(wdMakeDelegate(&wdQtButtonProxy::StatusUpdateEventHandler, this));

  if (m_pQtAction != nullptr)
  {
    m_pQtAction->deleteLater();
  }
  m_pQtAction = nullptr;
}

void wdQtButtonProxy::Update()
{
  if (m_pQtAction == nullptr)
    return;

  auto pButton = static_cast<wdButtonAction*>(m_pAction);


  const wdActionDescriptor* pDesc = m_pAction->GetDescriptorHandle().GetDescriptor();
  m_pQtAction->setShortcut(QKeySequence(QString::fromUtf8(pDesc->m_sShortcut.GetData())));

  const QString sDisplayShortcut = m_pQtAction->shortcut().toString(QKeySequence::NativeText);
  QString sTooltip = wdTranslateTooltip(pButton->GetName());

  wdStringBuilder sDisplay = wdTranslate(pButton->GetName());

  if (sTooltip.isEmpty())
  {
    sTooltip = sDisplay;
    sTooltip.replace("&", "");
  }

  if (!sDisplayShortcut.isEmpty())
  {
    sTooltip.append(" (");
    sTooltip.append(sDisplayShortcut);
    sTooltip.append(")");
  }

  if (!wdStringUtils::IsNullOrEmpty(pButton->GetAdditionalDisplayString()))
    sDisplay.Append(" '", pButton->GetAdditionalDisplayString(), "'"); // TODO: translate this as well?

  m_pQtAction->setIcon(wdQtUiServices::GetCachedIconResource(pButton->GetIconPath()));
  m_pQtAction->setText(QString::fromUtf8(sDisplay.GetData()));
  m_pQtAction->setToolTip(sTooltip);
  m_pQtAction->setCheckable(pButton->IsCheckable());
  m_pQtAction->setChecked(pButton->IsChecked());
  m_pQtAction->setEnabled(pButton->IsEnabled());
  m_pQtAction->setVisible(pButton->IsVisible());
}


void SetupQAction(wdAction* pAction, QPointer<QAction>& ref_pQtAction, QObject* pTarget)
{
  wdActionDescriptorHandle hDesc = pAction->GetDescriptorHandle();
  const wdActionDescriptor* pDesc = hDesc.GetDescriptor();

  if (ref_pQtAction == nullptr)
  {
    ref_pQtAction = new QAction(nullptr);
    WD_VERIFY(QObject::connect(ref_pQtAction, SIGNAL(triggered(bool)), pTarget, SLOT(OnTriggered())) != nullptr, "connection failed");

    switch (pDesc->m_Scope)
    {
      case wdActionScope::Global:
      {
        // Parent is null so the global actions don't get deleted.
        ref_pQtAction->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
      }
      break;
      case wdActionScope::Document:
      {
        // Parent is set to the window belonging to the document.
        wdQtDocumentWindow* pWindow = wdQtDocumentWindow::FindWindowByDocument(pAction->GetContext().m_pDocument);
        WD_ASSERT_DEBUG(pWindow != nullptr, "You can't map a wdActionScope::Document action without that document existing!");
        ref_pQtAction->setParent(pWindow);
        ref_pQtAction->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
      }
      break;
      case wdActionScope::Window:
      {
        ref_pQtAction->setParent(pAction->GetContext().m_pWindow);
        ref_pQtAction->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
      }
      break;
    }
  }
}

void wdQtButtonProxy::SetAction(wdAction* pAction)
{
  WD_ASSERT_DEV(m_pAction == nullptr, "Es darf nicht sein, es kann nicht sein!");

  wdQtProxy::SetAction(pAction);
  m_pAction->m_StatusUpdateEvent.AddEventHandler(wdMakeDelegate(&wdQtButtonProxy::StatusUpdateEventHandler, this));

  SetupQAction(m_pAction, m_pQtAction, this);

  Update();
}

QAction* wdQtButtonProxy::GetQAction()
{
  return m_pQtAction;
}

void wdQtButtonProxy::StatusUpdateEventHandler(wdAction* pAction)
{
  Update();
}

void wdQtButtonProxy::OnTriggered()
{
  // make sure all focus is lost, to trigger pending changes
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  if (pFocusWidget)
    QApplication::focusWidget()->clearFocus();

  m_pAction->Execute(m_pQtAction->isChecked());

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

void wdQtDynamicMenuProxy::SetAction(wdAction* pAction)
{
  wdQtMenuProxy::SetAction(pAction);

  WD_VERIFY(connect(m_pMenu, SIGNAL(aboutToShow()), this, SLOT(SlotMenuAboutToShow())) != nullptr, "signal/slot connection failed");
}

void wdQtDynamicMenuProxy::SlotMenuAboutToShow()
{
  m_pMenu->clear();

  static_cast<wdDynamicMenuAction*>(m_pAction)->GetEntries(m_Entries);

  if (m_Entries.IsEmpty())
  {
    m_pMenu->addAction("<empty>")->setEnabled(false);
  }
  else
  {
    for (wdUInt32 i = 0; i < m_Entries.GetCount(); ++i)
    {
      const auto& p = m_Entries[i];

      if (p.m_ItemFlags.IsSet(wdDynamicMenuAction::Item::ItemFlags::Separator))
      {
        m_pMenu->addSeparator();
      }
      else
      {
        auto pAction = m_pMenu->addAction(QString::fromUtf8(p.m_sDisplay.GetData()));
        pAction->setData(i);
        pAction->setIcon(p.m_Icon);
        pAction->setCheckable(p.m_CheckState != wdDynamicMenuAction::Item::CheckMark::NotCheckable);
        pAction->setChecked(p.m_CheckState == wdDynamicMenuAction::Item::CheckMark::Checked);

        WD_VERIFY(connect(pAction, SIGNAL(triggered()), this, SLOT(SlotMenuEntryTriggered())) != nullptr, "signal/slot connection failed");
      }
    }
  }
}

void wdQtDynamicMenuProxy::SlotMenuEntryTriggered()
{
  QAction* pAction = qobject_cast<QAction*>(sender());
  if (!pAction)
    return;

  // make sure all focus is lost, to trigger pending changes
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  if (pFocusWidget)
    QApplication::focusWidget()->clearFocus();

  wdUInt32 index = pAction->data().toUInt();
  m_pAction->Execute(m_Entries[index].m_UserValue);

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

//////////////////////////////////////////////////////////////////////////
//////////////////// wdQtDynamicActionAndMenuProxy /////////////////////
//////////////////////////////////////////////////////////////////////////

wdQtDynamicActionAndMenuProxy::wdQtDynamicActionAndMenuProxy()
{
  m_pQtAction = nullptr;
}

wdQtDynamicActionAndMenuProxy::~wdQtDynamicActionAndMenuProxy()
{
  m_pAction->m_StatusUpdateEvent.RemoveEventHandler(wdMakeDelegate(&wdQtDynamicActionAndMenuProxy::StatusUpdateEventHandler, this));

  if (m_pQtAction != nullptr)
  {
    m_pQtAction->deleteLater();
  }
  m_pQtAction = nullptr;
}


void wdQtDynamicActionAndMenuProxy::Update()
{
  wdQtDynamicMenuProxy::Update();

  if (m_pQtAction == nullptr)
    return;

  auto pButton = static_cast<wdDynamicActionAndMenuAction*>(m_pAction);

  const wdActionDescriptor* pDesc = m_pAction->GetDescriptorHandle().GetDescriptor();
  m_pQtAction->setShortcut(QKeySequence(QString::fromUtf8(pDesc->m_sShortcut.GetData())));

  wdStringBuilder sDisplay = wdTranslate(pButton->GetName());

  if (!wdStringUtils::IsNullOrEmpty(pButton->GetAdditionalDisplayString()))
    sDisplay.Append(" '", pButton->GetAdditionalDisplayString(), "'"); // TODO: translate this as well?

  const QString sDisplayShortcut = m_pQtAction->shortcut().toString(QKeySequence::NativeText);
  QString sTooltip = wdTranslateTooltip(pButton->GetName());

  if (sTooltip.isEmpty())
  {
    sTooltip = sDisplay;
    sTooltip.replace("&", "");
  }

  if (!sDisplayShortcut.isEmpty())
  {
    sTooltip.append(" (");
    sTooltip.append(sDisplayShortcut);
    sTooltip.append(")");
  }

  m_pQtAction->setIcon(wdQtUiServices::GetCachedIconResource(pButton->GetIconPath()));
  m_pQtAction->setText(QString::fromUtf8(sDisplay.GetData()));
  m_pQtAction->setToolTip(sTooltip);
  m_pQtAction->setEnabled(pButton->IsEnabled());
  m_pQtAction->setVisible(pButton->IsVisible());
}


void wdQtDynamicActionAndMenuProxy::SetAction(wdAction* pAction)
{
  wdQtDynamicMenuProxy::SetAction(pAction);

  m_pAction->m_StatusUpdateEvent.AddEventHandler(wdMakeDelegate(&wdQtDynamicActionAndMenuProxy::StatusUpdateEventHandler, this));

  SetupQAction(m_pAction, m_pQtAction, this);

  Update();
}

QAction* wdQtDynamicActionAndMenuProxy::GetQAction()
{
  return m_pQtAction;
}

void wdQtDynamicActionAndMenuProxy::OnTriggered()
{
  // make sure all focus is lost, to trigger pending changes
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  if (pFocusWidget)
    QApplication::focusWidget()->clearFocus();

  m_pAction->Execute(wdVariant());

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

void wdQtDynamicActionAndMenuProxy::StatusUpdateEventHandler(wdAction* pAction)
{
  Update();
}


//////////////////////////////////////////////////////////////////////////
//////////////////// wdQtSliderProxy /////////////////////
//////////////////////////////////////////////////////////////////////////

wdQtSliderWidgetAction::wdQtSliderWidgetAction(QWidget* pParent)
  : QWidgetAction(pParent)
{
}

wdQtLabeledSlider::wdQtLabeledSlider(QWidget* pParent)
  : QWidget(pParent)
{
  m_pLabel = new QLabel(this);
  m_pSlider = new QSlider(this);
  setLayout(new QHBoxLayout(this));

  layout()->addWidget(m_pLabel);
  layout()->addWidget(m_pSlider);

  setMaximumWidth(300);
}

void wdQtSliderWidgetAction::setMinimum(int value)
{
  m_iMinimum = value;

  const QList<QWidget*> widgets = createdWidgets();

  for (QWidget* pWidget : widgets)
  {
    wdQtLabeledSlider* pGroup = qobject_cast<wdQtLabeledSlider*>(pWidget);
    pGroup->m_pSlider->setMinimum(m_iMinimum);
  }
}

void wdQtSliderWidgetAction::setMaximum(int value)
{
  m_iMaximum = value;

  const QList<QWidget*> widgets = createdWidgets();

  for (QWidget* pWidget : widgets)
  {
    wdQtLabeledSlider* pGroup = qobject_cast<wdQtLabeledSlider*>(pWidget);
    pGroup->m_pSlider->setMaximum(m_iMaximum);
  }
}

void wdQtSliderWidgetAction::setValue(int value)
{
  m_iValue = value;

  const QList<QWidget*> widgets = createdWidgets();

  for (QWidget* pWidget : widgets)
  {
    wdQtLabeledSlider* pGroup = qobject_cast<wdQtLabeledSlider*>(pWidget);
    pGroup->m_pSlider->setValue(m_iValue);
  }
}

void wdQtSliderWidgetAction::OnValueChanged(int value)
{
  Q_EMIT valueChanged(value);
}

QWidget* wdQtSliderWidgetAction::createWidget(QWidget* parent)
{
  wdQtLabeledSlider* pGroup = new wdQtLabeledSlider(parent);
  pGroup->m_pSlider->setOrientation(Qt::Orientation::Horizontal);

  WD_VERIFY(connect(pGroup->m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int))) != nullptr, "connection failed");

  pGroup->m_pLabel->setText(text());
  pGroup->m_pLabel->installEventFilter(this);
  pGroup->m_pLabel->setToolTip(toolTip());
  pGroup->installEventFilter(this);
  pGroup->m_pSlider->setMinimum(m_iMinimum);
  pGroup->m_pSlider->setMaximum(m_iMaximum);
  pGroup->m_pSlider->setValue(m_iValue);
  pGroup->m_pSlider->setToolTip(toolTip());

  return pGroup;
}

bool wdQtSliderWidgetAction::eventFilter(QObject* obj, QEvent* e)
{
  if (e->type() == QEvent::Type::MouseButtonPress || e->type() == QEvent::Type::MouseButtonRelease || e->type() == QEvent::Type::MouseButtonDblClick)
  {
    e->accept();
    return true;
  }

  return false;
}

wdQtSliderProxy::wdQtSliderProxy()
{
  m_pQtAction = nullptr;
}

wdQtSliderProxy::~wdQtSliderProxy()
{
  m_pAction->m_StatusUpdateEvent.RemoveEventHandler(wdMakeDelegate(&wdQtSliderProxy::StatusUpdateEventHandler, this));

  if (m_pQtAction != nullptr)
  {
    m_pQtAction->deleteLater();
  }
  m_pQtAction = nullptr;
}

void wdQtSliderProxy::Update()
{
  if (m_pQtAction == nullptr)
    return;

  auto pAction = static_cast<wdSliderAction*>(m_pAction);

  const wdActionDescriptor* pDesc = m_pAction->GetDescriptorHandle().GetDescriptor();

  wdQtSliderWidgetAction* pSliderAction = qobject_cast<wdQtSliderWidgetAction*>(m_pQtAction);
  wdQtScopedBlockSignals bs(pSliderAction);

  wdInt32 minVal, maxVal;
  pAction->GetRange(minVal, maxVal);
  pSliderAction->setMinimum(minVal);
  pSliderAction->setMaximum(maxVal);
  pSliderAction->setValue(pAction->GetValue());
  pSliderAction->setText(wdTranslate(pAction->GetName()));
  pSliderAction->setToolTip(wdTranslateTooltip(pAction->GetName()));
  pSliderAction->setEnabled(pAction->IsEnabled());
  pSliderAction->setVisible(pAction->IsVisible());
}

void wdQtSliderProxy::SetAction(wdAction* pAction)
{
  WD_ASSERT_DEV(m_pAction == nullptr, "Es darf nicht sein, es kann nicht sein!");

  wdQtProxy::SetAction(pAction);
  m_pAction->m_StatusUpdateEvent.AddEventHandler(wdMakeDelegate(&wdQtSliderProxy::StatusUpdateEventHandler, this));

  wdActionDescriptorHandle hDesc = m_pAction->GetDescriptorHandle();
  const wdActionDescriptor* pDesc = hDesc.GetDescriptor();

  if (m_pQtAction == nullptr)
  {
    m_pQtAction = new wdQtSliderWidgetAction(nullptr);

    WD_VERIFY(connect(m_pQtAction, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int))) != nullptr, "connection failed");
  }

  Update();
}

QAction* wdQtSliderProxy::GetQAction()
{
  return m_pQtAction;
}


void wdQtSliderProxy::OnValueChanged(int value)
{
  // make sure all focus is lost, to trigger pending changes
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  if (pFocusWidget)
    QApplication::focusWidget()->clearFocus();

  // make sure all instances of the slider get updated, by setting the new value
  m_pQtAction->setValue(value);
  m_pAction->Execute(value);

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

void wdQtSliderProxy::StatusUpdateEventHandler(wdAction* pAction)
{
  Update();
}
