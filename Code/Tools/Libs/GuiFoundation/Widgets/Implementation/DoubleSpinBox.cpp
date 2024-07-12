/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Widgets/DoubleSpinBox.moc.h>
#include <GuiFoundation/Widgets/WidgetUtils.h>
#include <QApplication>
#include <QLineEdit>
#include <QMouseEvent>
#include <QStyleOption>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

nsQtDoubleSpinBox::nsQtDoubleSpinBox(QWidget* pParent, bool bIntMode)
  : QDoubleSpinBox(pParent)
{
  m_fDefaultValue = 0.0;
  m_fDisplayedValue = nsMath::NaN<float>();
  m_bInvalid = false;
  m_bModified = false;
  m_bIntMode = bIntMode;
  m_bDragging = false;
  m_fStartDragValue = 0;
  m_iDragDelta = 0;
  setKeyboardTracking(false); // see https://stackoverflow.com/questions/35608600/qdoublespinbox-signals-valuechanged-before-the-edit-is-completely-done
  setDecimals(6);
  setSingleStep(0.1f);
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QWidget::customContextMenuRequested, this, &nsQtDoubleSpinBox::onCustomContextMenuRequested);
}

void nsQtDoubleSpinBox::SetIntMode(bool bEnable)
{
  m_bIntMode = bEnable;
}

void nsQtDoubleSpinBox::setDisplaySuffix(const char* szSuffix)
{
  m_sSuffix = QString::fromUtf8(szSuffix);
}

void nsQtDoubleSpinBox::setDefaultValue(double value)
{
  m_fDefaultValue = value;
}


void nsQtDoubleSpinBox::setDefaultValue(const nsVariant& val)
{
  double fValue = 0;
  if (nsToolsReflectionUtils::GetFloatFromVariant(val, fValue))
    setDefaultValue(fValue);
}

void nsQtDoubleSpinBox::setMinimum(const nsVariant& val)
{
  double fValue = 0;
  if (nsToolsReflectionUtils::GetFloatFromVariant(val, fValue))
    setMinimum(fValue);
}


void nsQtDoubleSpinBox::setMaximum(const nsVariant& val)
{
  double fValue = 0;
  if (nsToolsReflectionUtils::GetFloatFromVariant(val, fValue))
    setMaximum(fValue);
}

QString nsQtDoubleSpinBox::textFromValue(double fVal) const
{
  if (m_bInvalid)
    return QString();

  if (hasFocus() && fVal == m_fDisplayedValue && !nsMath::IsNaN(m_fDisplayedValue))
  {
    return m_sDisplayedText;
  }

  if (fVal == 0.0)
  {
    m_fDisplayedValue = 0;
    m_sDisplayedText = "0";
    QString res = QLatin1String("0");
    res += m_sSuffix;
    return res;
  }

  if (m_bIntMode)
    fVal = nsMath::Round(QDoubleSpinBox::value());

  QString sText = QDoubleSpinBox::textFromValue(fVal);

  while (sText.startsWith('0'))
    sText.remove(0, 1);

  if (sText.contains('.') || sText.contains(','))
  {
    while (sText.endsWith("0"))
      sText.chop(1);
    if (sText.endsWith(',') || sText.endsWith('.'))
      sText.chop(1);
    if (sText.startsWith(',') || sText.startsWith('.'))
      sText.insert(0, '0');
  }

  m_fDisplayedValue = fVal;
  m_sDisplayedText = sText;

  if (!hasFocus())
  {
    sText += m_sSuffix;
  }

  return sText;
}

double nsQtDoubleSpinBox::valueFromText(const QString& sText) const
{
  if (m_bInvalid)
  {
    m_bInvalid = false;
  }

  QString sFixedText = sText;

  if (sFixedText.isEmpty())
  {
    sFixedText = "0";
  }

  if (!m_sSuffix.isEmpty() && sFixedText.endsWith(m_sSuffix))
  {
    sFixedText.chop(m_sSuffix.length());
  }

  if (sFixedText.contains(','))
    sFixedText.replace(',', ".");

  const double val = QDoubleSpinBox::valueFromText(sFixedText);

  if (hasFocus())
  {
    m_sDisplayedText = sText;
    m_fDisplayedValue = val;
  }

  return val;
}

void nsQtDoubleSpinBox::setValueInvalid()
{
  m_bInvalid = true;
  m_sDisplayedText = QString();
  m_fDisplayedValue = nsMath::NaN<float>();
  QDoubleSpinBox::setValue(minimum());
}

void nsQtDoubleSpinBox::setValue(double fVal)
{
  NS_ASSERT_DEBUG(nsMath::IsFinite(fVal), "Spin box value must be finite!");
  m_bInvalid = false;
  m_fDisplayedValue = nsMath::NaN<float>();
  QDoubleSpinBox::setValue(fVal);
}

void nsQtDoubleSpinBox::setValue(const nsVariant& val)
{
  double fValue = 0;
  if (nsToolsReflectionUtils::GetFloatFromVariant(val, fValue))
    setValue(fValue);
  else
    setValueInvalid();
}

double nsQtDoubleSpinBox::value() const
{
  if (m_bInvalid)
    return 0.0;

  NS_ASSERT_DEBUG(!nsMath::IsNaN(QDoubleSpinBox::value()), "Spin box valid value should never be NaN!");
  return m_bIntMode ? nsMath::Round(QDoubleSpinBox::value()) : QDoubleSpinBox::value();
}

void nsQtDoubleSpinBox::focusInEvent(QFocusEvent* event)
{
  if (!m_sSuffix.isEmpty())
  {
    QString s = lineEdit()->text();

    if (!m_sSuffix.isEmpty() && s.endsWith(m_sSuffix))
    {
      s.chop(m_sSuffix.length());
    }

    lineEdit()->setText(s);
  }

  QDoubleSpinBox::focusInEvent(event);
}

void nsQtDoubleSpinBox::focusOutEvent(QFocusEvent* event)
{
  QDoubleSpinBox::focusOutEvent(event);
}

void nsQtDoubleSpinBox::mousePressEvent(QMouseEvent* event)
{
  if (!isReadOnly())
  {
    QStyleOptionSpinBox opt;
    initStyleOption(&opt);
    opt.subControls = QStyle::SC_All;
    QStyle::SubControl hoverControl = style()->hitTestComplexControl(QStyle::CC_SpinBox, &opt, event->pos(), this);

    if (event->button() == Qt::LeftButton && (hoverControl == QStyle::SC_SpinBoxUp || hoverControl == QStyle::SC_SpinBoxDown))
    {
      m_fStartDragValue = value();
      m_bDragging = true;
      m_iDragDelta = 0;
      m_bModified = false;
      m_LastDragPos = event->globalPosition().toPoint();
      grabMouse();
      event->accept();
      return;
    }
  }

  QDoubleSpinBox::mousePressEvent(event);
}

void nsQtDoubleSpinBox::mouseReleaseEvent(QMouseEvent* event)
{
  if (!isReadOnly())
  {
    if (event->button() == Qt::LeftButton && m_bDragging)
    {
      m_fStartDragValue = 0;
      m_bDragging = false;
      m_iDragDelta = 0;
      if (m_bModified)
      {
        m_bModified = false;
        Q_EMIT editingFinished();
      }
      else
      {
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        opt.subControls = QStyle::SC_All;
        QStyle::SubControl hoverControl = style()->hitTestComplexControl(QStyle::CC_SpinBox, &opt, event->pos(), this);
        if (hoverControl == QStyle::SC_SpinBoxUp)
        {
          stepUp();
        }
        else if (hoverControl == QStyle::SC_SpinBoxDown)
        {
          stepDown();
        }
        // editingFinished sent on leave focus
      }
      releaseMouse();
      event->accept();
      return;
    }
  }

  QDoubleSpinBox::mouseReleaseEvent(event);
}

void nsQtDoubleSpinBox::mouseMoveEvent(QMouseEvent* event)
{
  if (!isReadOnly())
  {
    if (m_bDragging)
    {
      int iDelta = m_LastDragPos.y() - event->globalPosition().toPoint().y();
      m_iDragDelta += iDelta;
      {
        m_LastDragPos = event->globalPosition().toPoint();
        const QRect dsize = nsWidgetUtils::GetClosestScreen(event->globalPosition().toPoint()).availableGeometry();
        if (m_LastDragPos.y() < (dsize.top() + 10))
        {
          m_LastDragPos.setY(dsize.bottom() - 10);
          QCursor::setPos(m_LastDragPos);
        }
        else if (m_LastDragPos.y() > (dsize.bottom() - 10))
        {
          m_LastDragPos.setY(dsize.top() + 10);
          QCursor::setPos(m_LastDragPos);
        }
      }

      double fValue = m_fStartDragValue;
      if (m_bIntMode)
        fValue += ((double)m_iDragDelta / 8.0);
      else
        fValue += ((double)m_iDragDelta * 0.01);

      setValue(fValue);
      m_bModified = true;
    }
  }

  QDoubleSpinBox::mouseMoveEvent(event);
}

void nsQtDoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
  {
    QString t = lineEdit()->text();
    double val = valueFromText(t);
    QDoubleSpinBox::setValue(val);
    Q_EMIT editingFinished();
    lineEdit()->setText(t);
    return;
  }

  QDoubleSpinBox::keyPressEvent(event);
}

bool nsQtDoubleSpinBox::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent == QKeySequence::Redo || keyEvent == QKeySequence::Undo)
      return true;
  }
  return QDoubleSpinBox::event(event);
}

void nsQtDoubleSpinBox::onCustomContextMenuRequested()
{
  if (!isReadOnly())
  {
    m_sDisplayedText = QDoubleSpinBox::textFromValue(m_fDefaultValue);
    setValue(m_fDefaultValue);
  }
}
