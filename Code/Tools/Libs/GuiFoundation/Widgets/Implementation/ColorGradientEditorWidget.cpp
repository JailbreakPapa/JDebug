#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/ColorGradientEditorWidget.moc.h>

nsQtColorGradientEditorWidget::nsQtColorGradientEditorWidget(QWidget* pParent)
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
    GradientWidget, &nsQtColorGradientWidget::addColorCp, this, [this](double x, const nsColorGammaUB& color)
    { Q_EMIT ColorCpAdded(x, color); });
  connect(GradientWidget, &nsQtColorGradientWidget::moveColorCpToPos, this, [this](nsInt32 iIdx, double x)
    { Q_EMIT ColorCpMoved(iIdx, x); });
  connect(GradientWidget, &nsQtColorGradientWidget::deleteColorCp, this, [this](nsInt32 iIdx)
    { Q_EMIT ColorCpDeleted(iIdx); });

  connect(GradientWidget, &nsQtColorGradientWidget::addAlphaCp, this, [this](double x, nsUInt8 uiAlpha)
    { Q_EMIT AlphaCpAdded(x, uiAlpha); });
  connect(GradientWidget, &nsQtColorGradientWidget::moveAlphaCpToPos, this, [this](nsInt32 iIdx, double x)
    { Q_EMIT AlphaCpMoved(iIdx, x); });
  connect(GradientWidget, &nsQtColorGradientWidget::deleteAlphaCp, this, [this](nsInt32 iIdx)
    { Q_EMIT AlphaCpDeleted(iIdx); });

  connect(
    GradientWidget, &nsQtColorGradientWidget::addIntensityCp, this, [this](double x, float fIntensity)
    { Q_EMIT IntensityCpAdded(x, fIntensity); });
  connect(GradientWidget, &nsQtColorGradientWidget::moveIntensityCpToPos, this, [this](nsInt32 iIdx, double x)
    { Q_EMIT IntensityCpMoved(iIdx, x); });
  connect(GradientWidget, &nsQtColorGradientWidget::deleteIntensityCp, this, [this](nsInt32 iIdx)
    { Q_EMIT IntensityCpDeleted(iIdx); });

  connect(GradientWidget, &nsQtColorGradientWidget::beginOperation, this, [this]()
    { Q_EMIT BeginOperation(); });
  connect(GradientWidget, &nsQtColorGradientWidget::endOperation, this, [this](bool bCommit)
    { Q_EMIT EndOperation(bCommit); });

  connect(GradientWidget, &nsQtColorGradientWidget::triggerPickColor, this, [this]()
    { on_ButtonColor_clicked(); });
}


nsQtColorGradientEditorWidget::~nsQtColorGradientEditorWidget() = default;


void nsQtColorGradientEditorWidget::SetColorGradient(const nsColorGradient& gradient)
{
  bool clearSelection = false;

  // clear selection if the number of control points has changed
  {
    nsUInt32 numRgb = 0xFFFFFFFF, numRgb2 = 0xFFFFFFFF;
    nsUInt32 numAlpha = 0xFFFFFFFF, numAlpha2 = 0xFFFFFFFF;
    nsUInt32 numIntensity = 0xFFFFFFFF, numIntensity2 = 0xFFFFFFFF;

    gradient.GetNumControlPoints(numRgb, numAlpha, numIntensity);
    m_Gradient.GetNumControlPoints(numRgb2, numAlpha2, numIntensity2);

    if (numRgb != numRgb2 || numAlpha != numAlpha2 || numIntensity != numIntensity2)
      clearSelection = true;
  }

  // const bool wasEmpty = m_Gradient.IsEmpty();

  m_Gradient = gradient;

  {
    nsQtScopedUpdatesDisabled ud(this);

    // if (wasEmpty)
    //  GradientWidget->FrameExtents();

    if (clearSelection)
      GradientWidget->ClearSelectedCP();
  }

  UpdateCpUi();

  GradientWidget->update();
}

void nsQtColorGradientEditorWidget::SetScrubberPosition(nsUInt64 uiTick)
{
  GradientWidget->SetScrubberPosition(uiTick / 4800.0);
}

void nsQtColorGradientEditorWidget::FrameGradient()
{
  GradientWidget->FrameExtents();
  GradientWidget->update();
}

void nsQtColorGradientEditorWidget::on_ButtonFrame_clicked()
{
  FrameGradient();
}

void nsQtColorGradientEditorWidget::on_GradientWidget_selectionChanged(nsInt32 colorCP, nsInt32 alphaCP, nsInt32 intensityCP)
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


void nsQtColorGradientEditorWidget::on_SpinPosition_valueChanged(double value)
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


void nsQtColorGradientEditorWidget::on_SpinAlpha_valueChanged(int value)
{
  if (m_iSelectedAlphaCP != -1)
  {
    Q_EMIT AlphaCpChanged(m_iSelectedAlphaCP, value);
  }
}

void nsQtColorGradientEditorWidget::on_SliderAlpha_valueChanged(int value)
{
  if (m_iSelectedAlphaCP != -1)
  {
    Q_EMIT AlphaCpChanged(m_iSelectedAlphaCP, value);
  }
}


void nsQtColorGradientEditorWidget::on_SliderAlpha_sliderPressed()
{
  Q_EMIT BeginOperation();
}


void nsQtColorGradientEditorWidget::on_SliderAlpha_sliderReleased()
{
  Q_EMIT EndOperation(true);
}

void nsQtColorGradientEditorWidget::on_SpinIntensity_valueChanged(double value)
{
  if (m_iSelectedIntensityCP != -1)
  {
    Q_EMIT IntensityCpChanged(m_iSelectedIntensityCP, value);
  }
}


void nsQtColorGradientEditorWidget::on_ButtonColor_clicked()
{
  if (m_iSelectedColorCP != -1)
  {
    const auto& cp = m_Gradient.GetColorControlPoint(m_iSelectedColorCP);

    m_PickColorStart = nsColorGammaUB(cp.m_GammaRed, cp.m_GammaGreen, cp.m_GammaBlue);
    m_PickColorCurrent = m_PickColorStart;

    Q_EMIT BeginOperation();

    nsQtUiServices::GetSingleton()->ShowColorDialog(
      m_PickColorStart, false, false, this, SLOT(onCurrentColorChanged(const nsColor&)), SLOT(onColorAccepted()), SLOT(onColorReset()));
  }
}

void nsQtColorGradientEditorWidget::onCurrentColorChanged(const nsColor& col)
{
  if (m_iSelectedColorCP != -1)
  {
    m_PickColorCurrent = col;

    Q_EMIT ColorCpChanged(m_iSelectedColorCP, m_PickColorCurrent);
  }
}

void nsQtColorGradientEditorWidget::onColorAccepted()
{
  if (m_iSelectedColorCP != -1)
  {
    Q_EMIT ColorCpChanged(m_iSelectedColorCP, m_PickColorCurrent);

    Q_EMIT EndOperation(true);
  }
}

void nsQtColorGradientEditorWidget::onColorReset()
{
  if (m_iSelectedColorCP != -1)
  {
    Q_EMIT ColorCpChanged(m_iSelectedColorCP, m_PickColorStart);

    Q_EMIT EndOperation(false);
  }
}


void nsQtColorGradientEditorWidget::on_ButtonNormalize_clicked()
{
  Q_EMIT NormalizeRange();
}

void nsQtColorGradientEditorWidget::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set palette.
  ButtonColor->setPalette(m_Pal);
  QWidget::showEvent(event);
}

void nsQtColorGradientEditorWidget::UpdateCpUi()
{
  nsQtScopedBlockSignals bs(this);
  nsQtScopedUpdatesDisabled ud(this);

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
