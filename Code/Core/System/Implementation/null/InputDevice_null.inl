#include <Core/System/Implementation/null/InputDevice_null.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsStandardInputDevice, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsStandardInputDevice::nsStandardInputDevice(nsUInt32 uiWindowNumber) {}
nsStandardInputDevice::~nsStandardInputDevice() = default;

void nsStandardInputDevice::SetShowMouseCursor(bool bShow) {}

bool nsStandardInputDevice::GetShowMouseCursor() const
{
  return false;
}

void nsStandardInputDevice::SetClipMouseCursor(nsMouseCursorClipMode::Enum mode) {}

nsMouseCursorClipMode::Enum nsStandardInputDevice::GetClipMouseCursor() const
{
  return nsMouseCursorClipMode::Default;
}

void nsStandardInputDevice::InitializeDevice() {}

void nsStandardInputDevice::RegisterInputSlots() {}
