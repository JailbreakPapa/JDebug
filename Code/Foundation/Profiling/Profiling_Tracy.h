#include <Foundation/Basics.h>
#include <Foundation/Strings/HashedString.h>

#if NS_ENABLED(NS_USE_PROFILING) && TRACY_ENABLE && !NS_DOCS

#  include <tracy/tracy/Tracy.hpp>

NS_ALWAYS_INLINE nsUInt32 __tracyNsStringLength(const char* szString)
{
  return nsStringUtils::GetStringElementCount(szString);
}

NS_ALWAYS_INLINE nsUInt32 __tracyNsStringLength(nsStringView sString)
{
  return sString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 __tracyNsStringLength(const nsString& sString)
{
  return sString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 __tracyNsStringLength(const nsStringBuilder& sString)
{
  return sString.GetElementCount();
}

NS_ALWAYS_INLINE nsUInt32 __tracyNsStringLength(const nsHashedString& sString)
{
  return sString.GetView().GetElementCount();
}

NS_ALWAYS_INLINE const char* __tracyNsStringToConstChar(const nsString& sString)
{
  return sString.GetData();
}

NS_ALWAYS_INLINE const char* __tracyNsStringToConstChar(const nsStringBuilder& sString)
{
  return sString.GetData();
}

NS_ALWAYS_INLINE const char* __tracyNsStringToConstChar(const nsHashedString& sString)
{
  return sString.GetData();
}

NS_ALWAYS_INLINE const char* __tracyNsStringToConstChar(const nsStringView& sString)
{
  // can just return the string views start pointer, because this is used together with __tracyNsStringLength
  return sString.GetStartPointer();
}

NS_ALWAYS_INLINE const char* __tracyNsStringToConstChar(const char* szString)
{
  return szString;
}

/// \brief Similar to NS_PROFILE_SCOPE, but only forwards to Tracy
#  define NS_TRACY_PROFILE_SCOPE(ScopeName) \
    ZoneScoped;                             \
    ZoneName(__tracyNsStringToConstChar(ScopeName), __tracyNsStringLength(ScopeName))

// Override the standard NS profiling macros and inject Tracy profiling scopes

#  undef NS_PROFILE_SCOPE
#  define NS_PROFILE_SCOPE(ScopeName)                                                                                 \
    nsProfilingScope NS_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ScopeName, NS_SOURCE_FUNCTION, nsTime::MakeZero()); \
    NS_TRACY_PROFILE_SCOPE(ScopeName)

#  undef NS_PROFILE_SCOPE_WITH_TIMEOUT
#  define NS_PROFILE_SCOPE_WITH_TIMEOUT(ScopeName, Timeout)                                                \
    nsProfilingScope NS_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ScopeName, NS_SOURCE_FUNCTION, Timeout); \
    NS_TRACY_PROFILE_SCOPE(ScopeName);

#  undef NS_PROFILE_LIST_SCOPE
#  define NS_PROFILE_LIST_SCOPE(ListName, FirstSectionName)                                                            \
    nsProfilingListScope NS_CONCAT(_nsProfilingScope, NS_SOURCE_LINE)(ListName, FirstSectionName, NS_SOURCE_FUNCTION); \
    NS_TRACY_PROFILE_SCOPE(ScopeName);

#  undef NS_PROFILER_FRAME_MARKER
#  define NS_PROFILER_FRAME_MARKER() FrameMark

#else

/// \brief Similar to NS_PROFILE_SCOPE, but only forwards to Tracy
#  define NS_TRACY_PROFILE_SCOPE(ScopeName)

#endif
