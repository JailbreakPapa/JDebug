#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <GuiFoundation/PropertyGrid/DefaultState.h>
#include <GuiFoundation/PropertyGrid/Implementation/AddSubElementButton.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/SearchableMenu.moc.h>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QPushButton>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

wdString wdQtAddSubElementButton::s_sLastMenuSearch;
bool wdQtAddSubElementButton::s_bShowInDevelopmentFeatures = false;

wdQtAddSubElementButton::wdQtAddSubElementButton()
  : wdQtPropertyWidget()
{
  // Reset base class size policy as we are put in a layout that would cause us to vanish instead.
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pButton = new QPushButton(this);
  m_pButton->setText("Add Item");
  m_pButton->setObjectName("Button");

  QSizePolicy policy = m_pButton->sizePolicy();
  policy.setHorizontalStretch(0);
  m_pButton->setSizePolicy(policy);

  m_pLayout->addSpacerItem(new QSpacerItem(0, 0));
  m_pLayout->setStretch(0, 1);
  m_pLayout->addWidget(m_pButton);
  m_pLayout->addSpacerItem(new QSpacerItem(0, 0));
  m_pLayout->setStretch(2, 1);

  m_pMenu = nullptr;
}

void wdQtAddSubElementButton::OnInit()
{
  if (m_pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
  {
    m_pMenu = new QMenu(m_pButton);
    m_pMenu->setToolTipsVisible(true);
    connect(m_pMenu, &QMenu::aboutToShow, this, &wdQtAddSubElementButton::onMenuAboutToShow);
    m_pButton->setMenu(m_pMenu);
    m_pButton->setObjectName("Button");
  }

  if (const wdMaxArraySizeAttribute* pAttr = m_pProp->GetAttributeByType<wdMaxArraySizeAttribute>())
  {
    m_uiMaxElements = pAttr->GetMaxSize();
  }

  if (const wdPreventDuplicatesAttribute* pAttr = m_pProp->GetAttributeByType<wdPreventDuplicatesAttribute>())
  {
    m_bPreventDuplicates = true;
  }

  m_pConstraint = m_pProp->GetAttributeByType<wdConstrainPointerAttribute>();

  QMetaObject::connectSlotsByName(this);
}

struct TypeComparer
{
  WD_FORCE_INLINE bool Less(const wdRTTI* a, const wdRTTI* b) const
  {
    const wdCategoryAttribute* pCatA = a->GetAttributeByType<wdCategoryAttribute>();
    const wdCategoryAttribute* pCatB = b->GetAttributeByType<wdCategoryAttribute>();
    if (pCatA != nullptr && pCatB == nullptr)
    {
      return true;
    }
    else if (pCatA == nullptr && pCatB != nullptr)
    {
      return false;
    }
    else if (pCatA != nullptr && pCatB != nullptr)
    {
      wdInt32 iRes = wdStringUtils::Compare(pCatA->GetCategory(), pCatB->GetCategory());
      if (iRes != 0)
      {
        return iRes < 0;
      }
    }

    return wdStringUtils::Compare(a->GetTypeName(), b->GetTypeName()) < 0;
  }
};

QMenu* wdQtAddSubElementButton::CreateCategoryMenu(const char* szCategory, wdMap<wdString, QMenu*>& existingMenus)
{
  if (wdStringUtils::IsNullOrEmpty(szCategory))
    return m_pMenu;


  auto it = existingMenus.Find(szCategory);
  if (it.IsValid())
    return it.Value();

  wdStringBuilder sPath = szCategory;
  sPath.PathParentDirectory();
  sPath.Trim("/");

  QMenu* pParentMenu = m_pMenu;

  if (!sPath.IsEmpty())
  {
    pParentMenu = CreateCategoryMenu(sPath, existingMenus);
  }

  sPath = szCategory;
  sPath = sPath.GetFileName();

  QMenu* pNewMenu = pParentMenu->addMenu(wdTranslate(sPath.GetData()));
  existingMenus[szCategory] = pNewMenu;

  return pNewMenu;
}

void wdQtAddSubElementButton::onMenuAboutToShow()
{
  if (m_Items.IsEmpty())
    return;

  if (m_pMenu->isEmpty())
  {
    auto pProp = GetProperty();

    if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
    {
      m_SupportedTypes.Clear();
      wdReflectionUtils::GatherTypesDerivedFromClass(pProp->GetSpecificType(), m_SupportedTypes, false);
    }
    m_SupportedTypes.Insert(pProp->GetSpecificType());

    wdVariant constraintValue;
    if (m_pConstraint)
    {
      const wdRTTI* pType = GetCommonBaseType(m_Items);
      if (const wdAbstractProperty* pConstraintValueProp = pType->FindPropertyByName(m_pConstraint->GetConstantValueProperty()))
      {
        if (pConstraintValueProp->GetCategory() == wdPropertyCategory::Constant)
        {
          constraintValue = static_cast<const wdAbstractConstantProperty*>(pConstraintValueProp)->GetConstant();
        }
        else if (pConstraintValueProp->GetCategory() == wdPropertyCategory::Member)
        {
          constraintValue = GetCommonValue(m_Items, pConstraintValueProp);
        }
        else
        {
          wdLog::Error("wdConstrainPointerAttribute set for '{0}' but the constant value property '{1}' has an unsupported type.",
            pType->GetTypeName(), m_pConstraint->GetConstantValueProperty().GetData());
        }
      }
      else
      {
        wdLog::Error("wdConstrainPointerAttribute set for '{0}' but the constant value property '{1}' does not exist.", pType->GetTypeName(),
          m_pConstraint->GetConstantValueProperty().GetData());
      }
    }
    // remove all types that are marked as hidden
    for (auto it = m_SupportedTypes.GetIterator(); it.IsValid();)
    {
      if (it.Key()->GetAttributeByType<wdHiddenAttribute>() != nullptr)
      {
        it = m_SupportedTypes.Remove(it);
        continue;
      }

      if (!s_bShowInDevelopmentFeatures)
      {
        if (auto pInDev = it.Key()->GetAttributeByType<wdInDevelopmentAttribute>())
        {
          it = m_SupportedTypes.Remove(it);
          continue;
        }
      }

      if (m_pConstraint)
      {
        const wdAbstractProperty* pConstraintProp = it.Key()->FindPropertyByName(m_pConstraint->GetConstantName());
        if (!constraintValue.IsValid() || !pConstraintProp || pConstraintProp->GetCategory() != wdPropertyCategory::Constant ||
            static_cast<const wdAbstractConstantProperty*>(pConstraintProp)->GetConstant() != constraintValue)
        {
          it = m_SupportedTypes.Remove(it);
          continue;
        }
      }

      ++it;
    }

    // Make category-sorted array of types
    wdDynamicArray<const wdRTTI*> supportedTypes;
    for (const wdRTTI* pRtti : m_SupportedTypes)
    {
      if (pRtti->GetTypeFlags().IsAnySet(wdTypeFlags::Abstract))
        continue;

      supportedTypes.PushBack(pRtti);
    }
    supportedTypes.Sort(TypeComparer());

    if (!m_bPreventDuplicates && supportedTypes.GetCount() > 10)
    {
      // only show a searchable menu when it makes some sense
      // also deactivating entries to prevent duplicates is currently not supported by the searchable menu
      m_pSearchableMenu = new wdQtSearchableMenu(m_pMenu);
    }

    wdStringBuilder sIconName;
    wdStringBuilder sCategory = "";

    wdMap<wdString, QMenu*> existingMenus;

    if (m_pSearchableMenu == nullptr)
    {
      // first round: create all sub menus
      for (const wdRTTI* pRtti : supportedTypes)
      {
        // Determine current menu
        const wdCategoryAttribute* pCatA = pRtti->GetAttributeByType<wdCategoryAttribute>();

        if (pCatA)
        {
          CreateCategoryMenu(pCatA->GetCategory(), existingMenus);
        }
      }
    }

    // second round: create the actions
    for (const wdRTTI* pRtti : supportedTypes)
    {
      sIconName.Set(":/TypeIcons/", pRtti->GetTypeName());
      const QIcon actionIcon = wdQtUiServices::GetCachedIconResource(sIconName.GetData());

      // Determine current menu
      const wdCategoryAttribute* pCatA = pRtti->GetAttributeByType<wdCategoryAttribute>();
      const wdInDevelopmentAttribute* pInDev = pRtti->GetAttributeByType<wdInDevelopmentAttribute>();

      if (m_pSearchableMenu != nullptr)
      {
        wdStringBuilder fullName;
        fullName = pCatA ? pCatA->GetCategory() : "";
        fullName.AppendPath(wdTranslate(pRtti->GetTypeName()));

        if (pInDev)
        {
          fullName.AppendFormat(" [ {} ]", pInDev->GetString());
        }

        m_pSearchableMenu->AddItem(fullName, QVariant::fromValue((void*)pRtti), actionIcon);
      }
      else
      {
        QMenu* pCat = CreateCategoryMenu(pCatA ? pCatA->GetCategory() : nullptr, existingMenus);

        wdStringBuilder fullName = wdTranslate(pRtti->GetTypeName());

        if (pInDev)
        {
          fullName.AppendFormat(" [ {} ]", pInDev->GetString());
        }

        // Add type action to current menu
        QAction* pAction = new QAction(fullName.GetData(), m_pMenu);
        pAction->setProperty("type", QVariant::fromValue((void*)pRtti));
        WD_VERIFY(connect(pAction, SIGNAL(triggered()), this, SLOT(OnMenuAction())) != nullptr, "connection failed");

        pAction->setIcon(actionIcon);

        pCat->addAction(pAction);
      }
    }

    if (m_pSearchableMenu != nullptr)
    {
      connect(m_pSearchableMenu, &wdQtSearchableMenu::MenuItemTriggered, m_pMenu, [this](const QString& sName, const QVariant& variant) {
        const wdRTTI* pRtti = static_cast<const wdRTTI*>(variant.value<void*>());

        OnAction(pRtti);
        m_pMenu->close(); });

      connect(m_pSearchableMenu, &wdQtSearchableMenu::SearchTextChanged, m_pMenu,
        [this](const QString& sText) { s_sLastMenuSearch = sText.toUtf8().data(); });

      m_pMenu->addAction(m_pSearchableMenu);

      // important to do this last to make sure the search bar gets focus
      m_pSearchableMenu->Finalize(s_sLastMenuSearch.GetData());
    }
  }

  if (m_uiMaxElements > 0) // 0 means unlimited
  {
    QList<QAction*> actions = m_pMenu->actions();

    for (auto& item : m_Items)
    {
      wdInt32 iCount = 0;
      m_pObjectAccessor->GetCount(item.m_pObject, m_pProp, iCount);

      if (iCount >= (wdInt32)m_uiMaxElements)
      {
        if (!m_bNoMoreElementsAllowed)
        {
          m_bNoMoreElementsAllowed = true;

          QAction* pAction = new QAction(QString("Maximum allowed elements in array is %1").arg(m_uiMaxElements));
          m_pMenu->insertAction(actions.isEmpty() ? nullptr : actions[0], pAction);

          for (auto pAct : actions)
          {
            pAct->setEnabled(false);
          }
        }

        return;
      }
    }

    if (m_bNoMoreElementsAllowed)
    {
      for (auto pAct : actions)
      {
        pAct->setEnabled(true);
      }

      m_bNoMoreElementsAllowed = false;
      delete m_pMenu->actions()[0]; // remove the dummy action
    }
  }

  if (m_bPreventDuplicates)
  {
    wdSet<const wdRTTI*> UsedTypes;

    for (auto& item : m_Items)
    {
      wdInt32 iCount = 0;
      m_pObjectAccessor->GetCount(item.m_pObject, m_pProp, iCount);

      for (wdInt32 i = 0; i < iCount; ++i)
      {
        wdUuid guid = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, i);

        if (guid.IsValid())
        {
          UsedTypes.Insert(m_pObjectAccessor->GetObject(guid)->GetType());
        }
      }

      QList<QAction*> actions = m_pMenu->actions();
      for (auto pAct : actions)
      {
        const wdRTTI* pRtti = static_cast<const wdRTTI*>(pAct->property("type").value<void*>());

        pAct->setEnabled(!UsedTypes.Contains(pRtti));
      }
    }
  }
}

void wdQtAddSubElementButton::on_Button_clicked()
{
  auto pProp = GetProperty();

  if (!pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
  {
    OnAction(pProp->GetSpecificType());
  }
}

void wdQtAddSubElementButton::OnMenuAction()
{
  const wdRTTI* pRtti = static_cast<const wdRTTI*>(sender()->property("type").value<void*>());

  OnAction(pRtti);
}

void wdQtAddSubElementButton::OnAction(const wdRTTI* pRtti)
{
  WD_ASSERT_DEV(pRtti != nullptr, "user data retrieval failed");
  wdVariant index = (wdInt32)-1;

  if (m_pProp->GetCategory() == wdPropertyCategory::Map)
  {
    QString text;
    bool bOk = false;
    while (!bOk)
    {
      text = QInputDialog::getText(this, "Set map key for new element", "Key:", QLineEdit::Normal, text, &bOk);
      if (!bOk)
        return;

      index = text.toUtf8().data();
      for (auto& item : m_Items)
      {
        wdVariant value;
        wdStatus res = m_pObjectAccessor->GetValue(item.m_pObject, m_pProp, value, index);
        if (res.m_Result.Succeeded())
        {
          bOk = false;
          break;
        }
      }
      if (!bOk)
      {
        wdQtUiServices::GetSingleton()->MessageBoxInformation("The selected key is already used in the selection.");
      }
    }
  }

  m_pObjectAccessor->StartTransaction("Add Element");

  wdStatus res;
  const bool bIsValueType = wdReflectionUtils::IsValueType(m_pProp);
  if (bIsValueType)
  {
    for (auto& item : m_Items)
    {
      res = m_pObjectAccessor->InsertValue(item.m_pObject, m_pProp, wdReflectionUtils::GetDefaultValue(GetProperty(), index), index);
      if (res.m_Result.Failed())
        break;
    }
  }
  else if (GetProperty()->GetFlags().IsSet(wdPropertyFlags::Class))
  {
    for (auto& item : m_Items)
    {
      wdUuid guid;
      res = m_pObjectAccessor->AddObject(item.m_pObject, m_pProp, index, pRtti, guid);
      if (res.m_Result.Failed())
        break;

      wdHybridArray<wdPropertySelection, 1> selection;
      selection.PushBack({m_pObjectAccessor->GetObject(guid), wdVariant()});
      wdDefaultObjectState defaultState(m_pObjectAccessor, selection);
      defaultState.RevertObject();
    }
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Adding sub-element to the property failed.");
}
