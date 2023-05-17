#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <uuid/uuid.h>


WD_CHECK_AT_COMPILETIME(sizeof(wdUInt64) * 2 == sizeof(uuid_t));

void wdUuid::CreateNewUuid()
{
  uuid_t uuid;
  uuid_generate(uuid);

  wdUInt64* uiUuidData = reinterpret_cast<wdUInt64*>(uuid);

  m_uiHigh = uiUuidData[0];
  m_uiLow = uiUuidData[1];
}
