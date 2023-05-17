#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/Compression.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  define ZSTD_STATIC_LINKING_ONLY // ZSTD_findDecompressedSize
#  include <zstd/zstd.h>
#endif

namespace wdCompressionUtils
{
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  static wdResult CompressZStd(wdArrayPtr<const wdUInt8> uncompressedData, wdDynamicArray<wdUInt8>& out_data)
  {
    size_t uiSizeBound = ZSTD_compressBound(uncompressedData.GetCount());
    if (uiSizeBound > wdMath::MaxValue<wdUInt32>())
    {
      wdLog::Error("Can't compress since the output container can't hold enough elements ({0})", static_cast<wdUInt64>(uiSizeBound));
      return WD_FAILURE;
    }

    out_data.SetCountUninitialized(static_cast<wdUInt32>(uiSizeBound));

    size_t const cSize = ZSTD_compress(out_data.GetData(), uiSizeBound, uncompressedData.GetPtr(), uncompressedData.GetCount(), 1);
    if (ZSTD_isError(cSize))
    {
      wdLog::Error("Compression failed with error: '{0}'.", ZSTD_getErrorName(cSize));
      return WD_FAILURE;
    }

    out_data.SetCount(static_cast<wdUInt32>(cSize));

    return WD_SUCCESS;
  }

  static wdResult DecompressZStd(wdArrayPtr<const wdUInt8> compressedData, wdDynamicArray<wdUInt8>& out_data)
  {
    wdUInt64 uiSize = ZSTD_findDecompressedSize(compressedData.GetPtr(), compressedData.GetCount());

    if (uiSize == ZSTD_CONTENTSIZE_ERROR)
    {
      wdLog::Error("Can't decompress since it wasn't compressed with ZStd");
      return WD_FAILURE;
    }
    else if (uiSize == ZSTD_CONTENTSIZE_UNKNOWN)
    {
      wdLog::Error("Can't decompress since the original size can't be determined, was the data compressed using the streaming variant?");
      return WD_FAILURE;
    }

    if (uiSize > wdMath::MaxValue<wdUInt32>())
    {
      wdLog::Error("Can't compress since the output container can't hold enough elements ({0})", uiSize);
      return WD_FAILURE;
    }

    out_data.SetCountUninitialized(static_cast<wdUInt32>(uiSize));

    size_t const uiActualSize = ZSTD_decompress(out_data.GetData(), wdMath::SafeConvertToSizeT(uiSize), compressedData.GetPtr(), compressedData.GetCount());

    if (uiActualSize != uiSize)
    {
      wdLog::Error("Error during ZStd decompression: '{0}'.", ZSTD_getErrorName(uiActualSize));
      return WD_FAILURE;
    }

    return WD_SUCCESS;
  }
#endif

  wdResult Compress(wdArrayPtr<const wdUInt8> uncompressedData, wdCompressionMethod method, wdDynamicArray<wdUInt8>& out_data)
  {
    out_data.Clear();

    if (uncompressedData.IsEmpty())
    {
      return WD_SUCCESS;
    }

    switch (method)
    {
      case wdCompressionMethod::ZStd:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
        return CompressZStd(uncompressedData, out_data);
#else
        wdLog::Error("ZStd compression disabled in build settings!");
        return WD_FAILURE;
#endif
      default:
        wdLog::Error("Unsupported compression method {0}!", static_cast<wdUInt32>(method));
        return WD_FAILURE;
    }
  }

  wdResult Decompress(wdArrayPtr<const wdUInt8> compressedData, wdCompressionMethod method, wdDynamicArray<wdUInt8>& out_data)
  {
    out_data.Clear();

    if (compressedData.IsEmpty())
    {
      return WD_SUCCESS;
    }

    switch (method)
    {
      case wdCompressionMethod::ZStd:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
        return DecompressZStd(compressedData, out_data);
#else
        wdLog::Error("ZStd compression disabled in build settings!");
        return WD_FAILURE;
#endif
      default:
        wdLog::Error("Unsupported compression method {0}!", static_cast<wdUInt32>(method));
        return WD_FAILURE;
    }
  }
} // namespace wdCompressionUtils


WD_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_Compression);
