#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Types/Uuid.h>

#if __has_include(<uuid/uuid.h>)
#  include <uuid/uuid.h>
#  define HAS_UUID 1
#else
// #  error "uuid.h does not exist on this distro."
#  define HAS_UUID 0
#endif

#if HAS_UUID

NS_CHECK_AT_COMPILETIME(sizeof(nsUInt64) * 2 == sizeof(uuid_t));

nsUuid nsUuid::MakeUuid()
{
  uuid_t uuid;
  uuid_generate(uuid);

  nsUInt64* uiUuidData = reinterpret_cast<nsUInt64*>(uuid);

  return nsUuid(uiUuidData[1], uiUuidData[0]);
}

#else

nsUuid nsUuid::MakeUuid()
{
  NS_REPORT_FAILURE("This distro doesn't have support for UUID generation.");
  return nsUuid();
}

#endif
