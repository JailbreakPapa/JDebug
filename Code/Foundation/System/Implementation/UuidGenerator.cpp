#include <Foundation/FoundationPCH.h>

#include <Foundation/Types/Uuid.h>

nsUuid nsUuid::MakeStableUuidFromString(nsStringView sString)
{
  nsUuid NewUuid;
  NewUuid.m_uiLow = nsHashingUtils::xxHash64String(sString);
  NewUuid.m_uiHigh = nsHashingUtils::xxHash64String(sString, 0x7FFFFFFFFFFFFFE7u);

  return NewUuid;
}

nsUuid nsUuid::MakeStableUuidFromInt(nsInt64 iInt)
{
  nsUuid NewUuid;
  NewUuid.m_uiLow = nsHashingUtils::xxHash64(&iInt, sizeof(nsInt64));
  NewUuid.m_uiHigh = nsHashingUtils::xxHash64(&iInt, sizeof(nsInt64), 0x7FFFFFFFFFFFFFE7u);

  return NewUuid;
}
