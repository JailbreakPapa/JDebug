#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/PropertyGrid/Implementation/VarianceWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QBoxLayout>
#include <QSlider>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

wdQtVarianceTypeWidget::wdQtVarianceTypeWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pValueWidget = new wdQtDoubleSpinBox(this);
  m_pValueWidget->installEventFilter(m_pValueWidget);
  m_pValueWidget->setMinimum(-wdMath::Infinity<double>());
  m_pValueWidget->setMaximum(wdMath::Infinity<double>());
  m_pValueWidget->setSingleStep(0.1f);
  m_pValueWidget->setAccelerated(true);
  m_pValueWidget->setDecimals(2);

  m_pVarianceWidget = new QSlider(this);
  m_pVarianceWidget->setOrientation(Qt::Orientation::Horizontal);
  m_pVarianceWidget->setMinimum(0);
  m_pVarianceWidget->setMaximum(100);
  m_pVarianceWidget->setSingleStep(1);

  m_pLayout->addWidget(m_pValueWidget);
  m_pLayout->addWidget(m_pVarianceWidget);

  connect(m_pValueWidget, SIGNAL(editingFinished()), this, SLOT(onEndTemporary()));
  connect(m_pValueWidget, SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  connect(m_pVarianceWidget, SIGNAL(sliderPressed()), this, SLOT(onBeginTemporary()));
  connect(m_pVarianceWidget, SIGNAL(sliderReleased()), this, SLOT(onEndTemporary()));
  connect(m_pVarianceWidget, SIGNAL(valueChanged(int)), this, SLOT(SlotVarianceChanged()));
}

void wdQtVarianceTypeWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtStandardPropertyWidget::SetSelection(items);
  WD_ASSERT_DEBUG(m_pProp->GetSpecificType()->IsDerivedFrom<wdVarianceTypeBase>(), "Selection does not match wdVarianceType.");
}

void wdQtVarianceTypeWidget::onBeginTemporary()
{
  if (!m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;
}

void wdQtVarianceTypeWidget::onEndTemporary()
{
  if (m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void wdQtVarianceTypeWidget::SlotValueChanged()
{
  onBeginTemporary();

  wdVariant value;
  wdToolsReflectionUtils::GetVariantFromFloat(m_pValueWidget->value(), m_pValueProp->GetSpecificType()->GetVariantType(), value);

  auto obj = m_OldValue.Get<wdTypedObject>();
  void* pCopy = wdReflectionSerializer::Clone(obj.m_pObject, obj.m_pType);
  wdReflectionUtils::SetMemberPropertyValue(m_pValueProp, pCopy, value);
  wdVariant newValue;
  newValue.MoveTypedObject(pCopy, obj.m_pType);

  BroadcastValueChanged(newValue);
}


void wdQtVarianceTypeWidget::SlotVarianceChanged()
{
  double variance = wdMath::Clamp<double>(m_pVarianceWidget->value() / 100.0, 0, 1);

  wdVariant newValue = m_OldValue;
  wdTypedPointer ptr = newValue.GetWriteAccess();
  wdReflectionUtils::SetMemberPropertyValue(m_pVarianceProp, ptr.m_pObject, variance);

  BroadcastValueChanged(newValue);
}

void wdQtVarianceTypeWidget::OnInit()
{
  m_pValueProp = static_cast<wdAbstractMemberProperty*>(GetProperty()->GetSpecificType()->FindPropertyByName("Value"));
  m_pVarianceProp = static_cast<wdAbstractMemberProperty*>(GetProperty()->GetSpecificType()->FindPropertyByName("Variance"));

  // Property type adjustments
  wdQtScopedBlockSignals bs(m_pValueWidget);
  const wdRTTI* pValueType = m_pValueProp->GetSpecificType();
  if (pValueType == wdGetStaticRTTI<wdTime>())
  {
    m_pValueWidget->setDisplaySuffix(" sec");
  }
  else if (pValueType == wdGetStaticRTTI<wdAngle>())
  {
    m_pValueWidget->setDisplaySuffix(wdStringUtf8(L"\u00B0").GetData());
  }

  // Handle attributes
  if (const wdSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<wdSuffixAttribute>())
  {
    m_pValueWidget->setDisplaySuffix(pSuffix->GetSuffix());
  }
  if (const wdClampValueAttribute* pClamp = m_pProp->GetAttributeByType<wdClampValueAttribute>())
  {
    if (pClamp->GetMinValue().CanConvertTo<double>())
    {
      m_pValueWidget->setMinimum(pClamp->GetMinValue());
    }
    else if (const wdRTTI* pType = pClamp->GetMinValue().GetReflectedType(); pType && pType->IsDerivedFrom<wdVarianceTypeBase>())
    {
      m_pValueWidget->setMinimum(pClamp->GetMinValue()["Value"]);
      m_pVarianceWidget->setMinimum(static_cast<wdInt32>(pClamp->GetMinValue()["Variance"].ConvertTo<double>() * 100.0));
    }
    if (pClamp->GetMaxValue().CanConvertTo<double>())
    {
      m_pValueWidget->setMaximum(pClamp->GetMaxValue());
    }
    else if (const wdRTTI* pType = pClamp->GetMaxValue().GetReflectedType(); pType && pType->IsDerivedFrom<wdVarianceTypeBase>())
    {
      m_pValueWidget->setMaximum(pClamp->GetMaxValue()["Value"]);
      m_pVarianceWidget->setMaximum(static_cast<wdInt32>(pClamp->GetMaxValue()["Variance"].ConvertTo<double>() * 100.0));
    }
  }
  if (const wdDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<wdDefaultValueAttribute>())
  {
    if (pDefault->GetValue().CanConvertTo<double>())
    {
      m_pValueWidget->setDefaultValue(pDefault->GetValue());
    }
    else if (const wdRTTI* pType = pDefault->GetValue().GetReflectedType(); pType && pType->IsDerivedFrom<wdVarianceTypeBase>())
    {
      m_pValueWidget->setDefaultValue(pDefault->GetValue()["Value"]);
    }
  }
}

void wdQtVarianceTypeWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals bs(m_pValueWidget, m_pVarianceWidget);
  if (value.IsValid())
  {
    m_pValueWidget->setValue(value["Value"]);
    m_pVarianceWidget->setValue(value["Variance"].ConvertTo<double>() * 100.0);
  }
  else
  {
    m_pValueWidget->setValueInvalid();
    m_pVarianceWidget->setValue(50);
  }
}
