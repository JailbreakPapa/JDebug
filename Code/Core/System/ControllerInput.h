#pragma once

#include <Core/CoreDLL.h>

class nsInputDeviceController;

class NS_CORE_DLL nsControllerInput
{
public:
  // \brief Returns if a global controller input device exists.
  static bool HasDevice();

  // \brief Returns the global controller input device. May be nullptr.
  static nsInputDeviceController* GetDevice();

  // \brief Set the global controller input device.
  static void SetDevice(nsInputDeviceController* pDevice);
};
