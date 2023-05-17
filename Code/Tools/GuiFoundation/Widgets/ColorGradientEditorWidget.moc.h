#pragma once

#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Tracks/ColorGradient.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_ColorGradientEditorWidget.h>

#include <QWidget>

class QMouseEvent;

class WD_GUIFOUNDATION_DLL wdQtColorGradientEditorWidget : public QWidget, public Ui_ColorGradientEditorWidget
{
  Q_OBJECT

public:
  explicit wdQtColorGradientEditorWidget(QWidget* pParent);
  ~wdQtColorGradientEditorWidget();

  void SetColorGradient(const wdColorGradient& gradient);
  const wdColorGradient& GetColorGradient() const { return m_Gradient; }

  void ShowColorPicker() { on_ButtonColor_clicked(); }
  void SetScrubberPosition(wdUInt64 uiTick);

  void FrameGradient();

Q_SIGNALS:
  void ColorCpAdded(double fPosX, const wdColorGammaUB& color);
  void ColorCpMoved(wdInt32 iIndex, float fNewPosX);
  void ColorCpDeleted(wdInt32 iIndex);
  void ColorCpChanged(wdInt32 iIndex, const wdColorGammaUB& color);

  void AlphaCpAdded(double fPosX, wdUInt8 uiAlpha);
  void AlphaCpMoved(wdInt32 iIndex, double fNewPosX);
  void AlphaCpDeleted(wdInt32 iIndex);
  void AlphaCpChanged(wdInt32 iIndex, wdUInt8 uiAlpha);

  void IntensityCpAdded(double fPosX, float fIntensity);
  void IntensityCpMoved(wdInt32 iIndex, double fNewPosX);
  void IntensityCpDeleted(wdInt32 iIndex);
  void IntensityCpChanged(wdInt32 iIndex, float fIntensity);

  void NormalizeRange();

  void BeginOperation();
  void EndOperation(bool bCommit);

private Q_SLOTS:
  void on_ButtonFrame_clicked();
  void on_GradientWidget_selectionChanged(wdInt32 colorCP, wdInt32 alphaCP, wdInt32 intensityCP);
  void on_SpinPosition_valueChanged(double value);
  void on_SpinAlpha_valueChanged(int value);
  void on_SliderAlpha_valueChanged(int value);
  void on_SliderAlpha_sliderPressed();
  void on_SliderAlpha_sliderReleased();
  void on_SpinIntensity_valueChanged(double value);
  void on_ButtonColor_clicked();
  void onCurrentColorChanged(const wdColor& col);
  void onColorAccepted();
  void onColorReset();
  void on_ButtonNormalize_clicked();

protected:
  virtual void showEvent(QShowEvent* event) override;

private:
  void UpdateCpUi();

  QPalette m_Pal;
  wdInt32 m_iSelectedColorCP;
  wdInt32 m_iSelectedAlphaCP;
  wdInt32 m_iSelectedIntensityCP;
  wdColorGradient m_Gradient;

  wdColorGammaUB m_PickColorStart;
  wdColorGammaUB m_PickColorCurrent;
};

