#pragma once

/// \file

#include <Foundation/Basics.h>

#include <Foundation/Application/Implementation/ApplicationEntryPoint.h>
#include <Foundation/Utilities/CommandLineUtils.h>

#include <Foundation/Profiling/Profiling.h>

class nsApplication;

/// \brief Platform independent run function for main loop based systems (e.g. Win32, ..)
///
/// This is automatically called by NS_APPLICATION_ENTRY_POINT() and NS_CONSOLEAPP_ENTRY_POINT().
///
/// nsRun simply calls nsRun_Startup(), nsRun_MainLoop() and nsRun_Shutdown().
NS_FOUNDATION_DLL void nsRun(nsApplication* pApplicationInstance);

/// \brief [internal] Called by nsRun()
NS_FOUNDATION_DLL nsResult nsRun_Startup(nsApplication* pApplicationInstance);
/// \brief [internal] Called by nsRun()
NS_FOUNDATION_DLL void nsRun_MainLoop(nsApplication* pApplicationInstance);
/// \brief [internal] Called by nsRun()
NS_FOUNDATION_DLL void nsRun_Shutdown(nsApplication* pApplicationInstance);

/// \brief Base class to be used by applications based on nsEngine.
///
/// The platform abstraction layer will ensure that the correct functions are called independent of the basic main loop structure
/// (traditional or event-based). Derive an application specific class from nsApplication and implement at least the abstract Run()
/// function. Additional virtual functions allow to hook into specific events to run application specific code at the correct times.
///
/// Finally pass the name of your derived class to one of the macros NS_APPLICATION_ENTRY_POINT() or NS_CONSOLEAPP_ENTRY_POINT().
/// Those are used to abstract away the platform specific code to run an application.
///
/// A simple example how to get started is as follows:
///
/// \code{.cpp}
///   class nsSampleApp : public nsApplication
///   {
///   public:
///
///     virtual void AfterCoreSystemsStartup() override
///     {
///       // Setup Filesystem, Logging, etc.
///     }
///
///     virtual void BeforeCoreSystemsShutdown() override
///     {
///       // Close log file, etc.
///     }
///
///     virtual nsApplication::Execution Run() override
///     {
///       // Either run a one-time application (e.g. console script) and return nsApplication::Quit
///       // Or run one update (frame) of your game loop and return nsApplication::Continue
///
///       return nsApplication::Quit;
///     }
///   };
///
///   NS_APPLICATION_ENTRY_POINT(nsSampleApp);
/// \endcode
class NS_FOUNDATION_DLL nsApplication
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsApplication);

public:
  /// \brief Defines the possible return values for the nsApplication::Run() function.
  enum class Execution
  {
    Continue, ///< The 'Run' function should return this to keep the application running
    Quit,     ///< The 'Run' function should return this to quit the application
  };

  /// \brief Constructor.
  nsApplication(nsStringView sAppName);

  /// \brief Virtual destructor.
  virtual ~nsApplication();

  /// \brief Changes the application name
  void SetApplicationName(nsStringView sAppName);

  /// \brief Returns the application name
  const nsString& GetApplicationName() const { return m_sAppName; }

  /// \brief This function is called before any kind of engine initialization is done.
  ///
  /// Override this function to be able to configure subsystems, before they are initialized.
  /// After this function returns, nsStartup::StartupCoreSystems() is automatically called.
  /// If you need to set up custom allocators, this is the place to do this.
  virtual nsResult BeforeCoreSystemsStartup();

  /// \brief This function is called after basic engine initialization has been done.
  ///
  /// nsApplication will automatically call nsStartup::StartupCoreSystems() to initialize the application.
  /// This function can be overridden to do additional application specific initialization.
  /// To startup entire subsystems, you should however use the features provided by nsStartup and nsSubSystem.
  virtual void AfterCoreSystemsStartup() {}

  /// \brief This function is called after the application main loop has run for the last time, before engine deinitialization.
  ///
  /// After this function call, nsApplication executes nsStartup::ShutdownHighLevelSystems().
  ///
  /// \note nsApplication does NOT call nsStartup::StartupHighLevelSystems() as it may be a window-less application.
  /// This is left to nsGameApplicationBase to do. However, it does make sure to shut down the high-level systems,
  /// in case they were started.
  virtual void BeforeHighLevelSystemsShutdown() {}

  /// \brief Called after nsStartup::ShutdownHighLevelSystems() has been executed.
  virtual void AfterHighLevelSystemsShutdown() {}

  /// \brief This function is called after the application main loop has run for the last time, before engine deinitialization.
  ///
  /// Override this function to do application specific deinitialization that still requires a running engine.
  /// After this function returns nsStartup::ShutdownCoreSystems() is called and thus everything, including allocators, is shut down.
  /// To shut down entire subsystems, you should, however, use the features provided by nsStartup and nsSubSystem.
  virtual void BeforeCoreSystemsShutdown() {}

  /// \brief This function is called after nsStartup::ShutdownCoreSystems() has been called.
  ///
  /// It is unlikely that there is any kind of deinitialization left, that can still be run at this point.
  virtual void AfterCoreSystemsShutdown() {}

  /// \brief This function is called when an application is moved to the background.
  ///
  /// On Windows that might simply mean that the main window lost the focus.
  /// On other devices this might mean that the application is not visible at all anymore and
  /// might even get shut down later. Override this function to be able to put the application
  /// into a proper sleep mode.
  virtual void BeforeEnterBackground() {}

  /// \brief This function is called whenever an application is resumed from background mode.
  ///
  /// On Windows that might simply mean that the main window received focus again.
  /// On other devices this might mean that the application was suspended and is now active again.
  /// Override this function to reload the apps state or other resources, etc.
  virtual void BeforeEnterForeground() {}

  /// \brief Main run function which is called periodically. This function must be overridden.
  ///
  /// Return Execution::Quit when the application should quit. You may set a return code via SetReturnCode() beforehand.
  virtual Execution Run() = 0;

  /// \brief Sets the value that the application will return to the OS.
  /// You can call this function at any point during execution to update the return value of the application.
  /// Default is zero.
  inline void SetReturnCode(nsInt32 iReturnCode) { m_iReturnCode = iReturnCode; }

  /// \brief Returns the currently set value that the application will return to the OS.
  inline nsInt32 GetReturnCode() const { return m_iReturnCode; }

  /// \brief If the return code is not zero, this function might be called to get a string to print the error code in human readable form.
  virtual const char* TranslateReturnCode() const { return ""; }

  /// \brief Will set the command line arguments that were passed to the app by the OS.
  /// This is automatically called by NS_APPLICATION_ENTRY_POINT() and NS_CONSOLEAPP_ENTRY_POINT().
  void SetCommandLineArguments(nsUInt32 uiArgumentCount, const char** pArguments);

  /// \brief Returns the one instance of nsApplication that is available.
  static nsApplication* GetApplicationInstance() { return s_pApplicationInstance; }

  /// \brief Returns the number of command line arguments that were passed to the application.
  ///
  /// Note that the very first command line argument is typically the path to the application itself.
  nsUInt32 GetArgumentCount() const { return m_uiArgumentCount; }

  /// \brief Returns one of the command line arguments that was passed to the application.
  const char* GetArgument(nsUInt32 uiArgument) const;

  /// \brief Returns the complete array of command line arguments that were passed to the application.
  const char** GetArgumentsArray() const { return m_pArguments; }

  void EnableMemoryLeakReporting(bool bEnable) { m_bReportMemoryLeaks = bEnable; }

  bool IsMemoryLeakReportingEnabled() const { return m_bReportMemoryLeaks; }

  /// \brief Calling this function requests that the application quits after the current invocation of Run() finishes.
  ///
  /// Sets the m_bWasQuitRequested to true as an indicator for derived application objects to engage shutdown procedures.
  /// Can be overridden to implement custom behavior. There is no other logic associated with this
  /// function and flag so the respective derived application class has to implement logic to perform the actual
  /// quit when this function is called or m_bWasQuitRequested is set to true.
  virtual void RequestQuit();

  /// \brief Returns whether RequestQuit() was called.
  NS_ALWAYS_INLINE bool WasQuitRequested() const { return m_bWasQuitRequested; }

protected:
  bool m_bWasQuitRequested = false;

private:
  nsInt32 m_iReturnCode = 0;

  nsUInt32 m_uiArgumentCount = 0;

  const char** m_pArguments = nullptr;

  bool m_bReportMemoryLeaks = true;

  nsString m_sAppName;

  static nsApplication* s_pApplicationInstance;

  friend NS_FOUNDATION_DLL_FRIEND void nsRun(nsApplication* pApplicationInstance);
  friend NS_FOUNDATION_DLL_FRIEND nsResult nsRun_Startup(nsApplication* pApplicationInstance);
  friend NS_FOUNDATION_DLL_FRIEND void nsRun_MainLoop(nsApplication* pApplicationInstance);
  friend NS_FOUNDATION_DLL_FRIEND void nsRun_Shutdown(nsApplication* pApplicationInstance);
};
