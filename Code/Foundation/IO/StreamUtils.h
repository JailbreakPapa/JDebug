
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/Stream.h>

namespace nsStreamUtils
{
  /// \brief Reads all the remaining data in \a stream and appends it to \a destination.
  NS_FOUNDATION_DLL void ReadAllAndAppend(nsStreamReader& inout_stream, nsDynamicArray<nsUInt8>& ref_destination);

} // namespace nsStreamUtils
