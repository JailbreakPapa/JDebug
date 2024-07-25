#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/Implementation/TagSetPropertyWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>

#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Settings/ToolsTagRegistry.h>

/// *** Tag Set ***

nsQtPropertyEditorTagSetWidget::nsQtPropertyEditorTagSetWidget()
  : nsQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QPushButton(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pMenu = nullptr;
  m_pMenu = new QMenu(m_pWidget);
  m_pMenu->setToolTipsVisible(true);
  m_pWidget->setMenu(m_pMenu);
  m_pLayout->addWidget(m_pWidget);

  connect(m_pMenu, SIGNAL(aboutToShow()), this, SLOT(on_Menu_aboutToShow()));
}

nsQtPropertyEditorTagSetWidget::~nsQtPropertyEditorTagSetWidget()
{
  m_Tags.Clear();
  m_pWidget->setMenu(nullptr);

  delete m_pMenu;
  m_pMenu = nullptr;
}

void nsQtPropertyEditorTagSetWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtPropertyWidget::SetSelection(items);
  InternalUpdateValue();
}

void nsQtPropertyEditorTagSetWidget::OnInit()
{
  NS_ASSERT_DEV(m_pProp->GetCategory() == nsPropertyCategory::Set && m_pProp->GetSpecificType() == nsGetStaticRTTI<nsConstCharPtr>(),
    "nsQtPropertyEditorTagSetWidget only works with nsTagSet.");

  // Retrieve tag categories.
  const nsTagSetWidgetAttribute* pAssetAttribute = m_pProp->GetAttributeByType<nsTagSetWidgetAttribute>();
  NS_ASSERT_DEV(pAssetAttribute != nullptr, "nsQtPropertyEditorTagSetWidget needs nsTagSetWidgetAttribute to be set.");
  nsStringBuilder sTagFilter = pAssetAttribute->GetTagFilter();
  nsHybridArray<nsStringView, 4> categories;
  sTagFilter.Split(false, categories, ";");

  // Get tags by categories.
  nsHybridArray<const nsToolsTag*, 16> tags;
  nsToolsTagRegistry::GetTagsByCategory(categories, tags);

  const char* szCurrentCategory = "";

  // Add valid tags to menu.
  for (const nsToolsTag* pTag : tags)
  {
    if (!pTag->m_sCategory.IsEqual(szCurrentCategory))
    {
      /*QAction* pCategory = */ m_pMenu->addSection(nsQtUiServices::GetSingleton()->GetCachedIconResource(":/EditorFramework/Icons/Tag.svg"),
        QLatin1String("[") + QString(pTag->m_sCategory.GetData()) + QLatin1String("]"));

      szCurrentCategory = pTag->m_sCategory;

      // remove category from list, as it was added once

      /// \todo nsStringView is POD? -> array<stringview>::Remove(stringview) fails, because of memcmp
      // categories.Remove(szCurrentCategory);

      for (nsUInt32 i = 0; i < categories.GetCount(); ++i)
      {
        if (categories[i] == szCurrentCategory)
        {
          categories.RemoveAtAndCopy(i);
          break;
        }
      }
    }

    QWidgetAction* pAction = new QWidgetAction(m_pMenu);
    QCheckBox* pCheckBox = new QCheckBox(pTag->m_sName.GetData(), m_pMenu);
    pCheckBox->setCheckable(true);
    pCheckBox->setCheckState(Qt::Unchecked);
    pCheckBox->setProperty("Tag", pTag->m_sName.GetData());
    connect(pCheckBox, &QCheckBox::clicked, this, &nsQtPropertyEditorTagSetWidget::onCheckBoxClicked);
    pAction->setDefaultWidget(pCheckBox);

    m_Tags.PushBack(pCheckBox);
    m_pMenu->addAction(pAction);
  }

  nsStringBuilder tmp;

  // if a tag category is empty, it will never show up in the menu, thus the user doesn't know the name of the valid category
  // therefore, for every empty category, add an entry
  for (const auto& catname : categories)
  {
    /*QAction* pCategory = */ m_pMenu->addSection(nsQtUiServices::GetSingleton()->GetCachedIconResource(":/EditorFramework/Icons/Tag.svg"),
      QLatin1String("[") + QString(catname.GetData(tmp)) + QLatin1String("]"));
  }
}

void nsQtPropertyEditorTagSetWidget::InternalUpdateValue()
{
  nsMap<nsString, nsUInt32> tags;
  // Count used tags of each object in the selection.
  for (auto& item : m_Items)
  {
    nsHybridArray<nsVariant, 16> currentSetValues;
    nsStatus status = m_pObjectAccessor->GetValues(item.m_pObject, m_pProp, currentSetValues);
    NS_ASSERT_DEV(status.m_Result.Succeeded(), "Failed to get tag keys!");
    for (const nsVariant& key : currentSetValues)
    {
      NS_ASSERT_DEV(key.GetType() == nsVariantType::String, "Tags are supposed to be of type string!");
      tags[key.Get<nsString>()]++;
    }
  }

  // Update checkbox state
  QString sText;
  nsUInt32 uiCount = m_Items.GetCount();
  for (QCheckBox* pCheckBox : m_Tags)
  {
    nsString value = pCheckBox->property("Tag").toString().toUtf8().data();
    nsUInt32 uiUsed = tags[value];

    nsQtScopedBlockSignals b(pCheckBox);
    if (uiUsed == 0)
    {
      pCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
    else if (uiUsed == uiCount)
    {
      pCheckBox->setCheckState(Qt::CheckState::Checked);
      sText += value.GetData();
      sText += "|";
    }
    else
    {
      pCheckBox->setCheckState(Qt::CheckState::PartiallyChecked);
      sText = "<Multiple Values>|"; // string is shrunk by one character (see below), so | is a dummy
    }
  }


  nsQtScopedBlockSignals b(m_pWidget);
  if (!sText.isEmpty())
    sText = sText.left(sText.size() - 1);
  else
    sText = "<none>";

  // m_pWidget->setIcon(nsQtUiServices::GetSingleton()->GetCachedIconResource(":/EditorFramework/Icons/Tag.svg"));
  m_pWidget->setText(sText);
}

void nsQtPropertyEditorTagSetWidget::on_Menu_aboutToShow()
{
  m_pMenu->setMinimumWidth(m_pWidget->geometry().width());
}

void nsQtPropertyEditorTagSetWidget::onCheckBoxClicked(bool bChecked)
{
  QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(sender());
  nsVariant value = pCheckBox->property("Tag").toString().toUtf8().data();
  if (pCheckBox->isChecked())
  {
    m_pObjectAccessor->StartTransaction("Add Tag");

    // Add tag to all objects in selection that don't have it yet.
    for (auto& item : m_Items)
    {
      nsHybridArray<nsVariant, 16> currentSetValues;

      nsStatus status = m_pObjectAccessor->GetValues(item.m_pObject, m_pProp, currentSetValues);
      NS_ASSERT_DEV(status.m_Result.Succeeded(), "Failed to get tag keys!");
      if (!currentSetValues.Contains(value))
      {
        auto res = m_pObjectAccessor->InsertValue(item.m_pObject, m_pProp, value, -1);
        if (res.m_Result.Failed())
        {
          NS_REPORT_FAILURE("Failed to add '{0}' tag to tag set", value.Get<nsString>());
        }
      }
    }
  }
  else
  {
    m_pObjectAccessor->StartTransaction("Remove Tag");

    nsRemoveObjectPropertyCommand cmd;
    cmd.m_sProperty = m_pProp->GetPropertyName();

    // Remove tag from all objects in selection that have it.
    for (auto& item : m_Items)
    {
      nsHybridArray<nsVariant, 16> currentSetValues;
      nsStatus status = m_pObjectAccessor->GetValues(item.m_pObject, m_pProp, currentSetValues);
      NS_ASSERT_DEV(status.m_Result.Succeeded(), "Failed to get tag keys!");
      nsUInt32 uiIndex = currentSetValues.IndexOf(value);
      if (uiIndex != -1)
      {
        auto res = m_pObjectAccessor->RemoveValue(item.m_pObject, m_pProp, uiIndex);
        if (res.m_Result.Failed())
        {
          NS_REPORT_FAILURE("Failed to remove '{0}' tag from tag set", value.Get<nsString>());
        }
      }
    }
  }

  m_pObjectAccessor->FinishTransaction();
}
