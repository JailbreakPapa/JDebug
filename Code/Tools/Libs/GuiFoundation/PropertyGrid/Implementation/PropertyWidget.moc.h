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
class nsDocumentObject;
class nsQtDoubleSpinBox;
class QSlider;

/// *** CHECKBOX ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorCheckboxWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorCheckboxWidget();

  virtual void mousePressEvent(QMouseEvent* pEv) override;

private Q_SLOTS:
  void on_StateChanged_triggered(int state);

protected:
  virtual void OnInit() override {}
  virtual void InternalSetValue(const nsVariant& value) override;

  QHBoxLayout* m_pLayout;
  QCheckBox* m_pWidget;
};



/// *** DOUBLE SPINBOX ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorDoubleSpinboxWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorDoubleSpinboxWidget(nsInt8 iNumComponents);

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

  bool m_bUseTemporaryTransaction = false;
  bool m_bTemporaryCommand = false;
  nsInt8 m_iNumComponents = 0;
  nsEnum<nsVariantType> m_OriginalType;
  QHBoxLayout* m_pLayout = nullptr;
  nsQtDoubleSpinBox* m_pWidget[4] = {};
};

/// *** TIME SPINBOX ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorTimeWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorTimeWidget();

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

  bool m_bTemporaryCommand;
  QHBoxLayout* m_pLayout;
  nsQtDoubleSpinBox* m_pWidget;
};

/// *** ANGLE SPINBOX ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorAngleWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorAngleWidget();

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

  bool m_bTemporaryCommand;
  QHBoxLayout* m_pLayout;
  nsQtDoubleSpinBox* m_pWidget;
};

/// *** INT SPINBOX ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorIntSpinboxWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorIntSpinboxWidget(nsInt8 iNumComponents, nsInt32 iMinValue, nsInt32 iMaxValue);
  ~nsQtPropertyEditorIntSpinboxWidget();

  void SetReadOnly(bool bReadOnly = true) override;

private Q_SLOTS:
  void SlotValueChanged();
  void SlotSliderValueChanged(int value);
  void on_EditingFinished_triggered();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

  bool m_bUseTemporaryTransaction = false;
  bool m_bTemporaryCommand = false;
  nsInt8 m_iNumComponents = 0;
  nsEnum<nsVariantType> m_OriginalType;
  QHBoxLayout* m_pLayout = nullptr;
  nsQtDoubleSpinBox* m_pWidget[4] = {};
  QSlider* m_pSlider = nullptr;
};

/// *** SLIDER ***

class NS_GUIFOUNDATION_DLL nsQtImageSliderWidget : public QWidget
{
  Q_OBJECT
public:
  using ImageGeneratorFunc = QImage (*)(nsUInt32 uiWidth, nsUInt32 uiHeight, double fMinValue, double fMaxValue);

  nsQtImageSliderWidget(ImageGeneratorFunc generator, double fMinValue, double fMaxValue, QWidget* pParent);

  static nsMap<nsString, ImageGeneratorFunc> s_ImageGenerators;

  double GetValue() const { return m_fValue; }
  void SetValue(double fValue);

Q_SIGNALS:
  void valueChanged(double x);
  void sliderReleased();

protected:
  virtual void paintEvent(QPaintEvent*) override;
  virtual void mouseMoveEvent(QMouseEvent*) override;
  virtual void mousePressEvent(QMouseEvent*) override;
  virtual void mouseReleaseEvent(QMouseEvent*) override;

  void UpdateImage();

  ImageGeneratorFunc m_Generator = nullptr;
  QImage m_Image;
  double m_fValue = 0;
  double m_fMinValue = 0;
  double m_fMaxValue = 0;
};

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorSliderWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorSliderWidget();
  ~nsQtPropertyEditorSliderWidget();

private Q_SLOTS:
  void SlotSliderValueChanged(double fValue);
  void on_EditingFinished_triggered();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

  bool m_bTemporaryCommand = false;
  nsEnum<nsVariantType> m_OriginalType;
  QHBoxLayout* m_pLayout = nullptr;
  nsQtImageSliderWidget* m_pSlider = nullptr;

  double m_fMinValue = 0;
  double m_fMaxValue = 0;
};

/// *** QUATERNION ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorQuaternionWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorQuaternionWidget();

private Q_SLOTS:
  void on_EditingFinished_triggered();
  void SlotValueChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

protected:
  bool m_bTemporaryCommand;
  QHBoxLayout* m_pLayout;
  nsQtDoubleSpinBox* m_pWidget[3];
};


/// *** LINEEDIT ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorLineEditWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorLineEditWidget();

  void SetReadOnly(bool bReadOnly = true) override;

protected Q_SLOTS:
  void on_TextChanged_triggered(const QString& value);
  void on_TextFinished_triggered();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

protected:
  QHBoxLayout* m_pLayout;
  QLineEdit* m_pWidget;
  nsEnum<nsVariantType> m_OriginalType;
};


/// *** COLOR ***

class NS_GUIFOUNDATION_DLL nsQtColorButtonWidget : public QFrame
{
  Q_OBJECT

public:
  explicit nsQtColorButtonWidget(QWidget* pParent);
  void SetColor(const nsVariant& color);

Q_SIGNALS:
  void clicked();

protected:
  virtual void showEvent(QShowEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;

  virtual QSize sizeHint() const override;
  virtual QSize minimumSizeHint() const override;

private:
  QPalette m_Pal;
};

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorColorWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorColorWidget();

private Q_SLOTS:
  void on_Button_triggered();
  void on_CurrentColor_changed(const nsColor& color);
  void on_Color_reset();
  void on_Color_accepted();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

protected:
  bool m_bExposeAlpha;
  QHBoxLayout* m_pLayout;
  nsQtColorButtonWidget* m_pWidget;
  nsVariant m_OriginalValue;
};


/// *** ENUM COMBOBOX ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorEnumWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorEnumWidget();

private Q_SLOTS:
  void on_CurrentEnum_changed(int iEnum);

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

protected:
  QHBoxLayout* m_pLayout;
  QComboBox* m_pWidget;
  nsInt64 m_iCurrentEnum;
};


/// *** BITFLAGS COMBOBOX ***

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorBitflagsWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorBitflagsWidget();
  virtual ~nsQtPropertyEditorBitflagsWidget();

private Q_SLOTS:
  void on_Menu_aboutToShow();
  void on_Menu_aboutToHide();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;
  void SetAllChecked(bool bChecked);

protected:
  nsMap<nsInt64, QCheckBox*> m_Constants;
  QHBoxLayout* m_pLayout = nullptr;
  QPushButton* m_pWidget = nullptr;
  QPushButton* m_pAllButton = nullptr;
  QPushButton* m_pClearButton = nullptr;
  QMenu* m_pMenu = nullptr;
  nsInt64 m_iCurrentBitflags = 0;
};


/// *** CURVE1D ***

class NS_GUIFOUNDATION_DLL nsQtCurve1DButtonWidget : public QLabel
{
  Q_OBJECT

public:
  explicit nsQtCurve1DButtonWidget(QWidget* pParent);

  void UpdatePreview(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pCurveObject, QColor color, double fLowerExtents, bool bLowerFixed, double fUpperExtents, bool bUpperFixed, double fDefaultValue, double fLowerRange, double fUpperRange);

Q_SIGNALS:
  void clicked();

protected:
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
};

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorCurve1DWidget : public nsQtPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorCurve1DWidget();

private Q_SLOTS:
  void on_Button_triggered();

protected:
  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;
  virtual void OnInit() override;
  virtual void DoPrepareToDie() override;
  void UpdatePreview();

protected:
  QHBoxLayout* m_pLayout = nullptr;
  nsQtCurve1DButtonWidget* m_pButton = nullptr;
};
