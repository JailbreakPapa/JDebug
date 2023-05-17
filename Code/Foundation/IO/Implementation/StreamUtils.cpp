#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/StreamUtils.h>

void wdStreamUtils::ReadAllAndAppend(wdStreamReader& inout_stream, wdDynamicArray<wdUInt8>& ref_destination)
{
  wdUInt8 temp[1024 * 4];

  while (true)
  {
    const wdUInt32 uiRead = (wdUInt32)inout_stream.ReadBytes(temp, WD_ARRAY_SIZE(temp));

    if (uiRead == 0)
      return;

    ref_destination.PushBackRange(wdArrayPtr<wdUInt8>(temp, uiRead));
  }
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_StreamUtils);
