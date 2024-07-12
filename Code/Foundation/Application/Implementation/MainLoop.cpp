#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Configuration/Startup.h>

#if defined(LIVEPP_ENABLED)
#  include <LPP_API_x64_CPP.h>
inline bool allow_hotreload = false;
inline lpp::LppDefaultAgent lppAgent;
#endif

nsResult nsRun_Startup(nsApplication* pApplicationInstance)
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT) && defined(LIVEPP_ENABLED)
  // create a synchronized agent, loading the Live++ agent from the given path, e.g. "ThirdParty/LivePP"
  lppAgent = lpp::LppCreateDefaultAgent(nullptr, L"LivePP");
  // bail out in case the agent is not valid
  if (!lpp::LppIsValidDefaultAgent(&lppAgent))
  {
    nsLog::Warning("Failed to create Live++ agent.");
  }
  else
  {
    nsLog::Info("Live++ agent created.");
    allow_hotreload = true;
    lppAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_NONE, nullptr, nullptr);
    // make Live++ handle dynamically loaded modules automatically, enabling them on load, disabling them on unload
    lppAgent.EnableAutomaticHandlingOfDynamicallyLoadedModules(nullptr, nullptr);
  }
#endif
  NS_ASSERT_ALWAYS(pApplicationInstance != nullptr, "nsRun() requires a valid non-null application instance pointer.");
  NS_ASSERT_ALWAYS(nsApplication::s_pApplicationInstance == nullptr, "There can only be one nsApplication.");

  // Set application instance pointer to the supplied instance
  nsApplication::s_pApplicationInstance = pApplicationInstance;

  NS_SUCCEED_OR_RETURN(pApplicationInstance->BeforeCoreSystemsStartup());

  // this will startup all base and core systems
  // 'StartupHighLevelSystems' must not be done before a window is available (if at all)
  // so we don't do that here
  nsStartup::StartupCoreSystems();

  pApplicationInstance->AfterCoreSystemsStartup();
  return NS_SUCCESS;
}

void nsRun_MainLoop(nsApplication* pApplicationInstance)
{
  while (pApplicationInstance->Run() == nsApplication::Execution::Continue)
  {
    // do nothing
  }
}

void nsRun_Shutdown(nsApplication* pApplicationInstance)
{
  // high level systems shutdown
  // may do nothing, if the high level systems were never initialized
  {
    pApplicationInstance->BeforeHighLevelSystemsShutdown();
    nsStartup::ShutdownHighLevelSystems();
    pApplicationInstance->AfterHighLevelSystemsShutdown();
  }

  // core systems shutdown
  {
    pApplicationInstance->BeforeCoreSystemsShutdown();
    nsStartup::ShutdownCoreSystems();
    pApplicationInstance->AfterCoreSystemsShutdown();
  }

  // Flush standard output to make log available.
  fflush(stdout);
  fflush(stderr);

  // Reset application instance so code running after the app will trigger asserts etc. to be cleaned up
  // Destructor is called by entry point function
  nsApplication::s_pApplicationInstance = nullptr;

  #if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT) && defined(LIVEPP_ENABLED)
  // destroy the Live++ agent
  lpp::LppDestroyDefaultAgent(&lppAgent);
#endif

  // memory leak reporting cannot be done here, because the application instance is still alive and may still hold on to memory that needs
  // to be freed first
}

void nsRun(nsApplication* pApplicationInstance)
{
  if (nsRun_Startup(pApplicationInstance).Succeeded())
  {
    nsRun_MainLoop(pApplicationInstance);
  }
  nsRun_Shutdown(pApplicationInstance);
}
