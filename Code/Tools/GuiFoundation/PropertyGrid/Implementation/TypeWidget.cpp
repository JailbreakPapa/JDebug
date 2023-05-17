#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Reflection/Implementation/PropertyAttributes.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <Foundation/Types/Variant.h>
#include <GuiFoundation/PropertyGrid/DefaultState.h>
#include <GuiFoundation/PropertyGrid/Implementation/ManipulatorLabel.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/PropertyWidget.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/TypeWidget.moc.h>
#include <GuiFoundation/PropertyGrid/ManipulatorManager.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/CollapsibleGroupBox.moc.h>
#include <GuiFoundation/Widgets/GroupBoxBase.moc.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

#include <QGridLayout>
#include <QLabel>


wdQtTypeWidget::wdQtTypeWidget(QWidget* pParent, wdQtPropertyGridWidget* pGrid, wdObjectAccessorBase* pObjectAccessor, const wdRTTI* pType,
  const char* szIncludeProperties, const char* szExcludeProperties)
  : QWidget(pParent)
  , m_pGrid(pGrid)
  , m_pObjectAccessor(pObjectAccessor)
  , m_pType(pType)
{
  WD_ASSERT_DEBUG(m_pGrid && m_pObjectAccessor && m_pType, "");
  m_Pal = palette();
  setAutoFillBackground(true);

  m_pLayout = new QGridLayout(this);
  m_pLayout->setColumnStretch(0, 1);
  m_pLayout->setColumnStretch(1, 0);
  m_pLayout->setColumnMinimumWidth(1, 5);
  m_pLayout->setColumnStretch(2, 2);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  m_pLayout->setSpacing(0);
  setLayout(m_pLayout);

  m_pGrid->GetObjectManager()->m_PropertyEvents.AddEventHandler(wdMakeDelegate(&wdQtTypeWidget::PropertyEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.AddEventHandler(wdMakeDelegate(&wdQtTypeWidget::CommandHistoryEventHandler, this));
  wdManipulatorManager::GetSingleton()->m_Events.AddEventHandler(wdMakeDelegate(&wdQtTypeWidget::ManipulatorManagerEventHandler, this));

  BuildUI(pType, szIncludeProperties, szExcludeProperties);
}

wdQtTypeWidget::~wdQtTypeWidget()
{
  m_pGrid->GetObjectManager()->m_PropertyEvents.RemoveEventHandler(wdMakeDelegate(&wdQtTypeWidget::PropertyEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtTypeWidget::CommandHistoryEventHandler, this));
  wdManipulatorManager::GetSingleton()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtTypeWidget::ManipulatorManagerEventHandler, this));
}

void wdQtTypeWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtScopedUpdatesDisabled _(this);

  m_Items = items;

  UpdatePropertyMetaState();

  for (auto it = m_PropertyWidgets.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_pWidget->SetSelection(m_Items);

    if (it.Value().m_pLabel)
    {
      it.Value().m_pLabel->SetSelection(m_Items);
    }
  }

  for (auto it = m_PropertyWidgets.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_pLabel)
      it.Value().m_pLabel->SetSelection(m_Items);
  }

  wdManipulatorManagerEvent e;
  e.m_pDocument = m_pGrid->GetDocument();
  e.m_pManipulator = wdManipulatorManager::GetSingleton()->GetActiveManipulator(e.m_pDocument, e.m_pSelection);
  e.m_bHideManipulators = false; // irrelevant for this
  ManipulatorManagerEventHandler(e);
}


void wdQtTypeWidget::PrepareToDie()
{
  if (!m_bUndead)
  {
    m_bUndead = true;
    for (auto it = m_PropertyWidgets.GetIterator(); it.IsValid(); ++it)
    {
      it.Value().m_pWidget->PrepareToDie();
    }
  }
}

void wdQtTypeWidget::BuildUI(const wdRTTI* pType, const wdMap<wdString, const wdManipulatorAttribute*>& manipulatorMap,
  const wdDynamicArray<wdUniquePtr<PropertyGroup>>& groups, const char* szIncludeProperties, const char* szExcludeProperties)
{
  wdQtScopedUpdatesDisabled _(this);

  for (wdUInt32 p = 0; p < groups.GetCount(); p++)
  {
    const wdUniquePtr<PropertyGroup>& group = groups[p];

    wdQtCollapsibleGroupBox* pGroupBox = new wdQtCollapsibleGroupBox(this);
    pGroupBox->setContentsMargins(0, 0, 0, 0);
    pGroupBox->layout()->setSpacing(0);
    if (group->m_sGroup.IsEmpty())
    {
      pGroupBox->GetHeader()->hide();
    }
    else
    {
      pGroupBox->SetTitle(group->m_sGroup.GetData());
      pGroupBox->SetBoldTitle(true);

      m_pGrid->SetCollapseState(pGroupBox);
      connect(pGroupBox, &wdQtGroupBoxBase::CollapseStateChanged, m_pGrid, &wdQtPropertyGridWidget::OnCollapseStateChanged);

      if (!group->m_sIconName.IsEmpty())
      {
        wdStringBuilder sIcon(":/GroupIcons/", group->m_sIconName, ".png");
        pGroupBox->SetIcon(wdQtUiServices::GetCachedIconResource(sIcon));
      }
    }
    QGridLayout* pLayout = new QGridLayout();
    pLayout->setColumnStretch(0, 1);
    pLayout->setColumnStretch(1, 0);
    pLayout->setColumnMinimumWidth(1, 5);
    pLayout->setColumnStretch(2, 2);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pLayout->setSpacing(0);
    pGroupBox->GetContent()->setLayout(pLayout);

    for (wdUInt32 i = 0; i < group->m_Properties.GetCount(); ++i)
    {
      const wdAbstractProperty* pProp = group->m_Properties[i];

      wdQtPropertyWidget* pNewWidget = wdQtPropertyGridWidget::CreatePropertyWidget(pProp);
      WD_ASSERT_DEV(pNewWidget != nullptr, "No property editor defined for '{0}'", pProp->GetPropertyName());
      pNewWidget->setParent(this);
      pNewWidget->Init(m_pGrid, m_pObjectAccessor, pType, pProp);
      auto& ref = m_PropertyWidgets[pProp->GetPropertyName()];

      ref.m_pWidget = pNewWidget;
      ref.m_pLabel = nullptr;

      if (pNewWidget->HasLabel())
      {
        wdStringBuilder tmp;
        wdQtManipulatorLabel* pLabel = new wdQtManipulatorLabel(this);
        pLabel->setText(QString::fromUtf8(pNewWidget->GetLabel(tmp)));
        pLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        pLabel->setContentsMargins(0, 0, 0, 0); // 18 is a hacked value to align label with group boxes.
        pLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

        connect(pLabel, &QWidget::customContextMenuRequested, pNewWidget, &wdQtPropertyWidget::OnCustomContextMenu);

        pLayout->addWidget(pLabel, i, 0, 1, 1);
        pLayout->addWidget(pNewWidget, i, 2, 1, 1);

        auto itManip = manipulatorMap.Find(pProp->GetPropertyName());
        if (itManip.IsValid())
        {
          pLabel->SetManipulator(itManip.Value());
        }

        ref.m_pLabel = pLabel;
        ref.m_sOriginalLabelText = pNewWidget->GetLabel(tmp);
      }
      else
      {
        pLayout->addWidget(pNewWidget, i, 0, 1, 3);
      }
    }
    if (p != groups.GetCount() - 1)
    {
      pLayout->addItem(new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed), group->m_Properties.GetCount(), 0, 1, 3);
    }
    wdUInt32 iRows = m_pLayout->rowCount();
    m_pLayout->addWidget(pGroupBox, iRows, 0, 1, 3);
  }
}

void wdQtTypeWidget::BuildUI(const wdRTTI* pType, const char* szIncludeProperties, const char* szExcludeProperties)
{
  wdMap<wdString, const wdManipulatorAttribute*> manipulatorMap;
  wdHybridArray<wdUniquePtr<PropertyGroup>, 6> groups;
  PropertyGroup* pCurrentGroup = nullptr;
  float fOrder = -1.0f;

  auto AddProperty = [&](const wdAbstractProperty* pProp) {
    const wdGroupAttribute* pGroup = pProp->GetAttributeByType<wdGroupAttribute>();
    if (pGroup != nullptr)
    {
      wdUniquePtr<PropertyGroup>* pFound =
        std::find_if(begin(groups), end(groups), [&](const wdUniquePtr<PropertyGroup>& g) { return g->m_sGroup == pGroup->GetGroup(); });
      if (pFound != end(groups))
      {
        pCurrentGroup = pFound->Borrow();
        pCurrentGroup->MergeGroup(pGroup);
      }
      else
      {
        wdUniquePtr<PropertyGroup> group = WD_DEFAULT_NEW(PropertyGroup, pGroup, fOrder);
        pCurrentGroup = group.Borrow();
        groups.PushBack(std::move(group));
      }
    }
    if (pCurrentGroup == nullptr)
    {
      wdUniquePtr<PropertyGroup>* pFound =
        std::find_if(begin(groups), end(groups), [&](const wdUniquePtr<PropertyGroup>& g) { return g->m_sGroup.IsEmpty(); });
      if (pFound != end(groups))
      {
        pCurrentGroup = pFound->Borrow();
      }
      else
      {
        wdUniquePtr<PropertyGroup> group = WD_DEFAULT_NEW(PropertyGroup, nullptr, fOrder);
        pCurrentGroup = group.Borrow();
        groups.PushBack(std::move(group));
      }
    }

    pCurrentGroup->m_Properties.PushBack(pProp);
  };

  // Build type hierarchy array.
  wdHybridArray<const wdRTTI*, 6> typeHierarchy;
  const wdRTTI* pParentType = pType;
  while (pParentType != nullptr)
  {
    typeHierarchy.PushBack(pParentType);
    pParentType = pParentType->GetParentType();
  }

  // Build UI starting from base class.
  for (wdInt32 i = (wdInt32)typeHierarchy.GetCount() - 1; i >= 0; --i)
  {
    const wdRTTI* pCurrentType = typeHierarchy[i];
    const auto& attr = pCurrentType->GetAttributes();

    // Traverse type attributes
    for (wdPropertyAttribute* pAttr : attr)
    {
      if (pAttr->GetDynamicRTTI()->IsDerivedFrom<wdManipulatorAttribute>())
      {
        const wdManipulatorAttribute* pManipAttr = static_cast<const wdManipulatorAttribute*>(pAttr);

        if (!pManipAttr->m_sProperty1.IsEmpty())
          manipulatorMap[pManipAttr->m_sProperty1] = pManipAttr;
        if (!pManipAttr->m_sProperty2.IsEmpty())
          manipulatorMap[pManipAttr->m_sProperty2] = pManipAttr;
        if (!pManipAttr->m_sProperty3.IsEmpty())
          manipulatorMap[pManipAttr->m_sProperty3] = pManipAttr;
        if (!pManipAttr->m_sProperty4.IsEmpty())
          manipulatorMap[pManipAttr->m_sProperty4] = pManipAttr;
        if (!pManipAttr->m_sProperty5.IsEmpty())
          manipulatorMap[pManipAttr->m_sProperty5] = pManipAttr;
        if (!pManipAttr->m_sProperty6.IsEmpty())
          manipulatorMap[pManipAttr->m_sProperty6] = pManipAttr;
      }
    }

    // Traverse properties
    for (wdUInt32 j = 0; j < pCurrentType->GetProperties().GetCount(); ++j)
    {
      const wdAbstractProperty* pProp = pCurrentType->GetProperties()[j];

      if (pProp->GetFlags().IsSet(wdPropertyFlags::Hidden))
        continue;

      if (pProp->GetAttributeByType<wdHiddenAttribute>() != nullptr)
        continue;

      if (pProp->GetSpecificType()->GetAttributeByType<wdHiddenAttribute>() != nullptr)
        continue;

      if (pProp->GetCategory() == wdPropertyCategory::Constant)
        continue;

      if (!wdStringUtils::IsNullOrEmpty(szIncludeProperties) &&
          wdStringUtils::FindSubString(szIncludeProperties, pProp->GetPropertyName()) == nullptr)
        continue;

      if (!wdStringUtils::IsNullOrEmpty(szExcludeProperties) &&
          wdStringUtils::FindSubString(szExcludeProperties, pProp->GetPropertyName()) != nullptr)
        continue;

      AddProperty(pProp);
    }

    // Groups should not be inherited by derived class properties
    pCurrentGroup = nullptr;
  }

  groups.Sort([](const wdUniquePtr<PropertyGroup>& lhs, const wdUniquePtr<PropertyGroup>& rhs) -> bool { return lhs->m_fOrder < rhs->m_fOrder; });

  BuildUI(pType, manipulatorMap, groups, szIncludeProperties, szExcludeProperties);
}

void wdQtTypeWidget::PropertyEventHandler(const wdDocumentObjectPropertyEvent& e)
{
  if (m_bUndead)
    return;

  UpdateProperty(e.m_pObject, e.m_sProperty);
}

void wdQtTypeWidget::CommandHistoryEventHandler(const wdCommandHistoryEvent& e)
{
  if (m_bUndead)
    return;

  switch (e.m_Type)
  {
    case wdCommandHistoryEvent::Type::UndoEnded:
    case wdCommandHistoryEvent::Type::RedoEnded:
    case wdCommandHistoryEvent::Type::TransactionEnded:
    case wdCommandHistoryEvent::Type::TransactionCanceled:
    {
      FlushQueuedChanges();
    }
    break;

    default:
      break;
  }
}


void wdQtTypeWidget::ManipulatorManagerEventHandler(const wdManipulatorManagerEvent& e)
{
  if (m_bUndead)
    return;

  if (m_pGrid->GetDocument() != e.m_pDocument)
    return;

  const bool bActiveOnThis = (e.m_pSelection != nullptr) && (m_Items == *e.m_pSelection);

  for (auto it = m_PropertyWidgets.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_pLabel)
    {
      if (bActiveOnThis && e.m_pManipulator == it.Value().m_pLabel->GetManipulator())
      {
        it.Value().m_pLabel->SetManipulatorActive(true);
      }
      else
      {
        it.Value().m_pLabel->SetManipulatorActive(false);
      }
    }
  }
}

void wdQtTypeWidget::UpdateProperty(const wdDocumentObject* pObject, const wdString& sProperty)
{
  if (std::none_of(cbegin(m_Items), cend(m_Items), [=](const wdPropertySelection& sel) { return pObject == sel.m_pObject; }))
    return;


  if (!m_QueuedChanges.Contains(sProperty))
  {
    m_QueuedChanges.PushBack(sProperty);
  }

  // In case the change happened outside the command history we have to update at once.
  if (!m_pGrid->GetCommandHistory()->IsInTransaction() && !m_pGrid->GetCommandHistory()->IsInUndoRedo())
    FlushQueuedChanges();
}

void wdQtTypeWidget::FlushQueuedChanges()
{
  for (const wdString& sProperty : m_QueuedChanges)
  {
    for (auto it = m_PropertyWidgets.GetIterator(); it.IsValid(); ++it)
    {
      if (it.Key().StartsWith(sProperty))
      {
        wdQtScopedUpdatesDisabled _(this);
        it.Value().m_pWidget->SetSelection(m_Items);
        break;
      }
    }
  }

  m_QueuedChanges.Clear();

  UpdatePropertyMetaState();
}

void wdQtTypeWidget::UpdatePropertyMetaState()
{
  wdPropertyMetaState* pMeta = wdPropertyMetaState::GetSingleton();
  wdMap<wdString, wdPropertyUiState> PropertyStates;
  pMeta->GetTypePropertiesState(m_Items, PropertyStates);

  wdDefaultObjectState defaultState(m_pObjectAccessor, m_Items);

  wdQtPropertyWidget::SetPaletteBackgroundColor(defaultState.GetBackgroundColor(), m_Pal);
  setPalette(m_Pal);

  for (auto it = m_PropertyWidgets.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_pWidget->GetProperty();
    auto itData = PropertyStates.Find(it.Key());

    const bool bReadOnly = (it.Value().m_pWidget->GetProperty()->GetFlags().IsSet(wdPropertyFlags::ReadOnly)) ||
                           (it.Value().m_pWidget->GetProperty()->GetAttributeByType<wdReadOnlyAttribute>() != nullptr);
    const bool bIsDefaultValue = defaultState.IsDefaultValue(it.Key());
    wdPropertyUiState::Visibility state = wdPropertyUiState::Default;
    if (itData.IsValid())
    {
      state = itData.Value().m_Visibility;
    }

    if (it.Value().m_pLabel)
    {
      it.Value().m_pLabel->setVisible(state != wdPropertyUiState::Invisible);
      it.Value().m_pLabel->setEnabled(!bReadOnly && state != wdPropertyUiState::Disabled);
      it.Value().m_pLabel->SetIsDefault(bIsDefaultValue);

      if (itData.IsValid() && !itData.Value().m_sNewLabelText.IsEmpty())
      {
        const char* szLabelText = itData.Value().m_sNewLabelText;
        it.Value().m_pLabel->setText(QString::fromUtf8(wdTranslate(szLabelText)));
        it.Value().m_pLabel->setToolTip(QString::fromUtf8(wdTranslateTooltip(szLabelText)));
      }
      else
      {
        bool temp = wdTranslatorLogMissing::s_bActive;
        wdTranslatorLogMissing::s_bActive = false;

        // unless there is a specific override, we want to show the exact property name
        // also we don't want to force people to add translations for each and every property name
        it.Value().m_pLabel->setText(QString::fromUtf8(wdTranslate(it.Value().m_sOriginalLabelText)));

        // though do try to get a tooltip for the property
        // this will not log an error message, if the string is not translated
        it.Value().m_pLabel->setToolTip(QString::fromUtf8(wdTranslateTooltip(it.Value().m_sOriginalLabelText)));

        wdTranslatorLogMissing::s_bActive = temp;
      }
    }

    it.Value().m_pWidget->setVisible(state != wdPropertyUiState::Invisible);
    it.Value().m_pWidget->setEnabled(!bReadOnly && state != wdPropertyUiState::Disabled);
    it.Value().m_pWidget->SetIsDefault(bIsDefaultValue);
  }
}

void wdQtTypeWidget::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set palette.
  setPalette(m_Pal);
  QWidget::showEvent(event);
}
