#pragma once

#include <Core/Input/DeviceTypes/MouseKeyboard.h>

class NS_CORE_DLL nsStandardInputDevice : public nsInputDeviceMouseKeyboard
{
  NS_ADD_DYNAMIC_REFLECTION(nsStandardInputDevice, nsInputDeviceMouseKeyboard);

public:
  nsStandardInputDevice(nsUInt32 uiWindowNumber);
  ~nsStandardInputDevice();

  virtual void SetShowMouseCursor(bool bShow) override;
  virtual bool GetShowMouseCursor() const override;
  virtual void SetClipMouseCursor(nsMouseCursorClipMode::Enum mode) override;
  virtual nsMouseCursorClipMode::Enum GetClipMouseCursor() const override;

private:
  virtual void InitializeDevice() override;
  virtual void RegisterInputSlots() override;
};
