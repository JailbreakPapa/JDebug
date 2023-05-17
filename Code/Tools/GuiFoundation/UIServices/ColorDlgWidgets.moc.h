#pragma once

#include <Foundation/Math/Color.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QWidget>

class WD_GUIFOUNDATION_DLL wdQtColorAreaWidget : public QWidget
{
  Q_OBJECT
public:
  wdQtColorAreaWidget(QWidget* pParent);

  float GetHue() const { return m_fHue; }
  void SetHue(float fHue);

  float GetSaturation() const { return m_fSaturation; }
  void SetSaturation(float fSat);

  float GetValue() const { return m_fValue; }
  void SetValue(float fVal);

Q_SIGNALS:
  void valueChanged(double x, double y);

protected:
  virtual void paintEvent(QPaintEvent*) override;
  virtual void mouseMoveEvent(QMouseEvent*) override;
  virtual void mousePressEvent(QMouseEvent*) override;

  void UpdateImage();

  QImage m_Image;
  float m_fHue;
  float m_fSaturation;
  float m_fValue;
};

class WD_GUIFOUNDATION_DLL wdQtColorRangeWidget : public QWidget
{
  Q_OBJECT
public:
  wdQtColorRangeWidget(QWidget* pParent);

  float GetHue() const { return m_fHue; }
  void SetHue(float fHue);

Q_SIGNALS:
  void valueChanged(double x);

protected:
  virtual void paintEvent(QPaintEvent*) override;
  virtual void mouseMoveEvent(QMouseEvent*) override;
  virtual void mousePressEvent(QMouseEvent*) override;

  void UpdateImage();

  QImage m_Image;
  float m_fHue;
};

class WD_GUIFOUNDATION_DLL wdQtColorCompareWidget : public QWidget
{
  Q_OBJECT
public:
  wdQtColorCompareWidget(QWidget* pParent);

  void SetNewColor(const wdColor& color);
  void SetInitialColor(const wdColor& color);

protected:
  virtual void paintEvent(QPaintEvent*) override;

  wdColor m_InitialColor;
  wdColor m_NewColor;
};

