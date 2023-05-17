#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Types/Variant.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>

#include <QFrame>
#include <QLabel>

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QLabel;
class QHBoxLayout;
class QLineEdit;
class QPushButton;
class QComboBox;
class QStandardItemModel;
class QStandardItem;
class QToolButton;
class QMenu;
class wdDocumentObject;
class wdQtDoubleSpinBox;
class QSlider;

/// *** CHECKBOX ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorCheckboxWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorCheckboxWidget();

  virtual void mousePressEvent(QMouseEvent* pEv) override;

private Q_SLOTS:
  void on_StateChanged_triggered(int state);

protected:
  virtual void OnInit() override {}
  virtual void InternalSetValue(const wdVariant& value) override;

  QHBoxLayout* m_pLayout;
  QCheckBox* m_pWidget;
};



/// *** DOUBLE SPINBOX ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorDoubleSpinboxWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorDoubleSpinboxWidget(wdInt8 iNumComponents);

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

  bool m_bTemporaryCommand;
  wdInt8 m_iNumComponents;
  QHBoxLayout* m_pLayout;
  wdQtDoubleSpinBox* m_pWidget[4];
};

/// *** TIME SPINBOX ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorTimeWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorTimeWidget();

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

  bool m_bTemporaryCommand;
  QHBoxLayout* m_pLayout;
  wdQtDoubleSpinBox* m_pWidget;
};

/// *** ANGLE SPINBOX ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorAngleWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorAngleWidget();

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

  bool m_bTemporaryCommand;
  QHBoxLayout* m_pLayout;
  wdQtDoubleSpinBox* m_pWidget;
};

/// *** INT SPINBOX ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorIntSpinboxWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorIntSpinboxWidget(wdInt8 iNumComponents, wdInt32 iMinValue, wdInt32 iMaxValue);
  ~wdQtPropertyEditorIntSpinboxWidget();

private Q_SLOTS:
  void SlotValueChanged();
  void SlotSliderValueChanged(int value);
  void on_EditingFinished_triggered();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

  bool m_bTemporaryCommand;
  wdInt8 m_iNumComponents;
  QHBoxLayout* m_pLayout;
  wdQtDoubleSpinBox* m_pWidget[4];
  QSlider* m_pSlider = nullptr;
};

/// *** QUATERNION ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorQuaternionWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorQuaternionWidget();

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

protected:
  bool m_bTemporaryCommand;
  QHBoxLayout* m_pLayout;
  wdQtDoubleSpinBox* m_pWidget[3];
};


/// *** LINEEDIT ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorLineEditWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorLineEditWidget();

protected Q_SLOTS:
  void on_TextChanged_triggered(const QString& value);
  void on_TextFinished_triggered();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

protected:
  QHBoxLayout* m_pLayout;
  QLineEdit* m_pWidget;
};


/// *** COLOR ***

class WD_GUIFOUNDATION_DLL wdQtColorButtonWidget : public QFrame
{
  Q_OBJECT

public:
  explicit wdQtColorButtonWidget(QWidget* pParent);
  void SetColor(const wdVariant& color);

Q_SIGNALS:
  void clicked();

protected:
  virtual void showEvent(QShowEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
  QPalette m_Pal;
};

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorColorWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorColorWidget();

private Q_SLOTS:
  void on_Button_triggered();
  void on_CurrentColor_changed(const wdColor& color);
  void on_Color_reset();
  void on_Color_accepted();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

protected:
  bool m_bExposeAlpha;
  QHBoxLayout* m_pLayout;
  wdQtColorButtonWidget* m_pWidget;
  wdVariant m_OriginalValue;
};


/// *** ENUM COMBOBOX ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorEnumWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorEnumWidget();

private Q_SLOTS:
  void on_CurrentEnum_changed(int iEnum);

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

protected:
  QHBoxLayout* m_pLayout;
  QComboBox* m_pWidget;
  wdInt64 m_iCurrentEnum;
};


/// *** BITFLAGS COMBOBOX ***

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorBitflagsWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorBitflagsWidget();
  virtual ~wdQtPropertyEditorBitflagsWidget();

private Q_SLOTS:
  void on_Menu_aboutToShow();
  void on_Menu_aboutToHide();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

protected:
  wdMap<wdInt64, QCheckBox*> m_Constants;
  QHBoxLayout* m_pLayout;
  QPushButton* m_pWidget;
  QMenu* m_pMenu;
  wdInt64 m_iCurrentBitflags;
};


/// *** CURVE1D ***

class WD_GUIFOUNDATION_DLL wdQtCurve1DButtonWidget : public QLabel
{
  Q_OBJECT

public:
  explicit wdQtCurve1DButtonWidget(QWidget* pParent);

  void UpdatePreview(wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pCurveObject, QColor color, double fLowerExtents, bool bLowerFixed, double fUpperExtents, bool bUpperFixed, double fDefaultValue, double fLowerRange, double fUpperRange);

Q_SIGNALS:
  void clicked();

protected:
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
};

class WD_GUIFOUNDATION_DLL wdQtPropertyEditorCurve1DWidget : public wdQtPropertyWidget
{
  Q_OBJECT

public:
  wdQtPropertyEditorCurve1DWidget();

private Q_SLOTS:
  void on_Button_triggered();

protected:
  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;
  virtual void OnInit() override;
  virtual void DoPrepareToDie() override;
  void UpdatePreview();

protected:
  QHBoxLayout* m_pLayout = nullptr;
  wdQtCurve1DButtonWidget* m_pButton = nullptr;
};
