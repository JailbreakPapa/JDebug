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

wdQtPropertyEditorCheckboxWidget::wdQtPropertyEditorCheckboxWidget()
  : wdQtStandardPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QCheckBox(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pLayout->addWidget(m_pWidget);

  WD_VERIFY(connect(m_pWidget, SIGNAL(stateChanged(int)), this, SLOT(on_StateChanged_triggered(int))) != nullptr, "signal/slot connection failed");
}

void wdQtPropertyEditorCheckboxWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals b(m_pWidget);

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

void wdQtPropertyEditorCheckboxWidget::mousePressEvent(QMouseEvent* pEv)
{
  QWidget::mousePressEvent(pEv);

  m_pWidget->toggle();
}

void wdQtPropertyEditorCheckboxWidget::on_StateChanged_triggered(int state)
{
  if (state == Qt::PartiallyChecked)
  {
    wdQtScopedBlockSignals b(m_pWidget);

    m_pWidget->setCheckState(Qt::Checked);
    m_pWidget->setTristate(false);
  }

  BroadcastValueChanged((state != Qt::Unchecked) ? true : false);
}


/// *** DOUBLE SPINBOX ***

wdQtPropertyEditorDoubleSpinboxWidget::wdQtPropertyEditorDoubleSpinboxWidget(wdInt8 iNumComponents)
  : wdQtStandardPropertyWidget()
{
  WD_ASSERT_DEBUG(iNumComponents <= 4, "Only up to 4 components are supported");

  m_iNumComponents = iNumComponents;
  m_bTemporaryCommand = false;

  m_pWidget[0] = nullptr;
  m_pWidget[1] = nullptr;
  m_pWidget[2] = nullptr;
  m_pWidget[3] = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  for (wdInt32 c = 0; c < m_iNumComponents; ++c)
  {
    m_pWidget[c] = new wdQtDoubleSpinBox(this);
    m_pWidget[c]->installEventFilter(this);
    m_pWidget[c]->setMinimum(-wdMath::Infinity<double>());
    m_pWidget[c]->setMaximum(wdMath::Infinity<double>());
    m_pWidget[c]->setSingleStep(0.1f);
    m_pWidget[c]->setAccelerated(true);

    policy.setHorizontalStretch(2);
    m_pWidget[c]->setSizePolicy(policy);

    m_pLayout->addWidget(m_pWidget[c]);

    connect(m_pWidget[c], SIGNAL(editingFinished()), this, SLOT(on_EditingFinished_triggered()));
    connect(m_pWidget[c], SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  }
}

void wdQtPropertyEditorDoubleSpinboxWidget::OnInit()
{
  const wdClampValueAttribute* pClamp = m_pProp->GetAttributeByType<wdClampValueAttribute>();
  const wdDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<wdDefaultValueAttribute>();
  const wdSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<wdSuffixAttribute>();

  if (pClamp)
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0]);
        m_pWidget[0]->setMinimum(pClamp->GetMinValue());
        m_pWidget[0]->setMaximum(pClamp->GetMaxValue());
        break;
      }
      case 2:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pClamp->GetMinValue().CanConvertTo<wdVec2>())
        {
          wdVec2 value = pClamp->GetMinValue().ConvertTo<wdVec2>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
        }
        if (pClamp->GetMaxValue().CanConvertTo<wdVec2>())
        {
          wdVec2 value = pClamp->GetMaxValue().ConvertTo<wdVec2>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
        }
        break;
      }
      case 3:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pClamp->GetMinValue().CanConvertTo<wdVec3>())
        {
          wdVec3 value = pClamp->GetMinValue().ConvertTo<wdVec3>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
        }
        if (pClamp->GetMaxValue().CanConvertTo<wdVec3>())
        {
          wdVec3 value = pClamp->GetMaxValue().ConvertTo<wdVec3>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
        }
        break;
      }
      case 4:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pClamp->GetMinValue().CanConvertTo<wdVec4>())
        {
          wdVec4 value = pClamp->GetMinValue().ConvertTo<wdVec4>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
          m_pWidget[3]->setMinimum(value.w);
        }
        if (pClamp->GetMaxValue().CanConvertTo<wdVec4>())
        {
          wdVec4 value = pClamp->GetMaxValue().ConvertTo<wdVec4>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
          m_pWidget[3]->setMaximum(value.w);
        }
        break;
      }
    }
  }

  if (pDefault)
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0]);

        if (pDefault->GetValue().CanConvertTo<double>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<double>());
        }
        break;
      }
      case 2:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pDefault->GetValue().CanConvertTo<wdVec2>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec2>().x);
          m_pWidget[1]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec2>().y);
        }
        break;
      }
      case 3:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pDefault->GetValue().CanConvertTo<wdVec3>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec3>().x);
          m_pWidget[1]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec3>().y);
          m_pWidget[2]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec3>().z);
        }
        break;
      }
      case 4:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pDefault->GetValue().CanConvertTo<wdVec4>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4>().x);
          m_pWidget[1]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4>().y);
          m_pWidget[2]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4>().z);
          m_pWidget[3]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4>().w);
        }
        break;
      }
    }
  }

  if (pSuffix)
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setDisplaySuffix(pSuffix->GetSuffix());
    }
  }

  const wdMinValueTextAttribute* pMinValueText = m_pProp->GetAttributeByType<wdMinValueTextAttribute>();
  if (pMinValueText)
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setSpecialValueText(pMinValueText->GetText());
    }
  }
}

void wdQtPropertyEditorDoubleSpinboxWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

  if (value.IsValid())
  {
    switch (m_iNumComponents)
    {
      case 1:
        m_pWidget[0]->setValue(value.ConvertTo<double>());
        break;
      case 2:
        m_pWidget[0]->setValue(value.ConvertTo<wdVec2>().x);
        m_pWidget[1]->setValue(value.ConvertTo<wdVec2>().y);
        break;
      case 3:
        m_pWidget[0]->setValue(value.ConvertTo<wdVec3>().x);
        m_pWidget[1]->setValue(value.ConvertTo<wdVec3>().y);
        m_pWidget[2]->setValue(value.ConvertTo<wdVec3>().z);
        break;
      case 4:
        m_pWidget[0]->setValue(value.ConvertTo<wdVec4>().x);
        m_pWidget[1]->setValue(value.ConvertTo<wdVec4>().y);
        m_pWidget[2]->setValue(value.ConvertTo<wdVec4>().z);
        m_pWidget[3]->setValue(value.ConvertTo<wdVec4>().w);
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

void wdQtPropertyEditorDoubleSpinboxWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void wdQtPropertyEditorDoubleSpinboxWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  switch (m_iNumComponents)
  {
    case 1:
      BroadcastValueChanged(m_pWidget[0]->value());
      break;
    case 2:
      BroadcastValueChanged(wdVec2(m_pWidget[0]->value(), m_pWidget[1]->value()));
      break;
    case 3:
      BroadcastValueChanged(wdVec3(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value()));
      break;
    case 4:
      BroadcastValueChanged(wdVec4(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value(), m_pWidget[3]->value()));
      break;
  }
}


/// *** TIME SPINBOX ***

wdQtPropertyEditorTimeWidget::wdQtPropertyEditorTimeWidget()
  : wdQtStandardPropertyWidget()
{
  m_bTemporaryCommand = false;

  m_pWidget = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  {
    m_pWidget = new wdQtDoubleSpinBox(this);
    m_pWidget->installEventFilter(this);
    m_pWidget->setDisplaySuffix(" sec");
    m_pWidget->setMinimum(-wdMath::Infinity<double>());
    m_pWidget->setMaximum(wdMath::Infinity<double>());
    m_pWidget->setSingleStep(0.1f);
    m_pWidget->setAccelerated(true);

    policy.setHorizontalStretch(2);
    m_pWidget->setSizePolicy(policy);

    m_pLayout->addWidget(m_pWidget);

    connect(m_pWidget, SIGNAL(editingFinished()), this, SLOT(on_EditingFinished_triggered()));
    connect(m_pWidget, SIGNAL(valueChanged(double)), this, SLOT(SlotValueChanged()));
  }
}

void wdQtPropertyEditorTimeWidget::OnInit()
{
  const wdClampValueAttribute* pClamp = m_pProp->GetAttributeByType<wdClampValueAttribute>();
  if (pClamp)
  {
    wdQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setMinimum(pClamp->GetMinValue());
    m_pWidget->setMaximum(pClamp->GetMaxValue());
  }

  const wdDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<wdDefaultValueAttribute>();
  if (pDefault)
  {
    wdQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setDefaultValue(pDefault->GetValue());
  }
}

void wdQtPropertyEditorTimeWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals b0(m_pWidget);
  m_pWidget->setValue(value);
}

void wdQtPropertyEditorTimeWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void wdQtPropertyEditorTimeWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  BroadcastValueChanged(wdTime::Seconds(m_pWidget->value()));
}


/// *** ANGLE SPINBOX ***

wdQtPropertyEditorAngleWidget::wdQtPropertyEditorAngleWidget()
  : wdQtStandardPropertyWidget()
{
  m_bTemporaryCommand = false;

  m_pWidget = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  {
    m_pWidget = new wdQtDoubleSpinBox(this);
    m_pWidget->installEventFilter(this);
    m_pWidget->setDisplaySuffix(wdStringUtf8(L"\u00B0").GetData());
    m_pWidget->setMinimum(-wdMath::Infinity<double>());
    m_pWidget->setMaximum(wdMath::Infinity<double>());
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

void wdQtPropertyEditorAngleWidget::OnInit()
{
  const wdClampValueAttribute* pClamp = m_pProp->GetAttributeByType<wdClampValueAttribute>();
  if (pClamp)
  {
    wdQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setMinimum(pClamp->GetMinValue());
    m_pWidget->setMaximum(pClamp->GetMaxValue());
  }

  const wdDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<wdDefaultValueAttribute>();
  if (pDefault)
  {
    wdQtScopedBlockSignals bs(m_pWidget);
    m_pWidget->setDefaultValue(pDefault->GetValue());
  }

  const wdSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<wdSuffixAttribute>();
  if (pSuffix)
  {
    m_pWidget->setDisplaySuffix(pSuffix->GetSuffix());
  }

  const wdMinValueTextAttribute* pMinValueText = m_pProp->GetAttributeByType<wdMinValueTextAttribute>();
  if (pMinValueText)
  {
    m_pWidget->setSpecialValueText(pMinValueText->GetText());
  }
}

void wdQtPropertyEditorAngleWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals b0(m_pWidget);
  m_pWidget->setValue(value);
}

void wdQtPropertyEditorAngleWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void wdQtPropertyEditorAngleWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  BroadcastValueChanged(wdAngle::Degree(m_pWidget->value()));
}

/// *** INT SPINBOX ***


wdQtPropertyEditorIntSpinboxWidget::wdQtPropertyEditorIntSpinboxWidget(wdInt8 iNumComponents, wdInt32 iMinValue, wdInt32 iMaxValue)
  : wdQtStandardPropertyWidget()
{
  m_iNumComponents = iNumComponents;
  m_bTemporaryCommand = false;

  m_pWidget[0] = nullptr;
  m_pWidget[1] = nullptr;
  m_pWidget[2] = nullptr;
  m_pWidget[3] = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();
  policy.setHorizontalStretch(2);

  for (wdInt32 c = 0; c < m_iNumComponents; ++c)
  {
    m_pWidget[c] = new wdQtDoubleSpinBox(this, true);
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


wdQtPropertyEditorIntSpinboxWidget::~wdQtPropertyEditorIntSpinboxWidget() = default;

void wdQtPropertyEditorIntSpinboxWidget::OnInit()
{
  if (const wdClampValueAttribute* pClamp = m_pProp->GetAttributeByType<wdClampValueAttribute>())
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        const wdInt32 iMinValue = pClamp->GetMinValue().ConvertTo<wdInt32>();
        const wdInt32 iMaxValue = pClamp->GetMaxValue().ConvertTo<wdInt32>();

        wdQtScopedBlockSignals bs(m_pWidget[0]);
        m_pWidget[0]->setMinimum(pClamp->GetMinValue());
        m_pWidget[0]->setMaximum(pClamp->GetMaxValue());

        if (pClamp->GetMinValue().IsValid() && pClamp->GetMaxValue().IsValid() && (iMaxValue - iMinValue) < 256)
        {
          wdQtScopedBlockSignals bs2(m_pSlider);

          // we have to create the slider here, because in the constructor we don't know the real
          // min and max values from the wdClampValueAttribute (only the rough type ranges)
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
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pClamp->GetMinValue().CanConvertTo<wdVec2I32>())
        {
          wdVec2I32 value = pClamp->GetMinValue().ConvertTo<wdVec2I32>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
        }
        if (pClamp->GetMaxValue().CanConvertTo<wdVec2I32>())
        {
          wdVec2I32 value = pClamp->GetMaxValue().ConvertTo<wdVec2I32>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
        }
        break;
      }
      case 3:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pClamp->GetMinValue().CanConvertTo<wdVec3I32>())
        {
          wdVec3I32 value = pClamp->GetMinValue().ConvertTo<wdVec3I32>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
        }
        if (pClamp->GetMaxValue().CanConvertTo<wdVec3I32>())
        {
          wdVec3I32 value = pClamp->GetMaxValue().ConvertTo<wdVec3I32>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
        }
        break;
      }
      case 4:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pClamp->GetMinValue().CanConvertTo<wdVec4I32>())
        {
          wdVec4I32 value = pClamp->GetMinValue().ConvertTo<wdVec4I32>();
          m_pWidget[0]->setMinimum(value.x);
          m_pWidget[1]->setMinimum(value.y);
          m_pWidget[2]->setMinimum(value.z);
          m_pWidget[3]->setMinimum(value.w);
        }
        if (pClamp->GetMaxValue().CanConvertTo<wdVec4I32>())
        {
          wdVec4I32 value = pClamp->GetMaxValue().ConvertTo<wdVec4I32>();
          m_pWidget[0]->setMaximum(value.x);
          m_pWidget[1]->setMaximum(value.y);
          m_pWidget[2]->setMaximum(value.z);
          m_pWidget[3]->setMaximum(value.w);
        }
        break;
      }
    }
  }

  if (const wdDefaultValueAttribute* pDefault = m_pProp->GetAttributeByType<wdDefaultValueAttribute>())
  {
    switch (m_iNumComponents)
    {
      case 1:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pSlider);

        if (pDefault->GetValue().CanConvertTo<wdInt32>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<wdInt32>());

          if (m_pSlider)
          {
            m_pSlider->setValue(pDefault->GetValue().ConvertTo<wdInt32>());
          }
        }
        break;
      }
      case 2:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1]);

        if (pDefault->GetValue().CanConvertTo<wdVec2I32>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec2I32>().x);
          m_pWidget[1]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec2I32>().y);
        }
        break;
      }
      case 3:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2]);

        if (pDefault->GetValue().CanConvertTo<wdVec3I32>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec3I32>().x);
          m_pWidget[1]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec3I32>().y);
          m_pWidget[2]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec3I32>().z);
        }
        break;
      }
      case 4:
      {
        wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3]);

        if (pDefault->GetValue().CanConvertTo<wdVec4I32>())
        {
          m_pWidget[0]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4I32>().x);
          m_pWidget[1]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4I32>().y);
          m_pWidget[2]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4I32>().z);
          m_pWidget[3]->setDefaultValue(pDefault->GetValue().ConvertTo<wdVec4I32>().w);
        }
        break;
      }
    }
  }

  if (const wdSuffixAttribute* pSuffix = m_pProp->GetAttributeByType<wdSuffixAttribute>())
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setDisplaySuffix(pSuffix->GetSuffix());
    }
  }

  if (const wdMinValueTextAttribute* pMinValueText = m_pProp->GetAttributeByType<wdMinValueTextAttribute>())
  {
    for (int i = 0; i < m_iNumComponents; ++i)
    {
      m_pWidget[i]->setSpecialValueText(pMinValueText->GetText());
    }
  }
}

void wdQtPropertyEditorIntSpinboxWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals bs(m_pWidget[0], m_pWidget[1], m_pWidget[2], m_pWidget[3], m_pSlider);

  switch (m_iNumComponents)
  {
    case 1:
      m_pWidget[0]->setValue(value.ConvertTo<wdInt32>());

      if (m_pSlider)
      {
        m_pSlider->setValue(value.ConvertTo<wdInt32>());
      }

      break;
    case 2:
      m_pWidget[0]->setValue(value.ConvertTo<wdVec2I32>().x);
      m_pWidget[1]->setValue(value.ConvertTo<wdVec2I32>().y);
      break;
    case 3:
      m_pWidget[0]->setValue(value.ConvertTo<wdVec3I32>().x);
      m_pWidget[1]->setValue(value.ConvertTo<wdVec3I32>().y);
      m_pWidget[2]->setValue(value.ConvertTo<wdVec3I32>().z);
      break;
    case 4:
      m_pWidget[0]->setValue(value.ConvertTo<wdVec4I32>().x);
      m_pWidget[1]->setValue(value.ConvertTo<wdVec4I32>().y);
      m_pWidget[2]->setValue(value.ConvertTo<wdVec4I32>().z);
      m_pWidget[3]->setValue(value.ConvertTo<wdVec4I32>().w);
      break;
  }
}

void wdQtPropertyEditorIntSpinboxWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  switch (m_iNumComponents)
  {
    case 1:
      BroadcastValueChanged((wdInt32)m_pWidget[0]->value());

      if (m_pSlider)
      {
        wdQtScopedBlockSignals b0(m_pSlider);
        m_pSlider->setValue((wdInt32)m_pWidget[0]->value());
      }

      break;
    case 2:
      BroadcastValueChanged(wdVec2I32(m_pWidget[0]->value(), m_pWidget[1]->value()));
      break;
    case 3:
      BroadcastValueChanged(wdVec3I32(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value()));
      break;
    case 4:
      BroadcastValueChanged(wdVec4I32(m_pWidget[0]->value(), m_pWidget[1]->value(), m_pWidget[2]->value(), m_pWidget[3]->value()));
      break;
  }
}

void wdQtPropertyEditorIntSpinboxWidget::SlotSliderValueChanged(int value)
{
  if (!m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  {
    wdQtScopedBlockSignals b0(m_pWidget[0]);
    m_pWidget[0]->setValue(value);
  }

  BroadcastValueChanged((wdInt32)m_pSlider->value());
}

void wdQtPropertyEditorIntSpinboxWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}


/// *** QUATERNION ***

wdQtPropertyEditorQuaternionWidget::wdQtPropertyEditorQuaternionWidget()
  : wdQtStandardPropertyWidget()
{
  m_bTemporaryCommand = false;

  m_pWidget[0] = nullptr;
  m_pWidget[1] = nullptr;
  m_pWidget[2] = nullptr;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  QSizePolicy policy = sizePolicy();

  for (wdInt32 c = 0; c < 3; ++c)
  {
    m_pWidget[c] = new wdQtDoubleSpinBox(this);
    m_pWidget[c]->installEventFilter(this);
    m_pWidget[c]->setMinimum(-wdMath::Infinity<double>());
    m_pWidget[c]->setMaximum(wdMath::Infinity<double>());
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

void wdQtPropertyEditorQuaternionWidget::OnInit() {}

void wdQtPropertyEditorQuaternionWidget::InternalSetValue(const wdVariant& value)
{
  if (m_bTemporaryCommand)
    return;

  wdQtScopedBlockSignals b0(m_pWidget[0]);
  wdQtScopedBlockSignals b1(m_pWidget[1]);
  wdQtScopedBlockSignals b2(m_pWidget[2]);

  if (value.IsValid())
  {
    const wdQuat qRot = value.ConvertTo<wdQuat>();
    wdAngle x, y, z;
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

void wdQtPropertyEditorQuaternionWidget::on_EditingFinished_triggered()
{
  if (m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::EndTemporary);

  m_bTemporaryCommand = false;
}

void wdQtPropertyEditorQuaternionWidget::SlotValueChanged()
{
  if (!m_bTemporaryCommand)
    Broadcast(wdPropertyEvent::Type::BeginTemporary);

  m_bTemporaryCommand = true;

  wdAngle x = wdAngle::Degree(m_pWidget[0]->value());
  wdAngle y = wdAngle::Degree(m_pWidget[1]->value());
  wdAngle z = wdAngle::Degree(m_pWidget[2]->value());

  wdQuat qRot;
  qRot.SetFromEulerAngles(x, y, z);

  BroadcastValueChanged(qRot);
}

/// *** LINEEDIT ***

wdQtPropertyEditorLineEditWidget::wdQtPropertyEditorLineEditWidget()
  : wdQtStandardPropertyWidget()
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

void wdQtPropertyEditorLineEditWidget::OnInit()
{
  if (m_pProp->GetAttributeByType<wdReadOnlyAttribute>() != nullptr || m_pProp->GetFlags().IsSet(wdPropertyFlags::ReadOnly))
  {
    setEnabled(true);

    wdQtScopedBlockSignals bs(m_pWidget);

    m_pWidget->setReadOnly(true);
    QPalette palette = m_pWidget->palette();
    palette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
    m_pWidget->setPalette(palette);
  }
}

void wdQtPropertyEditorLineEditWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals b(m_pWidget);

  if (!value.IsValid())
  {
    m_pWidget->setPlaceholderText(QStringLiteral("<Multiple Values>"));
  }
  else
  {
    m_pWidget->setPlaceholderText(QString());
    m_pWidget->setText(QString::fromUtf8(value.ConvertTo<wdString>().GetData()));
  }
}

void wdQtPropertyEditorLineEditWidget::on_TextChanged_triggered(const QString& value)
{
  BroadcastValueChanged(value.toUtf8().data());
}

void wdQtPropertyEditorLineEditWidget::on_TextFinished_triggered()
{
  BroadcastValueChanged(m_pWidget->text().toUtf8().data());
}


/// *** COLOR ***

wdQtColorButtonWidget::wdQtColorButtonWidget(QWidget* pParent)
  : QFrame(pParent)
{
  setAutoFillBackground(true);
  setCursor(Qt::PointingHandCursor);
}

void wdQtColorButtonWidget::SetColor(const wdVariant& color)
{
  if (color.IsValid())
  {
    wdColor col0 = color.ConvertTo<wdColor>();
    col0.NormalizeToLdrRange();

    const wdColorGammaUB col = col0;

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

void wdQtColorButtonWidget::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set palette.
  setPalette(m_Pal);
  QFrame::showEvent(event);
}

void wdQtColorButtonWidget::mouseReleaseEvent(QMouseEvent* event)
{
  Q_EMIT clicked();
}

wdQtPropertyEditorColorWidget::wdQtPropertyEditorColorWidget()
  : wdQtStandardPropertyWidget()
{
  m_bExposeAlpha = false;

  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new wdQtColorButtonWidget(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

  m_pLayout->addWidget(m_pWidget);

  WD_VERIFY(connect(m_pWidget, SIGNAL(clicked()), this, SLOT(on_Button_triggered())) != nullptr, "signal/slot connection failed");
}

void wdQtPropertyEditorColorWidget::OnInit()
{
  m_bExposeAlpha = (m_pProp->GetAttributeByType<wdExposeColorAlphaAttribute>() != nullptr);
}

void wdQtPropertyEditorColorWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals b(m_pWidget);

  m_OriginalValue = GetOldValue();
  m_pWidget->SetColor(value);
}

void wdQtPropertyEditorColorWidget::on_Button_triggered()
{
  Broadcast(wdPropertyEvent::Type::BeginTemporary);

  bool bShowHDR = false;

  wdColor temp = wdColor::White;
  if (m_OriginalValue.IsValid())
  {
    bShowHDR = m_OriginalValue.IsA<wdColor>();

    temp = m_OriginalValue.ConvertTo<wdColor>();
  }

  wdQtUiServices::GetSingleton()->ShowColorDialog(
    temp, m_bExposeAlpha, bShowHDR, this, SLOT(on_CurrentColor_changed(const wdColor&)), SLOT(on_Color_accepted()), SLOT(on_Color_reset()));
}

void wdQtPropertyEditorColorWidget::on_CurrentColor_changed(const wdColor& color)
{
  wdVariant col;

  if (m_OriginalValue.IsA<wdColorGammaUB>())
  {
    // wdVariant does not down-cast to wdColorGammaUB automatically
    col = wdColorGammaUB(color);
  }
  else
  {
    col = color;
  }

  m_pWidget->SetColor(col);
  BroadcastValueChanged(col);
}

void wdQtPropertyEditorColorWidget::on_Color_reset()
{
  m_pWidget->SetColor(m_OriginalValue);
  Broadcast(wdPropertyEvent::Type::CancelTemporary);
}

void wdQtPropertyEditorColorWidget::on_Color_accepted()
{
  m_OriginalValue = GetOldValue();
  Broadcast(wdPropertyEvent::Type::EndTemporary);
}


/// *** ENUM COMBOBOX ***

wdQtPropertyEditorEnumWidget::wdQtPropertyEditorEnumWidget()
  : wdQtStandardPropertyWidget()
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

void wdQtPropertyEditorEnumWidget::OnInit()
{
  const wdRTTI* enumType = m_pProp->GetSpecificType();

  wdQtScopedBlockSignals bs(m_pWidget);

  wdStringBuilder sTemp;
  const wdRTTI* pType = enumType;
  wdUInt32 uiCount = pType->GetProperties().GetCount();
  // Start at 1 to skip default value.
  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    auto pProp = pType->GetProperties()[i];

    if (pProp->GetCategory() != wdPropertyCategory::Constant)
      continue;

    const wdAbstractConstantProperty* pConstant = static_cast<const wdAbstractConstantProperty*>(pProp);

    m_pWidget->addItem(QString::fromUtf8(wdTranslate(pConstant->GetPropertyName())), pConstant->GetConstant().ConvertTo<wdInt64>());
  }
}

void wdQtPropertyEditorEnumWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals b(m_pWidget);

  if (value.IsValid())
  {
    wdInt32 iIndex = m_pWidget->findData(value.ConvertTo<wdInt64>());
    WD_ASSERT_DEV(iIndex != -1, "Enum widget is set to an invalid value!");
    m_pWidget->setCurrentIndex(iIndex);
  }
  else
  {
    m_pWidget->setCurrentIndex(-1);
  }
}

void wdQtPropertyEditorEnumWidget::on_CurrentEnum_changed(int iEnum)
{
  wdInt64 iValue = m_pWidget->itemData(iEnum).toLongLong();
  BroadcastValueChanged(iValue);
}


/// *** BITFLAGS COMBOBOX ***

wdQtPropertyEditorBitflagsWidget::wdQtPropertyEditorBitflagsWidget()
  : wdQtStandardPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pWidget = new QPushButton(this);
  m_pWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  m_pMenu = nullptr;
  m_pMenu = new QMenu(m_pWidget);
  m_pWidget->setMenu(m_pMenu);
  m_pLayout->addWidget(m_pWidget);

  connect(m_pMenu, SIGNAL(aboutToShow()), this, SLOT(on_Menu_aboutToShow()));
  connect(m_pMenu, SIGNAL(aboutToHide()), this, SLOT(on_Menu_aboutToHide()));
}

wdQtPropertyEditorBitflagsWidget::~wdQtPropertyEditorBitflagsWidget()
{
  m_Constants.Clear();
  m_pWidget->setMenu(nullptr);

  delete m_pMenu;
  m_pMenu = nullptr;
}

void wdQtPropertyEditorBitflagsWidget::OnInit()
{
  const wdRTTI* enumType = m_pProp->GetSpecificType();

  const wdRTTI* pType = enumType;
  wdUInt32 uiCount = pType->GetProperties().GetCount();

  // Start at 1 to skip default value.
  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    auto pProp = pType->GetProperties()[i];

    if (pProp->GetCategory() != wdPropertyCategory::Constant)
      continue;

    const wdAbstractConstantProperty* pConstant = static_cast<const wdAbstractConstantProperty*>(pProp);

    QWidgetAction* pAction = new QWidgetAction(m_pMenu);
    QCheckBox* pCheckBox = new QCheckBox(QString::fromUtf8(wdTranslate(pConstant->GetPropertyName())), m_pMenu);
    pCheckBox->setCheckable(true);
    pCheckBox->setCheckState(Qt::Unchecked);
    pAction->setDefaultWidget(pCheckBox);

    m_Constants[pConstant->GetConstant().ConvertTo<wdInt64>()] = pCheckBox;
    m_pMenu->addAction(pAction);
  }
}

void wdQtPropertyEditorBitflagsWidget::InternalSetValue(const wdVariant& value)
{
  wdQtScopedBlockSignals b(m_pWidget);
  m_iCurrentBitflags = value.ConvertTo<wdInt64>();

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

void wdQtPropertyEditorBitflagsWidget::on_Menu_aboutToShow()
{
  m_pMenu->setMinimumWidth(m_pWidget->geometry().width());
}

void wdQtPropertyEditorBitflagsWidget::on_Menu_aboutToHide()
{
  wdInt64 iValue = 0;
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

wdQtCurve1DButtonWidget::wdQtCurve1DButtonWidget(QWidget* pParent)
  : QLabel(pParent)
{
  setAutoFillBackground(true);
  setCursor(Qt::PointingHandCursor);
  setScaledContents(true);
}

void wdQtCurve1DButtonWidget::UpdatePreview(wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pCurveObject, QColor color, double fLowerExtents, bool bLowerFixed, double fUpperExtents, bool bUpperFixed, double fDefaultValue, double fLowerRange, double fUpperRange)
{
  wdInt32 iNumPoints = 0;
  pObjectAccessor->GetCount(pCurveObject, "ControlPoints", iNumPoints);

  wdVariant v;
  wdHybridArray<wdVec2d, 32> points;
  points.Reserve(iNumPoints);

  double minX = fLowerExtents * 4800.0;
  double maxX = fUpperExtents * 4800.0;

  double minY = fLowerRange;
  double maxY = fUpperRange;

  for (wdInt32 i = 0; i < iNumPoints; ++i)
  {
    const wdDocumentObject* pPoint = pObjectAccessor->GetChildObject(pCurveObject, "ControlPoints", i);

    wdVec2d p;

    pObjectAccessor->GetValue(pPoint, "Tick", v);
    p.x = v.ConvertTo<double>();

    pObjectAccessor->GetValue(pPoint, "Value", v);
    p.y = v.ConvertTo<double>();

    points.PushBack(p);

    if (!bLowerFixed)
      minX = wdMath::Min(minX, p.x);

    if (!bUpperFixed)
      maxX = wdMath::Max(maxX, p.x);

    minY = wdMath::Min(minY, p.y);
    maxY = wdMath::Max(maxY, p.y);
  }

  const double pW = wdMath::Max(10, size().width());
  const double pH = wdMath::Clamp(size().height(), 5, 24);

  QPixmap pixmap((int)pW, (int)pH);
  pixmap.fill(palette().base().color());

  QPainter pt(&pixmap);
  pt.setPen(color);
  pt.setRenderHint(QPainter::RenderHint::Antialiasing);

  if (!points.IsEmpty())
  {
    points.Sort([](const wdVec2d& lhs, const wdVec2d& rhs) -> bool { return lhs.x < rhs.x; });

    const double normX = 1.0 / (maxX - minX);
    const double normY = 1.0 / (maxY - minY);

    QPainterPath path;

    {
      double startX = wdMath::Min(minX, points[0].x);
      double startY = points[0].y;

      startX = (startX - minX) * normX;
      startY = 1.0 - ((startY - minY) * normY);

      path.moveTo((int)(startX * pW), (int)(startY * pH));
    }

    for (wdUInt32 i = 0; i < points.GetCount(); ++i)
    {
      auto pt0 = points[i];
      pt0.x = (pt0.x - minX) * normX;
      pt0.y = 1.0 - ((pt0.y - minY) * normY);

      path.lineTo((int)(pt0.x * pW), (int)(pt0.y * pH));
    }

    {
      double endX = wdMath::Max(maxX, points.PeekBack().x);
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

void wdQtCurve1DButtonWidget::mouseReleaseEvent(QMouseEvent* event)
{
  Q_EMIT clicked();
}

wdQtPropertyEditorCurve1DWidget::wdQtPropertyEditorCurve1DWidget()
  : wdQtPropertyWidget()
{
  m_pLayout = new QHBoxLayout(this);
  m_pLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(m_pLayout);

  m_pButton = new wdQtCurve1DButtonWidget(this);
  m_pButton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

  m_pLayout->addWidget(m_pButton);

  WD_VERIFY(connect(m_pButton, SIGNAL(clicked()), this, SLOT(on_Button_triggered())) != nullptr, "signal/slot connection failed");
}

void wdQtPropertyEditorCurve1DWidget::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  wdQtPropertyWidget::SetSelection(items);

  UpdatePreview();
}

void wdQtPropertyEditorCurve1DWidget::OnInit() {}
void wdQtPropertyEditorCurve1DWidget::DoPrepareToDie() {}

void wdQtPropertyEditorCurve1DWidget::UpdatePreview()
{
  if (m_Items.IsEmpty())
    return;

  const wdDocumentObject* pParent = m_Items[0].m_pObject;
  const wdDocumentObject* pCurve = m_pObjectAccessor->GetChildObject(pParent, m_pProp->GetPropertyName(), {});
  const wdColorAttribute* pColorAttr = m_pProp->GetAttributeByType<wdColorAttribute>();
  const wdCurveExtentsAttribute* pExtentsAttr = m_pProp->GetAttributeByType<wdCurveExtentsAttribute>();
  const wdDefaultValueAttribute* pDefAttr = m_pProp->GetAttributeByType<wdDefaultValueAttribute>();
  const wdClampValueAttribute* pClampAttr = m_pProp->GetAttributeByType<wdClampValueAttribute>();

  const bool bLowerFixed = pExtentsAttr ? pExtentsAttr->m_bLowerExtentFixed : false;
  const bool bUpperFixed = pExtentsAttr ? pExtentsAttr->m_bUpperExtentFixed : false;
  const double fLowerExt = pExtentsAttr ? pExtentsAttr->m_fLowerExtent : 0.0;
  const double fUpperExt = pExtentsAttr ? pExtentsAttr->m_fUpperExtent : 1.0;
  const wdColorGammaUB color = pColorAttr ? pColorAttr->GetColor() : wdColor::GreenYellow;
  const double fLowerRange = (pClampAttr && pClampAttr->GetMinValue().IsNumber()) ? pClampAttr->GetMinValue().ConvertTo<double>() : 0.0;
  const double fUpperRange = (pClampAttr && pClampAttr->GetMaxValue().IsNumber()) ? pClampAttr->GetMaxValue().ConvertTo<double>() : 1.0;
  const double fDefVal = (pDefAttr && pDefAttr->GetValue().IsNumber()) ? pDefAttr->GetValue().ConvertTo<double>() : 0.0;

  m_pButton->UpdatePreview(m_pObjectAccessor, pCurve, QColor(color.r, color.g, color.b), fLowerExt, bLowerFixed, fUpperExt, bUpperFixed, fDefVal, fLowerRange, fUpperRange);
}

void wdQtPropertyEditorCurve1DWidget::on_Button_triggered()
{
  const wdDocumentObject* pParent = m_Items[0].m_pObject;
  const wdDocumentObject* pCurve = m_pObjectAccessor->GetChildObject(pParent, m_pProp->GetPropertyName(), {});
  const wdColorAttribute* pColorAttr = m_pProp->GetAttributeByType<wdColorAttribute>();
  const wdCurveExtentsAttribute* pExtentsAttr = m_pProp->GetAttributeByType<wdCurveExtentsAttribute>();
  const wdClampValueAttribute* pClampAttr = m_pProp->GetAttributeByType<wdClampValueAttribute>();

  // TODO: would like to have one transaction open to finish/cancel at the end
  // but also be able to undo individual steps while editing
  //m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->StartTransaction("Edit Curve");

  wdQtCurveEditDlg* pDlg = new wdQtCurveEditDlg(m_pObjectAccessor, pCurve, this);
  pDlg->restoreGeometry(wdQtCurveEditDlg::GetLastDialogGeometry());

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
    const double fLower = pClampAttr->GetMinValue().IsNumber() ? pClampAttr->GetMinValue().ConvertTo<double>() : -wdMath::HighValue<double>();
    const double fUpper = pClampAttr->GetMaxValue().IsNumber() ? pClampAttr->GetMaxValue().ConvertTo<double>() : wdMath::HighValue<double>();

    pDlg->SetCurveRanges(fLower, fUpper);
  }

  if (pDlg->exec() == QDialog::Accepted)
  {
    //m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->FinishTransaction();

    UpdatePreview();
  }
  else
  {
    //m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->CancelTransaction();
  }

  delete pDlg;
}
