/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Math/Color.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QWidget>

class NS_GUIFOUNDATION_DLL nsQtColorAreaWidget : public QWidget
{
  Q_OBJECT
public:
  nsQtColorAreaWidget(QWidget* pParent);

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

class NS_GUIFOUNDATION_DLL nsQtColorRangeWidget : public QWidget
{
  Q_OBJECT
public:
  nsQtColorRangeWidget(QWidget* pParent);

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

class NS_GUIFOUNDATION_DLL nsQtColorCompareWidget : public QWidget
{
  Q_OBJECT
public:
  nsQtColorCompareWidget(QWidget* pParent);

  void SetNewColor(const nsColor& color);
  void SetInitialColor(const nsColor& color);

protected:
  virtual void paintEvent(QPaintEvent*) override;

  nsColor m_InitialColor;
  nsColor m_NewColor;
};

