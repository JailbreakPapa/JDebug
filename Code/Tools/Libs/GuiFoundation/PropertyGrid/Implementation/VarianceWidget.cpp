#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/PropertyGrid/Implementation/VarianceWidget.moc.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QBoxLayout>
#include <QSlider>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

nsQtVarianceTypeWidget::nsQtVarianceTypeWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pValueWidget = new nsQtDoubleSpinBox(this);
  m_pValueWidget->installEventFilter(m_pValueWidget);
  m_pValueWidget->setMinimum(-nsMath::Infinity<double>());
  m_pValueWidget->setMaximum(nsMath::Infinity<double>());
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

void nsQtVarianceTypeWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtStandardPropertyWidget::SetSelection(items);
  NS_ASSERT_DEBUG(m_pProp->GetSpecificType()->IsDerivedFrom<nsVarianceTypeBase>(), "Selection does not match nsVarianceType.");
}

void nsQtVarianceTypeWidget::onBeginTemporary()
{
  if (!m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;
}

void nsQtVarianceTypeWidget::onEndTemporary()
{
  if (m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void nsQtVarianceTypeWidget::SlotValueChanged()
{
  onBeginTemporary();

  nsVariant value;
  nsToolsReflectionUtils::GetVariantFromFloat(m_pValueWidget->value(), m_pValueProp->GetSpecificType()->GetVariantType(), value);

  auto obj = m_OldValue.Get<nsTypedObject>();
  void* pCopy = nsReflectionSerializer::Clone(obj.m_pObject, obj.m_pType);
  nsReflectionUtils::SetMemberPropertyValue(m_pValueProp, pCopy, value);
  nsVariant newValue;
  newValue.MoveTypedObject(pCopy, obj.m_pType);

  BroadcastValueChanged(newValue);
}


void nsQtVarianceTypeWidget::SlotVarianceChanged()
{
  double variance = nsMath::Clamp<double>(m_pVarianceWidget->value() / 100.0, 0, 1);

  nsVariant newValue = m_OldValue;
  nsTypedPointer ptr = newValue.GetWriteAccess();
  nsReflectionUtils::SetMemberPropertyValue(m_pVarianceProp, ptr.m_pObject, variance);

  BroadcastValueChanged(newValue);
}

void nsQtVarianceTypeWidget::OnInit()
{
  m_pValueProp = static_cast<const nsAbstractMemberProperty*>(GetProperty()->GetSpecificType()->FindPropertyByName("Value"));
  m_pVarianceProp = static_cast<const nsAbstractMemberProperty*>(GetProperty()->GetSpecificType()->FindPropertyByName("Variance"));

  // Property type adjustments
  nsQtScopedBlockSignals bs(m_pValueWidget);
  const nsRTTI* pValueType = m_pValueProp->GetSpecificType();
  if (pValueType == nsGetStaticRTTI<nsTime>())
  {
    m_pValueWidget->setDisplaySuffix(" sec");
  }
  else if (pValueType == nsGetStaticRTTI<nsAngle>())
  {
    m_pValueWidget->setDisplaySuffix(nsStringUtf8(L"\u00B0").GetData());
  }

  // Handle attributes
  if (const nsSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<nsSuffixAttribute>())
  {
    m_pValueWidget->setDisplaySuffix(pSuffix->GetSuffix());
  }
  if (const nsClampValueAttribute* pClamp = m_pProp->GetAttributeByType<nsClampValueAttribute>())
  {
    if (pClamp->GetMinValue().CanConvertTo<double>())
    {
      m_pValueWidget->setMinimum(pClamp->GetMinValue());
    }
    else if (const nsRTTI* pType = pClamp->GetMinValue().GetReflectedType(); pType && pType->IsDerivedFrom<nsVarianceTypeBase>())
    {
      m_pValueWidget->setMinimum(pClamp->GetMinValue()["Value"]);
      m_pVarianceWidget->setMinimum(static_cast<nsInt32>(pClamp->GetMinValue()["Variance"].ConvertTo<double>() * 100.0));
    }
    if (pClamp->GetMaxValue().CanConvertTo<double>())
    {
      m_pValueWidget->setMaximum(pClamp->GetMaxValue());
    }
    else if (const nsRTTI* pType = pClamp->GetMaxValue().GetReflectedType(); pType && pType->IsDerivedFrom<nsVarianceTypeBase>())
    {
      m_pValueWidget->setMaximum(pClamp->GetMaxValue()["Value"]);
      m_pVarianceWidget->setMaximum(static_cast<nsInt32>(pClamp->GetMaxValue()["Variance"].ConvertTo<double>() * 100.0));
    }
  }
  if (const nsDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<nsDefaultValueAttribute>())
  {
    if (pDefault->GetValue().CanConvertTo<double>())
    {
      m_pValueWidget->setDefaultValue(pDefault->GetValue());
    }
    else if (const nsRTTI* pType = pDefault->GetValue().GetReflectedType(); pType && pType->IsDerivedFrom<nsVarianceTypeBase>())
    {
      m_pValueWidget->setDefaultValue(pDefault->GetValue()["Value"]);
    }
  }
}

void nsQtVarianceTypeWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals bs(m_pValueWidget, m_pVarianceWidget);
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
