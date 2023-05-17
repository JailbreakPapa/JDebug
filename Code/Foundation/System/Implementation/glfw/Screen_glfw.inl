#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <GLFW/glfw3.h>

namespace {
  wdResult wdGlfwError(const char* file, size_t line)
  {
    const char* desc;
    int errorCode = glfwGetError(&desc);
    if(errorCode != GLFW_NO_ERROR)
    {
      wdLog::Error("GLFW error {} ({}): {} - {}", file, line, errorCode, desc);
      return WD_FAILURE;
    }
    return WD_SUCCESS;
  }
}

#define WD_GLFW_RETURN_FAILURE_ON_ERROR() do { if(wdGlfwError(__FILE__, __LINE__).Failed()) return WD_FAILURE; } while(false)

wdResult wdScreen::EnumerateScreens(wdHybridArray<wdScreenInfo, 2>& out_Screens)
{
  out_Screens.Clear();

  int iMonitorCount = 0;
  GLFWmonitor** pMonitors = glfwGetMonitors(&iMonitorCount);
  WD_GLFW_RETURN_FAILURE_ON_ERROR();
  if(iMonitorCount == 0)
  {
    return WD_FAILURE;
  }

  GLFWmonitor* pPrimaryMonitor = glfwGetPrimaryMonitor();
  WD_GLFW_RETURN_FAILURE_ON_ERROR();
  if(pPrimaryMonitor == nullptr)
  {
    return WD_FAILURE;
  }

  for(int i=0; i < iMonitorCount; ++i)
  {
    wdScreenInfo& screen = out_Screens.ExpandAndGetRef();
    screen.m_sDisplayName = glfwGetMonitorName(pMonitors[i]);
    WD_GLFW_RETURN_FAILURE_ON_ERROR();

    const GLFWvidmode* mode = glfwGetVideoMode(pMonitors[i]);
    WD_GLFW_RETURN_FAILURE_ON_ERROR();
    if(mode == nullptr)
    {
      return WD_FAILURE;
    }
    screen.m_iResolutionX = mode->width;
    screen.m_iResolutionY = mode->height;

    glfwGetMonitorPos(pMonitors[i], &screen.m_iOffsetX, &screen.m_iOffsetY);
    WD_GLFW_RETURN_FAILURE_ON_ERROR();
    
    screen.m_bIsPrimary = pMonitors[i] == pPrimaryMonitor;
  }

  return WD_SUCCESS;
}