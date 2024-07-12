#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/StaticRingBuffer.h>
#include <Foundation/System/Process.h>
#include <Foundation/Time/Time.h>

class nsStreamWriter;
class nsThread;

/// \brief This class encapsulates a profiling scope.
///
/// The constructor creates a new scope in the profiling system and the destructor pops the scope.
/// You shouldn't need to use this directly, just use the macro NS_PROFILE_SCOPE provided below.
class NS_FOUNDATION_DLL nsProfilingScope
{
public:
  nsProfilingScope(nsStringView sName, const char* szFunctionName, nsTime timeout);
  ~nsProfilingScope();

protected:
  nsStringView m_sName;
  const char* m_szFunction;
  nsTime m_BeginTime;
  nsTime m_Timeout;
};

/// \brief This class implements a profiling scope similar to nsProfilingScope, but with additional sub-scopes which can be added easily without
/// introducing actual C++ scopes.
///
/// The constructor pushes one surrounding scope on the stack and then a nested scope as the first section.
/// The function StartNextSection() will end the nested scope and start a new inner scope.
/// This allows to end one scope and start a new one, without having to add actual C++ scopes for starting/stopping profiling scopes.
///
/// You shouldn't need to use this directly, just use the macro NS_PROFILE_LIST_SCOPE provided below.
class nsProfilingListScope
{
public:
  NS_FOUNDATION_DLL nsProfilingListScope(nsStringView sListName, nsStringView sFirstSectionName, const char* szFunctionName);
  NS_FOUNDATION_DLL ~nsProfilingListScope();

  NS_FOUNDATION_DLL static void StartNextSection(nsStringView sNextSectionName);

protected:
  static thread_local nsProfilingListScope* s_pCurrentList;

  nsProfilingListScope* m_pPreviousList;

  nsStringView m_sListName;
  const char* m_szListFunction;
  nsTime m_ListBeginTime;

  nsStringView m_sCurSectionName;
  nsTime m_CurSectionBeginTime;
};

/// \brief Helper functionality of the profiling system.
class NS_FOUNDATION_DLL nsProfilingSystem
{
public:
  struct ThreadInfo
  {
    nsUInt64 m_uiThreadId;
    nsString m_sName;
  };

  struct CPUScope
  {
    NS_DECLARE_POD_TYPE();

    static constexpr nsUInt32 NAME_SIZE = 40;

    const char* m_szFunctionName;
    nsTime m_BeginTime;
    nsTime m_EndTime;
    char m_szName[NAME_SIZE];
  };

  struct CPUScopesBufferFlat
  {
    nsDynamicArray<CPUScope> m_Data;
    nsUInt64 m_uiThreadId = 0;
  };

  /// \brief Helper struct to hold GPU profiling data.
  struct GPUScope
  {
    NS_DECLARE_POD_TYPE();

    static constexpr nsUInt32 NAME_SIZE = 48;

    nsTime m_BeginTime;
    nsTime m_EndTime;
    char m_szName[NAME_SIZE];
  };

  struct NS_FOUNDATION_DLL ProfilingData
  {
    nsUInt32 m_uiFramesThreadID = 0;
    nsUInt32 m_uiProcessSortIndex = 0;
    nsOsProcessID m_uiProcessID = 0;

    nsHybridArray<ThreadInfo, 16> m_ThreadInfos;

    nsDynamicArray<CPUScopesBufferFlat> m_AllEventBuffers;

    nsUInt64 m_uiFrameCount = 0;
    nsDynamicArray<nsTime> m_FrameStartTimes;

    nsDynamicArray<nsDynamicArray<GPUScope>> m_GPUScopes;

    /// \brief Writes profiling data as JSON to the output stream.
    nsResult Write(nsStreamWriter& ref_outputStream) const;

    void Clear();

    /// \brief Concatenates all given ProfilingData instances into one merge struct
    static void Merge(ProfilingData& out_merged, nsArrayPtr<const ProfilingData*> inputs);
  };

public:
  static void Clear();

  static void Capture(nsProfilingSystem::ProfilingData& out_capture, bool bClearAfterCapture = false);

  /// \brief Scopes are discarded if their duration is shorter than the specified threshold. Default is 0.1ms.
  static void SetDiscardThreshold(nsTime threshold);

  using ScopeTimeoutDelegate = nsDelegate<void(nsStringView sName, nsStringView sFunctionName, nsTime duration)>;

  /// \brief Sets a callback that is triggered when a profiling scope takes longer than desired.
  static void SetScopeTimeoutCallback(ScopeTimeoutDelegate callback);

  /// \brief Should be called once per frame to capture the timestamp of the new frame.
  static void StartNewFrame();

  /// \brief Adds a new scoped event for the calling thread in the profiling system
  static void AddCPUScope(nsStringView sName, const char* szFunctionName, nsTime beginTime, nsTime endTime, nsTime scopeTimeout);

  /// \brief Get current frame counter
  static nsUInt64 GetFrameCount();

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, ProfilingSystem);
  friend nsUInt32 RunThread(nsThread* pThread);

  static void Initialize();
  /// \brief Removes profiling data of dead threads.
  static void Reset();

  /// \brief Sets the name of the current thread.
  static void SetThreadName(nsStringView sThreadName);
  /// \brief Removes the current thread from the profiling system.
  ///  Needs to be called before the thread exits to be able to release profiling memory of dead threads on Reset.
  static void RemoveThread();

public:
  /// \brief Initialized internal data structures for GPU profiling data. Needs to be called before adding any data.
  static void InitializeGPUData(nsUInt32 uiGpuCount = 1);

  /// \brief Adds a GPU profiling scope in the internal event ringbuffer.
  static void AddGPUScope(nsStringView sName, nsTime beginTime, nsTime endTime, nsUInt32 uiGpuIndex = 0);
};

#if NS_ENABLED(NS_USE_PROFILING) || defined(NS_DOCS)

/// \brief Profiles the current scope using the given name.
///
/// It is allowed to nest NS_PROFILE_SCOPE, also with NS_PROFILE_LIST_SCOPE. However NS_PROFILE_SCOPE should start and end within the same list scope
/// section.
///
/// \note The name string must not be destroyed before the current scope ends.
///
/// \sa nsProfilingScope
/// \sa NS_PROFILE_LIST_SCOPE
#  define NS_PROFILE_SCOPE(ScopeName) \
    nsProfilingScope NS_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ScopeName, NS_SOURCE_FUNCTION, nsTime::MakeZero())

/// \brief Same as NS_PROFILE_SCOPE but if the scope takes longer than 'Timeout', the nsProfilingSystem's timeout callback is executed.
///
/// This can be used to log an error or save a callstack, etc. when a scope exceeds an expected amount of time.
///
/// \sa nsProfilingSystem::SetScopeTimeoutCallback()
#  define NS_PROFILE_SCOPE_WITH_TIMEOUT(ScopeName, Timeout) \
    nsProfilingScope NS_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ScopeName, NS_SOURCE_FUNCTION, Timeout)

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
#  define NS_PROFILE_LIST_SCOPE(ListName, FirstSectionName) \
    nsProfilingListScope NS_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ListName, FirstSectionName, NS_SOURCE_FUNCTION)

/// \brief Starts a new section in a NS_PROFILE_LIST_SCOPE
///
/// \sa nsProfilingListScope
/// \sa NS_PROFILE_LIST_SCOPE
#  define NS_PROFILE_LIST_NEXT_SECTION(NextSectionName) \
    nsProfilingListScope::StartNextSection(NextSectionName)

/// \brief Used to indicate that a frame is finished and another starts.
#  define NS_PROFILER_FRAME_MARKER()

#else
#  define NS_PROFILE_SCOPE(ScopeName)
#  define NS_PROFILE_SCOPE_WITH_TIMEOUT(ScopeName, Timeout)
#  define NS_PROFILE_LIST_SCOPE(ListName, FirstSectionName)
#  define NS_PROFILE_LIST_NEXT_SECTION(NextSectionName)
#  define NS_PROFILER_FRAME_MARKER()
#endif

// Let Tracy override the macros.
#include <Foundation/Profiling/Profiling_Tracy.h>
