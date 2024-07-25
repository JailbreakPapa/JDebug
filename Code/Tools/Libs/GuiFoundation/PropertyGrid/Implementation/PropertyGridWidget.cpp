#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/Implementation/ExpressionPropertyWidget.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/PropertyWidget.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/TagSetPropertyWidget.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/VarianceWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <GuiFoundation/Widgets/CollapsibleGroupBox.moc.h>
#include <GuiFoundation/Widgets/CurveEditData.h>

#include <ToolsFoundation/Document/Document.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Types/VarianceTypes.h>
#include <Foundation/Types/VariantTypeRegistry.h>


nsRttiMappedObjectFactory<nsQtPropertyWidget> nsQtPropertyGridWidget::s_Factory;

static nsQtPropertyWidget* StandardTypeCreator(const nsRTTI* pRtti)
{
  NS_ASSERT_DEV(pRtti->GetTypeFlags().IsSet(nsTypeFlags::StandardType), "This function is only valid for StandardType properties, regardless of category");

  if (pRtti == nsGetStaticRTTI<nsVariant>())
  {
    return new nsQtVariantPropertyWidget();
  }

  switch (pRtti->GetVariantType())
  {
    case nsVariant::Type::Bool:
      return new nsQtPropertyEditorCheckboxWidget();

    case nsVariant::Type::Time:
      return new nsQtPropertyEditorTimeWidget();

    case nsVariant::Type::Float:
    case nsVariant::Type::Double:
      return new nsQtPropertyEditorDoubleSpinboxWidget(1);

    case nsVariant::Type::Vector2:
      return new nsQtPropertyEditorDoubleSpinboxWidget(2);

    case nsVariant::Type::Vector3:
      return new nsQtPropertyEditorDoubleSpinboxWidget(3);

    case nsVariant::Type::Vector4:
      return new nsQtPropertyEditorDoubleSpinboxWidget(4);

    case nsVariant::Type::Vector2I:
      return new nsQtPropertyEditorIntSpinboxWidget(2, -2147483645, 2147483645);

    case nsVariant::Type::Vector3I:
      return new nsQtPropertyEditorIntSpinboxWidget(3, -2147483645, 2147483645);

    case nsVariant::Type::Vector4I:
      return new nsQtPropertyEditorIntSpinboxWidget(4, -2147483645, 2147483645);

    case nsVariant::Type::Vector2U:
      return new nsQtPropertyEditorIntSpinboxWidget(2, 0, 2147483645);

    case nsVariant::Type::Vector3U:
      return new nsQtPropertyEditorIntSpinboxWidget(3, 0, 2147483645);

    case nsVariant::Type::Vector4U:
      return new nsQtPropertyEditorIntSpinboxWidget(4, 0, 2147483645);

    case nsVariant::Type::Quaternion:
      return new nsQtPropertyEditorQuaternionWidget();

    case nsVariant::Type::Int8:
      return new nsQtPropertyEditorIntSpinboxWidget(1, -127, 127);

    case nsVariant::Type::UInt8:
      return new nsQtPropertyEditorIntSpinboxWidget(1, 0, 255);

    case nsVariant::Type::Int16:
      return new nsQtPropertyEditorIntSpinboxWidget(1, -32767, 32767);

    case nsVariant::Type::UInt16:
      return new nsQtPropertyEditorIntSpinboxWidget(1, 0, 65535);

    case nsVariant::Type::Int32:
    case nsVariant::Type::Int64:
      return new nsQtPropertyEditorIntSpinboxWidget(1, -2147483645, 2147483645);

    case nsVariant::Type::UInt32:
    case nsVariant::Type::UInt64:
      return new nsQtPropertyEditorIntSpinboxWidget(1, 0, 2147483645);

    case nsVariant::Type::String:
    case nsVariant::Type::StringView:
      return new nsQtPropertyEditorLineEditWidget();

    case nsVariant::Type::Color:
    case nsVariant::Type::ColorGamma:
      return new nsQtPropertyEditorColorWidget();

    case nsVariant::Type::Angle:
      return new nsQtPropertyEditorAngleWidget();

    case nsVariant::Type::HashedString:
      return new nsQtPropertyEditorLineEditWidget();

    default:
      NS_REPORT_FAILURE("No default property widget available for type: {0}", pRtti->GetTypeName());
      return nullptr;
  }
}

static nsQtPropertyWidget* EnumCreator(const nsRTTI* pRtti)
{
  return new nsQtPropertyEditorEnumWidget();
}

static nsQtPropertyWidget* BitflagsCreator(const nsRTTI* pRtti)
{
  return new nsQtPropertyEditorBitflagsWidget();
}

static nsQtPropertyWidget* TagSetCreator(const nsRTTI* pRtti)
{
  return new nsQtPropertyEditorTagSetWidget();
}

static nsQtPropertyWidget* VarianceTypeCreator(const nsRTTI* pRtti)
{
  return new nsQtVarianceTypeWidget();
}

static nsQtPropertyWidget* Curve1DTypeCreator(const nsRTTI* pRtti)
{
  return new nsQtPropertyEditorCurve1DWidget();
}

static nsQtPropertyWidget* ExpressionTypeCreator(const nsRTTI* pRtti)
{
  return new nsQtPropertyEditorExpressionWidget();
}

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, PropertyGrid)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "ToolsFoundation", "PropertyMetaState"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<bool>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<float>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<double>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec2>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec3>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec4>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec2I32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec3I32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec4I32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec2U32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec3U32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVec4U32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsQuat>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsInt8>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsUInt8>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsInt16>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsUInt16>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsInt32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsUInt32>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsInt64>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsUInt64>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsConstCharPtr>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsString>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsStringView>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsTime>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsColor>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsColorGammaUB>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsAngle>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVariant>(), StandardTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsHashedString>(), StandardTypeCreator);

    // TODO: nsMat3, nsMat4, nsTransform, nsUuid, nsVariant
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsEnumBase>(), EnumCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsBitflagsBase>(), BitflagsCreator);

    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsTagSetWidgetAttribute>(), TagSetCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsVarianceTypeBase>(), VarianceTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsSingleCurveData>(), Curve1DTypeCreator);
    nsQtPropertyGridWidget::GetFactory().RegisterCreator(nsGetStaticRTTI<nsExpressionWidgetAttribute>(), ExpressionTypeCreator);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<bool>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<float>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<double>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec2>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec3>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec4>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec2I32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec3I32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec4I32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec2U32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec3U32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVec4U32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsQuat>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsInt8>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsUInt8>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsInt16>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsUInt16>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsInt32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsUInt32>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsInt64>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsUInt64>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsConstCharPtr>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsString>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsStringView>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsTime>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsColor>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsColorGammaUB>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsAngle>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVariant>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsHashedString>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsEnumBase>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsBitflagsBase>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsTagSetWidgetAttribute>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsVarianceTypeBase>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsSingleCurveData>());
    nsQtPropertyGridWidget::GetFactory().UnregisterCreator(nsGetStaticRTTI<nsExpressionWidgetAttribute>());
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsRttiMappedObjectFactory<nsQtPropertyWidget>& nsQtPropertyGridWidget::GetFactory()
{
  return s_Factory;
}

nsQtPropertyGridWidget::nsQtPropertyGridWidget(QWidget* pParent, nsDocument* pDocument, bool bBindToSelectionManager)
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

  s_Factory.m_Events.AddEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::FactoryEventHandler, this));
  nsPhantomRttiManager::s_Events.AddEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::TypeEventHandler, this));

  SetDocument(pDocument, bBindToSelectionManager);
}

nsQtPropertyGridWidget::~nsQtPropertyGridWidget()
{
  s_Factory.m_Events.RemoveEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::FactoryEventHandler, this));
  nsPhantomRttiManager::s_Events.RemoveEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::TypeEventHandler, this));

  if (m_pDocument)
  {
    m_pDocument->m_ObjectAccessorChangeEvents.RemoveEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::ObjectAccessorChangeEventHandler, this));
    m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::SelectionEventHandler, this));
  }
}


void nsQtPropertyGridWidget::SetDocument(nsDocument* pDocument, bool bBindToSelectionManager)
{
  m_bBindToSelectionManager = bBindToSelectionManager;
  if (m_pDocument)
  {
    m_pDocument->m_ObjectAccessorChangeEvents.RemoveEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::ObjectAccessorChangeEventHandler, this));
    m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::SelectionEventHandler, this));
  }

  m_pDocument = pDocument;

  if (m_pDocument)
  {
    m_pDocument->m_ObjectAccessorChangeEvents.AddEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::ObjectAccessorChangeEventHandler, this));
    m_pDocument->GetSelectionManager()->m_Events.AddEventHandler(nsMakeDelegate(&nsQtPropertyGridWidget::SelectionEventHandler, this));
  }
}

void nsQtPropertyGridWidget::ClearSelection()
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

void nsQtPropertyGridWidget::SetSelectionIncludeExcludeProperties(const char* szIncludeProperties /*= nullptr*/, const char* szExcludeProperties /*= nullptr*/)
{
  m_sSelectionIncludeProperties = szIncludeProperties;
  m_sSelectionExcludeProperties = szExcludeProperties;
}

void nsQtPropertyGridWidget::SetSelection(const nsDeque<const nsDocumentObject*>& selection)
{
  nsQtScopedUpdatesDisabled _(this);

  ClearSelection();

  m_Selection = selection;

  if (m_Selection.IsEmpty())
    return;

  {
    nsHybridArray<nsPropertySelection, 8> Items;
    Items.Reserve(m_Selection.GetCount());

    for (const auto* sel : m_Selection)
    {
      nsPropertySelection s;
      s.m_pObject = sel;

      Items.PushBack(s);
    }

    const nsRTTI* pCommonType = nsQtPropertyWidget::GetCommonBaseType(Items);
    m_pTypeWidget = new nsQtTypeWidget(m_pContent, this, GetObjectAccessor(), pCommonType, m_sSelectionIncludeProperties, m_sSelectionExcludeProperties);
    m_pTypeWidget->SetSelection(Items);

    m_pContentLayout->insertWidget(0, m_pTypeWidget, 0);
  }
}

const nsDocument* nsQtPropertyGridWidget::GetDocument() const
{
  return m_pDocument;
}

const nsDocumentObjectManager* nsQtPropertyGridWidget::GetObjectManager() const
{
  return m_pDocument->GetObjectManager();
}

nsCommandHistory* nsQtPropertyGridWidget::GetCommandHistory() const
{
  return m_pDocument->GetCommandHistory();
}


nsObjectAccessorBase* nsQtPropertyGridWidget::GetObjectAccessor() const
{
  return m_pDocument->GetObjectAccessor();
}

nsQtPropertyWidget* nsQtPropertyGridWidget::CreateMemberPropertyWidget(const nsAbstractProperty* pProp)
{
  // Try to create a registered widget for an existing nsTypeWidgetAttribute.
  const nsTypeWidgetAttribute* pAttrib = pProp->GetAttributeByType<nsTypeWidgetAttribute>();
  if (pAttrib != nullptr)
  {
    nsQtPropertyWidget* pWidget = nsQtPropertyGridWidget::GetFactory().CreateObject(pAttrib->GetDynamicRTTI());
    if (pWidget != nullptr)
      return pWidget;
  }

  // Try to create a registered widget for the given property type.
  nsQtPropertyWidget* pWidget = nsQtPropertyGridWidget::GetFactory().CreateObject(pProp->GetSpecificType());
  if (pWidget != nullptr)
    return pWidget;

  return new nsQtUnsupportedPropertyWidget("No property grid widget registered");
}

nsQtPropertyWidget* nsQtPropertyGridWidget::CreatePropertyWidget(const nsAbstractProperty* pProp)
{
  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      // Try to create a registered widget for an existing nsTypeWidgetAttribute.
      const nsTypeWidgetAttribute* pAttrib = pProp->GetAttributeByType<nsTypeWidgetAttribute>();
      if (pAttrib != nullptr)
      {
        nsQtPropertyWidget* pWidget = nsQtPropertyGridWidget::GetFactory().CreateObject(pAttrib->GetDynamicRTTI());
        if (pWidget != nullptr)
          return pWidget;
      }

      if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer))
      {
        if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
          return new nsQtPropertyPointerWidget();
        else
          return new nsQtUnsupportedPropertyWidget("Pointer: Use nsPropertyFlags::PointerOwner or provide derived nsTypeWidgetAttribute");
      }
      else
      {
        nsQtPropertyWidget* pWidget = nsQtPropertyGridWidget::GetFactory().CreateObject(pProp->GetSpecificType());
        if (pWidget != nullptr)
          return pWidget;

        if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
        {
          // Member struct / class
          return new nsQtPropertyTypeWidget(true);
        }
      }
    }
    break;
    case nsPropertyCategory::Set:
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Map:
    {
      // Try to create a registered container widget for an existing nsContainerWidgetAttribute.
      const nsContainerWidgetAttribute* pAttrib = pProp->GetAttributeByType<nsContainerWidgetAttribute>();
      if (pAttrib != nullptr)
      {
        nsQtPropertyWidget* pWidget = nsQtPropertyGridWidget::GetFactory().CreateObject(pAttrib->GetDynamicRTTI());
        if (pWidget != nullptr)
          return pWidget;
      }

      // Fallback to default container widgets.
      const bool bIsValueType = nsReflectionUtils::IsValueType(pProp);
      if (bIsValueType)
      {
        return new nsQtPropertyStandardTypeContainerWidget();
      }
      else
      {
        if (pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) && !pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
        {
          return new nsQtUnsupportedPropertyWidget("Pointer: Use nsPropertyFlags::PointerOwner or provide derived nsContainerWidgetAttribute");
        }

        return new nsQtPropertyTypeContainerWidget();
      }
    }
    break;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  return new nsQtUnsupportedPropertyWidget();
}

void nsQtPropertyGridWidget::SetCollapseState(nsQtGroupBoxBase* pBox)
{
  nsUInt32 uiHash = GetGroupBoxHash(pBox);
  bool bCollapsed = false;
  auto it = m_CollapseState.Find(uiHash);
  if (it.IsValid())
    bCollapsed = it.Value();

  pBox->SetCollapseState(bCollapsed);
}

void nsQtPropertyGridWidget::OnCollapseStateChanged(bool bCollapsed)
{
  nsQtGroupBoxBase* pBox = qobject_cast<nsQtGroupBoxBase*>(sender());
  nsUInt32 uiHash = GetGroupBoxHash(pBox);
  m_CollapseState[uiHash] = pBox->GetCollapseState();
}

void nsQtPropertyGridWidget::ObjectAccessorChangeEventHandler(const nsObjectAccessorChangeEvent& e)
{
  SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
}

void nsQtPropertyGridWidget::SelectionEventHandler(const nsSelectionManagerEvent& e)
{
  // TODO: even when not binding to the selection manager we need to test whether our selection is still valid.
  if (!m_bBindToSelectionManager)
    return;

  switch (e.m_Type)
  {
    case nsSelectionManagerEvent::Type::SelectionCleared:
    {
      ClearSelection();
    }
    break;
    case nsSelectionManagerEvent::Type::SelectionSet:
    case nsSelectionManagerEvent::Type::ObjectAdded:
    case nsSelectionManagerEvent::Type::ObjectRemoved:
    {
      SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
    }
    break;
  }
}

void nsQtPropertyGridWidget::FactoryEventHandler(const nsRttiMappedObjectFactory<nsQtPropertyWidget>::Event& e)
{
  if (m_bBindToSelectionManager)
    SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
  else
  {
    nsDeque<const nsDocumentObject*> selection = m_Selection;
    SetSelection(selection);
  }
}

void nsQtPropertyGridWidget::TypeEventHandler(const nsPhantomRttiManagerEvent& e)
{
  // Adding types cannot affect the property grid content.
  if (e.m_Type == nsPhantomRttiManagerEvent::Type::TypeAdded)
    return;

  NS_PROFILE_SCOPE("TypeEventHandler");
  if (m_bBindToSelectionManager)
    SetSelection(m_pDocument->GetSelectionManager()->GetSelection());
  else
  {
    nsDeque<const nsDocumentObject*> selection = m_Selection;
    SetSelection(selection);
  }
}

nsUInt32 nsQtPropertyGridWidget::GetGroupBoxHash(nsQtGroupBoxBase* pBox) const
{
  nsUInt32 uiHash = 0;

  QWidget* pCur = pBox;
  while (pCur != nullptr && pCur != this)
  {
    nsQtGroupBoxBase* pCurBox = qobject_cast<nsQtGroupBoxBase*>(pCur);
    if (pCurBox != nullptr)
    {
      const QByteArray name = pCurBox->GetTitle().toUtf8().data();
      uiHash += nsHashingUtils::xxHash32(name, name.length());
    }
    pCur = pCur->parentWidget();
  }
  return uiHash;
}
