#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>

///\brief The compression method to be used
enum class wdCompressionMethod : wdUInt16
{
  ZStd = 0 ///< Only available when ZStd support is enabled in the build (default)
};

/// \brief This namespace contains utilities which can be used to compress and decompress data.
namespace wdCompressionUtils
{
  ///\brief Compresses the given data using the compression method eMethod into the dynamic array given in out_Data.
  WD_FOUNDATION_DLL wdResult Compress(wdArrayPtr<const wdUInt8> uncompressedData, wdCompressionMethod method, wdDynamicArray<wdUInt8>& out_data);

  ///\brief Decompresses the given data using the compression method eMethod into the dynamic array given in out_Data.
  WD_FOUNDATION_DLL wdResult Decompress(wdArrayPtr<const wdUInt8> compressedData, wdCompressionMethod method, wdDynamicArray<wdUInt8>& out_data);
} // namespace wdCompressionUtils
