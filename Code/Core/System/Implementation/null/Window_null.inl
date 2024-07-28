#include <Core/System/Window.h>

nsResult nsWindow::Initialize()
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return NS_FAILURE;
}

nsResult nsWindow::Destroy()
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return NS_FAILURE;
}

nsResult nsWindow::Resize(const nsSizeU32& newWindowSize)
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return NS_FAILURE;
}

void nsWindow::ProcessWindowMessages()
{
  NS_ASSERT_NOT_IMPLEMENTED;
}

void nsWindow::OnResize(const nsSizeU32& newWindowSize)
{
  NS_ASSERT_NOT_IMPLEMENTED;
}

nsWindowHandle nsWindow::GetNativeWindowHandle() const
{
  return m_hWindowHandle;
}
