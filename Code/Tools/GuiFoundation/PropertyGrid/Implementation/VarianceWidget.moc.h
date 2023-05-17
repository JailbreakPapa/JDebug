#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Types/VarianceTypes.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <GuiFoundation/Widgets/DoubleSpinBox.moc.h>

class QSlider;

class wdQtVarianceTypeWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT

public:
  wdQtVarianceTypeWidget();

  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;

private Q_SLOTS:
  void onBeginTemporary();
  void onEndTemporary();
  void SlotValueChanged();
  void SlotVarianceChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const wdVariant& value) override;

  bool m_bTemporaryCommand = false;
  QHBoxLayout* m_pLayout = nullptr;
  wdQtDoubleSpinBox* m_pValueWidget = nullptr;
  QSlider* m_pVarianceWidget = nullptr;
  wdAbstractMemberProperty* m_pValueProp = nullptr;
  wdAbstractMemberProperty* m_pVarianceProp = nullptr;
};

