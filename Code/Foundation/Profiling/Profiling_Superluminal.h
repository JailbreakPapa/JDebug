#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/HashedString.h>

#if NS_ENABLED(NS_USE_PROFILING) || defined(NS_DOCS)
#  if defined(SUPERLUMINALAPI) && NS_DISABLED(NS_PLATFORM_PLAYSTATION_5)
#    include <PerformanceAPI.h>
#    if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#      include <Foundation/Basics/Platform/Win/IncludeWindows.h>

inline PerformanceAPI_Functions* Perf;
/// TODO: Abstract this into a overridable function that could be called anywhere.
/// NOTE: szPerformanceName is not used on Windows.
NS_ALWAYS_INLINE void LoadPerformanceLibrary(const char* szPerformanceName = "")
{
  // Superluminal requires windows, so we will be using Windows specific API's.
  HMODULE module = LoadLibraryW(L"PerformanceAPI.dll");
  if (module == NULL)
  {
    nsLog::Error("Failed to Load Superluminal Performance DLL. (is the dll misplaced?)");
    return;
  }
  else
  {
    PerformanceAPI_GetAPI_Func getAPI = (PerformanceAPI_GetAPI_Func)((void*)GetProcAddress(module, "PerformanceAPI_GetAPI"));
    if (getAPI == NULL || getAPI(PERFORMANCEAPI_VERSION, Perf))
    {
      nsLog::Error("Failed to Get Superluminal Performance API.");
      FreeLibrary(module);
      return;
    }
  }
}

NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const char* szString)
{
  return nsStringUtils::GetStringElementCount(szString);
}

NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(nsStringView szString)
{
  return szString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const nsString& szString)
{
  return szString.GetElementCount();
}
NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const nsStringBuilder& szString)
{
  return szString.GetElementCount();
}
NS_ALWAYS_INLINE nsUInt32 ___performanceGetStringLength(const nsHashedString& szString)
{
  return szString.GetView().GetElementCount();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsString& szString)
{
  return szString.GetData();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsStringBuilder& szString)
{
  return szString.GetData();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsHashedString& szString)
{
  return szString.GetData();
}
NS_ALWAYS_INLINE const char* __convertnsStringToConstChar(const nsStringView& szString)
{
  nsStringBuilder temp_str;
  szString.GetData(temp_str);
  return temp_str.GetData();
}
#      define PERFORMANCE_PROFILE_SCOPE_DYNAMIC(szScopeName) \
        PERFORMANCEAPI_INSTRUMENT(szScopeName)

/// \brief Profiles the current scope using the given name.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingScope
/// \sa NS_PROFILE_LIST_SCOPE
#      define NS_PROFILE_SCOPE(szScopeName) \
        PERFORMANCE_PROFILE_SCOPE_DYNAMIC(__convertnsStringToConstChar((const nsString&)szScopeName))

/// \brief Same as NS_PROFILE_SCOPE but if the scope takes longer than 'Timeout', the nsProfilingSystem's timeout callback is executed.
///
/// This can be used to log an error or save a callstack, etc. when a scope exceeds an expected amount of time.
///
/// \sa nsProfilingSystem::SetScopeTimeoutCallback()
#      define NS_PROFILE_SCOPE_WITH_TIMEOUT(szScopeName, Timeout) \
        PERFORMANCE_PROFILE_SCOPE_DYNAMIC(__convertnsStringToConstChar((const nsString&)szScopeName))
/// \brief Profiles the current scope using the given name as the overall list scope name and the section name for the first section in the list.
///
/// Use NS_PROFILE_LIST_NEXT_SECTION to start a new section in the list scope.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_NEXT_SECTION
#      define NS_PROFILE_LIST_SCOPE(szListName, szFirstSectionName)                                   \
        PERFORMANCE_PROFILE_SCOPE_DYNAMIC(__convertnsStringToConstChar((const nsString&)szScopeName)) \
        PerformanceAPI::BeginEvent(__convertnsStringToConstChar((const nsString&)szFirstSectionName))

/// \brief Starts a new section in a NS_PROFILE_LIST_SCOPE
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_SCOPE
#      define NS_PROFILE_LIST_NEXT_SECTION(szNextSectionName) \
        PerformanceAPI::BeginEvent(szNextSectionName)

#      define NS_PROFILER_END_FRAME \
        PerformanceAPI::EndEvent()

#    endif
#  elif NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)
// NOTE: Playstation Implementation was removed due to NDA Content.
// TODO: Clean this up.
/// \brief Profiles the current scope using the given name.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingScope
/// \sa NS_PROFILE_LIST_SCOPE
#    define NS_PROFILE_SCOPE(szScopeName) 

/// \brief Same as NS_PROFILE_SCOPE but if the scope takes longer than 'Timeout', the nsProfilingSystem's timeout callback is executed.
///
/// This can be used to log an error or save a callstack, etc. when a scope exceeds an expected amount of time.
///
/// \sa nsProfilingSystem::SetScopeTimeoutCallback()
#    define NS_PROFILE_SCOPE_WITH_TIMEOUT(szScopeName, Timeout) 

/// \brief Profiles the current scope using the given name as the overall list scope name and the section name for the first section in the list.
///
/// Use NS_PROFILE_LIST_NEXT_SECTION to start a new section in the list scope.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_NEXT_SECTION
#    define NS_PROFILE_LIST_SCOPE(szListName, szFirstSectionName) 

/// \brief Starts a new section in a NS_PROFILE_LIST_SCOPE
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_SCOPE
#    define NS_PROFILE_LIST_NEXT_SECTION(szNextSectionName) 

#    define NS_PROFILER_END_FRAME
#  endif
#endif
