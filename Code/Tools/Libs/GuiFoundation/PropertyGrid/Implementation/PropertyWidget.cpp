/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/Dialogs/CurveEditDlg.moc.h>
#include <GuiFoundation/PropertyGrid/Implementation/PropertyWidget.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/CurveEditData.h>
#include <GuiFoundation/Widgets/DoubleSpinBox.moc.h>
#include <QComboBox>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <qcheckbox.h>
#include <qlayout.h>


/// *** CHECKBOX ***

nsQtPropertyEditorCheckboxWidget::nsQtPropertyEditorCheckboxWidget()
  : nsQtStandardPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QCheckBox(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pLayout->addWidget(m_pWidget);

  NS_VERIFY(connect(m_pWidget, SIGNAL(stateChanged(int)), this, SLOT(on_StateChanged_triggered(int))) != nullptr, "signal/slot connection failed");
}

void nsQtPropertyEditorCheckboxWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals b(m_pWidget);

  if (value.IsValid())
  {
    m_pWidget->setTristate(false);
    m_pWidget->setChecked(value.ConvertTo<bool>() ? Qt::Checked : Qt::Unchecked);
  }
  else
  {
    m_pWidget->setTristate(true);
    m_pWidget->setCheckState(Qt::CheckState::PartiallyChecked);
  }
}

void nsQtPropertyEditorCheckboxWidget::mousePressEvent(QMouseEvent* pEv)
{
  QWidget::mousePressEvent(pEv);

  m_pWidget->toggle();
}

void nsQtPropertyEditorCheckboxWidget::on_StateChanged_triggered(int state)
{
  if (state == Qt::PartiallyChecked)
  {
    nsQtScopedBlockSignals b(m_pWidget);

    m_pWidget->setCheckState(Qt::Checked);
    m_pWidget->setTristate(false);
  }

  BroadcastValueChanged((state != Qt::Unchecked) ? true : false);
}


/// *** DOUBLE SPINBOX ***

nsQtPropertyEditorDoubleSpinboxWidget::nsQtPropertyEditorDoubleSpinboxWidget(nsInt8 iNumComponents)
  : nsQtStandardPropertyWidget()
{
  NS_ASSERT_DEBUG(iNumComponents <= 4, "Only up to 4 components are supported");

  m_iNumComponents = iNumComponents;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  for (nsInt32 c = 0; c < m_iNumComponents; ++c)
  {
    m_pWidget[c] = new nsQtDoubleSpinBox(this);
    m_pWidget[c]->installEventFilter(this);
    m_pWidget[c]->setMinimum(-nsMath::Infinity<double>());
    m_pWidget[c]->setMaximum(nsMath::Infinity<double>());
    m_pWidget[c]->setSingleStep(0.1f);
    m_pWidget[c]->setAccelerated(true);

    policy.setHorizontalStretch(2);
    m_pWidget[c]->setSizePolicy(policy);

    m_pLayout->addWidget(m_pWidget[c]);

    connect(m_pWidget[c], SIGNAL(editingFinished()), this, SLOT(on_EditingFinished_triggered()));
    connect(m_pWidget[c], SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  }
}

void nsQtPropertyEditorDoubleSpinboxWidget::OnInit()
{
  auto pNoTemporaryTransactions = m_pProp->GetAttributeByType<nsNoTemporaryTransactionsAttribute>();
  m_bUseTemporaryTransaction = (pNoTemporaryTransactions == nullptr);

  if (const nsClampValueAttribute* pClamp = m_pProp->GetAttributeByType<nsClampValueAttribute>())
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0]);
        m_pWidget[0]->setMinimum(pClamp->GetMinValue());
        m_pWidget[0]->setMaximum(pClamp->GetMaxValue());
        break;
      }
      case 2:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pClamp->GetMinValue().CanConvertTo<nsVec2>())
        {
          nsVec2 value = pClamp->GetMinValue().ConvertTo<nsVec2>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
        }
        if (pClamp->GetMaxValue().CanConvertTo<nsVec2>())
        {
          nsVec2 value = pClamp->GetMaxValue().ConvertTo<nsVec2>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
        }
        break;
      }
      case 3:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pClamp->GetMinValue().CanConvertTo<nsVec3>())
        {
          nsVec3 value = pClamp->GetMinValue().ConvertTo<nsVec3>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
        }
        if (pClamp->GetMaxValue().CanConvertTo<nsVec3>())
        {
          nsVec3 value = pClamp->GetMaxValue().ConvertTo<nsVec3>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
        }
        break;
      }
      case 4:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pClamp->GetMinValue().CanConvertTo<nsVec4>())
        {
          nsVec4 value = pClamp->GetMinValue().ConvertTo<nsVec4>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
          m_pWidget[3]->setMinimum(value.w);
        }
        if (pClamp->GetMaxValue().CanConvertTo<nsVec4>())
        {
          nsVec4 value = pClamp->GetMaxValue().ConvertTo<nsVec4>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
          m_pWidget[3]->setMaximum(value.w);
        }
        break;
      }
    }
  }

  if (const nsDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<nsDefaultValueAttribute>())
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0]);

        if (pDefault->GetValue().CanConvertTo<double>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<double>());
        }
        break;
      }
      case 2:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pDefault->GetValue().CanConvertTo<nsVec2>())
        {
          nsVec2 value = pDefault->GetValue().ConvertTo<nsVec2>();
          m_pWidget[0]->setDefaultValue(value.x);
          m_pWidget[1]->setDefaultValue(value.y);
        }
        break;
      }
      case 3:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pDefault->GetValue().CanConvertTo<nsVec3>())
        {
          nsVec3 value = pDefault->GetValue().ConvertTo<nsVec3>();
          m_pWidget[0]->setDefaultValue(value.x);
          m_pWidget[1]->setDefaultValue(value.y);
          m_pWidget[2]->setDefaultValue(value.z);
        }
        break;
      }
      case 4:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pDefault->GetValue().CanConvertTo<nsVec4>())
        {
          nsVec4 value = pDefault->GetValue().ConvertTo<nsVec4>();
          m_pWidget[0]->setDefaultValue(value.x);
          m_pWidget[1]->setDefaultValue(value.y);
          m_pWidget[2]->setDefaultValue(value.z);
          m_pWidget[3]->setDefaultValue(value.w);
        }
        break;
      }
    }
  }

  if (const nsSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<nsSuffixAttribute>())
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setDisplaySuffix(pSuffix->GetSuffix());
    }
  }

  if (const nsMinValueTextAttribute* pMinValueText = m_pProp->GetAttributeByType<nsMinValueTextAttribute>())
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setSpecialValueText(pMinValueText->GetText());
    }
  }
}

void nsQtPropertyEditorDoubleSpinboxWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

  m_OriginalType = value.GetType();

  if (value.IsValid())
  {
    switch (m_iNumComponents)
    {
      case 1:
        m_pWidget[0]->setValue(value.ConvertTo<double>());
        break;
      case 2:
        m_pWidget[0]->setValue(value.ConvertTo<nsVec2>().x);
        m_pWidget[1]->setValue(value.ConvertTo<nsVec2>().y);
        break;
      case 3:
        m_pWidget[0]->setValue(value.ConvertTo<nsVec3>().x);
        m_pWidget[1]->setValue(value.ConvertTo<nsVec3>().y);
        m_pWidget[2]->setValue(value.ConvertTo<nsVec3>().z);
        break;
      case 4:
        m_pWidget[0]->setValue(value.ConvertTo<nsVec4>().x);
        m_pWidget[1]->setValue(value.ConvertTo<nsVec4>().y);
        m_pWidget[2]->setValue(value.ConvertTo<nsVec4>().z);
        m_pWidget[3]->setValue(value.ConvertTo<nsVec4>().w);
        break;
    }
  }
  else
  {
    switch (m_iNumComponents)
    {
      case 1:
        m_pWidget[0]->setValueInvalid();
        break;
      case 2:
        m_pWidget[0]->setValueInvalid();
        m_pWidget[1]->setValueInvalid();
        break;
      case 3:
        m_pWidget[0]->setValueInvalid();
        m_pWidget[1]->setValueInvalid();
        m_pWidget[2]->setValueInvalid();
        break;
      case 4:
        m_pWidget[0]->setValueInvalid();
        m_pWidget[1]->setValueInvalid();
        m_pWidget[2]->setValueInvalid();
        m_pWidget[3]->setValueInvalid();
        break;
    }
  }
}

void nsQtPropertyEditorDoubleSpinboxWidget::on_EditingFinished_triggered()
{
  if (m_bUseTemporaryTransaction && m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void nsQtPropertyEditorDoubleSpinboxWidget::SlotValueChanged()
{
  if (m_bUseTemporaryTransaction && !m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  switch (m_iNumComponents)
  {
    case 1:
      BroadcastValueChanged(nsVariant(m_pWidget[0]->value()).ConvertTo(m_OriginalType));
      break;
    case 2:
      BroadcastValueChanged(nsVec2(m_pWidget[0]->value(), m_pWidget[1]->value()));
      break;
    case 3:
      BroadcastValueChanged(nsVec3(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value()));
      break;
    case 4:
      BroadcastValueChanged(nsVec4(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value(), m_pWidget[3]->value()));
      break;
  }
}


/// *** TIME SPINBOX ***

nsQtPropertyEditorTimeWidget::nsQtPropertyEditorTimeWidget()
  : nsQtStandardPropertyWidget()
{
  m_bTemporaryCommand = false;

  m_pWidget = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  {
    m_pWidget = new nsQtDoubleSpinBox(this);
    m_pWidget->installEventFilter(this);
    m_pWidget->setDisplaySuffix(" sec");
    m_pWidget->setMinimum(-nsMath::Infinity<double>());
    m_pWidget->setMaximum(nsMath::Infinity<double>());
    m_pWidget->setSingleStep(0.1f);
    m_pWidget->setAccelerated(true);

    policy.setHorizontalStretch(2);
    m_pWidget->setSizePolicy(policy);

    m_pLayout->addWidget(m_pWidget);

    connect(m_pWidget, SIGNAL(editingFinished()), this, SLOT(on_EditingFinished_triggered()));
    connect(m_pWidget, SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  }
}

void nsQtPropertyEditorTimeWidget::OnInit()
{
  const nsClampValueAttribute* pClamp = m_pProp->GetAttributeByType<nsClampValueAttribute>();
  if (pClamp)
  {
    nsQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setMinimum(pClamp->GetMinValue());
    m_pWidget->setMaximum(pClamp->GetMaxValue());
  }

  const nsDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<nsDefaultValueAttribute>();
  if (pDefault)
  {
    nsQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setDefaultValue(pDefault->GetValue());
  }
}

void nsQtPropertyEditorTimeWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals b0(m_pWidget);
  m_pWidget->setValue(value);
}

void nsQtPropertyEditorTimeWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void nsQtPropertyEditorTimeWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  BroadcastValueChanged(nsTime::MakeFromSeconds(m_pWidget->value()));
}


/// *** ANGLE SPINBOX ***

nsQtPropertyEditorAngleWidget::nsQtPropertyEditorAngleWidget()
  : nsQtStandardPropertyWidget()
{
  m_bTemporaryCommand = false;

  m_pWidget = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  {
    m_pWidget = new nsQtDoubleSpinBox(this);
    m_pWidget->installEventFilter(this);
    m_pWidget->setDisplaySuffix(nsStringUtf8(L"\u00B0").GetData());
    m_pWidget->setMinimum(-nsMath::Infinity<double>());
    m_pWidget->setMaximum(nsMath::Infinity<double>());
    m_pWidget->setSingleStep(0.1f);
    m_pWidget->setAccelerated(true);
    m_pWidget->setDecimals(1);

    policy.setHorizontalStretch(2);
    m_pWidget->setSizePolicy(policy);

    m_pLayout->addWidget(m_pWidget);

    connect(m_pWidget, SIGNAL(editingFinished()), this, SLOT(on_EditingFinished_triggered()));
    connect(m_pWidget, SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  }
}

void nsQtPropertyEditorAngleWidget::OnInit()
{
  const nsClampValueAttribute* pClamp = m_pProp->GetAttributeByType<nsClampValueAttribute>();
  if (pClamp)
  {
    nsQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setMinimum(pClamp->GetMinValue());
    m_pWidget->setMaximum(pClamp->GetMaxValue());
  }

  const nsDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<nsDefaultValueAttribute>();
  if (pDefault)
  {
    nsQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setDefaultValue(pDefault->GetValue());
  }

  const nsSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<nsSuffixAttribute>();
  if (pSuffix)
  {
    m_pWidget->setDisplaySuffix(pSuffix->GetSuffix());
  }

  const nsMinValueTextAttribute* pMinValueText = m_pProp->GetAttributeByType<nsMinValueTextAttribute>();
  if (pMinValueText)
  {
    m_pWidget->setSpecialValueText(pMinValueText->GetText());
  }
}

void nsQtPropertyEditorAngleWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals b0(m_pWidget);
  m_pWidget->setValue(value);
}

void nsQtPropertyEditorAngleWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void nsQtPropertyEditorAngleWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  BroadcastValueChanged(nsAngle::MakeFromDegree(m_pWidget->value()));
}

/// *** INT SPINBOX ***


nsQtPropertyEditorIntSpinboxWidget::nsQtPropertyEditorIntSpinboxWidget(nsInt8 iNumComponents, nsInt32 iMinValue, nsInt32 iMaxValue)
  : nsQtStandardPropertyWidget()
{
  m_iNumComponents = iNumComponents;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();
  policy.setHorizontalStretch(2);

  for (nsInt32 c = 0; c < m_iNumComponents; ++c)
  {
    m_pWidget[c] = new nsQtDoubleSpinBox(this, true);
    m_pWidget[c]->installEventFilter(this);
    m_pWidget[c]->setMinimum(iMinValue);
    m_pWidget[c]->setMaximum(iMaxValue);
    m_pWidget[c]->setSingleStep(1);
    m_pWidget[c]->setAccelerated(true);

    m_pWidget[c]->setSizePolicy(policy);

    m_pLayout->addWidget(m_pWidget[c]);

    connect(m_pWidget[c], SIGNAL(editingFinished()), this, SLOT(on_EditingFinished_triggered()));
    connect(m_pWidget[c], SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  }
}


nsQtPropertyEditorIntSpinboxWidget::~nsQtPropertyEditorIntSpinboxWidget() = default;

void nsQtPropertyEditorIntSpinboxWidget::OnInit()
{
  auto pNoTemporaryTransactions = m_pProp->GetAttributeByType<nsNoTemporaryTransactionsAttribute>();
  m_bUseTemporaryTransaction = (pNoTemporaryTransactions == nullptr);

  if (const nsClampValueAttribute* pClamp = m_pProp->GetAttributeByType<nsClampValueAttribute>())
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        const nsInt32 iMinValue = pClamp->GetMinValue().ConvertTo<nsInt32>();
        const nsInt32 iMaxValue = pClamp->GetMaxValue().ConvertTo<nsInt32>();

        nsQtScopedBlockSignals bs(m_pWidget[0]);
        m_pWidget[0]->setMinimum(pClamp->GetMinValue());
        m_pWidget[0]->setMaximum(pClamp->GetMaxValue());

        if (pClamp->GetMinValue().IsValid() && pClamp->GetMaxValue().IsValid() && (iMaxValue - iMinValue) < 256 && m_bUseTemporaryTransaction)
        {
          nsQtScopedBlockSignals bs2(m_pSlider);

          // we have to create the slider here, because in the constructor we don't know the real
          // min and max values from the nsClampValueAttribute (only the rough type ranges)
          m_pSlider = new QSlider(this);
          m_pSlider->installEventFilter(this);
          m_pSlider->setOrientation(Qt::Orientation::Horizontal);
          m_pSlider->setMinimum(iMinValue);
          m_pSlider->setMaximum(iMaxValue);

          m_pLayout->insertWidget(0, m_pSlider, 5); // make it take up most of the space
          connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(SlotSliderValueChanged(int)));
          connect(m_pSlider, SIGNAL(sliderReleased()), this, SLOT(on_EditingFinished_triggered()));
        }

        break;
      }
      case 2:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pClamp->GetMinValue().CanConvertTo<nsVec2I32>())
        {
          nsVec2I32 value = pClamp->GetMinValue().ConvertTo<nsVec2I32>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
        }
        if (pClamp->GetMaxValue().CanConvertTo<nsVec2I32>())
        {
          nsVec2I32 value = pClamp->GetMaxValue().ConvertTo<nsVec2I32>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
        }
        break;
      }
      case 3:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pClamp->GetMinValue().CanConvertTo<nsVec3I32>())
        {
          nsVec3I32 value = pClamp->GetMinValue().ConvertTo<nsVec3I32>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
        }
        if (pClamp->GetMaxValue().CanConvertTo<nsVec3I32>())
        {
          nsVec3I32 value = pClamp->GetMaxValue().ConvertTo<nsVec3I32>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
        }
        break;
      }
      case 4:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pClamp->GetMinValue().CanConvertTo<nsVec4I32>())
        {
          nsVec4I32 value = pClamp->GetMinValue().ConvertTo<nsVec4I32>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
          m_pWidget[3]->setMinimum(value.w);
        }
        if (pClamp->GetMaxValue().CanConvertTo<nsVec4I32>())
        {
          nsVec4I32 value = pClamp->GetMaxValue().ConvertTo<nsVec4I32>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
          m_pWidget[3]->setMaximum(value.w);
        }
        break;
      }
    }
  }

  if (const nsDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<nsDefaultValueAttribute>())
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pSlider);

        if (pDefault->GetValue().CanConvertTo<nsInt32>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<nsInt32>());

          if (m_pSlider)
          {
            m_pSlider->setValue(pDefault->GetValue().ConvertTo<nsInt32>());
          }
        }
        break;
      }
      case 2:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pDefault->GetValue().CanConvertTo<nsVec2I32>())
        {
          nsVec2I32 value = pDefault->GetValue().ConvertTo<nsVec2I32>();
          m_pWidget[0]->setDefaultValue(value.x);
          m_pWidget[1]->setDefaultValue(value.y);
        }
        break;
      }
      case 3:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pDefault->GetValue().CanConvertTo<nsVec3I32>())
        {
          nsVec3I32 value = pDefault->GetValue().ConvertTo<nsVec3I32>();
          m_pWidget[0]->setDefaultValue(value.x);
          m_pWidget[1]->setDefaultValue(value.y);
          m_pWidget[2]->setDefaultValue(value.z);
        }
        break;
      }
      case 4:
      {
        nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pDefault->GetValue().CanConvertTo<nsVec4I32>())
        {
          nsVec4I32 value = pDefault->GetValue().ConvertTo<nsVec4I32>();
          m_pWidget[0]->setDefaultValue(value.x);
          m_pWidget[1]->setDefaultValue(value.y);
          m_pWidget[2]->setDefaultValue(value.z);
          m_pWidget[3]->setDefaultValue(value.w);
        }
        break;
      }
    }
  }

  if (const nsSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<nsSuffixAttribute>())
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setDisplaySuffix(pSuffix->GetSuffix());
    }
  }

  if (const nsMinValueTextAttribute* pMinValueText = m_pProp->GetAttributeByType<nsMinValueTextAttribute>())
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setSpecialValueText(pMinValueText->GetText());
    }
  }
}

void nsQtPropertyEditorIntSpinboxWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3], m_pSlider);

  m_OriginalType = value.GetType();

  switch (m_iNumComponents)
  {
    case 1:
      m_pWidget[0]->setValue(value.ConvertTo<nsInt32>());

      if (m_pSlider)
      {
        m_pSlider->setValue(value.ConvertTo<nsInt32>());
      }

      break;
    case 2:
      m_pWidget[0]->setValue(value.ConvertTo<nsVec2I32>().x);
      m_pWidget[1]->setValue(value.ConvertTo<nsVec2I32>().y);
      break;
    case 3:
      m_pWidget[0]->setValue(value.ConvertTo<nsVec3I32>().x);
      m_pWidget[1]->setValue(value.ConvertTo<nsVec3I32>().y);
      m_pWidget[2]->setValue(value.ConvertTo<nsVec3I32>().z);
      break;
    case 4:
      m_pWidget[0]->setValue(value.ConvertTo<nsVec4I32>().x);
      m_pWidget[1]->setValue(value.ConvertTo<nsVec4I32>().y);
      m_pWidget[2]->setValue(value.ConvertTo<nsVec4I32>().z);
      m_pWidget[3]->setValue(value.ConvertTo<nsVec4I32>().w);
      break;
  }
}

void nsQtPropertyEditorIntSpinboxWidget::SlotValueChanged()
{
  if (m_bUseTemporaryTransaction && !m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  nsVariant newValue;
  switch (m_iNumComponents)
  {
    case 1:
      newValue = m_pWidget[0]->value();

      if (m_pSlider)
      {
        nsQtScopedBlockSignals b0(m_pSlider);
        m_pSlider->setValue((nsInt32)m_pWidget[0]->value());
      }

      break;
    case 2:
      newValue = nsVec2I32(m_pWidget[0]->value(), m_pWidget[1]->value());
      break;
    case 3:
      newValue = nsVec3I32(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value());
      break;
    case 4:
      newValue = nsVec4I32(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value(), m_pWidget[3]->value());
      break;
  }

  BroadcastValueChanged(newValue.ConvertTo(m_OriginalType));
}

void nsQtPropertyEditorIntSpinboxWidget::SlotSliderValueChanged(int value)
{
  if (m_bUseTemporaryTransaction && !m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  {
    nsQtScopedBlockSignals b0(m_pWidget[0]);
    m_pWidget[0]->setValue(value);
  }

  BroadcastValueChanged(nsVariant(m_pSlider->value()).ConvertTo(m_OriginalType));
}

void nsQtPropertyEditorIntSpinboxWidget::on_EditingFinished_triggered()
{
  if (m_bUseTemporaryTransaction && m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}


/// *** QUATERNION ***

nsQtPropertyEditorQuaternionWidget::nsQtPropertyEditorQuaternionWidget()
  : nsQtStandardPropertyWidget()
{
  m_bTemporaryCommand = false;

  m_pWidget[0] = nullptr;
  m_pWidget[1] = nullptr;
  m_pWidget[2] = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  for (nsInt32 c = 0; c < 3; ++c)
  {
    m_pWidget[c] = new nsQtDoubleSpinBox(this);
    m_pWidget[c]->installEventFilter(this);
    m_pWidget[c]->setMinimum(-nsMath::Infinity<double>());
    m_pWidget[c]->setMaximum(nsMath::Infinity<double>());
    m_pWidget[c]->setSingleStep(1.0);
    m_pWidget[c]->setAccelerated(true);
    m_pWidget[c]->setDisplaySuffix("\xC2\xB0");

    policy.setHorizontalStretch(2);
    m_pWidget[c]->setSizePolicy(policy);

    m_pLayout->addWidget(m_pWidget[c]);

    connect(m_pWidget[c], SIGNAL(editingFinished()), this, SLOT(on_EditingFinished_triggered()));
    connect(m_pWidget[c], SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  }
}

void nsQtPropertyEditorQuaternionWidget::OnInit() {}

void nsQtPropertyEditorQuaternionWidget::InternalSetValue(const nsVariant& value)
{
  if (m_bTemporaryCommand)
    return;

  nsQtScopedBlockSignals b0(m_pWidget[0]);
  nsQtScopedBlockSignals b1(m_pWidget[1]);
  nsQtScopedBlockSignals b2(m_pWidget[2]);

  if (value.IsValid())
  {
    const nsQuat qRot = value.ConvertTo<nsQuat>();
    nsAngle x, y, z;
    qRot.GetAsEulerAngles(x, y, z);

    m_pWidget[0]->setValue(x.GetDegree());
    m_pWidget[1]->setValue(y.GetDegree());
    m_pWidget[2]->setValue(z.GetDegree());
  }
  else
  {
    m_pWidget[0]->setValueInvalid();
    m_pWidget[1]->setValueInvalid();
    m_pWidget[2]->setValueInvalid();
  }
}

void nsQtPropertyEditorQuaternionWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void nsQtPropertyEditorQuaternionWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(nsPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  nsAngle x = nsAngle::MakeFromDegree(m_pWidget[0]->value());
  nsAngle y = nsAngle::MakeFromDegree(m_pWidget[1]->value());
  nsAngle z = nsAngle::MakeFromDegree(m_pWidget[2]->value());

  nsQuat qRot = nsQuat::MakeFromEulerAngles(x, y, z);

  BroadcastValueChanged(qRot);
}

/// *** LINEEDIT ***

nsQtPropertyEditorLineEditWidget::nsQtPropertyEditorLineEditWidget()
  : nsQtStandardPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QLineEdit(this);
  m_pWidget->installEventFilter(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setFocusProxy(m_pWidget);

  m_pLayout->addWidget(m_pWidget);

  connect(m_pWidget, SIGNAL(editingFinished()), this, SLOT(on_TextFinished_triggered()));
}

void nsQtPropertyEditorLineEditWidget::OnInit()
{
  if (m_pProp->GetAttributeByType<nsReadOnlyAttribute>() != nullptr || m_pProp->GetFlags().IsSet(nsPropertyFlags::ReadOnly))
  {
    setEnabled(true);

    nsQtScopedBlockSignals bs(m_pWidget);

    m_pWidget->setReadOnly(true);
    QPalette palette = m_pWidget->palette();
    palette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
    m_pWidget->setPalette(palette);
  }
}

void nsQtPropertyEditorLineEditWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals b(m_pWidget);

  m_OriginalType = value.GetType();

  if (!value.IsValid())
  {
    m_pWidget->setPlaceholderText(QStringLiteral("<Multiple Values>"));
  }
  else
  {
    m_pWidget->setPlaceholderText(QString());
    m_pWidget->setText(QString::fromUtf8(value.ConvertTo<nsString>().GetData()));
  }
}

void nsQtPropertyEditorLineEditWidget::on_TextChanged_triggered(const QString& value)
{
  BroadcastValueChanged(nsVariant(value.toUtf8().data()).ConvertTo(m_OriginalType));
}

void nsQtPropertyEditorLineEditWidget::on_TextFinished_triggered()
{
  BroadcastValueChanged(nsVariant(m_pWidget->text().toUtf8().data()).ConvertTo(m_OriginalType));
}


/// *** COLOR ***

nsQtColorButtonWidget::nsQtColorButtonWidget(QWidget* pParent)
  : QFrame(pParent)
{
  setAutoFillBackground(true);
  setCursor(Qt::PointingHandCursor);
}

void nsQtColorButtonWidget::SetColor(const nsVariant& color)
{
  if (color.IsValid())
  {
    nsColor col0 = color.ConvertTo<nsColor>();
    col0.NormalizeToLdrRange();

    const nsColorGammaUB col = col0;

    QColor qol;
    qol.setRgb(col.r, col.g, col.b, col.a);

    m_Pal.setBrush(QPalette::Window, QBrush(qol, Qt::SolidPattern));
    setPalette(m_Pal);
  }
  else
  {
    setPalette(m_Pal);
  }
}

void nsQtColorButtonWidget::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set palette.
  setPalette(m_Pal);
  QFrame::showEvent(event);
}

void nsQtColorButtonWidget::mouseReleaseEvent(QMouseEvent* event)
{
  Q_EMIT clicked();
}

QSize nsQtColorButtonWidget::sizeHint() const
{
  return minimumSizeHint();
}

QSize nsQtColorButtonWidget::minimumSizeHint() const
{
  QFontMetrics fm(font());

  QStyleOptionFrame opt;
  initStyleOption(&opt);
  return style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(20, fm.height()), this);
}

nsQtPropertyEditorColorWidget::nsQtPropertyEditorColorWidget()
  : nsQtStandardPropertyWidget()
{
  m_bExposeAlpha = false;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new nsQtColorButtonWidget(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

  m_pLayout->addWidget(m_pWidget);

  NS_VERIFY(connect(m_pWidget, SIGNAL(clicked()), this, SLOT(on_Button_triggered())) != nullptr, "signal/slot connection failed");
}

void nsQtPropertyEditorColorWidget::OnInit()
{
  m_bExposeAlpha = (m_pProp->GetAttributeByType<nsExposeColorAlphaAttribute>() != nullptr);
}

void nsQtPropertyEditorColorWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals b(m_pWidget);

  m_OriginalValue = GetOldValue();
  m_pWidget->SetColor(value);
}

void nsQtPropertyEditorColorWidget::on_Button_triggered()
{
  Broadcast(nsPropertyEvent::Type::BeginTemporary);

  bool bShowHDR = false;

  nsColor temp = nsColor::White;
  if (m_OriginalValue.IsValid())
  {
    bShowHDR = m_OriginalValue.IsA<nsColor>();

    temp = m_OriginalValue.ConvertTo<nsColor>();
  }

  nsQtUiServices::GetSingleton()->ShowColorDialog(
    temp, m_bExposeAlpha, bShowHDR, this, SLOT(on_CurrentColor_changed(const nsColor&)), SLOT(on_Color_accepted()), SLOT(on_Color_reset()));
}

void nsQtPropertyEditorColorWidget::on_CurrentColor_changed(const nsColor& color)
{
  nsVariant col;

  if (m_OriginalValue.IsA<nsColorGammaUB>())
  {
    // nsVariant does not down-cast to nsColorGammaUB automatically
    col = nsColorGammaUB(color);
  }
  else
  {
    col = color;
  }

  m_pWidget->SetColor(col);
  BroadcastValueChanged(col);
}

void nsQtPropertyEditorColorWidget::on_Color_reset()
{
  m_pWidget->SetColor(m_OriginalValue);
  Broadcast(nsPropertyEvent::Type::CancelTemporary);
}

void nsQtPropertyEditorColorWidget::on_Color_accepted()
{
  m_OriginalValue = GetOldValue();
  Broadcast(nsPropertyEvent::Type::EndTemporary);
}


/// *** ENUM COMBOBOX ***

nsQtPropertyEditorEnumWidget::nsQtPropertyEditorEnumWidget()
  : nsQtStandardPropertyWidget()
{

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QComboBox(this);
  m_pWidget->installEventFilter(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pLayout->addWidget(m_pWidget);

  connect(m_pWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(on_CurrentEnum_changed(int)));
}

void nsQtPropertyEditorEnumWidget::OnInit()
{
  const nsRTTI* pType = m_pProp->GetSpecificType();

  nsQtScopedBlockSignals bs(m_pWidget);

  nsUInt32 uiCount = pType->GetProperties().GetCount();
  // Start at 1 to skip default value.
  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    auto pProp = pType->GetProperties()[i];

    if (pProp->GetCategory() != nsPropertyCategory::Constant)
      continue;

    const nsAbstractConstantProperty* pConstant = static_cast<const nsAbstractConstantProperty*>(pProp);

    m_pWidget->addItem(QString::fromUtf8(nsTranslate(pConstant->GetPropertyName())), pConstant->GetConstant().ConvertTo<nsInt64>());
  }
}

void nsQtPropertyEditorEnumWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals b(m_pWidget);

  if (value.IsValid())
  {
    nsInt32 iIndex = m_pWidget->findData(value.ConvertTo<nsInt64>());
    NS_ASSERT_DEV(iIndex != -1, "Enum widget is set to an invalid value!");
    m_pWidget->setCurrentIndex(iIndex);
  }
  else
  {
    m_pWidget->setCurrentIndex(-1);
  }
}

void nsQtPropertyEditorEnumWidget::on_CurrentEnum_changed(int iEnum)
{
  nsInt64 iValue = m_pWidget->itemData(iEnum).toLongLong();
  BroadcastValueChanged(iValue);
}


/// *** BITFLAGS COMBOBOX ***

nsQtPropertyEditorBitflagsWidget::nsQtPropertyEditorBitflagsWidget()
  : nsQtStandardPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QPushButton(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pMenu = new QMenu(m_pWidget);
  m_pWidget->setMenu(m_pMenu);
  m_pLayout->addWidget(m_pWidget);

  connect(m_pMenu, SIGNAL(aboutToShow()), this, SLOT(on_Menu_aboutToShow()));
  connect(m_pMenu, SIGNAL(aboutToHide()), this, SLOT(on_Menu_aboutToHide()));
}

nsQtPropertyEditorBitflagsWidget::~nsQtPropertyEditorBitflagsWidget()
{
  m_pWidget->setMenu(nullptr);
  delete m_pMenu;
}

void nsQtPropertyEditorBitflagsWidget::OnInit()
{
  const nsRTTI* enumType = m_pProp->GetSpecificType();

  const nsRTTI* pType = enumType;
  nsUInt32 uiCount = pType->GetProperties().GetCount();

  // Start at 1 to skip default value.
  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    auto pProp = pType->GetProperties()[i];

    if (pProp->GetCategory() != nsPropertyCategory::Constant)
      continue;

    const nsAbstractConstantProperty* pConstant = static_cast<const nsAbstractConstantProperty*>(pProp);

    QWidgetAction* pAction = new QWidgetAction(m_pMenu);
    QCheckBox* pCheckBox = new QCheckBox(QString::fromUtf8(nsTranslate(pConstant->GetPropertyName())), m_pMenu);
    pCheckBox->setCheckable(true);
    pCheckBox->setCheckState(Qt::Unchecked);
    pAction->setDefaultWidget(pCheckBox);

    m_Constants[pConstant->GetConstant().ConvertTo<nsInt64>()] = pCheckBox;
    m_pMenu->addAction(pAction);
  }

  // sets all bits to clear or set
  {
    QWidgetAction* pAllAction = new QWidgetAction(m_pMenu);
    m_pAllButton = new QPushButton(QString::fromUtf8("All"), m_pMenu);
    connect(m_pAllButton, &QPushButton::clicked, this, [this](bool bChecked){ SetAllChecked(true); });
    pAllAction->setDefaultWidget(m_pAllButton);
    m_pMenu->addAction(pAllAction);

    QWidgetAction* pClearAction = new QWidgetAction(m_pMenu);
    m_pClearButton = new QPushButton(QString::fromUtf8("Clear"), m_pMenu);
    connect(m_pClearButton, &QPushButton::clicked, this, [this](bool bChecked){ SetAllChecked(false); });
    pClearAction->setDefaultWidget(m_pClearButton);
    m_pMenu->addAction(pClearAction);
  }
}

void nsQtPropertyEditorBitflagsWidget::InternalSetValue(const nsVariant& value)
{
  nsQtScopedBlockSignals b(m_pWidget);
  m_iCurrentBitflags = value.ConvertTo<nsInt64>();

  QString sText;
  for (auto it = m_Constants.GetIterator(); it.IsValid(); ++it)
  {
    bool bChecked = (it.Key() & m_iCurrentBitflags) != 0;
    QString sName = it.Value()->text();
    if (bChecked)
    {
      sText += sName + "|";
    }
    it.Value()->setCheckState(bChecked ? Qt::Checked : Qt::Unchecked);
  }
  if (!sText.isEmpty())
    sText = sText.left(sText.size() - 1);

  m_pWidget->setText(sText);
}

void nsQtPropertyEditorBitflagsWidget::SetAllChecked(bool bChecked)
{
  for (auto& pCheckBox : m_Constants)
  {
    pCheckBox.Value()->setCheckState(bChecked ? Qt::Checked : Qt::Unchecked);
  }
}

void nsQtPropertyEditorBitflagsWidget::on_Menu_aboutToShow()
{
  m_pMenu->setMinimumWidth(m_pWidget->geometry().width());
}

void nsQtPropertyEditorBitflagsWidget::on_Menu_aboutToHide()
{
  nsInt64 iValue = 0;
  QString sText;
  for (auto it = m_Constants.GetIterator(); it.IsValid(); ++it)
  {
    bool bChecked = it.Value()->checkState() == Qt::Checked;
    QString sName = it.Value()->text();
    if (bChecked)
    {
      sText += sName + "|";
      iValue |= it.Key();
    }
  }
  if (!sText.isEmpty())
    sText = sText.left(sText.size() - 1);

  m_pWidget->setText(sText);

  if (m_iCurrentBitflags != iValue)
  {
    m_iCurrentBitflags = iValue;
    BroadcastValueChanged(m_iCurrentBitflags);
  }
}


/// *** CURVE1D ***

nsQtCurve1DButtonWidget::nsQtCurve1DButtonWidget(QWidget* pParent)
  : QLabel(pParent)
{
  setAutoFillBackground(true);
  setCursor(Qt::PointingHandCursor);
  setScaledContents(true);
}

void nsQtCurve1DButtonWidget::UpdatePreview(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pCurveObject, QColor color, double fLowerExtents, bool bLowerFixed, double fUpperExtents, bool bUpperFixed, double fDefaultValue, double fLowerRange, double fUpperRange)
{
  nsInt32 iNumPoints = 0;
  pObjectAccessor->GetCount(pCurveObject, "ControlPoints", iNumPoints).AssertSuccess();

  nsVariant v;
  nsHybridArray<nsVec2d, 32> points;
  points.Reserve(iNumPoints);

  double minX = fLowerExtents * 4800.0;
  double maxX = fUpperExtents * 4800.0;

  double minY = fLowerRange;
  double maxY = fUpperRange;

  for (nsInt32 i = 0; i < iNumPoints; ++i)
  {
    const nsDocumentObject* pPoint = pObjectAccessor->GetChildObject(pCurveObject, "ControlPoints", i);

    nsVec2d p;

    pObjectAccessor->GetValue(pPoint, "Tick", v).AssertSuccess();
    p.x = v.ConvertTo<double>();

    pObjectAccessor->GetValue(pPoint, "Value", v).AssertSuccess();
    p.y = v.ConvertTo<double>();

    points.PushBack(p);

    if (!bLowerFixed)
      minX = nsMath::Min(minX, p.x);

    if (!bUpperFixed)
      maxX = nsMath::Max(maxX, p.x);

    minY = nsMath::Min(minY, p.y);
    maxY = nsMath::Max(maxY, p.y);
  }

  const double pW = nsMath::Max(10, size().width());
  const double pH = nsMath::Clamp(size().height(), 5, 24);

  QPixmap pixmap((int)pW, (int)pH);
  pixmap.fill(palette().base().color());

  QPainter pt(&pixmap);
  pt.setPen(color);
  pt.setRenderHint(QPainter::RenderHint::Antialiasing);

  if (!points.IsEmpty())
  {
    points.Sort([](const nsVec2d& lhs, const nsVec2d& rhs) -> bool { return lhs.x < rhs.x; });

    const double normX = 1.0 / (maxX - minX);
    const double normY = 1.0 / (maxY - minY);

    QPainterPath path;

    {
      double startX = nsMath::Min(minX, points[0].x);
      double startY = points[0].y;

      startX = (startX - minX) * normX;
      startY = 1.0 - ((startY - minY) * normY);

      path.moveTo((int)(startX * pW), (int)(startY * pH));
    }

    for (nsUInt32 i = 0; i < points.GetCount(); ++i)
    {
      auto pt0 = points[i];
      pt0.x = (pt0.x - minX) * normX;
      pt0.y = 1.0 - ((pt0.y - minY) * normY);

      path.lineTo((int)(pt0.x * pW), (int)(pt0.y * pH));
    }

    {
      double endX = nsMath::Max(maxX, points.PeekBack().x);
      double endY = points.PeekBack().y;

      endX = (endX - minX) * normX;
      endY = 1.0 - ((endY - minY) * normY);

      path.lineTo((int)(endX * pW), (int)(endY * pH));
    }

    pt.drawPath(path);
  }
  else
  {
    const double normY = 1.0 / (maxY - minY);
    double valY = 1.0 - ((fDefaultValue - minY) * normY);

    pt.drawLine(0, (int)(valY * pH), (int)pW, (int)(valY * pH));
  }

  setPixmap(pixmap);
}

void nsQtCurve1DButtonWidget::mouseReleaseEvent(QMouseEvent* event)
{
  Q_EMIT clicked();
}

nsQtPropertyEditorCurve1DWidget::nsQtPropertyEditorCurve1DWidget()
  : nsQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pButton = new nsQtCurve1DButtonWidget(this);
  m_pButton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

  m_pLayout->addWidget(m_pButton);

  NS_VERIFY(connect(m_pButton, SIGNAL(clicked()), this, SLOT(on_Button_triggered())) != nullptr, "signal/slot connection failed");
}

void nsQtPropertyEditorCurve1DWidget::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  nsQtPropertyWidget::SetSelection(items);

  UpdatePreview();
}

void nsQtPropertyEditorCurve1DWidget::OnInit() {}
void nsQtPropertyEditorCurve1DWidget::DoPrepareToDie() {}

void nsQtPropertyEditorCurve1DWidget::UpdatePreview()
{
  if (m_Items.IsEmpty())
    return;

  const nsDocumentObject* pParent = m_Items[0].m_pObject;
  const nsDocumentObject* pCurve = m_pObjectAccessor->GetChildObject(pParent, m_pProp->GetPropertyName(), {});
  const nsColorAttribute* pColorAttr = m_pProp->GetAttributeByType<nsColorAttribute>();
  const nsCurveExtentsAttribute* pExtentsAttr = m_pProp->GetAttributeByType<nsCurveExtentsAttribute>();
  const nsDefaultValueAttribute* pDefAttr = m_pProp->GetAttributeByType<nsDefaultValueAttribute>();
  const nsClampValueAttribute* pClampAttr = m_pProp->GetAttributeByType<nsClampValueAttribute>();

  const bool bLowerFixed = pExtentsAttr ? pExtentsAttr->m_bLowerExtentFixed : false;
  const bool bUpperFixed = pExtentsAttr ? pExtentsAttr->m_bUpperExtentFixed : false;
  const double fLowerExt = pExtentsAttr ? pExtentsAttr->m_fLowerExtent : 0.0;
  const double fUpperExt = pExtentsAttr ? pExtentsAttr->m_fUpperExtent : 1.0;
  const nsColorGammaUB color = pColorAttr ? pColorAttr->GetColor() : nsColor::GreenYellow;
  const double fLowerRange = (pClampAttr && pClampAttr->GetMinValue().IsNumber()) ? pClampAttr->GetMinValue().ConvertTo<double>() : 0.0;
  const double fUpperRange = (pClampAttr && pClampAttr->GetMaxValue().IsNumber()) ? pClampAttr->GetMaxValue().ConvertTo<double>() : 1.0;
  const double fDefVal = (pDefAttr && pDefAttr->GetValue().IsNumber()) ? pDefAttr->GetValue().ConvertTo<double>() : 0.0;

  m_pButton->UpdatePreview(m_pObjectAccessor, pCurve, QColor(color.r, color.g, color.b), fLowerExt, bLowerFixed, fUpperExt, bUpperFixed, fDefVal, fLowerRange, fUpperRange);
}

void nsQtPropertyEditorCurve1DWidget::on_Button_triggered()
{
  const nsDocumentObject* pParent = m_Items[0].m_pObject;
  const nsDocumentObject* pCurve = m_pObjectAccessor->GetChildObject(pParent, m_pProp->GetPropertyName(), {});
  const nsColorAttribute* pColorAttr = m_pProp->GetAttributeByType<nsColorAttribute>();
  const nsCurveExtentsAttribute* pExtentsAttr = m_pProp->GetAttributeByType<nsCurveExtentsAttribute>();
  const nsClampValueAttribute* pClampAttr = m_pProp->GetAttributeByType<nsClampValueAttribute>();

  // TODO: would like to have one transaction open to finish/cancel at the end
  // but also be able to undo individual steps while editing
  // m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->StartTransaction("Edit Curve");

  nsQtCurveEditDlg* pDlg = new nsQtCurveEditDlg(m_pObjectAccessor, pCurve, this);
  pDlg->restoreGeometry(nsQtCurveEditDlg::GetLastDialogGeometry());

  if (pColorAttr)
  {
    pDlg->SetCurveColor(pColorAttr->GetColor());
  }

  if (pExtentsAttr)
  {
    pDlg->SetCurveExtents(pExtentsAttr->m_fLowerExtent, pExtentsAttr->m_bLowerExtentFixed, pExtentsAttr->m_fUpperExtent, pExtentsAttr->m_bUpperExtentFixed);
  }

  if (pClampAttr)
  {
    const double fLower = pClampAttr->GetMinValue().IsNumber() ? pClampAttr->GetMinValue().ConvertTo<double>() : -nsMath::HighValue<double>();
    const double fUpper = pClampAttr->GetMaxValue().IsNumber() ? pClampAttr->GetMaxValue().ConvertTo<double>() : nsMath::HighValue<double>();

    pDlg->SetCurveRanges(fLower, fUpper);
  }

  if (pDlg->exec() == QDialog::Accepted)
  {
    // m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->FinishTransaction();

    UpdatePreview();
  }
  else
  {
    // m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->CancelTransaction();
  }

  delete pDlg;
}
