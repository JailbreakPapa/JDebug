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

nsRttiMappedObjectFactory<nsQtProxy> nsQtProxy::s_Factory;
nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>> nsQtProxy::s_GlobalActions;
nsMap<const nsDocument*, nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>>> nsQtProxy::s_DocumentActions;
nsMap<QWidget*, nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>>> nsQtProxy::s_WindowActions;
QObject* nsQtProxy::s_pSignalProxy = nullptr;

static nsQtProxy* QtMenuProxyCreator(const nsRTTI* pRtti)
{
  return new (nsQtMenuProxy);
}

static nsQtProxy* QtCategoryProxyCreator(const nsRTTI* pRtti)
{
  return new (nsQtCategoryProxy);
}

static nsQtProxy* QtButtonProxyCreator(const nsRTTI* pRtti)
{
  return new (nsQtButtonProxy);
}

static nsQtProxy* QtDynamicMenuProxyCreator(const nsRTTI* pRtti)
{
  return new (nsQtDynamicMenuProxy);
}

static nsQtProxy* QtDynamicActionAndMenuProxyCreator(const nsRTTI* pRtti)
{
  return new (nsQtDynamicActionAndMenuProxy);
}

static nsQtProxy* QtSliderProxyCreator(const nsRTTI* pRtti)
{
  return new (nsQtSliderProxy);
}

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, QtProxies)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "ToolsFoundation",
  "ActionManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsQtProxy::GetFactory().RegisterCreator(nsGetStaticRTTI<nsMenuAction>(), QtMenuProxyCreator);
    nsQtProxy::GetFactory().RegisterCreator(nsGetStaticRTTI<nsCategoryAction>(), QtCategoryProxyCreator);
    nsQtProxy::GetFactory().RegisterCreator(nsGetStaticRTTI<nsDynamicMenuAction>(), QtDynamicMenuProxyCreator);
    nsQtProxy::GetFactory().RegisterCreator(nsGetStaticRTTI<nsDynamicActionAndMenuAction>(), QtDynamicActionAndMenuProxyCreator);
    nsQtProxy::GetFactory().RegisterCreator(nsGetStaticRTTI<nsButtonAction>(), QtButtonProxyCreator);
    nsQtProxy::GetFactory().RegisterCreator(nsGetStaticRTTI<nsSliderAction>(), QtSliderProxyCreator);
    nsQtProxy::s_pSignalProxy = new QObject;
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsQtProxy::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsMenuAction>());
    nsQtProxy::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsCategoryAction>());
    nsQtProxy::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsDynamicMenuAction>());
    nsQtProxy::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsDynamicActionAndMenuAction>());
    nsQtProxy::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsButtonAction>());
    nsQtProxy::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsSliderAction>());
    nsQtProxy::s_GlobalActions.Clear();
    nsQtProxy::s_DocumentActions.Clear();
    nsQtProxy::s_WindowActions.Clear();
    delete nsQtProxy::s_pSignalProxy;
    nsQtProxy::s_pSignalProxy = nullptr;
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

bool nsQtProxy::TriggerDocumentAction(nsDocument* pDocument, QKeyEvent* pEvent, bool bTestOnly)
{
  auto CheckActions = [&](QKeyEvent* pEvent, nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>>& ref_actions) -> bool
  {
    for (auto weakActionProxy : ref_actions)
    {
      if (auto pProxy = weakActionProxy.Value().toStrongRef())
      {
        QAction* pQAction = nullptr;
        if (auto pActionProxy = qobject_cast<nsQtActionProxy*>(pProxy))
        {
          pQAction = pActionProxy->GetQAction();
        }
        else if (auto pActionProxy2 = qobject_cast<nsQtDynamicActionAndMenuProxy*>(pProxy))
        {
          pQAction = pActionProxy2->GetQAction();
        }

        if (pQAction)
        {
          QKeySequence ks = pQAction->shortcut();
          if (pQAction->isEnabled() && QKeySequence(pEvent->key() | pEvent->modifiers()) == ks)
          {
            if (!bTestOnly)
            {
              pQAction->trigger();
            }
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
    nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>>& actions = s_DocumentActions[pDocument];
    if (CheckActions(pEvent, actions))
      return true;
  }
  return CheckActions(pEvent, s_GlobalActions);
}

nsRttiMappedObjectFactory<nsQtProxy>& nsQtProxy::GetFactory()
{
  return s_Factory;
}
QSharedPointer<nsQtProxy> nsQtProxy::GetProxy(nsActionContext& ref_context, nsActionDescriptorHandle hDesc)
{
  QSharedPointer<nsQtProxy> pProxy;
  const nsActionDescriptor* pDesc = hDesc.GetDescriptor();
  if (pDesc->m_Type != nsActionType::Action && pDesc->m_Type != nsActionType::ActionAndMenu)
  {
    auto pAction = pDesc->CreateAction(ref_context);
    pProxy = QSharedPointer<nsQtProxy>(nsQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
    NS_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
    pProxy->SetAction(pAction);
    NS_ASSERT_DEV(pProxy->GetAction()->GetContext().m_pDocument == ref_context.m_pDocument, "invalid document pointer");
    return pProxy;
  }

  // nsActionType::Action will be cached to ensure only one QAction exist in its scope to prevent shortcut collisions.
  switch (pDesc->m_Scope)
  {
    case nsActionScope::Global:
    {
      QWeakPointer<nsQtProxy> pTemp = s_GlobalActions[hDesc];
      if (pTemp.isNull())
      {
        auto pAction = pDesc->CreateAction(ref_context);
        pProxy = QSharedPointer<nsQtProxy>(nsQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
        NS_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
        pProxy->SetAction(pAction);
        s_GlobalActions[hDesc] = pProxy.toWeakRef();
      }
      else
      {
        pProxy = pTemp.toStrongRef();
      }

      break;
    }

    case nsActionScope::Document:
    {
      const nsDocument* pDocument = ref_context.m_pDocument; // may be null

      QWeakPointer<nsQtProxy> pTemp = s_DocumentActions[pDocument][hDesc];
      if (pTemp.isNull())
      {
        auto pAction = pDesc->CreateAction(ref_context);
        pProxy = QSharedPointer<nsQtProxy>(nsQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
        NS_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
        pProxy->SetAction(pAction);
        s_DocumentActions[pDocument][hDesc] = pProxy;
      }
      else
      {
        pProxy = pTemp.toStrongRef();
      }

      break;
    }

    case nsActionScope::Window:
    {
      bool bExisted = true;
      auto it = s_WindowActions.FindOrAdd(ref_context.m_pWindow, &bExisted);
      if (!bExisted)
      {
        s_pSignalProxy->connect(ref_context.m_pWindow, &QObject::destroyed, s_pSignalProxy, [ref_context]()
          { s_WindowActions.Remove(ref_context.m_pWindow); });
      }
      QWeakPointer<nsQtProxy> pTemp = it.Value()[hDesc];
      if (pTemp.isNull())
      {
        auto pAction = pDesc->CreateAction(ref_context);
        pProxy = QSharedPointer<nsQtProxy>(nsQtProxy::GetFactory().CreateObject(pAction->GetDynamicRTTI()));
        NS_ASSERT_DEBUG(pProxy != nullptr, "No proxy assigned to action '{0}'", pDesc->m_sActionName);
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
    // if this assert fires, you might have tried to map an action into multiple documents, which uses nsActionScope::Global
    nsAction* pAction = pProxy->GetAction();
    const nsActionContext& ctxt = pAction->GetContext();
    nsDocument* pDoc = ctxt.m_pDocument;
    NS_ASSERT_DEV(pDoc == ref_context.m_pDocument, "invalid document pointer");
  }
  return pProxy;
}

nsQtProxy::nsQtProxy()
{
  m_pAction = nullptr;
}

nsQtProxy::~nsQtProxy()
{
  if (m_pAction != nullptr)
    nsActionManager::GetActionDescriptor(m_pAction->GetDescriptorHandle())->DeleteAction(m_pAction);
}

void nsQtProxy::SetAction(nsAction* pAction)
{
  m_pAction = pAction;
}

//////////////////// nsQtMenuProxy /////////////////////

nsQtMenuProxy::nsQtMenuProxy()
{
  m_pMenu = nullptr;
}

nsQtMenuProxy::~nsQtMenuProxy()
{
  m_pMenu->deleteLater();
  delete m_pMenu;
}

void nsQtMenuProxy::Update()
{
  auto pMenu = static_cast<nsMenuAction*>(m_pAction);

  m_pMenu->setIcon(nsQtUiServices::GetCachedIconResource(pMenu->GetIconPath()));
  m_pMenu->setTitle(nsMakeQString(nsTranslate(pMenu->GetName())));
}

void nsQtMenuProxy::SetAction(nsAction* pAction)
{
  nsQtProxy::SetAction(pAction);

  m_pMenu = new QMenu();
  m_pMenu->setToolTipsVisible(true);
  Update();
}

QMenu* nsQtMenuProxy::GetQMenu()
{
  return m_pMenu;
}

//////////////////////////////////////////////////////////////////////////
//////////////////// nsQtButtonProxy /////////////////////
//////////////////////////////////////////////////////////////////////////

nsQtButtonProxy::nsQtButtonProxy()
{
  m_pQtAction = nullptr;
}

nsQtButtonProxy::~nsQtButtonProxy()
{
  m_pAction->m_StatusUpdateEvent.RemoveEventHandler(nsMakeDelegate(&nsQtButtonProxy::StatusUpdateEventHandler, this));

  if (m_pQtAction != nullptr)
  {
    m_pQtAction->deleteLater();
  }
  m_pQtAction = nullptr;
}

void nsQtButtonProxy::Update()
{
  if (m_pQtAction == nullptr)
    return;

  auto pButton = static_cast<nsButtonAction*>(m_pAction);


  const nsActionDescriptor* pDesc = m_pAction->GetDescriptorHandle().GetDescriptor();
  m_pQtAction->setShortcut(QKeySequence(QString::fromUtf8(pDesc->m_sShortcut.GetData())));

  const QString sDisplayShortcut = m_pQtAction->shortcut().toString(QKeySequence::NativeText);
  QString sTooltip = nsMakeQString(nsTranslateTooltip(pButton->GetName()));

  nsStringBuilder sDisplay = nsTranslate(pButton->GetName());

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

  if (!pButton->GetAdditionalDisplayString().IsEmpty())
    sDisplay.Append(" '", pButton->GetAdditionalDisplayString(), "'"); // TODO: translate this as well?

  m_pQtAction->setIcon(nsQtUiServices::GetCachedIconResource(pButton->GetIconPath()));
  m_pQtAction->setText(QString::fromUtf8(sDisplay.GetData()));
  m_pQtAction->setToolTip(sTooltip);
  m_pQtAction->setCheckable(pButton->IsCheckable());
  m_pQtAction->setChecked(pButton->IsChecked());
  m_pQtAction->setEnabled(pButton->IsEnabled());
  m_pQtAction->setVisible(pButton->IsVisible());
}


void SetupQAction(nsAction* pAction, QPointer<QAction>& ref_pQtAction, QObject* pTarget)
{
  nsActionDescriptorHandle hDesc = pAction->GetDescriptorHandle();
  const nsActionDescriptor* pDesc = hDesc.GetDescriptor();

  if (ref_pQtAction == nullptr)
  {
    ref_pQtAction = new QAction(nullptr);
    NS_VERIFY(QObject::connect(ref_pQtAction, SIGNAL(triggered(bool)), pTarget, SLOT(OnTriggered())) != nullptr, "connection failed");

    switch (pDesc->m_Scope)
    {
      case nsActionScope::Global:
      {
        // Parent is null so the global actions don't get deleted.
        ref_pQtAction->setShortcutContext(Qt::ShortcutContext::ApplicationShortcut);
      }
      break;
      case nsActionScope::Document:
      {
        // Parent is set to the window belonging to the document.
        nsQtDocumentWindow* pWindow = nsQtDocumentWindow::FindWindowByDocument(pAction->GetContext().m_pDocument);
        NS_ASSERT_DEBUG(pWindow != nullptr, "You can't map a nsActionScope::Document action without that document existing!");
        ref_pQtAction->setParent(pWindow);
        ref_pQtAction->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
      }
      break;
      case nsActionScope::Window:
      {
        ref_pQtAction->setParent(pAction->GetContext().m_pWindow);
        ref_pQtAction->setShortcutContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
      }
      break;
    }
  }
}

void nsQtButtonProxy::SetAction(nsAction* pAction)
{
  NS_ASSERT_DEV(m_pAction == nullptr, "Es darf nicht sein, es kann nicht sein!");

  nsQtProxy::SetAction(pAction);
  m_pAction->m_StatusUpdateEvent.AddEventHandler(nsMakeDelegate(&nsQtButtonProxy::StatusUpdateEventHandler, this));

  SetupQAction(m_pAction, m_pQtAction, this);

  Update();
}

QAction* nsQtButtonProxy::GetQAction()
{
  return m_pQtAction;
}

void nsQtButtonProxy::StatusUpdateEventHandler(nsAction* pAction)
{
  Update();
}

void nsQtButtonProxy::OnTriggered()
{
  // make sure all focus is lost, to trigger pending changes
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  if (pFocusWidget)
    QApplication::focusWidget()->clearFocus();

  m_pAction->Execute(m_pQtAction->isChecked());

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

void nsQtDynamicMenuProxy::SetAction(nsAction* pAction)
{
  nsQtMenuProxy::SetAction(pAction);

  NS_VERIFY(connect(m_pMenu, SIGNAL(aboutToShow()), this, SLOT(SlotMenuAboutToShow())) != nullptr, "signal/slot connection failed");
}

void nsQtDynamicMenuProxy::SlotMenuAboutToShow()
{
  m_pMenu->clear();

  static_cast<nsDynamicMenuAction*>(m_pAction)->GetEntries(m_Entries);

  if (m_Entries.IsEmpty())
  {
    m_pMenu->addAction("<empty>")->setEnabled(false);
  }
  else
  {
    for (nsUInt32 i = 0; i < m_Entries.GetCount(); ++i)
    {
      const auto& p = m_Entries[i];

      if (p.m_ItemFlags.IsSet(nsDynamicMenuAction::Item::ItemFlags::Separator))
      {
        m_pMenu->addSeparator();
      }
      else
      {
        auto pAction = m_pMenu->addAction(QString::fromUtf8(p.m_sDisplay.GetData()));
        pAction->setData(i);
        pAction->setIcon(p.m_Icon);
        pAction->setCheckable(p.m_CheckState != nsDynamicMenuAction::Item::CheckMark::NotCheckable);
        pAction->setChecked(p.m_CheckState == nsDynamicMenuAction::Item::CheckMark::Checked);

        NS_VERIFY(connect(pAction, SIGNAL(triggered()), this, SLOT(SlotMenuEntryTriggered())) != nullptr, "signal/slot connection failed");
      }
    }
  }
}

void nsQtDynamicMenuProxy::SlotMenuEntryTriggered()
{
  QAction* pAction = qobject_cast<QAction*>(sender());
  if (!pAction)
    return;

  // make sure all focus is lost, to trigger pending changes
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  if (pFocusWidget)
    QApplication::focusWidget()->clearFocus();

  nsUInt32 index = pAction->data().toUInt();
  m_pAction->Execute(m_Entries[index].m_UserValue);

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

//////////////////////////////////////////////////////////////////////////
//////////////////// nsQtDynamicActionAndMenuProxy /////////////////////
//////////////////////////////////////////////////////////////////////////

nsQtDynamicActionAndMenuProxy::nsQtDynamicActionAndMenuProxy()
{
  m_pQtAction = nullptr;
}

nsQtDynamicActionAndMenuProxy::~nsQtDynamicActionAndMenuProxy()
{
  m_pAction->m_StatusUpdateEvent.RemoveEventHandler(nsMakeDelegate(&nsQtDynamicActionAndMenuProxy::StatusUpdateEventHandler, this));

  if (m_pQtAction != nullptr)
  {
    m_pQtAction->deleteLater();
  }
  m_pQtAction = nullptr;
}


void nsQtDynamicActionAndMenuProxy::Update()
{
  nsQtDynamicMenuProxy::Update();

  if (m_pQtAction == nullptr)
    return;

  auto pButton = static_cast<nsDynamicActionAndMenuAction*>(m_pAction);

  const nsActionDescriptor* pDesc = m_pAction->GetDescriptorHandle().GetDescriptor();
  m_pQtAction->setShortcut(QKeySequence(QString::fromUtf8(pDesc->m_sShortcut.GetData())));

  nsStringBuilder sDisplay = nsTranslate(pButton->GetName());

  if (!pButton->GetAdditionalDisplayString().IsEmpty())
    sDisplay.Append(" '", pButton->GetAdditionalDisplayString(), "'"); // TODO: translate this as well?

  const QString sDisplayShortcut = m_pQtAction->shortcut().toString(QKeySequence::NativeText);
  QString sTooltip = nsMakeQString(nsTranslateTooltip(pButton->GetName()));

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

  m_pQtAction->setIcon(nsQtUiServices::GetCachedIconResource(pButton->GetIconPath()));
  m_pQtAction->setText(QString::fromUtf8(sDisplay.GetData()));
  m_pQtAction->setToolTip(sTooltip);
  m_pQtAction->setEnabled(pButton->IsEnabled());
  m_pQtAction->setVisible(pButton->IsVisible());
}


void nsQtDynamicActionAndMenuProxy::SetAction(nsAction* pAction)
{
  nsQtDynamicMenuProxy::SetAction(pAction);

  m_pAction->m_StatusUpdateEvent.AddEventHandler(nsMakeDelegate(&nsQtDynamicActionAndMenuProxy::StatusUpdateEventHandler, this));

  SetupQAction(m_pAction, m_pQtAction, this);

  Update();
}

QAction* nsQtDynamicActionAndMenuProxy::GetQAction()
{
  return m_pQtAction;
}

void nsQtDynamicActionAndMenuProxy::OnTriggered()
{
  // make sure all focus is lost, to trigger pending changes
  QPointer<QWidget> pFocusWidget = QApplication::focusWidget();
  if (pFocusWidget)
    QApplication::focusWidget()->clearFocus();

  m_pAction->Execute(nsVariant());

  if (pFocusWidget)
    pFocusWidget->setFocus();
}

void nsQtDynamicActionAndMenuProxy::StatusUpdateEventHandler(nsAction* pAction)
{
  Update();
}


//////////////////////////////////////////////////////////////////////////
//////////////////// nsQtSliderProxy /////////////////////
//////////////////////////////////////////////////////////////////////////

nsQtSliderWidgetAction::nsQtSliderWidgetAction(QWidget* pParent)
  : QWidgetAction(pParent)
{
}

nsQtLabeledSlider::nsQtLabeledSlider(QWidget* pParent)
  : QWidget(pParent)
{
  m_pLabel = new QLabel(this);
  m_pSlider = new QSlider(this);
  setLayout(new QHBoxLayout(this));

  layout()->addWidget(m_pLabel);
  layout()->addWidget(m_pSlider);

  setMaximumWidth(300);
}

void nsQtSliderWidgetAction::setMinimum(int value)
{
  m_iMinimum = value;

  const QList<QWidget*> widgets = createdWidgets();

  for (QWidget* pWidget : widgets)
  {
    nsQtLabeledSlider* pGroup = qobject_cast<nsQtLabeledSlider*>(pWidget);
    pGroup->m_pSlider->setMinimum(m_iMinimum);
  }
}

void nsQtSliderWidgetAction::setMaximum(int value)
{
  m_iMaximum = value;

  const QList<QWidget*> widgets = createdWidgets();

  for (QWidget* pWidget : widgets)
  {
    nsQtLabeledSlider* pGroup = qobject_cast<nsQtLabeledSlider*>(pWidget);
    pGroup->m_pSlider->setMaximum(m_iMaximum);
  }
}

void nsQtSliderWidgetAction::setValue(int value)
{
  m_iValue = value;

  const QList<QWidget*> widgets = createdWidgets();

  for (QWidget* pWidget : widgets)
  {
    nsQtLabeledSlider* pGroup = qobject_cast<nsQtLabeledSlider*>(pWidget);
    pGroup->m_pSlider->setValue(m_iValue);
  }
}

void nsQtSliderWidgetAction::OnValueChanged(int value)
{
  Q_EMIT valueChanged(value);
}

QWidget* nsQtSliderWidgetAction::createWidget(QWidget* parent)
{
  nsQtLabeledSlider* pGroup = new nsQtLabeledSlider(parent);
  pGroup->m_pSlider->setOrientation(Qt::Orientation::Horizontal);

  NS_VERIFY(connect(pGroup->m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int))) != nullptr, "connection failed");

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

bool nsQtSliderWidgetAction::eventFilter(QObject* obj, QEvent* e)
{
  if (e->type() == QEvent::Type::MouseButtonPress || e->type() == QEvent::Type::MouseButtonRelease || e->type() == QEvent::Type::MouseButtonDblClick)
  {
    e->accept();
    return true;
  }

  return false;
}

nsQtSliderProxy::nsQtSliderProxy()
{
  m_pQtAction = nullptr;
}

nsQtSliderProxy::~nsQtSliderProxy()
{
  m_pAction->m_StatusUpdateEvent.RemoveEventHandler(nsMakeDelegate(&nsQtSliderProxy::StatusUpdateEventHandler, this));

  if (m_pQtAction != nullptr)
  {
    m_pQtAction->deleteLater();
  }
  m_pQtAction = nullptr;
}

void nsQtSliderProxy::Update()
{
  if (m_pQtAction == nullptr)
    return;

  auto pAction = static_cast<nsSliderAction*>(m_pAction);

  const nsActionDescriptor* pDesc = m_pAction->GetDescriptorHandle().GetDescriptor();

  nsQtSliderWidgetAction* pSliderAction = qobject_cast<nsQtSliderWidgetAction*>(m_pQtAction);
  nsQtScopedBlockSignals bs(pSliderAction);

  nsInt32 minVal, maxVal;
  pAction->GetRange(minVal, maxVal);
  pSliderAction->setMinimum(minVal);
  pSliderAction->setMaximum(maxVal);
  pSliderAction->setValue(pAction->GetValue());
  pSliderAction->setText(nsMakeQString(nsTranslate(pAction->GetName())));
  pSliderAction->setToolTip(nsMakeQString(nsTranslateTooltip(pAction->GetName())));
  pSliderAction->setEnabled(pAction->IsEnabled());
  pSliderAction->setVisible(pAction->IsVisible());
}

void nsQtSliderProxy::SetAction(nsAction* pAction)
{
  NS_ASSERT_DEV(m_pAction == nullptr, "Es darf nicht sein, es kann nicht sein!");

  nsQtProxy::SetAction(pAction);
  m_pAction->m_StatusUpdateEvent.AddEventHandler(nsMakeDelegate(&nsQtSliderProxy::StatusUpdateEventHandler, this));

  nsActionDescriptorHandle hDesc = m_pAction->GetDescriptorHandle();
  const nsActionDescriptor* pDesc = hDesc.GetDescriptor();

  if (m_pQtAction == nullptr)
  {
    m_pQtAction = new nsQtSliderWidgetAction(nullptr);

    NS_VERIFY(connect(m_pQtAction, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged(int))) != nullptr, "connection failed");
  }

  Update();
}

QAction* nsQtSliderProxy::GetQAction()
{
  return m_pQtAction;
}


void nsQtSliderProxy::OnValueChanged(int value)
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

void nsQtSliderProxy::StatusUpdateEventHandler(nsAction* pAction)
{
  Update();
}
