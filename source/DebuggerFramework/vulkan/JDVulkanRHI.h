#pragma once
#include <DebuggerFramework/stdafx.h>
#include <DebuggerFramework/vulkan/vkinclude.h>

#include "QObject"

namespace jdebug {
// The rhi class will be a QObject to interface better with qt std.(when using
// with the GUI)
class DEBUGGERFRAMEWORK_EXPORT JDVulkanRHI : public QObject {
  QOBJECT_H
 private:
  uint32_t _extendcount = 0;

 public:
};
}  // namespace jdebug