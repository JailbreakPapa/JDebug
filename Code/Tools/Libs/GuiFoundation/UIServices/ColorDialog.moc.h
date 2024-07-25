#pragma once

#include <Foundation/Math/Color.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/ColorDlgWidgets.moc.h>
#include <GuiFoundation/ui_ColorDialog.h>
#include <QDialog>

class QLineEdit;
class nsQtDoubleSpinBox;
class QPushButton;
class QSlider;


class NS_GUIFOUNDATION_DLL nsQtColorDialog : public QDialog, Ui_ColorDialog
{
  Q_OBJECT
public:
  nsQtColorDialog(const nsColor& initial, QWidget* pParent);
  ~nsQtColorDialog();

  void ShowAlpha(bool bEnable);
  void ShowHDR(bool bEnable);

  static QByteArray GetLastDialogGeometry() { return s_LastDialogGeometry; }

Q_SIGNALS:
  void CurrentColorChanged(const nsColor& color);
  void ColorSelected(const nsColor& color);

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

  nsUInt16 m_uiHue;
  nsUInt8 m_uiSaturation;

  nsUInt8 m_uiGammaRed;
  nsUInt8 m_uiGammaGreen;
  nsUInt8 m_uiGammaBlue;

  nsUInt8 m_uiAlpha;
  float m_fExposureValue;

  nsColor m_CurrentColor;

  static QByteArray s_LastDialogGeometry;

private:
  void ApplyColor();

  void RecomputeHDR();

  void ExtractColorRGB();
  void ExtractColorHSV();

  void ComputeRgbAndHsv(const nsColor& color);
  void RecomputeRGB();
  void RecomputeHSV();
};
