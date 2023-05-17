#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Math/Color8UNorm.h>
#include <GuiFoundation/UIServices/ColorDlgWidgets.moc.h>
#include <QPainter>
#include <qevent.h>

wdQtColorAreaWidget::wdQtColorAreaWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setAutoFillBackground(false);

  m_fHue = -1.0f;
}

void wdQtColorAreaWidget::SetHue(float fHue)
{
  if (m_fHue == fHue)
    return;

  m_fHue = fHue;
  UpdateImage();
  update();
}

void wdQtColorAreaWidget::SetSaturation(float fSat)
{
  if (m_fSaturation == fSat)
    return;

  m_fSaturation = fSat;
  update();
}

void wdQtColorAreaWidget::SetValue(float fVal)
{
  if (m_fValue == fVal)
    return;

  m_fValue = fVal;
  update();
}

void wdQtColorAreaWidget::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::RenderHint::Antialiasing);

  const QRect area = rect();

  painter.drawTiledPixmap(area, QPixmap::fromImage(m_Image));

  QPointF center;
  center.setX((int)(area.width() * m_fSaturation) + 0.5f);
  center.setY((int)(area.height() * (1.0f - m_fValue)) + 0.5f);

  const QColor col = qRgb(40, 40, 40);

  painter.setPen(col);
  painter.drawEllipse(center, 5.5f, 5.5f);
}

void wdQtColorAreaWidget::UpdateImage()
{
  const int width = rect().width();
  const int height = rect().height();

  m_Image = QImage(width, height, QImage::Format::Format_RGB32);


  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      wdColor c;
      c.SetHSV(m_fHue, (double)x / (width - 1), (double)y / (height - 1));

      wdColorGammaUB cg = c;
      m_Image.setPixel(x, (height - 1) - y, qRgb(cg.r, cg.g, cg.b));
    }
  }
}

void wdQtColorAreaWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons().testFlag(Qt::LeftButton))
  {
    const int width = rect().width();
    const int height = rect().height();

    QPoint coord = event->pos();
    const int sat = wdMath::Clamp(coord.x(), 0, width - 1);
    const int val = wdMath::Clamp((height - 1) - coord.y(), 0, height - 1);

    const double fsat = (double)sat / (width - 1);
    const double fval = (double)val / (height - 1);

    valueChanged(fsat, fval);
  }
}

void wdQtColorAreaWidget::mousePressEvent(QMouseEvent* event)
{
  mouseMoveEvent(event);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdQtColorRangeWidget::wdQtColorRangeWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setAutoFillBackground(false);
}

void wdQtColorRangeWidget::SetHue(float fHue)
{
  if (m_fHue == fHue)
    return;

  m_fHue = fHue;
  update();
}

void wdQtColorRangeWidget::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::RenderHint::Antialiasing);

  const QRect area = rect();

  if (area.width() != m_Image.width())
    UpdateImage();

  painter.drawTiledPixmap(area, QPixmap::fromImage(m_Image));

  const float pos = (int)(m_fHue / 359.0f * area.width()) + 0.5f;

  const float top = area.top() + 0.5f;
  const float bot = area.bottom() + 0.5f;
  const float len = 5.0f;
  const float wid = 2.0f;

  const QColor col = qRgb(80, 80, 80);

  painter.setPen(col);
  painter.setBrush(col);

  {
    QPainterPath path;
    path.moveTo(QPointF(pos - wid, top));
    path.lineTo(QPointF(pos, top + len));
    path.lineTo(QPointF(pos + wid, top));
    path.closeSubpath();

    painter.drawPath(path);
  }

  {
    QPainterPath path;
    path.moveTo(QPointF(pos - wid, bot));
    path.lineTo(QPointF(pos, bot - len));
    path.lineTo(QPointF(pos + wid, bot));
    path.closeSubpath();

    painter.drawPath(path);
  }
}

void wdQtColorRangeWidget::UpdateImage()
{
  const int width = rect().width();
  const int height = rect().height();

  m_Image = QImage(width, 1, QImage::Format::Format_RGB32);

  for (int x = 0; x < width; ++x)
  {
    wdColor c;
    c.SetHSV(((double)x / (width - 1.0)) * 360.0, 1, 1);

    wdColorGammaUB cg = c;
    m_Image.setPixel(x, 0, qRgb(cg.r, cg.g, cg.b));
  }
}

void wdQtColorRangeWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons().testFlag(Qt::LeftButton))
  {
    const int width = rect().width();
    const int height = rect().height();

    QPoint coord = event->pos();
    const int x = wdMath::Clamp(coord.x(), 0, width - 1);

    const double fx = (double)x / (width - 1);

    valueChanged(fx);
  }
}

void wdQtColorRangeWidget::mousePressEvent(QMouseEvent* event)
{
  mouseMoveEvent(event);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdQtColorCompareWidget::wdQtColorCompareWidget(QWidget* pParent)
{
  setAutoFillBackground(false);
}

void wdQtColorCompareWidget::SetNewColor(const wdColor& color)
{
  if (m_NewColor == color)
    return;

  m_NewColor = color;
  update();
}

void wdQtColorCompareWidget::SetInitialColor(const wdColor& color)
{
  m_InitialColor = color;
  m_NewColor = color;
}

void wdQtColorCompareWidget::paintEvent(QPaintEvent*)
{
  const QRect area = rect();
  const QRect areaTop(area.left(), area.top(), area.width(), area.height() / 2);
  const QRect areaBot(area.left(), areaTop.bottom(), area.width(), area.height() - areaTop.height()); // rounding ...

  QPainter p(this);

  wdColor inLDR = m_InitialColor;
  float fMultiplier = m_InitialColor.ComputeHdrMultiplier();

  if (fMultiplier > 1.0f)
  {
    inLDR.ScaleRGB(1.0f / fMultiplier);
  }

  QColor qInCol = wdToQtColor(inLDR);

  wdColor newLDR = m_NewColor;
  fMultiplier = m_NewColor.ComputeHdrMultiplier();

  if (fMultiplier > 1.0f)
  {
    newLDR.ScaleRGB(1.0f / fMultiplier);
  }

  QColor qNewCol = wdToQtColor(newLDR);

  p.fillRect(areaTop, qInCol);
  p.fillRect(areaBot, qNewCol);
}
