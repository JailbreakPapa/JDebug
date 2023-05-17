#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <combaseapi.h>
#include <rpc.h>

WD_CHECK_AT_COMPILETIME(sizeof(wdUInt64) * 2 == sizeof(UUID));

void wdUuid::CreateNewUuid()
{
  wdUInt64 uiUuidData[2];

  // this works on desktop Windows
  // UuidCreate(reinterpret_cast<UUID*>(uiUuidData));

  // this also works on UWP
  GUID* guid = reinterpret_cast<GUID*>(&uiUuidData[0]);
  HRESULT hr = CoCreateGuid(guid);
  WD_ASSERT_DEBUG(SUCCEEDED(hr), "CoCreateGuid failed, guid might be invalid!");

  m_uiHigh = uiUuidData[0];
  m_uiLow = uiUuidData[1];
}
