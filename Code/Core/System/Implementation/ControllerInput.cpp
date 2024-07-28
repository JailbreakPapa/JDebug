#include <Core/CorePCH.h>

#include <Core/System/ControllerInput.h>

namespace
{
  nsInputDeviceController* g_pInputDeviceController = nullptr;
}

bool nsControllerInput::HasDevice()
{
  return g_pInputDeviceController != nullptr;
}

nsInputDeviceController* nsControllerInput::GetDevice()
{
  return g_pInputDeviceController;
}

void nsControllerInput::SetDevice(nsInputDeviceController* pDevice)
{
  g_pInputDeviceController = pDevice;
}

#if NS_ENABLED(NS_SUPPORTS_GLFW)
#  include <Core/System/Implementation/glfw/ControllerInput_glfw.inl>
#endif
