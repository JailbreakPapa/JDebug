#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/StaticRingBuffer.h>
#include <Foundation/System/Process.h>
#include <Foundation/Time/Time.h>

class wdStreamWriter;
class wdThread;

/// \brief This class encapsulates a profiling scope.
///
/// The constructor creates a new scope in the profiling system and the destructor pops the scope.
/// You shouldn't need to use this directly, just use the macro WD_PROFILE_SCOPE provided below.
class WD_FOUNDATION_DLL wdProfilingScope
{
public:
  wdProfilingScope(wdStringView sName, const char* szFunctionName, wdTime timeout);
  ~wdProfilingScope();

protected:
  wdStringView m_sName;
  const char* m_szFunction;
  wdTime m_BeginTime;
  wdTime m_Timeout;
};

/// \brief This class implements a profiling scope similar to wdProfilingScope, but with additional sub-scopes which can be added easily without
/// introducing actual C++ scopes.
///
/// The constructor pushes one surrounding scope on the stack and then a nested scope as the first section.
/// The function StartNextSection() will end the nested scope and start a new inner scope.
/// This allows to end one scope and start a new one, without having to add actual C++ scopes for starting/stopping profiling scopes.
///
/// You shouldn't need to use this directly, just use the macro WD_PROFILE_LIST_SCOPE provided below.
class wdProfilingListScope
{
public:
  WD_FOUNDATION_DLL wdProfilingListScope(wdStringView sListName, wdStringView sFirstSectionName, const char* szFunctionName);
  WD_FOUNDATION_DLL ~wdProfilingListScope();

  WD_FOUNDATION_DLL static void StartNextSection(wdStringView sNextSectionName);

protected:
  static thread_local wdProfilingListScope* s_pCurrentList;

  wdProfilingListScope* m_pPreviousList;

  wdStringView m_sListName;
  const char* m_szListFunction;
  wdTime m_ListBeginTime;

  wdStringView m_sCurSectionName;
  wdTime m_CurSectionBeginTime;
};

/// \brief Helper functionality of the profiling system.
class WD_FOUNDATION_DLL wdProfilingSystem
{
public:
  struct ThreadInfo
  {
    wdUInt64 m_uiThreadId;
    wdString m_sName;
  };

  struct CPUScope
  {
    WD_DECLARE_POD_TYPE();

    static constexpr wdUInt32 NAME_SIZE = 40;

    const char* m_szFunctionName;
    wdTime m_BeginTime;
    wdTime m_EndTime;
    char m_szName[NAME_SIZE];
  };

  struct CPUScopesBufferFlat
  {
    wdDynamicArray<CPUScope> m_Data;
    wdUInt64 m_uiThreadId = 0;
  };

  /// \brief Helper struct to hold GPU profiling data.
  struct GPUScope
  {
    WD_DECLARE_POD_TYPE();

    static constexpr wdUInt32 NAME_SIZE = 48;

    wdTime m_BeginTime;
    wdTime m_EndTime;
    char m_szName[NAME_SIZE];
  };

  struct WD_FOUNDATION_DLL ProfilingData
  {
    wdUInt32 m_uiFramesThreadID = 0;
    wdUInt32 m_uiProcessSortIndex = 0;
    wdOsProcessID m_uiProcessID = 0;

    wdHybridArray<ThreadInfo, 16> m_ThreadInfos;

    wdDynamicArray<CPUScopesBufferFlat> m_AllEventBuffers;

    wdUInt64 m_uiFrameCount = 0;
    wdDynamicArray<wdTime> m_FrameStartTimes;

    wdDynamicArray<wdDynamicArray<GPUScope>> m_GPUScopes;

    /// \brief Writes profiling data as JSON to the output stream.
    wdResult Write(wdStreamWriter& ref_outputStream) const;

    void Clear();

    /// \brief Concatenates all given ProfilingData instances into one merge struct
    static void Merge(ProfilingData& out_merged, wdArrayPtr<const ProfilingData*> inputs);
  };

public:
  static void Clear();

  static void Capture(wdProfilingSystem::ProfilingData& out_capture, bool bClearAfterCapture = false);

  /// \brief Scopes are discarded if their duration is shorter than the specified threshold. Default is 0.1ms.
  static void SetDiscardThreshold(wdTime threshold);

  using ScopeTimeoutDelegate = wdDelegate<void(wdStringView sName, wdStringView sFunctionName, wdTime duration)>;

  /// \brief Sets a callback that is triggered when a profiling scope takes longer than desired.
  static void SetScopeTimeoutCallback(ScopeTimeoutDelegate callback);

  /// \brief Should be called once per frame to capture the timestamp of the new frame.
  static void StartNewFrame();

  /// \brief Adds a new scoped event for the calling thread in the profiling system
  static void AddCPUScope(wdStringView sName, const char* szFunctionName, wdTime beginTime, wdTime endTime, wdTime scopeTimeout);

  /// \brief Get current frame counter
  static wdUInt64 GetFrameCount();

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, ProfilingSystem);
  friend wdUInt32 RunThread(wdThread* pThread);

  static void Initialize();
  /// \brief Removes profiling data of dead threads.
  static void Reset();

  /// \brief Sets the name of the current thread.
  static void SetThreadName(wdStringView sThreadName);
  /// \brief Removes the current thread from the profiling system.
  ///  Needs to be called before the thread exits to be able to release profiling memory of dead threads on Reset.
  static void RemoveThread();

public:
  /// \brief Initialized internal data structures for GPU profiling data. Needs to be called before adding any data.
  static void InitializeGPUData(wdUInt32 uiGpuCount = 1);

  /// \brief Adds a GPU profiling scope in the internal event ringbuffer.
  static void AddGPUScope(wdStringView sName, wdTime beginTime, wdTime endTime, wdUInt32 uiGpuIndex = 0);
};

#if WD_ENABLED(WD_USE_PROFILING) || defined(WD_DOCS)

#  ifdef USE_OPTICK
#include <optick/optick.h>
      /// CPU Profilings

      /// \brief Basic scoped performance counter. Use this counter 99% of the time. It automatically extracts the name
      /// of the current function. Users can also pass an optional name for this macro to override the name
      /// - WD_OPTICK_PROFILE_EVENT("szScopeName");. Useful for marking multiple scopes within one function.
      #  define WD_OPTICK_PROFILE_EVENT(...) OPTICK_EVENT(__VA_ARGS__);
      #else
        #  define WD_OPTICK_PROFILE_EVENT(...)
    #endif
#endif
/// \brief Profiles the current scope using the given name.
///
/// It is allowed to nest WD_PROFILE_SCOPE, also with WD_PROFILE_LIST_SCOPE. However WD_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa wdProfilingScope
/// \sa WD_PROFILE_LIST_SCOPE
#define WD_PROFILE_SCOPE(szScopeName)   \
  WD_OPTICK_PROFILE_EVENT(szScopeName); \
  wdProfilingScope WD_CONCAT(_wdProfilingScope, WD_SOURCE_LINE)(szScopeName, WD_SOURCE_FUNCTION, wdTime::Zero()) \




/// \brief Same as WD_PROFILE_SCOPE but if the scope takes longer than 'Timeout', the wdProfilingSystem's timeout callback is executed.
///
/// This can be used to log an error or save a callstack, etc. when a scope exceeds an expected amount of time.
/// 
/// \sa wdProfilingSystem::SetScopeTimeoutCallback()
#  define WD_PROFILE_SCOPE_WITH_TIMEOUT(szScopeName, Timeout)  wdProfilingScope WD_CONCAT(_wdProfilingScope, WD_SOURCE_LINE)(szScopeName, WD_SOURCE_FUNCTION, Timeout)

/// \brief Profiles the current scope using the given name as the overall list scope name and the section name for the first section in the list.
///
/// Use WD_PROFILE_LIST_NEXT_SECTION to start a new section in the list scope.
///
/// It is allowed to nest WD_PROFILE_SCOPE, also with WD_PROFILE_LIST_SCOPE. However WD_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa wdProfilingListScope
/// \sa WD_PROFILE_LIST_NEXT_SECTION
#  define WD_PROFILE_LIST_SCOPE(szListName, szFirstSectionName) \
    wdProfilingListScope WD_CONCAT(_wdProfilingScope, WD_SOURCE_LINE)(szListName, szFirstSectionName, WD_SOURCE_FUNCTION)

/// \brief Starts a new section in a WD_PROFILE_LIST_SCOPE
///
/// \sa wdProfilingListScope
/// \sa WD_PROFILE_LIST_SCOPE
#  define WD_PROFILE_LIST_NEXT_SECTION(szNextSectionName)  wdProfilingListScope::StartNextSection(szNextSectionName)

#if WD_DISABLED(WD_USE_PROFILING)

#  define WD_PROFILE_SCOPE(Name) /*empty*/

#  define WD_PROFILE_SCOPE_WITH_TIMEOUT(szScopeName, Timeout) /*empty*/

#  define WD_PROFILE_LIST_SCOPE(szListName, szFirstSectionName) /*empty*/

#  define WD_PROFILE_LIST_NEXT_SECTION(szNextSectionName) /*empty*/

#endif
