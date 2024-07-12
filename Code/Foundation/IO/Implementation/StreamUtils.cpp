#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/StreamUtils.h>

void nsStreamUtils::ReadAllAndAppend(nsStreamReader& inout_stream, nsDynamicArray<nsUInt8>& ref_destination)
{
  nsUInt8 temp[1024 * 4];

  while (true)
  {
    const nsUInt32 uiRead = (nsUInt32)inout_stream.ReadBytes(temp, NS_ARRAY_SIZE(temp));

    if (uiRead == 0)
      return;

    ref_destination.PushBackRange(nsArrayPtr<nsUInt8>(temp, uiRead));
  }
}
