
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/Stream.h>

namespace wdStreamUtils
{
  /// \brief Reads all the remaining data in \a stream and appends it to \a destination.
  WD_FOUNDATION_DLL void ReadAllAndAppend(wdStreamReader& inout_stream, wdDynamicArray<wdUInt8>& ref_destination);

} // namespace wdStreamUtils
