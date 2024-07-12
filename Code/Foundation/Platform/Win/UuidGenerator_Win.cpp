#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Types/Uuid.h>

#  include <combaseapi.h>
#  include <rpc.h>

NS_CHECK_AT_COMPILETIME(sizeof(nsUInt64) * 2 == sizeof(UUID));

nsUuid nsUuid::MakeUuid()
{
  nsUInt64 uiUuidData[2];

  // this works on desktop Windows
  // UuidCreate(reinterpret_cast<UUID*>(uiUuidData));

  // this also works on UWP
  GUID* guid = reinterpret_cast<GUID*>(&uiUuidData[0]);
  HRESULT hr = CoCreateGuid(guid);
  NS_IGNORE_UNUSED(hr);
  NS_ASSERT_DEBUG(SUCCEEDED(hr), "CoCreateGuid failed, guid might be invalid!");

  return nsUuid(uiUuidData[1], uiUuidData[0]);
}

#endif
