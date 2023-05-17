#pragma once

#include <Foundation/Math/Color.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/ColorDlgWidgets.moc.h>
#include <GuiFoundation/ui_ColorDialog.h>
#include <QDialog>

class QLineEdit;
class wdQtDoubleSpinBox;
class QPushButton;
class QSlider;


class WD_GUIFOUNDATION_DLL wdQtColorDialog : public QDialog, Ui_ColorDialog
{
  Q_OBJECT
public:
  wdQtColorDialog(const wdColor& initial, QWidget* pParent);
  ~wdQtColorDialog();

  void ShowAlpha(bool bEnable);
  void ShowHDR(bool bEnable);

  static QByteArray GetLastDialogGeometry() { return s_LastDialogGeometry; }

Q_SIGNALS:
  void CurrentColorChanged(const wdColor& color);
  void ColorSelected(const wdColor& color);

private Q_SLOTS:
  void ChangedRGB();
  void ChangedAlpha();
  void ChangedExposure();
  void ChangedHSV();
  void ChangedArea(double x, double y);
  void ChangedRange(double x);
  void ChangedHEX();

private:
  bool m_bAlpha;
  bool m_bHDR;

  float m_fHue;
  float m_fSaturation;
  float m_fValue;

  wdUInt16 m_uiHue;
  wdUInt8 m_uiSaturation;

  wdUInt8 m_uiGammaRed;
  wdUInt8 m_uiGammaGreen;
  wdUInt8 m_uiGammaBlue;

  wdUInt8 m_uiAlpha;
  float m_fExposureValue;

  wdColor m_CurrentColor;

  static QByteArray s_LastDialogGeometry;

private:
  void ApplyColor();

  void RecomputeHDR();

  void ExtractColorRGB();
  void ExtractColorHSV();

  void ComputeRgbAndHsv(const wdColor& color);
  void RecomputeRGB();
  void RecomputeHSV();
};

