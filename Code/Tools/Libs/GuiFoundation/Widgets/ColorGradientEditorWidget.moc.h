#pragma once

#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Tracks/ColorGradient.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_ColorGradientEditorWidget.h>

#include <QWidget>

class QMouseEvent;

class NS_GUIFOUNDATION_DLL nsQtColorGradientEditorWidget : public QWidget, public Ui_ColorGradientEditorWidget
{
  Q_OBJECT

public:
  explicit nsQtColorGradientEditorWidget(QWidget* pParent);
  ~nsQtColorGradientEditorWidget();

  void SetColorGradient(const nsColorGradient& gradient);
  const nsColorGradient& GetColorGradient() const { return m_Gradient; }

  void ShowColorPicker() { on_ButtonColor_clicked(); }
  void SetScrubberPosition(nsUInt64 uiTick);

  void FrameGradient();

Q_SIGNALS:
  void ColorCpAdded(double fPosX, const nsColorGammaUB& color);
  void ColorCpMoved(nsInt32 iIndex, float fNewPosX);
  void ColorCpDeleted(nsInt32 iIndex);
  void ColorCpChanged(nsInt32 iIndex, const nsColorGammaUB& color);

  void AlphaCpAdded(double fPosX, nsUInt8 uiAlpha);
  void AlphaCpMoved(nsInt32 iIndex, double fNewPosX);
  void AlphaCpDeleted(nsInt32 iIndex);
  void AlphaCpChanged(nsInt32 iIndex, nsUInt8 uiAlpha);

  void IntensityCpAdded(double fPosX, float fIntensity);
  void IntensityCpMoved(nsInt32 iIndex, double fNewPosX);
  void IntensityCpDeleted(nsInt32 iIndex);
  void IntensityCpChanged(nsInt32 iIndex, float fIntensity);

  void NormalizeRange();

  void BeginOperation();
  void EndOperation(bool bCommit);

private Q_SLOTS:
  void on_ButtonFrame_clicked();
  void on_GradientWidget_selectionChanged(nsInt32 colorCP, nsInt32 alphaCP, nsInt32 intensityCP);
  void on_SpinPosition_valueChanged(double value);
  void on_SpinAlpha_valueChanged(int value);
  void on_SliderAlpha_valueChanged(int value);
  void on_SliderAlpha_sliderPressed();
  void on_SliderAlpha_sliderReleased();
  void on_SpinIntensity_valueChanged(double value);
  void on_ButtonColor_clicked();
  void onCurrentColorChanged(const nsColor& col);
  void onColorAccepted();
  void onColorReset();
  void on_ButtonNormalize_clicked();

protected:
  virtual void showEvent(QShowEvent* event) override;

private:
  void UpdateCpUi();

  QPalette m_Pal;
  nsInt32 m_iSelectedColorCP;
  nsInt32 m_iSelectedAlphaCP;
  nsInt32 m_iSelectedIntensityCP;
  nsColorGradient m_Gradient;

  nsColorGammaUB m_PickColorStart;
  nsColorGammaUB m_PickColorCurrent;
};
