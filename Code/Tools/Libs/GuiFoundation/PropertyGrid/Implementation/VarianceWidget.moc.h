/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Types/VarianceTypes.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <GuiFoundation/Widgets/DoubleSpinBox.moc.h>

class QSlider;

class nsQtVarianceTypeWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT

public:
  nsQtVarianceTypeWidget();

  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;

private Q_SLOTS:
  void onBeginTemporary();
  void onEndTemporary();
  void SlotValueChanged();
  void SlotVarianceChanged();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;

  bool m_bTemporaryCommand = false;
  QHBoxLayout* m_pLayout = nullptr;
  nsQtDoubleSpinBox* m_pValueWidget = nullptr;
  QSlider* m_pVarianceWidget = nullptr;
  const nsAbstractMemberProperty* m_pValueProp = nullptr;
  const nsAbstractMemberProperty* m_pVarianceProp = nullptr;
};

