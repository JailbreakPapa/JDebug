#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/ColorGradientEditorWidget.moc.h>

wdQtColorGradientEditorWidget::wdQtColorGradientEditorWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  GradientWidget->setColorGradientData(&m_Gradient);
  GradientWidget->setEditMode(true);
  GradientWidget->FrameExtents();

  GradientWidget->setShowColorCPs(true);
  GradientWidget->setShowAlphaCPs(true);
  GradientWidget->setShowIntensityCPs(true);
  GradientWidget->setShowCoords(true, true);

  on_GradientWidget_selectionChanged(-1, -1, -1);

  connect(
    GradientWidget, &wdQtColorGradientWidget::addColorCp, this, [this](double x, const wdColorGammaUB& color) { Q_EMIT ColorCpAdded(x, color); });
  connect(GradientWidget, &wdQtColorGradientWidget::moveColorCpToPos, this, [this](wdInt32 iIdx, double x) { Q_EMIT ColorCpMoved(iIdx, x); });
  connect(GradientWidget, &wdQtColorGradientWidget::deleteColorCp, this, [this](wdInt32 iIdx) { Q_EMIT ColorCpDeleted(iIdx); });

  connect(GradientWidget, &wdQtColorGradientWidget::addAlphaCp, this, [this](double x, wdUInt8 uiAlpha) { Q_EMIT AlphaCpAdded(x, uiAlpha); });
  connect(GradientWidget, &wdQtColorGradientWidget::moveAlphaCpToPos, this, [this](wdInt32 iIdx, double x) { Q_EMIT AlphaCpMoved(iIdx, x); });
  connect(GradientWidget, &wdQtColorGradientWidget::deleteAlphaCp, this, [this](wdInt32 iIdx) { Q_EMIT AlphaCpDeleted(iIdx); });

  connect(
    GradientWidget, &wdQtColorGradientWidget::addIntensityCp, this, [this](double x, float fIntensity) { Q_EMIT IntensityCpAdded(x, fIntensity); });
  connect(GradientWidget, &wdQtColorGradientWidget::moveIntensityCpToPos, this, [this](wdInt32 iIdx, double x) { Q_EMIT IntensityCpMoved(iIdx, x); });
  connect(GradientWidget, &wdQtColorGradientWidget::deleteIntensityCp, this, [this](wdInt32 iIdx) { Q_EMIT IntensityCpDeleted(iIdx); });

  connect(GradientWidget, &wdQtColorGradientWidget::beginOperation, this, [this]() { Q_EMIT BeginOperation(); });
  connect(GradientWidget, &wdQtColorGradientWidget::endOperation, this, [this](bool bCommit) { Q_EMIT EndOperation(bCommit); });

  connect(GradientWidget, &wdQtColorGradientWidget::triggerPickColor, this, [this]() { on_ButtonColor_clicked(); });
}


wdQtColorGradientEditorWidget::~wdQtColorGradientEditorWidget() {}


void wdQtColorGradientEditorWidget::SetColorGradient(const wdColorGradient& gradient)
{
  bool clearSelection = false;

  // clear selection if the number of control points has changed
  {
    wdUInt32 numRgb = 0xFFFFFFFF, numRgb2 = 0xFFFFFFFF;
    wdUInt32 numAlpha = 0xFFFFFFFF, numAlpha2 = 0xFFFFFFFF;
    wdUInt32 numIntensity = 0xFFFFFFFF, numIntensity2 = 0xFFFFFFFF;

    gradient.GetNumControlPoints(numRgb, numAlpha, numIntensity);
    m_Gradient.GetNumControlPoints(numRgb2, numAlpha2, numIntensity2);

    if (numRgb != numRgb2 || numAlpha != numAlpha2 || numIntensity != numIntensity2)
      clearSelection = true;
  }

  // const bool wasEmpty = m_Gradient.IsEmpty();

  m_Gradient = gradient;

  {
    wdQtScopedUpdatesDisabled ud(this);

    // if (wasEmpty)
    //  GradientWidget->FrameExtents();

    if (clearSelection)
      GradientWidget->ClearSelectedCP();
  }

  UpdateCpUi();

  GradientWidget->update();
}

void wdQtColorGradientEditorWidget::SetScrubberPosition(wdUInt64 uiTick)
{
  GradientWidget->SetScrubberPosition(uiTick / 4800.0);
}

void wdQtColorGradientEditorWidget::FrameGradient()
{
  GradientWidget->FrameExtents();
  GradientWidget->update();
}

void wdQtColorGradientEditorWidget::on_ButtonFrame_clicked()
{
  FrameGradient();
}

void wdQtColorGradientEditorWidget::on_GradientWidget_selectionChanged(wdInt32 colorCP, wdInt32 alphaCP, wdInt32 intensityCP)
{
  m_iSelectedColorCP = colorCP;
  m_iSelectedAlphaCP = alphaCP;
  m_iSelectedIntensityCP = intensityCP;

  SpinPosition->setEnabled((m_iSelectedColorCP != -1) || (m_iSelectedAlphaCP != -1) || (m_iSelectedIntensityCP != -1));

  LabelColor->setVisible(m_iSelectedColorCP != -1);
  ButtonColor->setVisible(m_iSelectedColorCP != -1);

  LabelAlpha->setVisible(m_iSelectedAlphaCP != -1);
  SpinAlpha->setVisible(m_iSelectedAlphaCP != -1);
  SliderAlpha->setVisible(m_iSelectedAlphaCP != -1);

  LabelIntensity->setVisible(m_iSelectedIntensityCP != -1);
  SpinIntensity->setVisible(m_iSelectedIntensityCP != -1);

  UpdateCpUi();
}


void wdQtColorGradientEditorWidget::on_SpinPosition_valueChanged(double value)
{
  if (m_iSelectedColorCP != -1)
  {
    Q_EMIT ColorCpMoved(m_iSelectedColorCP, value);
  }
  else if (m_iSelectedAlphaCP != -1)
  {
    Q_EMIT AlphaCpMoved(m_iSelectedAlphaCP, value);
  }
  else if (m_iSelectedIntensityCP != -1)
  {
    Q_EMIT IntensityCpMoved(m_iSelectedIntensityCP, value);
  }
}


void wdQtColorGradientEditorWidget::on_SpinAlpha_valueChanged(int value)
{
  if (m_iSelectedAlphaCP != -1)
  {
    Q_EMIT AlphaCpChanged(m_iSelectedAlphaCP, value);
  }
}

void wdQtColorGradientEditorWidget::on_SliderAlpha_valueChanged(int value)
{
  if (m_iSelectedAlphaCP != -1)
  {
    Q_EMIT AlphaCpChanged(m_iSelectedAlphaCP, value);
  }
}


void wdQtColorGradientEditorWidget::on_SliderAlpha_sliderPressed()
{
  Q_EMIT BeginOperation();
}


void wdQtColorGradientEditorWidget::on_SliderAlpha_sliderReleased()
{
  Q_EMIT EndOperation(true);
}

void wdQtColorGradientEditorWidget::on_SpinIntensity_valueChanged(double value)
{
  if (m_iSelectedIntensityCP != -1)
  {
    Q_EMIT IntensityCpChanged(m_iSelectedIntensityCP, value);
  }
}


void wdQtColorGradientEditorWidget::on_ButtonColor_clicked()
{
  if (m_iSelectedColorCP != -1)
  {
    const auto& cp = m_Gradient.GetColorControlPoint(m_iSelectedColorCP);

    m_PickColorStart = wdColorGammaUB(cp.m_GammaRed, cp.m_GammaGreen, cp.m_GammaBlue);
    m_PickColorCurrent = m_PickColorStart;

    Q_EMIT BeginOperation();

    wdQtUiServices::GetSingleton()->ShowColorDialog(
      m_PickColorStart, false, false, this, SLOT(onCurrentColorChanged(const wdColor&)), SLOT(onColorAccepted()), SLOT(onColorReset()));
  }
}

void wdQtColorGradientEditorWidget::onCurrentColorChanged(const wdColor& col)
{
  if (m_iSelectedColorCP != -1)
  {
    m_PickColorCurrent = col;

    Q_EMIT ColorCpChanged(m_iSelectedColorCP, m_PickColorCurrent);
  }
}

void wdQtColorGradientEditorWidget::onColorAccepted()
{
  if (m_iSelectedColorCP != -1)
  {
    Q_EMIT ColorCpChanged(m_iSelectedColorCP, m_PickColorCurrent);

    Q_EMIT EndOperation(true);
  }
}

void wdQtColorGradientEditorWidget::onColorReset()
{
  if (m_iSelectedColorCP != -1)
  {
    Q_EMIT ColorCpChanged(m_iSelectedColorCP, m_PickColorStart);

    Q_EMIT EndOperation(false);
  }
}


void wdQtColorGradientEditorWidget::on_ButtonNormalize_clicked()
{
  Q_EMIT NormalizeRange();
}

void wdQtColorGradientEditorWidget::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set palette.
  ButtonColor->setPalette(m_Pal);
  QWidget::showEvent(event);
}

void wdQtColorGradientEditorWidget::UpdateCpUi()
{
  wdQtScopedBlockSignals bs(this);
  wdQtScopedUpdatesDisabled ud(this);

  if (m_iSelectedColorCP != -1)
  {
    const auto& cp = m_Gradient.GetColorControlPoint(m_iSelectedColorCP);

    SpinPosition->setValue(cp.m_PosX);

    QColor col;
    col.setRgb(cp.m_GammaRed, cp.m_GammaGreen, cp.m_GammaBlue);

    ButtonColor->setAutoFillBackground(true);
    m_Pal.setColor(QPalette::Button, col);
    ButtonColor->setPalette(m_Pal);
  }

  if (m_iSelectedAlphaCP != -1)
  {
    SpinPosition->setValue(m_Gradient.GetAlphaControlPoint(m_iSelectedAlphaCP).m_PosX);
    SpinAlpha->setValue(m_Gradient.GetAlphaControlPoint(m_iSelectedAlphaCP).m_Alpha);
    SliderAlpha->setValue(m_Gradient.GetAlphaControlPoint(m_iSelectedAlphaCP).m_Alpha);
  }

  if (m_iSelectedIntensityCP != -1)
  {
    SpinPosition->setValue(m_Gradient.GetIntensityControlPoint(m_iSelectedIntensityCP).m_PosX);
    SpinIntensity->setValue(m_Gradient.GetIntensityControlPoint(m_iSelectedIntensityCP).m_Intensity);
  }
}
