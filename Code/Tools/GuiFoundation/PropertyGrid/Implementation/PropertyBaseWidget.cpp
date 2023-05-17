#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <GuiFoundation/PropertyGrid/DefaultState.h>
#include <GuiFoundation/PropertyGrid/Implementation/AddSubElementButton.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/ElementGroupButton.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/CollapsibleGroupBox.moc.h>
#include <GuiFoundation/Widgets/InlinedGroupBox.moc.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

#include <QClipboard>
#include <QDragEnterEvent>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QScrollArea>
#include <QStringBuilder>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdPropertyClipboard, wdNoBase, 1, wdRTTIDefaultAllocator<wdPropertyClipboard>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("m_Type", m_Type),
    WD_MEMBER_PROPERTY("m_Value", m_Value),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

/// *** BASE ***
wdQtPropertyWidget::wdQtPropertyWidget()
  : QWidget(nullptr)
  , m_pGrid(nullptr)
  , m_pProp(nullptr)
{
  m_bUndead = false;
  m_bIsDefault = true;
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
}

wdQtPropertyWidget::~wdQtPropertyWidget() {}

void wdQtPropertyWidget::Init(
  wdQtPropertyGridWidget* pGrid, wdObjectAccessorBase* pObjectAccessor, const wdRTTI* pType, const wdAbstractProperty* pProp)
{
  m_pGrid = pGrid;
  m_pObjectAccessor = pObjectAccessor;
  m_pType = pType;
  m_pProp = pProp;
  WD_ASSERT_DEBUG(m_pGrid && m_pObjectAccessor && m_pType && m_pProp, "");

  if (pProp->GetAttributeByType<wdReadOnlyAttribute>() != nullptr || pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
    setEnabled(false);

  OnInit();
}

void wdQtPropertyWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  m_Items = items;
}

const char* wdQtPropertyWidget::GetLabel(wdStringBuilder& ref_sTmp) const
{
  ref_sTmp.Set(m_pType->GetTypeName(), "::", m_pProp->GetPropertyName());
  return ref_sTmp;
}

void wdQtPropertyWidget::ExtendContextMenu(QMenu& m)
{
  m.setToolTipsVisible(true);
  // revert
  {
    QAction* pRevert = m.addAction("Revert to Default");
    pRevert->setEnabled(!m_bIsDefault);
    connect(pRevert, &QAction::triggered, this, [this]()
      {
      m_pObjectAccessor->StartTransaction("Revert to Default");

      switch (m_pProp->GetCategory())
      {
        case wdPropertyCategory::Enum::Array:
        case wdPropertyCategory::Enum::Set:
        case wdPropertyCategory::Enum::Map:
        {

          wdStatus res = wdStatus(WD_SUCCESS);
          if (!m_Items[0].m_Index.IsValid())
          {
            // Revert container
            wdDefaultContainerState defaultState(m_pObjectAccessor, m_Items, m_pProp->GetPropertyName());
            res = defaultState.RevertContainer();
          }
          else
          {
            const bool bIsValueType = wdReflectionUtils::IsValueType(m_pProp) || m_pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags);
            if (bIsValueType)
            {
              // Revert container value type element
              wdDefaultContainerState defaultState(m_pObjectAccessor, m_Items, m_pProp->GetPropertyName());
              res = defaultState.RevertElement({});
            }
            else
            {
              // Revert objects pointed to by the object type element
              wdHybridArray<wdPropertySelection, 8> ResolvedObjects;
              for (const auto& item : m_Items)
              {
                wdUuid ObjectGuid = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, item.m_Index);
                if (ObjectGuid.IsValid())
                {
                  ResolvedObjects.PushBack({m_pObjectAccessor->GetObject(ObjectGuid), wdVariant()});
                }
              }
              wdDefaultObjectState defaultState(m_pObjectAccessor, ResolvedObjects);
              res = defaultState.RevertObject();
            }
          }
          if (res.Failed())
          {
            res.LogFailure();
            m_pObjectAccessor->CancelTransaction();
            return;
          }
        }
        break;
        default:
        {
          // Revert object member property
          wdDefaultObjectState defaultState(m_pObjectAccessor, m_Items);
          wdStatus res = defaultState.RevertProperty(m_pProp);
          if (res.Failed())
          {
            res.LogFailure();
            m_pObjectAccessor->CancelTransaction();
            return;
          }
        }
        break;
      }
      m_pObjectAccessor->FinishTransaction(); });
  }

  const char* szMimeType = "application/wdEditor.Property";
  bool bValueType = wdReflectionUtils::IsValueType(m_pProp) || m_pProp->GetFlags().IsAnySet(wdPropertyFlags::Bitflags | wdPropertyFlags::IsEnum);
  // Copy
  {
    wdVariant commonValue = GetCommonValue(m_Items, m_pProp);
    QAction* pCopy = m.addAction("Copy Value");
    if (!bValueType)
    {
      pCopy->setEnabled(false);
      pCopy->setToolTip("Not a value type");
    }
    else if (!commonValue.IsValid())
    {
      pCopy->setEnabled(false);
      pCopy->setToolTip("No common value in selection");
    }

    connect(pCopy, &QAction::triggered, this, [this, szMimeType, commonValue]()
      {
      wdPropertyClipboard content;
      content.m_Type = m_pProp->GetSpecificType()->GetTypeName();
      content.m_Value = commonValue;

      // Serialize
      wdContiguousMemoryStreamStorage streamStorage;
      wdMemoryStreamWriter memoryWriter(&streamStorage);
      wdReflectionSerializer::WriteObjectToDDL(memoryWriter, wdGetStaticRTTI<wdPropertyClipboard>(), &content);
      memoryWriter.WriteBytes("\0", 1).IgnoreResult(); // null terminate

      // Write to clipboard
      QClipboard* clipboard = QApplication::clipboard();
      QMimeData* mimeData = new QMimeData();
      QByteArray encodedData((const char*)streamStorage.GetData(), streamStorage.GetStorageSize32());

      mimeData->setData(szMimeType, encodedData);
      mimeData->setText(QString::fromUtf8((const char*)streamStorage.GetData()));
      clipboard->setMimeData(mimeData); });
  }

  // Paste
  {
    QAction* pPaste = m.addAction("Paste Value");

    QClipboard* clipboard = QApplication::clipboard();
    auto mimedata = clipboard->mimeData();

    if (!bValueType)
    {
      pPaste->setEnabled(false);
      pPaste->setToolTip("Not a value type");
    }
    else if (!isEnabled())
    {
      pPaste->setEnabled(false);
      pPaste->setToolTip("Property is read only");
    }
    else if (!mimedata->hasFormat(szMimeType))
    {
      pPaste->setEnabled(false);
      pPaste->setToolTip("No property in clipboard");
    }
    else
    {
      QByteArray ba = mimedata->data(szMimeType);
      wdRawMemoryStreamReader memoryReader(ba.data(), ba.count());

      wdPropertyClipboard content;
      wdReflectionSerializer::ReadObjectPropertiesFromDDL(memoryReader, *wdGetStaticRTTI<wdPropertyClipboard>(), &content);

      const bool bIsArray = m_pProp->GetCategory() == wdPropertyCategory::Array || m_pProp->GetCategory() == wdPropertyCategory::Set;
      const wdRTTI* pClipboardType = wdRTTI::FindTypeByName(content.m_Type);
      const bool bIsEnumeration = pClipboardType && (pClipboardType->IsDerivedFrom<wdEnumBase>() || pClipboardType->IsDerivedFrom<wdBitflagsBase>() || m_pProp->GetSpecificType()->IsDerivedFrom<wdEnumBase>() || m_pProp->GetSpecificType()->IsDerivedFrom<wdBitflagsBase>());
      const bool bEnumerationMissmatch = bIsEnumeration ? pClipboardType != m_pProp->GetSpecificType() : false;
      const wdResult clamped = wdReflectionUtils::ClampValue(content.m_Value, m_pProp->GetAttributeByType<wdClampValueAttribute>());

      if (content.m_Value.IsA<wdVariantArray>() != bIsArray)
      {
        pPaste->setEnabled(false);
        wdStringBuilder sTemp;
        sTemp.Format("Cannot convert clipboard and property content between arrays and members.");
        pPaste->setToolTip(sTemp.GetData());
      }
      else if (bEnumerationMissmatch || !content.m_Value.CanConvertTo(m_pProp->GetSpecificType()->GetVariantType()) && content.m_Type != m_pProp->GetSpecificType()->GetTypeName())
      {
        pPaste->setEnabled(false);
        wdStringBuilder sTemp;
        sTemp.Format("Cannot convert clipboard of type '{}' to property of type '{}'", content.m_Type, m_pProp->GetSpecificType()->GetTypeName());
        pPaste->setToolTip(sTemp.GetData());
      }
      else if (clamped.Failed())
      {
        pPaste->setEnabled(false);
        wdStringBuilder sTemp;
        sTemp.Format("The member property '{}' has an wdClampValueAttribute but wdReflectionUtils::ClampValue failed.", m_pProp->GetPropertyName());
      }

      connect(pPaste, &QAction::triggered, this, [this, content]()
        {
        m_pObjectAccessor->StartTransaction("Paste Value");
        if (content.m_Value.IsA<wdVariantArray>())
        {
          const wdVariantArray& values = content.m_Value.Get<wdVariantArray>();
          for (const wdPropertySelection& sel : m_Items)
          {
            if (m_pObjectAccessor->Clear(sel.m_pObject, m_pProp->GetPropertyName()).Failed())
            {
              m_pObjectAccessor->CancelTransaction();
              return;
            }
            for (const wdVariant& val : values)
            {
              if (m_pObjectAccessor->InsertValue(sel.m_pObject, m_pProp, val, -1).Failed())
              {
                m_pObjectAccessor->CancelTransaction();
                return;
              }
            }
          }
        }
        else
        {
          for (const wdPropertySelection& sel : m_Items)
          {
            if (m_pObjectAccessor->SetValue(sel.m_pObject, m_pProp, content.m_Value, sel.m_Index).Failed())
            {
              m_pObjectAccessor->CancelTransaction();
              return;
            }
          }
        }

        m_pObjectAccessor->FinishTransaction(); });
    }
  }

  // copy internal name
  {
    auto lambda = [this]()
    {
      QClipboard* clipboard = QApplication::clipboard();
      QMimeData* mimeData = new QMimeData();
      mimeData->setText(m_pProp->GetPropertyName());
      clipboard->setMimeData(mimeData);

      wdQtUiServices::GetSingleton()->ShowAllDocumentsTemporaryStatusBarMessage(
        wdFmt("Copied Property Name: {}", m_pProp->GetPropertyName()), wdTime::Seconds(5));
    };

    QAction* pAction = m.addAction("Copy Internal Property Name:");
    connect(pAction, &QAction::triggered, this, lambda);

    QAction* pAction2 = m.addAction(m_pProp->GetPropertyName());
    connect(pAction2, &QAction::triggered, this, lambda);
  }
}

const wdRTTI* wdQtPropertyWidget::GetCommonBaseType(const wdHybridArray<wdPropertySelection, 8>& items)
{
  const wdRTTI* pSubtype = nullptr;

  for (const auto& item : items)
  {
    const auto& accessor = item.m_pObject->GetTypeAccessor();

    if (pSubtype == nullptr)
      pSubtype = accessor.GetType();
    else
    {
      pSubtype = wdReflectionUtils::GetCommonBaseType(pSubtype, accessor.GetType());
    }
  }

  return pSubtype;
}

QColor wdQtPropertyWidget::SetPaletteBackgroundColor(wdColorGammaUB inputColor, QPalette& ref_palette)
{
  QColor qColor = qApp->palette().color(QPalette::Window);
  if (inputColor.a != 0)
  {
    const wdColor paletteColorLinear = qtToEzColor(qColor);
    const wdColor inputColorLinear = inputColor;

    wdColor blendedColor = wdMath::Lerp(paletteColorLinear, inputColorLinear, inputColorLinear.a);
    blendedColor.a = 1.0f;
    qColor = wdToQtColor(blendedColor);
  }

  ref_palette.setBrush(QPalette::Window, QBrush(qColor, Qt::SolidPattern));
  return qColor;
}

bool wdQtPropertyWidget::GetCommonVariantSubType(
  const wdHybridArray<wdPropertySelection, 8>& items, const wdAbstractProperty* pProperty, wdVariantType::Enum& out_type)
{
  bool bFirst = true;
  // check if we have multiple values
  for (const auto& item : items)
  {
    if (bFirst)
    {
      bFirst = false;
      wdVariant value;
      m_pObjectAccessor->GetValue(item.m_pObject, pProperty, value, item.m_Index);
      out_type = value.GetType();
    }
    else
    {
      wdVariant valueNext;
      m_pObjectAccessor->GetValue(item.m_pObject, pProperty, valueNext, item.m_Index);
      if (valueNext.GetType() != out_type)
      {
        out_type = wdVariantType::Invalid;
        return false;
      }
    }
  }
  return true;
}

wdVariant wdQtPropertyWidget::GetCommonValue(const wdHybridArray<wdPropertySelection, 8>& items, const wdAbstractProperty* pProperty)
{
  if (!items[0].m_Index.IsValid() && (m_pProp->GetCategory() == wdPropertyCategory::Array || m_pProp->GetCategory() == wdPropertyCategory::Set))
  {
    wdVariantArray values;
    // check if we have multiple values
    for (wdUInt32 i = 0; i < items.GetCount(); i++)
    {
      const auto& item = items[i];
      if (i == 0)
      {
        m_pObjectAccessor->GetValues(item.m_pObject, pProperty, values);
      }
      else
      {
        wdVariantArray valuesNext;
        m_pObjectAccessor->GetValues(item.m_pObject, pProperty, valuesNext);
        if (values != valuesNext)
        {
          return wdVariant();
        }
      }
    }
    return values;
  }
  else
  {
    wdVariant value;
    // check if we have multiple values
    for (const auto& item : items)
    {
      if (!value.IsValid())
      {
        m_pObjectAccessor->GetValue(item.m_pObject, pProperty, value, item.m_Index);
      }
      else
      {
        wdVariant valueNext;
        m_pObjectAccessor->GetValue(item.m_pObject, pProperty, valueNext, item.m_Index);
        if (value != valueNext)
        {
          value = wdVariant();
          break;
        }
      }
    }
    return value;
  }
}

void wdQtPropertyWidget::PrepareToDie()
{
  WD_ASSERT_DEBUG(!m_bUndead, "Object has already been marked for cleanup");

  m_bUndead = true;

  DoPrepareToDie();
}


void wdQtPropertyWidget::OnCustomContextMenu(const QPoint& pt)
{
  QMenu m;
  m.setToolTipsVisible(true);

  ExtendContextMenu(m);
  m_pGrid->ExtendContextMenu(m, m_Items, m_pProp);

  m.exec(pt); // pt is already in global space, because we fixed that
}

void wdQtPropertyWidget::Broadcast(wdPropertyEvent::Type type)
{
  wdPropertyEvent ed;
  ed.m_Type = type;
  ed.m_pProperty = m_pProp;
  PropertyChangedHandler(ed);
}

void wdQtPropertyWidget::PropertyChangedHandler(const wdPropertyEvent& ed)
{
  if (m_bUndead)
    return;


  switch (ed.m_Type)
  {
    case wdPropertyEvent::Type::SingleValueChanged:
    {
      wdStringBuilder sTemp;
      sTemp.Format("Change Property '{0}'", wdTranslate(ed.m_pProperty->GetPropertyName()));
      m_pObjectAccessor->StartTransaction(sTemp);

      wdStatus res;
      for (const auto& sel : *ed.m_pItems)
      {
        res = m_pObjectAccessor->SetValue(sel.m_pObject, ed.m_pProperty, ed.m_Value, sel.m_Index);
        if (res.m_Result.Failed())
          break;
      }

      if (res.m_Result.Failed())
        m_pObjectAccessor->CancelTransaction();
      else
        m_pObjectAccessor->FinishTransaction();

      wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Changing the property failed.");
    }
    break;

    case wdPropertyEvent::Type::BeginTemporary:
    {
      wdStringBuilder sTemp;
      sTemp.Format("Change Property '{0}'", wdTranslate(ed.m_pProperty->GetPropertyName()));
      m_pObjectAccessor->BeginTemporaryCommands(sTemp);
    }
    break;

    case wdPropertyEvent::Type::EndTemporary:
    {
      m_pObjectAccessor->FinishTemporaryCommands();
    }
    break;

    case wdPropertyEvent::Type::CancelTemporary:
    {
      m_pObjectAccessor->CancelTemporaryCommands();
    }
    break;
  }
}

bool wdQtPropertyWidget::eventFilter(QObject* pWatched, QEvent* pEvent)
{
  if (pEvent->type() == QEvent::Wheel)
  {
    if (pWatched->parent())
    {
      pWatched->parent()->event(pEvent);
    }

    return true;
  }

  return false;
}

/// *** wdQtUnsupportedPropertyWidget ***

wdQtUnsupportedPropertyWidget::wdQtUnsupportedPropertyWidget(const char* szMessage)
  : wdQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QLabel(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pLayout->addWidget(m_pWidget);
  m_sMessage = szMessage;
}

void wdQtUnsupportedPropertyWidget::OnInit()
{
  wdQtScopedBlockSignals bs(m_pWidget);

  QString sMessage = QStringLiteral("Unsupported Type: ") % QString::fromUtf8(m_pProp->GetSpecificType()->GetTypeName());
  if (!m_sMessage.IsEmpty())
    sMessage += QStringLiteral(" (") % QString::fromUtf8(m_sMessage, m_sMessage.GetElementCount()) % QStringLiteral(")");
  m_pWidget->setText(sMessage);
  m_pWidget->setToolTip(sMessage);
}


/// *** wdQtStandardPropertyWidget ***

wdQtStandardPropertyWidget::wdQtStandardPropertyWidget()
  : wdQtPropertyWidget()
{
}

void wdQtStandardPropertyWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtPropertyWidget::SetSelection(items);

  m_OldValue = GetCommonValue(items, m_pProp);
  InternalSetValue(m_OldValue);
}

void wdQtStandardPropertyWidget::BroadcastValueChanged(const wdVariant& NewValue)
{
  if (NewValue == m_OldValue)
    return;

  m_OldValue = NewValue;

  wdPropertyEvent ed;
  ed.m_Type = wdPropertyEvent::Type::SingleValueChanged;
  ed.m_pProperty = m_pProp;
  ed.m_Value = NewValue;
  ed.m_pItems = &m_Items;
  PropertyChangedHandler(ed);
}


/// *** wdQtPropertyPointerWidget ***

wdQtPropertyPointerWidget::wdQtPropertyPointerWidget()
  : wdQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pGroup = new wdQtCollapsibleGroupBox(this);
  m_pGroupLayout = new QHBoxLayout(nullptr);
  m_pGroupLayout->setSpacing(1);
  m_pGroupLayout->setContentsMargins(5, 0, 0, 0);
  m_pGroup->GetContent()->setLayout(m_pGroupLayout);

  m_pLayout->addWidget(m_pGroup);

  m_pAddButton = new wdQtAddSubElementButton();
  m_pGroup->GetHeader()->layout()->addWidget(m_pAddButton);

  m_pDeleteButton = new wdQtElementGroupButton(m_pGroup->GetHeader(), wdQtElementGroupButton::ElementAction::DeleteElement, this);
  m_pGroup->GetHeader()->layout()->addWidget(m_pDeleteButton);
  connect(m_pDeleteButton, &QToolButton::clicked, this, &wdQtPropertyPointerWidget::OnDeleteButtonClicked);

  m_pTypeWidget = nullptr;
}

wdQtPropertyPointerWidget::~wdQtPropertyPointerWidget()
{
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.RemoveEventHandler(
    wdMakeDelegate(&wdQtPropertyPointerWidget::StructureEventHandler, this));
}

void wdQtPropertyPointerWidget::OnInit()
{
  UpdateTitle();
  m_pGrid->SetCollapseState(m_pGroup);
  connect(m_pGroup, &wdQtGroupBoxBase::CollapseStateChanged, m_pGrid, &wdQtPropertyGridWidget::OnCollapseStateChanged);

  // Add Buttons
  auto pAttr = m_pProp->GetAttributeByType<wdContainerAttribute>();
  m_pAddButton->setVisible(!pAttr || pAttr->CanAdd());
  m_pDeleteButton->setVisible(!pAttr || pAttr->CanDelete());

  m_pAddButton->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.AddEventHandler(
    wdMakeDelegate(&wdQtPropertyPointerWidget::StructureEventHandler, this));
}

void wdQtPropertyPointerWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtScopedUpdatesDisabled _(this);

  wdQtPropertyWidget::SetSelection(items);

  if (m_pTypeWidget)
  {
    m_pGroupLayout->removeWidget(m_pTypeWidget);
    delete m_pTypeWidget;
    m_pTypeWidget = nullptr;
  }


  wdHybridArray<wdPropertySelection, 8> emptyItems;
  wdHybridArray<wdPropertySelection, 8> subItems;
  for (const auto& item : m_Items)
  {
    wdUuid ObjectGuid = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, item.m_Index);
    if (!ObjectGuid.IsValid())
    {
      emptyItems.PushBack(item);
    }
    else
    {
      wdPropertySelection sel;
      sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);

      subItems.PushBack(sel);
    }
  }

  auto pAttr = m_pProp->GetAttributeByType<wdContainerAttribute>();
  if (!pAttr || pAttr->CanAdd())
    m_pAddButton->setVisible(!emptyItems.IsEmpty());
  if (!pAttr || pAttr->CanDelete())
    m_pDeleteButton->setVisible(!subItems.IsEmpty());

  if (!emptyItems.IsEmpty())
  {
    m_pAddButton->SetSelection(emptyItems);
  }

  const wdRTTI* pCommonType = nullptr;
  if (!subItems.IsEmpty())
  {
    pCommonType = wdQtPropertyWidget::GetCommonBaseType(subItems);

    m_pTypeWidget = new wdQtTypeWidget(m_pGroup->GetContent(), m_pGrid, m_pObjectAccessor, pCommonType, nullptr, nullptr);
    m_pTypeWidget->SetSelection(subItems);

    m_pGroupLayout->addWidget(m_pTypeWidget);
  }

  UpdateTitle(pCommonType);
}


void wdQtPropertyPointerWidget::DoPrepareToDie()
{
  if (m_pTypeWidget)
  {
    m_pTypeWidget->PrepareToDie();
  }
}

void wdQtPropertyPointerWidget::UpdateTitle(const wdRTTI* pType /*= nullptr*/)
{
  wdStringBuilder sb = wdTranslate(m_pProp->GetPropertyName());
  if (pType != nullptr)
  {
    sb.Append(": ", wdTranslate(pType->GetTypeName()));
  }
  m_pGroup->SetTitle(sb);
}

void wdQtPropertyPointerWidget::OnDeleteButtonClicked()
{
  m_pObjectAccessor->StartTransaction("Delete Object");

  wdStatus res;
  const wdHybridArray<wdPropertySelection, 8> selection = m_pTypeWidget->GetSelection();
  for (auto& item : selection)
  {
    res = m_pObjectAccessor->RemoveObject(item.m_pObject);
    if (res.m_Result.Failed())
      break;
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Removing sub-element from the property failed.");
}

void wdQtPropertyPointerWidget::StructureEventHandler(const wdDocumentObjectStructureEvent& e)
{
  if (IsUndead())
    return;

  switch (e.m_EventType)
  {
    case wdDocumentObjectStructureEvent::Type::AfterObjectAdded:
    case wdDocumentObjectStructureEvent::Type::AfterObjectMoved:
    case wdDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    {
      if (!e.m_sParentProperty.IsEqual(m_pProp->GetPropertyName()))
        return;

      if (std::none_of(cbegin(m_Items), cend(m_Items),
            [&](const wdPropertySelection& sel)
            { return e.m_pNewParent == sel.m_pObject || e.m_pPreviousParent == sel.m_pObject; }))
        return;

      SetSelection(m_Items);
    }
    break;
    default:
      break;
  }
}

/// *** wdQtEmbeddedClassPropertyWidget ***

wdQtEmbeddedClassPropertyWidget::wdQtEmbeddedClassPropertyWidget()
  : wdQtPropertyWidget()
  , m_bTemporaryCommand(false)
  , m_pResolvedType(nullptr)
{
}


wdQtEmbeddedClassPropertyWidget::~wdQtEmbeddedClassPropertyWidget()
{
  m_pGrid->GetObjectManager()->m_PropertyEvents.RemoveEventHandler(wdMakeDelegate(&wdQtEmbeddedClassPropertyWidget::PropertyEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtEmbeddedClassPropertyWidget::CommandHistoryEventHandler, this));
}

void wdQtEmbeddedClassPropertyWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtScopedUpdatesDisabled _(this);

  wdQtPropertyWidget::SetSelection(items);

  // Retrieve the objects the property points to. This could be an embedded class or
  // an element of an array, be it pointer or embedded class.
  m_ResolvedObjects.Clear();
  for (const auto& item : m_Items)
  {
    wdUuid ObjectGuid = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, item.m_Index);
    wdPropertySelection sel;
    sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);
    // sel.m_Index; intentionally invalid as we just retrieved the value so it is a pointer to an object

    m_ResolvedObjects.PushBack(sel);
  }

  m_pResolvedType = m_pProp->GetSpecificType();
  if (m_pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
  {
    m_pResolvedType = wdQtPropertyWidget::GetCommonBaseType(m_ResolvedObjects);
  }
}

void wdQtEmbeddedClassPropertyWidget::SetPropertyValue(const wdAbstractProperty* pProperty, const wdVariant& NewValue)
{
  wdStatus res;
  for (const auto& sel : m_ResolvedObjects)
  {
    res = m_pObjectAccessor->SetValue(sel.m_pObject, pProperty, NewValue, sel.m_Index);
    if (res.m_Result.Failed())
      break;
  }
  // wdPropertyEvent ed;
  // ed.m_Type = wdPropertyEvent::Type::SingleValueChanged;
  // ed.m_pProperty = pProperty;
  // ed.m_Value = NewValue;
  // ed.m_pItems = &m_ResolvedObjects;

  // m_Events.Broadcast(ed);
}

void wdQtEmbeddedClassPropertyWidget::OnInit()
{
  m_pGrid->GetObjectManager()->m_PropertyEvents.AddEventHandler(wdMakeDelegate(&wdQtEmbeddedClassPropertyWidget::PropertyEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.AddEventHandler(wdMakeDelegate(&wdQtEmbeddedClassPropertyWidget::CommandHistoryEventHandler, this));
}


void wdQtEmbeddedClassPropertyWidget::DoPrepareToDie() {}

void wdQtEmbeddedClassPropertyWidget::PropertyEventHandler(const wdDocumentObjectPropertyEvent& e)
{
  if (IsUndead())
    return;

  if (std::none_of(cbegin(m_ResolvedObjects), cend(m_ResolvedObjects), [=](const wdPropertySelection& sel)
        { return e.m_pObject == sel.m_pObject; }))
    return;

  if (!m_QueuedChanges.Contains(e.m_sProperty))
  {
    m_QueuedChanges.PushBack(e.m_sProperty);
  }
}


void wdQtEmbeddedClassPropertyWidget::CommandHistoryEventHandler(const wdCommandHistoryEvent& e)
{
  if (IsUndead())
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

void wdQtEmbeddedClassPropertyWidget::FlushQueuedChanges()
{
  for (const wdString& sProperty : m_QueuedChanges)
  {
    OnPropertyChanged(sProperty);
  }

  m_QueuedChanges.Clear();
}

/// *** wdQtPropertyTypeWidget ***

wdQtPropertyTypeWidget::wdQtPropertyTypeWidget(bool bAddCollapsibleGroup)
  : wdQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);
  m_pGroup = nullptr;
  m_pGroupLayout = nullptr;

  if (bAddCollapsibleGroup)
  {
    m_pGroup = new wdQtCollapsibleGroupBox(this);
    m_pGroupLayout = new QHBoxLayout(nullptr);
    m_pGroupLayout->setSpacing(1);
    m_pGroupLayout->setContentsMargins(5, 0, 0, 0);
    m_pGroup->GetContent()->setLayout(m_pGroupLayout);

    m_pLayout->addWidget(m_pGroup);
  }
  m_pTypeWidget = nullptr;
}

wdQtPropertyTypeWidget::~wdQtPropertyTypeWidget() {}

void wdQtPropertyTypeWidget::OnInit()
{
  if (m_pGroup)
  {
    m_pGroup->SetTitle(wdTranslate(m_pProp->GetPropertyName()));
    m_pGrid->SetCollapseState(m_pGroup);
    connect(m_pGroup, &wdQtGroupBoxBase::CollapseStateChanged, m_pGrid, &wdQtPropertyGridWidget::OnCollapseStateChanged);
  }
}

void wdQtPropertyTypeWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtScopedUpdatesDisabled _(this);

  wdQtPropertyWidget::SetSelection(items);

  QHBoxLayout* pLayout = m_pGroup != nullptr ? m_pGroupLayout : m_pLayout;
  QWidget* pOwner = m_pGroup != nullptr ? m_pGroup->GetContent() : this;
  if (m_pTypeWidget)
  {
    pLayout->removeWidget(m_pTypeWidget);
    delete m_pTypeWidget;
    m_pTypeWidget = nullptr;
  }

  // Retrieve the objects the property points to. This could be an embedded class or
  // an element of an array, be it pointer or embedded class.
  wdHybridArray<wdPropertySelection, 8> ResolvedObjects;
  for (const auto& item : m_Items)
  {
    wdUuid ObjectGuid = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, item.m_Index);
    wdPropertySelection sel;
    sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);
    // sel.m_Index; intentionally invalid as we just retrieved the value so it is a pointer to an object

    ResolvedObjects.PushBack(sel);
  }

  const wdRTTI* pCommonType = nullptr;
  if (m_pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
  {
    pCommonType = wdQtPropertyWidget::GetCommonBaseType(ResolvedObjects);
  }
  else
  {
    // If we create a widget for a member class we already determined the common base type at the parent type widget.
    // As we are not dealing with a pointer in this case the type must match the property exactly.
    pCommonType = m_pProp->GetSpecificType();
  }
  m_pTypeWidget = new wdQtTypeWidget(pOwner, m_pGrid, m_pObjectAccessor, pCommonType, nullptr, nullptr);
  pLayout->addWidget(m_pTypeWidget);
  m_pTypeWidget->SetSelection(ResolvedObjects);
}


void wdQtPropertyTypeWidget::SetIsDefault(bool bIsDefault)
{
  // The default state set by the parent object / container only refers to the element's correct position in the container but the entire state of the object. As recursively checking an entire object if is has any non-default values is quite costly, we just pretend the object is never in its default state the the user can click revert to default on any object at any time.
  m_bIsDefault = false;
}

void wdQtPropertyTypeWidget::DoPrepareToDie()
{
  if (m_pTypeWidget)
  {
    m_pTypeWidget->PrepareToDie();
  }
}

/// *** wdQtPropertyContainerWidget ***

wdQtPropertyContainerWidget::wdQtPropertyContainerWidget()
  : wdQtPropertyWidget()
  , m_pAddButton(nullptr)
{
  m_Pal = palette();
  setAutoFillBackground(true);

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pGroup = new wdQtCollapsibleGroupBox(this);
  m_pGroupLayout = new QVBoxLayout(nullptr);
  m_pGroupLayout->setSpacing(1);
  m_pGroupLayout->setContentsMargins(5, 0, 0, 0);
  m_pGroup->GetContent()->setLayout(m_pGroupLayout);
  m_pGroup->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(m_pGroup, &QWidget::customContextMenuRequested, this, &wdQtPropertyContainerWidget::OnContainerContextMenu);

  setAcceptDrops(true);
  m_pLayout->addWidget(m_pGroup);
}

wdQtPropertyContainerWidget::~wdQtPropertyContainerWidget()
{
  Clear();
}

void wdQtPropertyContainerWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtPropertyWidget::SetSelection(items);

  UpdateElements();

  if (m_pAddButton)
  {
    m_pAddButton->SetSelection(m_Items);
  }
}

void wdQtPropertyContainerWidget::SetIsDefault(bool bIsDefault)
{
  // This is called from the type widget which we ignore as we have a tighter scoped default value provider for containers.
}

void wdQtPropertyContainerWidget::DoPrepareToDie()
{
  for (const auto& e : m_Elements)
  {
    e.m_pWidget->PrepareToDie();
  }
}

void wdQtPropertyContainerWidget::dragEnterEvent(QDragEnterEvent* event)
{
  updateDropIndex(event);
}

void wdQtPropertyContainerWidget::dragMoveEvent(QDragMoveEvent* event)
{
  updateDropIndex(event);
}

void wdQtPropertyContainerWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
  m_iDropSource = -1;
  m_iDropTarget = -1;
  update();
}

void wdQtPropertyContainerWidget::dropEvent(QDropEvent* event)
{
  if (updateDropIndex(event))
  {
    wdQtGroupBoxBase* pGroup = qobject_cast<wdQtGroupBoxBase*>(event->source());
    Element* pDragElement =
      std::find_if(begin(m_Elements), end(m_Elements), [pGroup](const Element& elem) -> bool
        { return elem.m_pSubGroup == pGroup; });
    if (pDragElement)
    {
      const wdAbstractProperty* pProp = pDragElement->m_pWidget->GetProperty();
      wdHybridArray<wdPropertySelection, 8> items = pDragElement->m_pWidget->GetSelection();
      if (m_iDropSource != m_iDropTarget && (m_iDropSource + 1) != m_iDropTarget)
      {
        MoveItems(items, m_iDropTarget - m_iDropSource);
      }
    }
  }
  m_iDropSource = -1;
  m_iDropTarget = -1;
  update();
}

void wdQtPropertyContainerWidget::paintEvent(QPaintEvent* event)
{
  wdQtPropertyWidget::paintEvent(event);
  if (m_iDropSource != -1 && m_iDropTarget != -1)
  {
    wdInt32 iYPos = 0;
    if (m_iDropTarget < (wdInt32)m_Elements.GetCount())
    {
      const QPoint globalPos = m_Elements[m_iDropTarget].m_pSubGroup->mapToGlobal(QPoint(0, 0));
      iYPos = mapFromGlobal(globalPos).y();
    }
    else
    {
      const QPoint globalPos = m_Elements[m_Elements.GetCount() - 1].m_pSubGroup->mapToGlobal(QPoint(0, 0));
      iYPos = mapFromGlobal(globalPos).y() + m_Elements[m_Elements.GetCount() - 1].m_pSubGroup->height();
    }

    QPainter painter(this);
    painter.setPen(QPen(Qt::PenStyle::NoPen));
    painter.setBrush(palette().brush(QPalette::Highlight));
    painter.drawRect(0, iYPos - 3, width(), 4);
  }
}

void wdQtPropertyContainerWidget::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set palette.
  setPalette(m_Pal);
  wdQtPropertyWidget::showEvent(event);
}

bool wdQtPropertyContainerWidget::updateDropIndex(QDropEvent* pEvent)
{
  if (pEvent->source() && pEvent->mimeData()->hasFormat("application/x-groupBoxDragProperty"))
  {
    // Is the drop source part of this widget?
    for (wdUInt32 i = 0; i < m_Elements.GetCount(); i++)
    {
      if (m_Elements[i].m_pSubGroup == pEvent->source())
      {
        pEvent->setDropAction(Qt::MoveAction);
        pEvent->accept();
        wdInt32 iNewDropTarget = -1;
        // Find closest drop target.
        const wdInt32 iGlobalYPos = mapToGlobal(pEvent->pos()).y();
        for (wdUInt32 j = 0; j < m_Elements.GetCount(); j++)
        {
          const QRect rect(m_Elements[j].m_pSubGroup->mapToGlobal(QPoint(0, 0)), m_Elements[j].m_pSubGroup->size());
          if (iGlobalYPos > rect.center().y())
          {
            iNewDropTarget = (wdInt32)j + 1;
          }
          else if (iGlobalYPos < rect.center().y())
          {
            iNewDropTarget = (wdInt32)j;
            break;
          }
        }
        if (m_iDropSource != (wdInt32)i || m_iDropTarget != iNewDropTarget)
        {
          m_iDropSource = (wdInt32)i;
          m_iDropTarget = iNewDropTarget;
          update();
        }
        return true;
      }
    }
  }

  if (m_iDropSource != -1 || m_iDropTarget != -1)
  {
    m_iDropSource = -1;
    m_iDropTarget = -1;
    update();
  }
  pEvent->ignore();
  return false;
}

void wdQtPropertyContainerWidget::OnElementButtonClicked()
{
  wdQtElementGroupButton* pButton = qobject_cast<wdQtElementGroupButton*>(sender());
  const wdAbstractProperty* pProp = pButton->GetGroupWidget()->GetProperty();
  wdHybridArray<wdPropertySelection, 8> items = pButton->GetGroupWidget()->GetSelection();

  switch (pButton->GetAction())
  {
    case wdQtElementGroupButton::ElementAction::MoveElementUp:
    {
      MoveItems(items, -1);
    }
    break;
    case wdQtElementGroupButton::ElementAction::MoveElementDown:
    {
      MoveItems(items, 2);
    }
    break;
    case wdQtElementGroupButton::ElementAction::DeleteElement:
    {
      DeleteItems(items);
    }
    break;

    case wdQtElementGroupButton::ElementAction::Help:
      // handled by custom lambda
      break;
  }
}

void wdQtPropertyContainerWidget::OnDragStarted(QMimeData& ref_mimeData)
{
  wdQtGroupBoxBase* pGroup = qobject_cast<wdQtGroupBoxBase*>(sender());
  Element* pDragElement =
    std::find_if(begin(m_Elements), end(m_Elements), [pGroup](const Element& elem) -> bool
      { return elem.m_pSubGroup == pGroup; });
  if (pDragElement)
  {
    ref_mimeData.setData("application/x-groupBoxDragProperty", QByteArray());
  }
}

void wdQtPropertyContainerWidget::OnContainerContextMenu(const QPoint& pt)
{
  wdQtGroupBoxBase* pGroup = qobject_cast<wdQtGroupBoxBase*>(sender());

  QMenu m;
  m.setToolTipsVisible(true);
  ExtendContextMenu(m);

  if (!m.isEmpty())
  {
    m.exec(pGroup->mapToGlobal(pt));
  }
}

void wdQtPropertyContainerWidget::OnCustomElementContextMenu(const QPoint& pt)
{
  wdQtGroupBoxBase* pGroup = qobject_cast<wdQtGroupBoxBase*>(sender());
  Element* pElement = std::find_if(begin(m_Elements), end(m_Elements), [pGroup](const Element& elem) -> bool
    { return elem.m_pSubGroup == pGroup; });

  if (pElement)
  {
    QMenu m;
    m.setToolTipsVisible(true);
    pElement->m_pWidget->ExtendContextMenu(m);

    m_pGrid->ExtendContextMenu(m, pElement->m_pWidget->GetSelection(), pElement->m_pWidget->GetProperty());

    if (!m.isEmpty())
    {
      m.exec(pGroup->mapToGlobal(pt));
    }
  }
}

wdQtGroupBoxBase* wdQtPropertyContainerWidget::CreateElement(QWidget* pParent)
{
  auto pBox = new wdQtCollapsibleGroupBox(pParent);
  pBox->SetFillColor(palette().window().color());
  return pBox;
}

wdQtPropertyWidget* wdQtPropertyContainerWidget::CreateWidget(wdUInt32 index)
{
  return new wdQtPropertyTypeWidget();
}

wdQtPropertyContainerWidget::Element& wdQtPropertyContainerWidget::AddElement(wdUInt32 index)
{
  wdQtGroupBoxBase* pSubGroup = CreateElement(m_pGroup);
  pSubGroup->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(pSubGroup, &wdQtGroupBoxBase::CollapseStateChanged, m_pGrid, &wdQtPropertyGridWidget::OnCollapseStateChanged);
  connect(pSubGroup, &QWidget::customContextMenuRequested, this, &wdQtPropertyContainerWidget::OnCustomElementContextMenu);

  QVBoxLayout* pSubLayout = new QVBoxLayout(nullptr);
  pSubLayout->setContentsMargins(5, 0, 0, 0);
  pSubLayout->setSpacing(1);
  pSubGroup->GetContent()->setLayout(pSubLayout);

  m_pGroupLayout->insertWidget((int)index, pSubGroup);

  wdQtPropertyWidget* pNewWidget = CreateWidget(index);

  pNewWidget->setParent(pSubGroup);
  pSubLayout->addWidget(pNewWidget);

  pNewWidget->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);

  // Add Buttons
  auto pAttr = m_pProp->GetAttributeByType<wdContainerAttribute>();
  if ((!pAttr || pAttr->CanMove()) && m_pProp->GetCategory() != wdPropertyCategory::Map)
  {
    pSubGroup->SetDraggable(true);
    connect(pSubGroup, &wdQtGroupBoxBase::DragStarted, this, &wdQtPropertyContainerWidget::OnDragStarted);
  }

  wdQtElementGroupButton* pHelpButton = new wdQtElementGroupButton(pSubGroup->GetHeader(), wdQtElementGroupButton::ElementAction::Help, pNewWidget);
  pSubGroup->GetHeader()->layout()->addWidget(pHelpButton);
  pHelpButton->setVisible(false); // added now, and shown later when we know the URL

  if (!pAttr || pAttr->CanDelete())
  {
    wdQtElementGroupButton* pDeleteButton =
      new wdQtElementGroupButton(pSubGroup->GetHeader(), wdQtElementGroupButton::ElementAction::DeleteElement, pNewWidget);
    pSubGroup->GetHeader()->layout()->addWidget(pDeleteButton);
    connect(pDeleteButton, &QToolButton::clicked, this, &wdQtPropertyContainerWidget::OnElementButtonClicked);
  }

  m_Elements.Insert(Element(pSubGroup, pNewWidget, pHelpButton), index);
  return m_Elements[index];
}

void wdQtPropertyContainerWidget::RemoveElement(wdUInt32 index)
{
  Element& elem = m_Elements[index];

  m_pGroupLayout->removeWidget(elem.m_pSubGroup);
  delete elem.m_pSubGroup;
  m_Elements.RemoveAtAndCopy(index);
}

void wdQtPropertyContainerWidget::UpdateElements()
{
  wdQtScopedUpdatesDisabled _(this);

  wdUInt32 iElements = GetRequiredElementCount();

  while (m_Elements.GetCount() > iElements)
  {
    RemoveElement(m_Elements.GetCount() - 1);
  }
  while (m_Elements.GetCount() < iElements)
  {
    AddElement(m_Elements.GetCount());
  }

  for (wdUInt32 i = 0; i < iElements; ++i)
  {
    UpdateElement(i);
  }

  UpdatePropertyMetaState();

  // Force re-layout of parent hierarchy to prevent flicker.
  QWidget* pCur = m_pGroup;
  while (pCur != nullptr && qobject_cast<QScrollArea*>(pCur) == nullptr)
  {
    pCur->updateGeometry();
    pCur = pCur->parentWidget();
  }
}

wdUInt32 wdQtPropertyContainerWidget::GetRequiredElementCount() const
{
  if (m_pProp->GetCategory() == wdPropertyCategory::Map)
  {
    m_Keys.Clear();
    WD_VERIFY(m_pObjectAccessor->GetKeys(m_Items[0].m_pObject, m_pProp, m_Keys).m_Result.Succeeded(), "GetKeys should always succeed.");
    wdHybridArray<wdVariant, 16> keys;
    for (wdUInt32 i = 1; i < m_Items.GetCount(); i++)
    {
      keys.Clear();
      WD_VERIFY(m_pObjectAccessor->GetKeys(m_Items[i].m_pObject, m_pProp, keys).m_Result.Succeeded(), "GetKeys should always succeed.");
      for (wdInt32 k = (wdInt32)m_Keys.GetCount() - 1; k >= 0; --k)
      {
        if (!keys.Contains(m_Keys[k]))
        {
          m_Keys.RemoveAtAndSwap(k);
        }
      }
    }
    m_Keys.Sort([](const wdVariant& a, const wdVariant& b)
      { return a.Get<wdString>().Compare(b.Get<wdString>()) < 0; });
    return m_Keys.GetCount();
  }
  else
  {
    wdInt32 iElements = 0x7FFFFFFF;
    for (const auto& item : m_Items)
    {
      wdInt32 iCount = 0;
      WD_VERIFY(m_pObjectAccessor->GetCount(item.m_pObject, m_pProp, iCount).m_Result.Succeeded(), "GetCount should always succeed.");
      iElements = wdMath::Min(iElements, iCount);
    }
    WD_ASSERT_DEV(iElements >= 0, "Mismatch between storage and RTTI ({0})", iElements);
    m_Keys.Clear();
    for (wdUInt32 i = 0; i < (wdUInt32)iElements; i++)
    {
      m_Keys.PushBack(i);
    }

    return wdUInt32(iElements);
  }
}

void wdQtPropertyContainerWidget::UpdatePropertyMetaState()
{
  wdPropertyMetaState* pMeta = wdPropertyMetaState::GetSingleton();
  wdHashTable<wdVariant, wdPropertyUiState> ElementStates;
  pMeta->GetContainerElementsState(m_Items, m_pProp->GetPropertyName(), ElementStates);

  wdDefaultContainerState defaultState(m_pObjectAccessor, m_Items, m_pProp->GetPropertyName());
  m_bIsDefault = defaultState.IsDefaultContainer();
  m_pGroup->SetBoldTitle(!m_bIsDefault);

  QColor qColor = wdQtPropertyWidget::SetPaletteBackgroundColor(defaultState.GetBackgroundColor(), m_Pal);
  setPalette(m_Pal);

  const bool bReadOnly = m_pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly) ||
                         (m_pProp->GetAttributeByType<wdReadOnlyAttribute>() != nullptr);
  for (wdUInt32 i = 0; i < m_Elements.GetCount(); i++)
  {
    Element& element = m_Elements[i];
    wdVariant& key = m_Keys[i];
    const bool bIsDefault = defaultState.IsDefaultElement(key);
    auto itData = ElementStates.Find(key);
    wdPropertyUiState::Visibility state = wdPropertyUiState::Default;
    if (itData.IsValid())
    {
      state = itData.Value().m_Visibility;
    }

    if (element.m_pSubGroup)
    {
      element.m_pSubGroup->setVisible(state != wdPropertyUiState::Invisible);
      element.m_pSubGroup->setEnabled(!bReadOnly && state != wdPropertyUiState::Disabled);
      element.m_pSubGroup->SetBoldTitle(!bIsDefault);

      // If the fill color is invalid that means no border is drawn and we don't want to change the color then.
      if (element.m_pSubGroup->GetFillColor().isValid())
      {
        element.m_pSubGroup->SetFillColor(qColor);
      }
    }
    if (element.m_pWidget)
    {
      element.m_pWidget->setVisible(state != wdPropertyUiState::Invisible);
      element.m_pSubGroup->setEnabled(!bReadOnly && state != wdPropertyUiState::Disabled);
      element.m_pWidget->SetIsDefault(bIsDefault);
    }
  }
}

void wdQtPropertyContainerWidget::Clear()
{
  while (m_Elements.GetCount() > 0)
  {
    RemoveElement(m_Elements.GetCount() - 1);
  }

  m_Elements.Clear();
}

void wdQtPropertyContainerWidget::OnInit()
{
  wdStringBuilder fullname(m_pType->GetTypeName(), "::", m_pProp->GetPropertyName());

  m_pGroup->SetTitle(wdTranslate(fullname));

  const wdContainerAttribute* pArrayAttr = m_pProp->GetAttributeByType<wdContainerAttribute>();
  if (!pArrayAttr || pArrayAttr->CanAdd())
  {
    m_pAddButton = new wdQtAddSubElementButton();
    m_pAddButton->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);
    m_pGroup->GetHeader()->layout()->addWidget(m_pAddButton);
  }

  m_pGrid->SetCollapseState(m_pGroup);
  connect(m_pGroup, &wdQtGroupBoxBase::CollapseStateChanged, m_pGrid, &wdQtPropertyGridWidget::OnCollapseStateChanged);
}

void wdQtPropertyContainerWidget::DeleteItems(wdHybridArray<wdPropertySelection, 8>& items)
{
  m_pObjectAccessor->StartTransaction("Delete Object");

  wdStatus res(WD_SUCCESS);
  const bool bIsValueType = wdReflectionUtils::IsValueType(m_pProp);

  if (bIsValueType)
  {
    for (auto& item : items)
    {
      res = m_pObjectAccessor->RemoveValue(item.m_pObject, m_pProp, item.m_Index);
      if (res.m_Result.Failed())
        break;
    }
  }
  else
  {
    wdRemoveObjectCommand cmd;

    for (auto& item : items)
    {
      wdUuid value = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, item.m_Index);
      const wdDocumentObject* pObject = m_pObjectAccessor->GetObject(value);
      res = m_pObjectAccessor->RemoveObject(pObject);
      if (res.m_Result.Failed())
        break;
    }
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Removing sub-element from the property failed.");
}

void wdQtPropertyContainerWidget::MoveItems(wdHybridArray<wdPropertySelection, 8>& items, wdInt32 iMove)
{
  WD_ASSERT_DEV(m_pProp->GetCategory() != wdPropertyCategory::Map, "Map entries can't be moved.");

  m_pObjectAccessor->StartTransaction("Reparent Object");

  wdStatus res(WD_SUCCESS);
  const bool bIsValueType = wdReflectionUtils::IsValueType(m_pProp);
  if (bIsValueType)
  {
    for (auto& item : items)
    {
      wdInt32 iCurIndex = item.m_Index.ConvertTo<wdInt32>() + iMove;
      if (iCurIndex < 0 || iCurIndex > m_pObjectAccessor->GetCount(item.m_pObject, m_pProp))
        continue;

      res = m_pObjectAccessor->MoveValue(item.m_pObject, m_pProp, item.m_Index, iCurIndex);
      if (res.m_Result.Failed())
        break;
    }
  }
  else
  {
    wdMoveObjectCommand cmd;

    for (auto& item : items)
    {
      wdInt32 iCurIndex = item.m_Index.ConvertTo<wdInt32>() + iMove;
      if (iCurIndex < 0 || iCurIndex > m_pObjectAccessor->GetCount(item.m_pObject, m_pProp))
        continue;

      wdUuid value = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, item.m_Index);
      const wdDocumentObject* pObject = m_pObjectAccessor->GetObject(value);

      res = m_pObjectAccessor->MoveObject(pObject, item.m_pObject, m_pProp, iCurIndex);
      if (res.m_Result.Failed())
        break;
    }
  }

  if (res.m_Result.Failed())
    m_pObjectAccessor->CancelTransaction();
  else
    m_pObjectAccessor->FinishTransaction();

  wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Moving sub-element failed.");
}


/// *** wdQtPropertyStandardTypeContainerWidget ***

wdQtPropertyStandardTypeContainerWidget::wdQtPropertyStandardTypeContainerWidget()
  : wdQtPropertyContainerWidget()
{
}

wdQtPropertyStandardTypeContainerWidget::~wdQtPropertyStandardTypeContainerWidget() {}

wdQtGroupBoxBase* wdQtPropertyStandardTypeContainerWidget::CreateElement(QWidget* pParent)
{
  auto* pBox = new wdQtInlinedGroupBox(pParent);
  pBox->SetFillColor(QColor::Invalid);
  return pBox;
}


wdQtPropertyWidget* wdQtPropertyStandardTypeContainerWidget::CreateWidget(wdUInt32 index)
{
  return wdQtPropertyGridWidget::CreateMemberPropertyWidget(m_pProp);
}

wdQtPropertyContainerWidget::Element& wdQtPropertyStandardTypeContainerWidget::AddElement(wdUInt32 index)
{
  wdQtPropertyContainerWidget::Element& elem = wdQtPropertyContainerWidget::AddElement(index);
  return elem;
}

void wdQtPropertyStandardTypeContainerWidget::RemoveElement(wdUInt32 index)
{
  wdQtPropertyContainerWidget::RemoveElement(index);
}

void wdQtPropertyStandardTypeContainerWidget::UpdateElement(wdUInt32 index)
{
  Element& elem = m_Elements[index];

  wdHybridArray<wdPropertySelection, 8> SubItems;

  for (const auto& item : m_Items)
  {
    wdPropertySelection sel;
    sel.m_pObject = item.m_pObject;
    sel.m_Index = m_Keys[index];

    SubItems.PushBack(sel);
  }

  wdStringBuilder sTitle;
  if (m_pProp->GetCategory() == wdPropertyCategory::Map)
    sTitle.Format("{0}", m_Keys[index].ConvertTo<wdString>());
  else
    sTitle.Format("[{0}]", m_Keys[index].ConvertTo<wdString>());

  elem.m_pSubGroup->SetTitle(sTitle);
  m_pGrid->SetCollapseState(elem.m_pSubGroup);
  elem.m_pWidget->SetSelection(SubItems);
}

/// *** wdQtPropertyTypeContainerWidget ***

wdQtPropertyTypeContainerWidget::wdQtPropertyTypeContainerWidget()
  : m_bNeedsUpdate(false)
{
}

wdQtPropertyTypeContainerWidget::~wdQtPropertyTypeContainerWidget()
{
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.RemoveEventHandler(
    wdMakeDelegate(&wdQtPropertyTypeContainerWidget::StructureEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtPropertyTypeContainerWidget::CommandHistoryEventHandler, this));
}

void wdQtPropertyTypeContainerWidget::OnInit()
{
  wdQtPropertyContainerWidget::OnInit();
  m_pGrid->GetDocument()->GetObjectManager()->m_StructureEvents.AddEventHandler(
    wdMakeDelegate(&wdQtPropertyTypeContainerWidget::StructureEventHandler, this));
  m_pGrid->GetCommandHistory()->m_Events.AddEventHandler(wdMakeDelegate(&wdQtPropertyTypeContainerWidget::CommandHistoryEventHandler, this));
}

void wdQtPropertyTypeContainerWidget::UpdateElement(wdUInt32 index)
{
  Element& elem = m_Elements[index];
  wdHybridArray<wdPropertySelection, 8> SubItems;

  // To be in line with all other wdQtPropertyWidget the container element will
  // be given a selection in the form of this is the parent object, this is the property and in this
  // specific case this is the index you are working on. So SubItems only decorates the items with the correct index.
  for (const auto& item : m_Items)
  {
    wdPropertySelection sel;
    sel.m_pObject = item.m_pObject;
    sel.m_Index = m_Keys[index];

    SubItems.PushBack(sel);
  }

  {
    // To get the correct name we actually need to resolve the selection to the actual objects
    // they are pointing to.
    wdHybridArray<wdPropertySelection, 8> ResolvedObjects;
    for (const auto& item : SubItems)
    {
      wdUuid ObjectGuid = m_pObjectAccessor->Get<wdUuid>(item.m_pObject, m_pProp, item.m_Index);
      wdPropertySelection sel;
      sel.m_pObject = m_pObjectAccessor->GetObject(ObjectGuid);
      ResolvedObjects.PushBack(sel);
    }

    const wdRTTI* pCommonType = wdQtPropertyWidget::GetCommonBaseType(ResolvedObjects);

    // Label
    {
      wdStringBuilder sTitle;
      sTitle.Format("[{0}] - {1}", m_Keys[index].ConvertTo<wdString>(), wdTranslate(pCommonType->GetTypeName()));

      if (auto pInDev = pCommonType->GetAttributeByType<wdInDevelopmentAttribute>())
      {
        sTitle.AppendFormat(" [ {} ]", pInDev->GetString());
      }

      elem.m_pSubGroup->SetTitle(sTitle);
    }

    // Icon
    {
      wdStringBuilder sIconName;
      sIconName.Set(":/TypeIcons/", pCommonType->GetTypeName());
      elem.m_pSubGroup->SetIcon(wdQtUiServices::GetCachedIconResource(sIconName.GetData()));
    }

    // help URL
    {
      QString url = wdTranslateHelpURL(pCommonType->GetTypeName());

      if (!url.isEmpty())
      {
        elem.m_pHelpButton->setVisible(true);
        connect(elem.m_pHelpButton, &QToolButton::clicked, this, [=]()
          { QDesktopServices::openUrl(QUrl(url)); });
      }
      else
      {
        elem.m_pHelpButton->setVisible(false);
      }
    }
  }


  m_pGrid->SetCollapseState(elem.m_pSubGroup);
  elem.m_pWidget->SetSelection(SubItems);
}

void wdQtPropertyTypeContainerWidget::StructureEventHandler(const wdDocumentObjectStructureEvent& e)
{
  if (IsUndead())
    return;

  switch (e.m_EventType)
  {
    case wdDocumentObjectStructureEvent::Type::AfterObjectAdded:
    case wdDocumentObjectStructureEvent::Type::AfterObjectMoved:
    case wdDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    {
      if (!e.m_sParentProperty.IsEqual(m_pProp->GetPropertyName()))
        return;

      if (std::none_of(cbegin(m_Items), cend(m_Items),
            [&](const wdPropertySelection& sel)
            { return e.m_pNewParent == sel.m_pObject || e.m_pPreviousParent == sel.m_pObject; }))
        return;

      m_bNeedsUpdate = true;
    }
    break;
    default:
      break;
  }
}

void wdQtPropertyTypeContainerWidget::CommandHistoryEventHandler(const wdCommandHistoryEvent& e)
{
  if (IsUndead())
    return;

  switch (e.m_Type)
  {
    case wdCommandHistoryEvent::Type::UndoEnded:
    case wdCommandHistoryEvent::Type::RedoEnded:
    case wdCommandHistoryEvent::Type::TransactionEnded:
    case wdCommandHistoryEvent::Type::TransactionCanceled:
    {
      if (m_bNeedsUpdate)
      {
        m_bNeedsUpdate = false;
        UpdateElements();
      }
    }
    break;

    default:
      break;
  }
}

/// *** wdQtVariantPropertyWidget ***

wdQtVariantPropertyWidget::wdQtVariantPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);
}


wdQtVariantPropertyWidget::~wdQtVariantPropertyWidget() {}

void wdQtVariantPropertyWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtStandardPropertyWidget::SetSelection(items);
}

void wdQtVariantPropertyWidget::ExtendContextMenu(QMenu& ref_menu)
{
  wdQtStandardPropertyWidget::ExtendContextMenu(ref_menu);

  QMenu* ctm = ref_menu.addMenu(QStringLiteral("Change Type"));
  for (int i = wdVariantType::FirstStandardType + 1; i < wdVariantType::LastStandardType; ++i)
  {
    if (i == wdVariantType::StringView || i == wdVariantType::DataBuffer)
      continue;
    auto type = static_cast<wdVariantType::Enum>(i);
    const wdRTTI* pVariantEnum = wdRTTI::FindTypeByName("wdVariantType");
    wdStringBuilder sName;
    bool res = wdReflectionUtils::EnumerationToString(pVariantEnum, type, sName);
    QAction* action = ctm->addAction(sName.GetData(), [this, type]()
      { ChangeVariantType(type); });
    if (m_OldValue.GetType() == type)
      action->setChecked(true);
  }
}

void wdQtVariantPropertyWidget::InternalSetValue(const wdVariant& value)
{
  wdVariantType::Enum commonType = wdVariantType::Invalid;
  const bool sameType = GetCommonVariantSubType(m_Items, m_pProp, commonType);
  const wdRTTI* pNewtSubType = commonType != wdVariantType::Invalid ? wdReflectionUtils::GetTypeFromVariant(commonType) : nullptr;
  if (pNewtSubType != m_pCurrentSubType || m_pWidget == nullptr)
  {
    if (m_pWidget)
    {
      m_pWidget->PrepareToDie();
      m_pWidget->deleteLater();
      m_pWidget = nullptr;
    }
    m_pCurrentSubType = pNewtSubType;
    if (pNewtSubType)
    {
      m_pWidget = wdQtPropertyGridWidget::GetFactory().CreateObject(pNewtSubType);
      if (!m_pWidget)
        m_pWidget = new wdQtUnsupportedPropertyWidget("Unsupported type");
    }
    else if (!sameType)
    {
      m_pWidget = new wdQtUnsupportedPropertyWidget("Multi-selection has varying types, RMB to change.");
    }
    else
    {
      m_pWidget = new wdQtUnsupportedPropertyWidget("Variant set to invalid, RMB to change.");
    }
    m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_pWidget->setParent(this);
    m_pLayout->addWidget(m_pWidget);
    m_pWidget->Init(m_pGrid, m_pObjectAccessor, m_pType, m_pProp);
  }
  m_pWidget->SetSelection(m_Items);
}

void wdQtVariantPropertyWidget::ChangeVariantType(wdVariantType::Enum type)
{

  m_pObjectAccessor->StartTransaction("Change variant type");
  // check if we have multiple values
  for (const auto& item : m_Items)
  {
    wdVariant value;
    WD_VERIFY(m_pObjectAccessor->GetValue(item.m_pObject, m_pProp, value, item.m_Index).Succeeded(), "");
    if (value.CanConvertTo(type))
    {
      WD_VERIFY(m_pObjectAccessor->SetValue(item.m_pObject, m_pProp, value.ConvertTo(type), item.m_Index).Succeeded(), "");
    }
    else
    {
      WD_VERIFY(
        m_pObjectAccessor->SetValue(item.m_pObject, m_pProp, wdReflectionUtils::GetDefaultVariantFromType(type), item.m_Index).Succeeded(), "");
    }
  }
  m_pObjectAccessor->FinishTransaction();
}

void wdQtVariantPropertyWidget::DoPrepareToDie()
{
  if (m_pWidget)
    m_pWidget->PrepareToDie();
}
