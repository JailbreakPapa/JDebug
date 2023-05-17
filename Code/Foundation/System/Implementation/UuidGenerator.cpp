#include <Foundation/FoundationPCH.h>

#include <Foundation/Types/Uuid.h>


// Include inline file
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/System/Implementation/Win/UuidGenerator_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX)
#  include <Foundation/System/Implementation/Posix/UuidGenerator_posix.h>
#elif WD_ENABLED(WD_PLATFORM_LINUX)
#  include <Foundation/System/Implementation/Posix/UuidGenerator_posix.h>
#elif WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/System/Implementation/Android/UuidGenerator_android.h>
#else
#  error "Uuid generation functions are not implemented on current platform"
#endif

wdUuid wdUuid::StableUuidForString(wdStringView sString)
{
  wdUuid NewUuid;
  NewUuid.m_uiLow = wdHashingUtils::xxHash64String(sString);
  NewUuid.m_uiHigh = wdHashingUtils::xxHash64String(sString, 0x7FFFFFFFFFFFFFE7u);

  return NewUuid;
}

wdUuid wdUuid::StableUuidForInt(wdInt64 iInt)
{
  wdUuid NewUuid;
  NewUuid.m_uiLow = wdHashingUtils::xxHash64(&iInt, sizeof(wdInt64));
  NewUuid.m_uiHigh = wdHashingUtils::xxHash64(&iInt, sizeof(wdInt64), 0x7FFFFFFFFFFFFFE7u);

  return NewUuid;
}

WD_STATICLINK_FILE(Foundation, Foundation_System_Implementation_UuidGenerator);
