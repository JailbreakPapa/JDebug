#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_OSX)

#  if NS_ENABLED(NS_SUPPORTS_GLFW)
#    include <Foundation/Platform/GLFW/Screen_GLFW.h>
#  else
#    include <Foundation/Platform/NoImpl/Screen_NoImpl.h>
#  endif

#endif
