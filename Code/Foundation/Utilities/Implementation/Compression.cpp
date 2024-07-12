#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/Compression.h>

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
#  define ZSTD_STATIC_LINKING_ONLY // ZSTD_findDecompressedSize
#  include <zstd/zstd.h>
#endif

namespace nsCompressionUtils
{
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  static nsResult CompressZStd(nsArrayPtr<const nsUInt8> uncompressedData, nsDynamicArray<nsUInt8>& out_data)
  {
    size_t uiSizeBound = ZSTD_compressBound(uncompressedData.GetCount());
    if (uiSizeBound > nsMath::MaxValue<nsUInt32>())
    {
      nsLog::Error("Can't compress since the output container can't hold enough elements ({0})", static_cast<nsUInt64>(uiSizeBound));
      return NS_FAILURE;
    }

    out_data.SetCountUninitialized(static_cast<nsUInt32>(uiSizeBound));

    size_t const cSize = ZSTD_compress(out_data.GetData(), uiSizeBound, uncompressedData.GetPtr(), uncompressedData.GetCount(), 1);
    if (ZSTD_isError(cSize))
    {
      nsLog::Error("Compression failed with error: '{0}'.", ZSTD_getErrorName(cSize));
      return NS_FAILURE;
    }

    out_data.SetCount(static_cast<nsUInt32>(cSize));

    return NS_SUCCESS;
  }

  static nsResult DecompressZStd(nsArrayPtr<const nsUInt8> compressedData, nsDynamicArray<nsUInt8>& out_data)
  {
    nsUInt64 uiSize = ZSTD_findDecompressedSize(compressedData.GetPtr(), compressedData.GetCount());

    if (uiSize == ZSTD_CONTENTSIZE_ERROR)
    {
      nsLog::Error("Can't decompress since it wasn't compressed with ZStd");
      return NS_FAILURE;
    }
    else if (uiSize == ZSTD_CONTENTSIZE_UNKNOWN)
    {
      nsLog::Error("Can't decompress since the original size can't be determined, was the data compressed using the streaming variant?");
      return NS_FAILURE;
    }

    if (uiSize > nsMath::MaxValue<nsUInt32>())
    {
      nsLog::Error("Can't compress since the output container can't hold enough elements ({0})", uiSize);
      return NS_FAILURE;
    }

    out_data.SetCountUninitialized(static_cast<nsUInt32>(uiSize));

    size_t const uiActualSize = ZSTD_decompress(out_data.GetData(), nsMath::SafeConvertToSizeT(uiSize), compressedData.GetPtr(), compressedData.GetCount());

    if (uiActualSize != uiSize)
    {
      nsLog::Error("Error during ZStd decompression: '{0}'.", ZSTD_getErrorName(uiActualSize));
      return NS_FAILURE;
    }

    return NS_SUCCESS;
  }
#endif

  nsResult Compress(nsArrayPtr<const nsUInt8> uncompressedData, nsCompressionMethod method, nsDynamicArray<nsUInt8>& out_data)
  {
    out_data.Clear();

    if (uncompressedData.IsEmpty())
    {
      return NS_SUCCESS;
    }

    switch (method)
    {
      case nsCompressionMethod::ZStd:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
        return CompressZStd(uncompressedData, out_data);
#else
        nsLog::Error("ZStd compression disabled in build settings!");
        return NS_FAILURE;
#endif
      default:
        nsLog::Error("Unsupported compression method {0}!", static_cast<nsUInt32>(method));
        return NS_FAILURE;
    }
  }

  nsResult Decompress(nsArrayPtr<const nsUInt8> compressedData, nsCompressionMethod method, nsDynamicArray<nsUInt8>& out_data)
  {
    out_data.Clear();

    if (compressedData.IsEmpty())
    {
      return NS_SUCCESS;
    }

    switch (method)
    {
      case nsCompressionMethod::ZStd:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
        return DecompressZStd(compressedData, out_data);
#else
        nsLog::Error("ZStd compression disabled in build settings!");
        return NS_FAILURE;
#endif
      default:
        nsLog::Error("Unsupported compression method {0}!", static_cast<nsUInt32>(method));
        return NS_FAILURE;
    }
  }
} // namespace nsCompressionUtils
