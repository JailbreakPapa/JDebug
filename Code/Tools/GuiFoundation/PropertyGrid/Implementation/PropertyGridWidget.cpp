#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Types/VarianceTypes.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <GuiFoundation/PropertyGrid/Implementation/PropertyWidget.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/TagSetPropertyWidget.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/VarianceWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <GuiFoundation/Widgets/CollapsibleGroupBox.moc.h>
#include <ToolsFoundation/Document/Document.h>

#include <Foundation/Profiling/Profiling.h>
#include <GuiFoundation/Widgets/CurveEditData.h>
#include <QLayout>
#include <QScrollArea>

wdRttiMappedObjectFactory<wdQtPropertyWidget> wdQtPropertyGridWidget::s_Factory;

static wdQtPropertyWidget* StandardTypeCreator(const wdRTTI* pRtti)
{
  WD_ASSERT_DEV(pRtti->GetTypeFlags().IsSet(wdTypeFlags::StandardType), "This function is only valid for StandardType properties, regardless of category");

  if (pRtti == wdGetStaticRTTI<wdVariant>())
  {
    return new wdQtVariantPropertyWidget();
  }

  switch (pRtti->GetVariantType())
  {
    case wdVariant::Type::Bool:
      return new wdQtPropertyEditorCheckboxWidget();

    case wdVariant::Type::Time:
      return new wdQtPropertyEditorTimeWidget();

    case wdVariant::Type::Float:
    case wdVariant::Type::Double:
      return new wdQtPropertyEditorDoubleSpinboxWidget(1);

    case wdVariant::Type::Vector2:
      return new wdQtPropertyEditorDoubleSpinboxWidget(2);

    case wdVariant::Type::Vector3:
      return new wdQtPropertyEditorDoubleSpinboxWidget(3);

    case wdVariant::Type::Vector4:
      return new wdQtPropertyEditorDoubleSpinboxWidget(4);

    case wdVariant::Type::Vector2I:
      return new wdQtPropertyEditorIntSpinboxWidget(2, -2147483645, 2147483645);

    case wdVariant::Type::Vector3I:
      return new wdQtPropertyEditorIntSpinboxWidget(3, -2147483645, 2147483645);

    case wdVariant::Type::Vector4I:
      return new wdQtPropertyEditorIntSpinboxWidget(4, -2147483645, 2147483645);

    case wdVariant::Type::Vector2U:
      return new wdQtPropertyEditorIntSpinboxWidget(2, 0, 2147483645);

    case wdVariant::Type::Vector3U:
      return new wdQtPropertyEditorIntSpinboxWidget(3, 0, 2147483645);

    case wdVariant::Type::Vector4U:
      return new wdQtPropertyEditorIntSpinboxWidget(4, 0, 2147483645);

    case wdVariant::Type::Quaternion:
      return new wdQtPropertyEditorQuaternionWidget();

    case wdVariant::Type::Int8:
      return new wdQtPropertyEditorIntSpinboxWidget(1, -127, 127);

    case wdVariant::Type::UInt8:
      return new wdQtPropertyEditorIntSpinboxWidget(1, 0, 255);

    case wdVariant::Type::Int16:
      return new wdQtPropertyEditorIntSpinboxWidget(1, -32767, 32767);

    case wdVariant::Type::UInt16:
      return new wdQtPropertyEditorIntSpinboxWidget(1, 0, 65535);

    case wdVariant::Type::Int32:
    case wdVariant::Type::Int64:
      return new wdQtPropertyEditorIntSpinboxWidget(1, -2147483645, 2147483645);

    case wdVariant::Type::UInt32:
    case wdVariant::Type::UInt64:
      return new wdQtPropertyEditorIntSpinboxWidget(1, 0, 2147483645);

    case wdVariant::Type::String:
      return new wdQtPropertyEditorLineEditWidget();

    case wdVariant::Type::Color:
    case wdVariant::Type::ColorGamma:
      return new wdQtPropertyEditorColorWidget();

    case wdVariant::Type::Angle:
      return new wdQtPropertyEditorAngleWidget();


    default:
      WD_REPORT_FAILURE("No default property widget available for type: {0}", pRtti->GetTypeName());
      return nullptr;
  }
}

static wdQtPropertyWidget* EnumCreator(const wdRTTI* pRtti)
{
  return new wdQtPropertyEditorEnumWidget();
}

static wdQtPropertyWidget* BitflagsCreator(const wdRTTI* pRtti)
{
  return new wdQtPropertyEditorBitflagsWidget();
}

static wdQtPropertyWidget* TagSetCreator(const wdRTTI* pRtti)
{
  return new wdQtPropertyEditorTagSetWidget();
}

static wdQtPropertyWidget* VarianceTypeCreator(const wdRTTI* pRtti)
{
  return new wdQtVarianceTypeWidget();
}

static wdQtPropertyWidget* Curve1DTypeCreator(const wdRTTI* pRtti)
{
  return new wdQtPropertyEditorCurve1DWidget();
}

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, PropertyGrid)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "ToolsFoundation", "PropertyMetaState"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<bool>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<float>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<double>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec2>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec3>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec4>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec2I32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec3I32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec4I32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec2U32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec3U32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVec4U32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdQuat>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdInt8>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdUInt8>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdInt16>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdUInt16>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdInt32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdUInt32>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdInt64>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdUInt64>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdConstCharPtr>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdString>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdTime>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdColor>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdColorGammaUB>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdAngle>(), StandardTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVariant>(), StandardTypeCreator);

    // TODO: wdMat3, wdMat4, wdTransform, wdUuid, wdVariant
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdEnumBase>(), EnumCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdBitflagsBase>(), BitflagsCreator);

    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdTagSetWidgetAttribute>(), TagSetCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdVarianceTypeBase>(), VarianceTypeCreator);
    wdQtPropertyGridWidget::GetFactory().RegisterCreator(wdGetStaticRTTI<wdSingleCurveData>(), Curve1DTypeCreator);


  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<bool>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<float>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<double>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec2>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec3>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec4>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec2I32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec3I32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec4I32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec2U32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec3U32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVec4U32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdQuat>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdInt8>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdUInt8>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdInt16>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdUInt16>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdInt32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdUInt32>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdInt64>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdUInt64>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdConstCharPtr>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdString>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdTime>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdColor>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdColorGammaUB>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdAngle>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVariant>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdEnumBase>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdBitflagsBase>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdTagSetWidgetAttribute>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdVarianceTypeBase>());
    wdQtPropertyGridWidget::GetFactory().UnregisterCreator(wdGetStaticRTTI<wdSingleCurveData>());
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdRttiMappedObjectFactory<wdQtPropertyWidget>& wdQtPropertyGridWidget::GetFactory()
{
  return s_Factory;
}

wdQtPropertyGridWidget::wdQtPropertyGridWidget(QWidget* pParent, wdDocument* pDocument, bool bBindToSelectionManager)
  : QWidget(pParent)
{
  m_pDocument = nullptr;

  m_pScroll = new QScrollArea(this);
  m_pScroll->setContentsMargins(0, 0, 0, 0);

  m_pLayout = new QVBoxLayout(this);
  m_pLayout->setSpacing(0);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);
  m_pLayout->addWidget(m_pScroll);

  m_pContent = new QWidget(this);
  m_pScroll->setWidget(m_pContent);
  m_pScroll->setWidgetResizable(true);
  m_pContent->setBackgroundRole(QPalette::ColorRole::Window);
  m_pContent->setAutoFillBackground(true);

  m_pContentLayout = new QVBoxLayout(m_pContent);
  m_pContentLayout->setSpacing(1);
  m_pContentLayout->setContentsMargins(0, 0, 0, 0);
  m_pContent->setLayout(m_pContentLayout);

  m_pSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  m_pContentLayout->addSpacerItem(m_pSpacer);

  m_pTypeWidget = nullptr;

  s_Factory.m_Events.AddEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::FactoryEventHandler, this));
  wdPhantomRttiManager::s_Events.AddEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::TypeEventHandler, this));

  SetDocument(pDocument, bBindToSelectionManager);
}

wdQtPropertyGridWidget::~wdQtPropertyGridWidget()
{
  s_Factory.m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::FactoryEventHandler, this));
  wdPhantomRttiManager::s_Events.RemoveEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::TypeEventHandler, this));

  if (m_pDocument)
  {
    m_pDocument->m_ObjectAccessorChangeEvents.RemoveEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::ObjectAccessorChangeEventHandler, this));
    m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::SelectionEventHandler, this));
  }
}


void wdQtPropertyGridWidget::SetDocument(wdDocument* pDocument, bool bBindToSelectionManager)
{
  m_bBindToSelectionManager = bBindToSelectionManager;
  if (m_pDocument)
  {
    m_pDocument->m_ObjectAccessorChangeEvents.RemoveEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::ObjectAccessorChangeEventHandler, this));
    m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::SelectionEventHandler, this));
  }

  m_pDocument = pDocument;

  if (m_pDocument)
  {
    m_pDocument->m_ObjectAccessorChangeEvents.AddEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::ObjectAccessorChangeEventHandler, this));
    m_pDocument->GetSelectionManager()->m_Events.AddEventHandler(wdMakeDelegate(&wdQtPropertyGridWidget::SelectionEventHandler, this));
  }
}

void wdQtPropertyGridWidget::ClearSelection()
{
  if (m_pTypeWidget)
  {
    m_pContentLayout->removeWidget(m_pTypeWidget);
    m_pTypeWidget->hide();

    m_pTypeWidget->PrepareToDie();

    m_pTypeWidget->deleteLater();
    m_pTypeWidget = nullptr;
  }

  m_Selection.Clear();
}

void wdQtPropertyGridWidget::SetSelectionIncludeExcludeProperties(const char* szIncludeProperties /*= nullptr*/, const char* szExcludeProperties /*= nullptr*/)
{
  m_sSelectionIncludeProperties = szIncludeProperties;
  m_sSelectionExcludeProperties = szExcludeProperties;
}

void wdQtPropertyGridWidget::SetSelection(const wdDeque<const wdDocumentObject*>& selection)
{
  wdQtScopedUpdatesDisabled _(this);

  ClearSelection();

  m_Selection = selection;

  if (m_Selection.IsEmpty())
    return;

  {
    wdHybridArray<wdPropertySelection, 8> Items;
    Items.Reserve(m_Selection.GetCount());

    for (const auto* sel : m_Selection)
    {
      wdPropertySelection s;
      s.m_pObject = sel;

      Items.PushBack(s);
    }

    const wdRTTI* pCommonType = wdQtPropertyWidget::GetCommonBaseType(Items);
    m_pTypeWidget = new wdQtTypeWidget(m_pContent, this, GetObjectAccessor(), pCommonType, m_sSelectionIncludeProperties, m_sSelectionExcludeProperties);
    m_pTypeWidget->SetSelection(Items);

    m_pContentLayout->insertWidget(0, m_pTypeWidget, 0);
  }
}

const wdDocument* wdQtPropertyGridWidget::GetDocument() const
{
  return m_pDocument;
}

const wdDocumentObjectManager* wdQtPropertyGridWidget::GetObjectManager() const
{
  return m_pDocument->GetObjectManager();
}

wdCommandHistory* wdQtPropertyGridWidget::GetCommandHistory() const
{
  return m_pDocument->GetCommandHistory();
}


wdObjectAccessorBase* wdQtPropertyGridWidget::GetObjectAccessor() const
{
  return m_pDocument->GetObjectAccessor();
}

wdQtPropertyWidget* wdQtPropertyGridWidget::CreateMemberPropertyWidget(const wdAbstractProperty* pProp)
{
  // Try to create a registered widget for an existing wdTypeWidgetAttribute.
  const wdTypeWidgetAttribute* pAttrib = pProp->GetAttributeByType<wdTypeWidgetAttribute>();
  if (pAttrib != nullptr)
  {
    wdQtPropertyWidget* pWidget = wdQtPropertyGridWidget::GetFactory().CreateObject(pAttrib->GetDynamicRTTI());
    if (pWidget != nullptr)
      return pWidget;
  }

  // Try to create a registered widget for the given property type.
  wdQtPropertyWidget* pWidget = wdQtPropertyGridWidget::GetFactory().CreateObject(pProp->GetSpecificType());
  if (pWidget != nullptr)
    return pWidget;

  return new wdQtUnsupportedPropertyWidget("No property grid widget registered");
}

wdQtPropertyWidget* wdQtPropertyGridWidget::CreatePropertyWidget(const wdAbstractProperty* pProp)
{
  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      // Try to create a registered widget for an existing wdTypeWidgetAttribute.
      const wdTypeWidgetAttribute* pAttrib = pProp->GetAttributeByType<wdTypeWidgetAttribute>();
      if (pAttrib != nullptr)
      {
        wdQtPropertyWidget* pWidget = wdQtPropertyGridWidget::GetFactory().CreateObject(pAttrib->GetDynamicRTTI());
        if (pWidget != nullptr)
          return pWidget;
      }

      if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer))
      {
        if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
          return new wdQtPropertyPointerWidget();
        else
          return new wdQtUnsupportedPropertyWidget("Pointer: Use wdPropertyFlags::PointerOwner or provide derived wdTypeWidgetAttribute");
      }
      else
      {
        wdQtPropertyWidget* pWidget = wdQtPropertyGridWidget::GetFactory().CreateObject(pProp->GetSpecificType());
        if (pWidget != nullptr)
          return pWidget;

        if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
        {
          // Member struct / class
          return new wdQtPropertyTypeWidget(true);
        }
      }
    }
    break;
    case wdPropertyCategory::Set:
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Map:
    {
      // Try to create a registered container widget for an existing wdContainerWidgetAttribute.
      const wdContainerWidgetAttribute* pAttrib = pProp->GetAttributeByType<wdContainerWidgetAttribute>();
      if (pAttrib != nullptr)
      {
        wdQtPropertyWidget* pWidget = wdQtPropertyGridWidget::GetFactory().CreateObject(pAttrib->GetDynamicRTTI());
        if (pWidget != nullptr)
          return pWidget;
      }

      // Fallback to default container widgets.
      const bool bIsValueType = wdReflectionUtils::IsValueType(pProp);
      if (bIsValueType)
      {
        return new wdQtPropertyStandardTypeContainerWidget();
      }
      else
      {
        if (pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
        {
          return new wdQtUnsupportedPropertyWidget("Pointer: Use wdPropertyFlags::PointerOwner or provide derived wdContainerWidgetAttribute");
        }

        return new wdQtPropertyTypeContainerWidget();
      }
    }
    break;

    default:
      WD_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  return new wdQtUnsupportedPropertyWidget();
}

void wdQtPropertyGridWidget::SetCollapseState(wdQtGroupBoxBase* pBox)
{
  wdUInt32 uiHash = GetGroupBoxHash(pBox);
  bool bCollapsed = false;
  auto it = m_CollapseState.Find(uiHash);
  if (it.IsValid())
    bCollapsed = it.Value();

  pBox->SetCollapseState(bCollapsed);
}

void wdQtPropertyGridWidget::OnCollapseStateChanged(bool bCollapsed)
{
  wdQtGroupBoxBase* pBox = qobject_cast<wdQtGroupBoxBase*>(sender());
  wdUInt32 uiHash = GetGroupBoxHash(pBox);
  m_CollapseState[uiHash] = pBox->GetCollapseState();
}

void wdQtPropertyGridWidget::ObjectAccessorChangeEventHandler(const wdObjectAccessorChangeEvent& e)
{
  SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
}

void wdQtPropertyGridWidget::SelectionEventHandler(const wdSelectionManagerEvent& e)
{
  // TODO: even when not binding to the selection manager we need to test whether our selection is still valid.
  if (!m_bBindToSelectionManager)
    return;

  switch (e.m_Type)
  {
    case wdSelectionManagerEvent::Type::SelectionCleared:
    {
      ClearSelection();
    }
    break;
    case wdSelectionManagerEvent::Type::SelectionSet:
    case wdSelectionManagerEvent::Type::ObjectAdded:
    case wdSelectionManagerEvent::Type::ObjectRemoved:
    {
      SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
    }
    break;
  }
}

void wdQtPropertyGridWidget::FactoryEventHandler(const wdRttiMappedObjectFactory<wdQtPropertyWidget>::Event& e)
{
  if (m_bBindToSelectionManager)
    SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
  else
  {
    wdDeque<const wdDocumentObject*> selection = m_Selection;
    SetSelection(selection);
  }
}

void wdQtPropertyGridWidget::TypeEventHandler(const wdPhantomRttiManagerEvent& e)
{
  // Adding types cannot affect the property grid content.
  if (e.m_Type == wdPhantomRttiManagerEvent::Type::TypeAdded)
    return;

  WD_PROFILE_SCOPE("TypeEventHandler");
  if (m_bBindToSelectionManager)
    SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
  else
  {
    wdDeque<const wdDocumentObject*> selection = m_Selection;
    SetSelection(selection);
  }
}

wdUInt32 wdQtPropertyGridWidget::GetGroupBoxHash(wdQtGroupBoxBase* pBox) const
{
  wdUInt32 uiHash = 0;

  QWidget* pCur = pBox;
  while (pCur != nullptr && pCur != this)
  {
    wdQtGroupBoxBase* pCurBox = qobject_cast<wdQtGroupBoxBase*>(pCur);
    if (pCurBox != nullptr)
    {
      const QByteArray name = pCurBox->GetTitle().toUtf8().data();
      uiHash += wdHashingUtils::xxHash32(name, name.length());
    }
    pCur = pCur->parentWidget();
  }
  return uiHash;
}
