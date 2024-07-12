#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/Screen.h>
#include <GLFW/glfw3.h>

namespace
{
  nsResult nsGlfwError(const char* file, size_t line)
  {
    const char* desc;
    int errorCode = glfwGetError(&desc);
    if (errorCode != GLFW_NO_ERROR)
    {
      nsLog::Error("GLFW error {} ({}): {} - {}", file, line, errorCode, desc);
      return NS_FAILURE;
    }
    return NS_SUCCESS;
  }
} // namespace

#define NS_GLFW_RETURN_FAILURE_ON_ERROR()         \
  do                                              \
  {                                               \
    if (nsGlfwError(__FILE__, __LINE__).Failed()) \
      return NS_FAILURE;                          \
  } while (false)

nsResult nsScreen::EnumerateScreens(nsDynamicArray<nsScreenInfo>& out_Screens)
{
  out_Screens.Clear();

  int iMonitorCount = 0;
  GLFWmonitor** pMonitors = glfwGetMonitors(&iMonitorCount);
  NS_GLFW_RETURN_FAILURE_ON_ERROR();
  if (iMonitorCount == 0)
  {
    return NS_FAILURE;
  }

  GLFWmonitor* pPrimaryMonitor = glfwGetPrimaryMonitor();
  NS_GLFW_RETURN_FAILURE_ON_ERROR();
  if (pPrimaryMonitor == nullptr)
  {
    return NS_FAILURE;
  }

  for (int i = 0; i < iMonitorCount; ++i)
  {
    nsScreenInfo& screen = out_Screens.ExpandAndGetRef();
    screen.m_sDisplayName = glfwGetMonitorName(pMonitors[i]);
    NS_GLFW_RETURN_FAILURE_ON_ERROR();

    const GLFWvidmode* mode = glfwGetVideoMode(pMonitors[i]);
    NS_GLFW_RETURN_FAILURE_ON_ERROR();
    if (mode == nullptr)
    {
      return NS_FAILURE;
    }
    screen.m_iResolutionX = mode->width;
    screen.m_iResolutionY = mode->height;

    glfwGetMonitorPos(pMonitors[i], &screen.m_iOffsetX, &screen.m_iOffsetY);
    NS_GLFW_RETURN_FAILURE_ON_ERROR();

    screen.m_bIsPrimary = pMonitors[i] == pPrimaryMonitor;
  }

  return NS_SUCCESS;
}
