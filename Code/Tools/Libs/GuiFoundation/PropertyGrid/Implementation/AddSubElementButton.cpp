/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
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

nsString nsQtAddSubElementButton::s_sLastMenuSearch;
bool nsQtAddSubElementButton::s_bShowInDevelopmentFeatures = false;

nsQtAddSubElementButton::nsQtAddSubElementButton()
  : nsQtPropertyWidget()
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

void nsQtAddSubElementButton::OnInit()
{
  if (m_pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
  {
    m_pMenu = new QMenu(m_pButton);
    m_pMenu->setToolTipsVisible(true);
    connect(m_pMenu, &QMenu::aboutToShow, this, &nsQtAddSubElementButton::onMenuAboutToShow);
    m_pButton->setMenu(m_pMenu);
    m_pButton->setObjectName("Button");
  }

  if (const nsMaxArraySizeAttribute* pAttr = m_pProp->GetAttributeByType<nsMaxArraySizeAttribute>())
  {
    m_uiMaxElements = pAttr->GetMaxSize();
  }

  if (const nsPreventDuplicatesAttribute* pAttr = m_pProp->GetAttributeByType<nsPreventDuplicatesAttribute>())
  {
    m_bPreventDuplicates = true;
  }

  QMetaObject::connectSlotsByName(this);
}

struct TypeComparer
{
  NS_FORCE_INLINE bool Less(const nsRTTI* a, const nsRTTI* b) const
  {
    const nsCategoryAttribute* pCatA = a->GetAttributeByType<nsCategoryAttribute>();
    const nsCategoryAttribute* pCatB = b->GetAttributeByType<nsCategoryAttribute>();
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
      nsInt32 iRes = nsStringUtils::Compare(pCatA->GetCategory(), pCatB->GetCategory());
      if (iRes != 0)
      {
        return iRes < 0;
      }
    }

    return a->GetTypeName().Compare(b->GetTypeName()) < 0;
  }
};

QMenu* nsQtAddSubElementButton::CreateCategoryMenu(const char* szCategory, nsMap<nsString, QMenu*>& existingMenus)
{
  if (nsStringUtils::IsNullOrEmpty(szCategory))
    return m_pMenu;


  auto it = existingMenus.Find(szCategory);
  if (it.IsValid())
    return it.Value();

  nsStringBuilder sPath = szCategory;
  sPath.PathParentDirectory();
  sPath.Trim("/");

  QMenu* pParentMenu = m_pMenu;

  if (!sPath.IsEmpty())
  {
    pParentMenu = CreateCategoryMenu(sPath, existingMenus);
  }

  sPath = szCategory;
  sPath = sPath.GetFileName();

  QMenu* pNewMenu = pParentMenu->addMenu(nsTranslate(sPath));
  existingMenus[szCategory] = pNewMenu;

  return pNewMenu;
}

void nsQtAddSubElementButton::onMenuAboutToShow()
{
  if (m_Items.IsEmpty())
    return;

  if (m_pMenu->isEmpty())
  {
    auto pProp = GetProperty();

    if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
    {
      m_SupportedTypes.Clear();
      nsReflectionUtils::GatherTypesDerivedFromClass(pProp->GetSpecificType(), m_SupportedTypes);
    }
    m_SupportedTypes.Insert(pProp->GetSpecificType());

    // remove all types that are marked as hidden
    for (auto it = m_SupportedTypes.GetIterator(); it.IsValid();)
    {
      if (it.Key()->GetAttributeByType<nsHiddenAttribute>() != nullptr)
      {
        it = m_SupportedTypes.Remove(it);
        continue;
      }

      if (!s_bShowInDevelopmentFeatures)
      {
        if (auto pInDev = it.Key()->GetAttributeByType<nsInDevelopmentAttribute>())
        {
          it = m_SupportedTypes.Remove(it);
          continue;
        }
      }

      ++it;
    }

    // Make category-sorted array of types
    nsDynamicArray<const nsRTTI*> supportedTypes;
    for (const nsRTTI* pRtti : m_SupportedTypes)
    {
      if (pRtti->GetTypeFlags().IsAnySet(nsTypeFlags::Abstract))
        continue;

      supportedTypes.PushBack(pRtti);
    }
    supportedTypes.Sort(TypeComparer());

    if (!m_bPreventDuplicates && supportedTypes.GetCount() > 10)
    {
      // only show a searchable menu when it makes some sense
      // also deactivating entries to prevent duplicates is currently not supported by the searchable menu
      m_pSearchableMenu = new nsQtSearchableMenu(m_pMenu);
    }

    nsStringBuilder sIconName;
    nsStringBuilder sCategory = "";

    nsMap<nsString, QMenu*> existingMenus;

    if (m_pSearchableMenu == nullptr)
    {
      // first round: create all sub menus
      for (const nsRTTI* pRtti : supportedTypes)
      {
        // Determine current menu
        const nsCategoryAttribute* pCatA = pRtti->GetAttributeByType<nsCategoryAttribute>();

        if (pCatA)
        {
          CreateCategoryMenu(pCatA->GetCategory(), existingMenus);
        }
      }
    }

    nsStringBuilder tmp;

    // second round: create the actions
    for (const nsRTTI* pRtti : supportedTypes)
    {
      sIconName.Set(":/TypeIcons/", pRtti->GetTypeName(), ".svg");

      // Determine current menu
      const nsCategoryAttribute* pCatA = pRtti->GetAttributeByType<nsCategoryAttribute>();
      const nsInDevelopmentAttribute* pInDev = pRtti->GetAttributeByType<nsInDevelopmentAttribute>();
      const nsColorAttribute* pColA = pRtti->GetAttributeByType<nsColorAttribute>();

      nsColor iconColor = nsColor::MakeZero();

      if (pColA)
      {
        iconColor = pColA->GetColor();
      }
      else if (pCatA && iconColor == nsColor::MakeZero())
      {
        iconColor = nsColorScheme::GetCategoryColor(pCatA->GetCategory(), nsColorScheme::CategoryColorUsage::MenuEntryIcon);
      }

      const QIcon actionIcon = nsQtUiServices::GetCachedIconResource(sIconName.GetData(), iconColor);


      if (m_pSearchableMenu != nullptr)
      {
        nsStringBuilder sFullPath;
        sFullPath = pCatA ? pCatA->GetCategory() : "";
        sFullPath.AppendPath(pRtti->GetTypeName());

        nsStringBuilder sDisplayName = nsTranslate(pRtti->GetTypeName().GetData(tmp));
        if (pInDev)
        {
          sDisplayName.AppendFormat(" [ {} ]", pInDev->GetString());
        }

        m_pSearchableMenu->AddItem(sDisplayName, sFullPath, QVariant::fromValue((void*)pRtti), actionIcon);
      }
      else
      {
        QMenu* pCat = CreateCategoryMenu(pCatA ? pCatA->GetCategory() : nullptr, existingMenus);

        nsStringBuilder fullName = nsTranslate(pRtti->GetTypeName().GetData(tmp));

        if (pInDev)
        {
          fullName.AppendFormat(" [ {} ]", pInDev->GetString());
        }

        // Add type action to current menu
        QAction* pAction = new QAction(fullName.GetData(), m_pMenu);
        pAction->setProperty("type", QVariant::fromValue((void*)pRtti));
        NS_VERIFY(connect(pAction, SIGNAL(triggered()), this, SLOT(OnMenuAction())) != nullptr, "connection failed");

        pAction->setIcon(actionIcon);

        pCat->addAction(pAction);
      }
    }

    if (m_pSearchableMenu != nullptr)
    {
      connect(m_pSearchableMenu, &nsQtSearchableMenu::MenuItemTriggered, m_pMenu, [this](const QString& sName, const QVariant& variant) {
        const nsRTTI* pRtti = static_cast<const nsRTTI*>(variant.value<void*>());

        OnAction(pRtti);
        m_pMenu->close(); });

      connect(m_pSearchableMenu, &nsQtSearchableMenu::SearchTextChanged, m_pMenu,
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
      nsInt32 iCount = 0;
      m_pObjectAccessor->GetCount(item.m_pObject, m_pProp, iCount).AssertSuccess();

      if (iCount >= (nsInt32)m_uiMaxElements)
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
    nsSet<const nsRTTI*> UsedTypes;

    for (auto& item : m_Items)
    {
      nsInt32 iCount = 0;
      m_pObjectAccessor->GetCount(item.m_pObject, m_pProp, iCount).AssertSuccess();

      for (nsInt32 i = 0; i < iCount; ++i)
      {
        nsUuid guid = m_pObjectAccessor->Get<nsUuid>(item.m_pObject, m_pProp, i);

        if (guid.IsValid())
        {
          UsedTypes.Insert(m_pObjectAccessor->GetObject(guid)->GetType());
        }
      }

      QList<QAction*> actions = m_pMenu->actions();
      for (auto pAct : actions)
      {
        const nsRTTI* pRtti = static_cast<const nsRTTI*>(pAct->property("type").value<void*>());

        pAct->setEnabled(!UsedTypes.Contains(pRtti));
      }
    }
  }
}

void nsQtAddSubElementButton::on_Button_clicked()
{
  auto pProp = GetProperty();

  if (!pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
  {
    OnAction(pProp->GetSpecificType());
  }
}

void nsQtAddSubElementButton::OnMenuAction()
{
  const nsRTTI* pRtti = static_cast<const nsRTTI*>(sender()->property("type").value<void*>());

  OnAction(pRtti);
}

void nsQtAddSubElementButton::OnAction(const nsRTTI* pRtti)
{
  NS_ASSERT_DEV(pRtti != nullptr, "user data retrieval failed");
  nsVariant index = (nsInt32)-1;

  if (m_pProp->GetCategory() == nsPropertyCategory::Map)
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
        nsVariant value;
        nsStatus res = m_pObjectAccessor->GetValue(item.m_pObject, m_pProp, value, index);
        if (res.m_Result.Succeeded())
        {
          bOk = false;
          break;
        }
      }
      if (!bOk)
      {
        nsQtUiServices::GetSingleton()->MessageBoxInformation("The selected key is already used in the selection.");
      }
    }
  }

  m_pObjectAccessor->StartTransaction("Add Element");

  nsStatus res;
  const bool bIsValueType = nsReflectionUtils::IsValueType(m_pProp);
  if (bIsValueType)
  {
    for (auto& item : m_Items)
    {
      res = m_pObjectAccessor->InsertValue(item.m_pObject, m_pProp, nsReflectionUtils::GetDefaultValue(GetProperty(), index), index);
      if (res.m_Result.Failed())
        break;
    }
  }
  else if (GetProperty()->GetFlags().IsSet(nsPropertyFlags::Class))
  {
    for (auto& item : m_Items)
    {
      nsUuid guid;
      res = m_pObjectAccessor->AddObject(item.m_pObject, m_pProp, index, pRtti, guid);
      if (res.m_Result.Failed())
        break;

      nsHybridArray<nsPropertySelection, 1> selection;
      selection.PushBack({m_pObjectAccessor->GetObject(guid), nsVariant()});
      nsDefaultObjectState defaultState(m_pObjectAccessor, selection);
      defaultState.RevertObject().AssertSuccess();
    }
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Adding sub-element to the property failed.");
}
