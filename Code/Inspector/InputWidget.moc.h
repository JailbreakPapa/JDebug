#pragma once

#include <Core/Input/InputManager.h>
#include <Foundation/Basics.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Inspector/ui_InputWidget.h>
#include <ads/DockWidget.h>

class nsQtInputWidget : public ads::CDockWidget, public Ui_InputWidget
{
public:
  Q_OBJECT

public:
  nsQtInputWidget(QWidget* pParent = 0);

  static nsQtInputWidget* s_pWidget;

private Q_SLOTS:
  virtual void on_ButtonClearSlots_clicked();
  virtual void on_ButtonClearActions_clicked();

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();

private:
  void ClearSlots();
  void ClearActions();

  void UpdateSlotTable(bool bRecreate);
  void UpdateActionTable(bool bRecreate);

  struct SlotData
  {
    nsInt32 m_iTableRow;
    nsUInt16 m_uiSlotFlags;
    nsKeyState::Enum m_KeyState;
    float m_fValue;
    float m_fDeadZone;

    SlotData()
    {
      m_iTableRow = -1;
      m_uiSlotFlags = 0;
      m_KeyState = nsKeyState::Up;
      m_fValue = 0;
      m_fDeadZone = 0;
    }
  };

  nsMap<nsString, SlotData> m_InputSlots;

  struct ActionData
  {
    nsInt32 m_iTableRow;
    nsKeyState::Enum m_KeyState;
    float m_fValue;
    bool m_bUseTimeScaling;

    nsString m_sTrigger[nsInputActionConfig::MaxInputSlotAlternatives];
    float m_fTriggerScaling[nsInputActionConfig::MaxInputSlotAlternatives];

    ActionData()
    {
      m_iTableRow = -1;
      m_KeyState = nsKeyState::Up;
      m_fValue = 0;
      m_bUseTimeScaling = false;
    }
  };

  nsMap<nsString, ActionData> m_InputActions;
};
