#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Math/Size.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/UniquePtr.h>

class nsOpenDdlWriter;
class nsOpenDdlReader;
class nsOpenDdlReaderElement;

// Include the proper Input implementation to use
#if NS_ENABLED(NS_SUPPORTS_GLFW)
#  include <Core/System/Implementation/glfw/InputDevice_glfw.h>
#elif NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#  include <Core/System/Implementation/Win/InputDevice_win32.h>
#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  include <Core/System/Implementation/uwp/InputDevice_uwp.h>
#elif NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Core/System/Implementation/android/InputDevice_android.h>
#else
#  include <Core/System/Implementation/null/InputDevice_null.h>
#endif

// Currently the following scenarios are possible
// - Windows native implementation, using HWND
// - GLFW on windows, using GLFWWindow* internally and HWND to pass windows around
// - GLFW / XCB on linux. Runtime uses GLFWWindow*. Editor uses xcb-window. Tagged union is passed around as window handle.

#if NS_ENABLED(NS_SUPPORTS_GLFW)

extern "C"
{
  typedef struct GLFWwindow GLFWwindow;
}

#  if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#    include <Foundation/Basics/Platform/Win/MinWindows.h>
using nsWindowHandle = nsMinWindows::HWND;
using nsWindowInternalHandle = GLFWwindow*;
#    define INVALID_WINDOW_HANDLE_VALUE (nsWindowHandle)(0)
#    define INVALID_INTERNAL_WINDOW_HANDLE_VALUE nullptr
#  elif NS_ENABLED(NS_PLATFORM_LINUX)

extern "C"
{
  typedef struct xcb_connection_t xcb_connection_t;
}

struct nsXcbWindowHandle
{
  xcb_connection_t* m_pConnection;
  nsUInt32 m_Window;
};

struct nsWindowHandle
{
  enum class Type
  {
    Invalid = 0,
    GLFW = 1, // Used by the runtime
    XCB = 2   // Used by the editor
  };

  Type type;
  union
  {
    GLFWwindow* glfwWindow;
    nsXcbWindowHandle xcbWindow;
  };

  bool operator==(nsWindowHandle& rhs)
  {
    if (type != rhs.type)
      return false;

    if (type == Type::GLFW)
    {
      return glfwWindow == rhs.glfwWindow;
    }
    else
    {
      // We don't compare the connection because we only want to know if we reference the same window.
      return xcbWindow.m_Window == rhs.xcbWindow.m_Window;
    }
  }
};

using nsWindowInternalHandle = nsWindowHandle;
#    define INVALID_WINDOW_HANDLE_VALUE \
      nsWindowHandle {}
#  else
using nsWindowHandle = GLFWwindow*;
using nsWindowInternalHandle = GLFWwindow*;
#    define INVALID_WINDOW_HANDLE_VALUE (GLFWwindow*)(0)
#  endif

#elif NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Basics/Platform/Win/MinWindows.h>
using nsWindowHandle = nsMinWindows::HWND;
using nsWindowInternalHandle = nsWindowHandle;
#  define INVALID_WINDOW_HANDLE_VALUE (nsWindowHandle)(0)

#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

using nsWindowHandle = IUnknown*;
using nsWindowInternalHandle = nsWindowHandle;
#  define INVALID_WINDOW_HANDLE_VALUE nullptr

#elif NS_ENABLED(NS_PLATFORM_ANDROID)

struct ANativeWindow;
using nsWindowHandle = ANativeWindow*;
using nsWindowInternalHandle = nsWindowHandle;
#  define INVALID_WINDOW_HANDLE_VALUE nullptr

#else

using nsWindowHandle = void*;
using nsWindowInternalHandle = nsWindowHandle;
#  define INVALID_WINDOW_HANDLE_VALUE nullptr

#endif

#ifndef INVALID_INTERNAL_WINDOW_HANDLE_VALUE
#  define INVALID_INTERNAL_WINDOW_HANDLE_VALUE INVALID_WINDOW_HANDLE_VALUE
#endif

/// \brief Base class of all window classes that have a client area and a native window handle.
class NS_CORE_DLL nsWindowBase
{
public:
  virtual ~nsWindowBase() = default;

  virtual nsSizeU32 GetClientAreaSize() const = 0;
  virtual nsWindowHandle GetNativeWindowHandle() const = 0;

  /// \brief Whether the window is a fullscreen window
  /// or should be one - some platforms may enforce this via the GALSwapchain)
  ///
  /// If bOnlyProperFullscreenMode, the caller accepts borderless windows that cover the entire screen as "fullscreen".
  virtual bool IsFullscreenWindow(bool bOnlyProperFullscreenMode = false) const = 0;

  virtual void ProcessWindowMessages() = 0;

  virtual void AddReference() = 0;
  virtual void RemoveReference() = 0;
};

/// \brief Determines how the position and resolution for a window are picked
struct NS_CORE_DLL nsWindowMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    WindowFixedResolution,                ///< The resolution and size are what the user picked and will not be changed. The window will not be resizable.
    WindowResizable,                      ///< The resolution and size are what the user picked and will not be changed. Allows window resizing by the user.
    FullscreenBorderlessNativeResolution, ///< A borderless window, the position and resolution are taken from the monitor on which the
                                          ///< window shall appear.
    FullscreenFixedResolution,            ///< A fullscreen window using the user provided resolution. Tries to change the monitor resolution
                                          ///< accordingly.

    Default = WindowFixedResolution
  };

  /// \brief Returns whether the window covers an entire monitor. This includes borderless windows and proper fullscreen modes.
  static constexpr bool IsFullscreen(Enum e) { return e == FullscreenBorderlessNativeResolution || e == FullscreenFixedResolution; }
};

/// \brief Parameters for creating a window, such as position and resolution
struct NS_CORE_DLL nsWindowCreationDesc
{
  /// \brief Adjusts the position and size members, depending on the current value of m_WindowMode and m_iMonitor.
  ///
  /// For windowed mode, this does nothing.
  /// For fullscreen modes, the window position is taken from the given monitor.
  /// For borderless fullscreen mode, the window resolution is also taken from the given monitor.
  ///
  /// This function can only fail if nsScreen::EnumerateScreens fails to enumerate the available screens.
  nsResult AdjustWindowSizeAndPosition();

  /// Serializes the configuration to DDL.
  void SaveToDDL(nsOpenDdlWriter& ref_writer);

  /// Serializes the configuration to DDL.
  nsResult SaveToDDL(nsStringView sFile);

  /// Deserializes the configuration from DDL.
  void LoadFromDDL(const nsOpenDdlReaderElement* pParentElement);

  /// Deserializes the configuration from DDL.
  nsResult LoadFromDDL(nsStringView sFile);


  /// The window title to be displayed.
  nsString m_Title = "nsEngine";

  /// Defines how the window size is determined.
  nsEnum<nsWindowMode> m_WindowMode;

  /// The monitor index is as given by nsScreen::EnumerateScreens.
  /// -1 as the index means to pick the primary monitor.
  nsInt8 m_iMonitor = -1;

  /// The virtual position of the window. Determines on which monitor the window ends up.
  nsVec2I32 m_Position = nsVec2I32(0x80000000, 0x80000000); // Magic number on windows that positions the window at a 'good default position'

  /// The pixel resolution of the window.
  nsSizeU32 m_Resolution = nsSizeU32(1280, 720);

  /// The number of the window. This is mostly used for setting up the input system, which then reports
  /// different mouse positions for each window.
  nsUInt8 m_uiWindowNumber = 0;

  /// Whether the mouse cursor should be trapped inside the window or not.
  /// \see nsStandardInputDevice::SetClipMouseCursor
  bool m_bClipMouseCursor = true;

  /// Whether the mouse cursor should be visible or not.
  /// \see nsStandardInputDevice::SetShowMouseCursor
  bool m_bShowMouseCursor = false;

  /// Whether the window is activated and focussed on Initialize()
  bool m_bSetForegroundOnInit = true;

  /// Whether the window is centered on the display.
  bool m_bCenterWindowOnDisplay = true;
};

/// \brief A simple abstraction for platform specific window creation.
///
/// Will handle basic message looping. Notable events can be listened to by overriding the corresponding callbacks.
/// You should call ProcessWindowMessages every frame to keep the window responsive.
/// Input messages will not be forwarded automatically. You can do so by overriding the OnWindowMessage function.
class NS_CORE_DLL nsWindow : public nsWindowBase
{
public:
  /// \brief Creates empty window instance with standard settings
  ///
  /// You need to call Initialize to actually create a window.
  /// \see nsWindow::Initialize
  nsWindow();

  /// \brief Destroys the window if not already done.
  virtual ~nsWindow();

  /// \brief Returns the currently active description struct.
  inline const nsWindowCreationDesc& GetCreationDescription() const { return m_CreationDescription; }

  /// \brief Returns the size of the client area / ie. the window resolution.
  virtual nsSizeU32 GetClientAreaSize() const override { return m_CreationDescription.m_Resolution; }

  /// \brief Returns the platform specific window handle.
  virtual nsWindowHandle GetNativeWindowHandle() const override;

  /// \brief Returns whether the window covers an entire monitor.
  ///
  /// If bOnlyProperFullscreenMode == false, this includes borderless windows.
  virtual bool IsFullscreenWindow(bool bOnlyProperFullscreenMode = false) const override
  {
    if (bOnlyProperFullscreenMode)
      return m_CreationDescription.m_WindowMode == nsWindowMode::FullscreenFixedResolution;

    return nsWindowMode::IsFullscreen(m_CreationDescription.m_WindowMode);
  }

  virtual void AddReference() override { m_iReferenceCount.Increment(); }
  virtual void RemoveReference() override { m_iReferenceCount.Decrement(); }


  /// \brief Runs the platform specific message pump.
  ///
  /// You should call ProcessWindowMessages every frame to keep the window responsive.
  virtual void ProcessWindowMessages() override;

  /// \brief Creates a new platform specific window with the current settings
  ///
  /// Will automatically call nsWindow::Destroy if window is already initialized.
  ///
  /// \see nsWindow::Destroy, nsWindow::Initialize
  nsResult Initialize();

  /// \brief Creates a new platform specific window with the given settings.
  ///
  /// Will automatically call nsWindow::Destroy if window is already initialized.
  ///
  /// \param creationDescription
  ///   Struct with various settings for window creation. Will be saved internally for later lookup.
  ///
  /// \see nsWindow::Destroy, nsWindow::Initialize
  nsResult Initialize(const nsWindowCreationDesc& creationDescription)
  {
    m_CreationDescription = creationDescription;
    return Initialize();
  }

  /// \brief Gets if the window is up and running.
  inline bool IsInitialized() const { return m_bInitialized; }

  /// \brief Destroys the window.
  nsResult Destroy();

  /// \brief Tries to resize the window.
  /// Override OnResize to get the actual new window size.
  nsResult Resize(const nsSizeU32& newWindowSize);

  /// \brief Called on window resize messages.
  ///
  /// \param newWindowSize
  ///   New window size in pixel.
  /// \see OnWindowMessage
  virtual void OnResize(const nsSizeU32& newWindowSize);

  /// \brief Called when the window position is changed. Not possible on all OSes.
  virtual void OnWindowMove(const nsInt32 iNewPosX, const nsInt32 iNewPosY) {}

  /// \brief Called when the window gets focus or loses focus.
  virtual void OnFocus(bool bHasFocus) {}

  /// \brief Called when the close button of the window is clicked. Does nothing by default.
  virtual void OnClickClose() {}


#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  /// \brief Called on any window message.
  ///
  /// You can use this function for example to dispatch the message to another system.
  ///
  /// \remarks
  ///   Will be called <i>after</i> the On[...] callbacks!
  ///
  /// \see OnResizeMessage
  virtual void OnWindowMessage(nsMinWindows::HWND hWnd, nsMinWindows::UINT msg, nsMinWindows::WPARAM wparam, nsMinWindows::LPARAM lparam);

#elif NS_ENABLED(NS_PLATFORM_OSX)

#elif NS_ENABLED(NS_PLATFORM_LINUX)

#elif NS_ENABLED(NS_PLATFORM_ANDROID)

#else
#  error "Missing code for nsWindow on this platform!"
#endif

  /// \brief Returns the input device that is attached to this window and typically provides mouse / keyboard input.
  nsStandardInputDevice* GetInputDevice() const { return m_pInputDevice.Borrow(); }

  /// \brief Returns a number that can be used as a window number in nsWindowCreationDesc
  ///
  /// This number just increments every time an nsWindow is created. It starts at zero.
  static nsUInt8 GetNextUnusedWindowNumber();

protected:
  /// Description at creation time. nsWindow will not update this in any method other than Initialize.
  /// \remarks That means that messages like Resize will also have no effect on this variable.
  nsWindowCreationDesc m_CreationDescription;

private:
  bool m_bInitialized = false;

  nsUniquePtr<nsStandardInputDevice> m_pInputDevice;

  mutable nsWindowInternalHandle m_hWindowHandle = nsWindowInternalHandle();

#if NS_ENABLED(NS_SUPPORTS_GLFW)
  static void SizeCallback(GLFWwindow* window, int width, int height);
  static void PositionCallback(GLFWwindow* window, int xpos, int ypos);
  static void CloseCallback(GLFWwindow* window);
  static void FocusCallback(GLFWwindow* window, int focused);
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void CharacterCallback(GLFWwindow* window, unsigned int codepoint);
  static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
  static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
#endif

  /// increased every time an nsWindow is created, to be able to get a free window index easily
  static nsUInt8 s_uiNextUnusedWindowNumber;
  nsAtomicInteger32 m_iReferenceCount = 0;
};
