#include <Texture/TexturePCH.h>

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/ImageConversion.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsImageConversionStep);

namespace
{
  struct TableEntry
  {
    TableEntry() = default;

    TableEntry(const nsImageConversionStep* pStep, const nsImageConversionEntry& entry)
    {
      m_step = pStep;
      m_sourceFormat = entry.m_sourceFormat;
      m_targetFormat = entry.m_targetFormat;
      m_numChannels = nsMath::Min(nsImageFormat::GetNumChannels(entry.m_sourceFormat), nsImageFormat::GetNumChannels(entry.m_targetFormat));

      float sourceBpp = nsImageFormat::GetExactBitsPerPixel(m_sourceFormat);
      float targetBpp = nsImageFormat::GetExactBitsPerPixel(m_targetFormat);

      m_flags = entry.m_flags;

      // Base cost is amount of bits processed
      m_cost = sourceBpp + targetBpp;

      // Penalty for non-inplace conversion
      if ((m_flags & nsImageConversionFlags::InPlace) == 0)
      {
        m_cost *= 2;
      }

      // Penalize formats that aren't aligned to powers of two
      if (!nsImageFormat::IsCompressed(m_sourceFormat) && !nsImageFormat::IsCompressed(m_targetFormat))
      {
        auto sourceBppInt = static_cast<nsUInt32>(sourceBpp);
        auto targetBppInt = static_cast<nsUInt32>(targetBpp);
        if (!nsMath::IsPowerOf2(sourceBppInt) || !nsMath::IsPowerOf2(targetBppInt))
        {
          m_cost *= 2;
        }
      }

      m_cost += entry.m_additionalPenalty;
    }

    const nsImageConversionStep* m_step = nullptr;
    nsImageFormat::Enum m_sourceFormat = nsImageFormat::UNKNOWN;
    nsImageFormat::Enum m_targetFormat = nsImageFormat::UNKNOWN;
    nsBitflags<nsImageConversionFlags> m_flags;
    float m_cost = nsMath::MaxValue<float>();
    nsUInt32 m_numChannels = 0;

    static TableEntry chain(const TableEntry& a, const TableEntry& b)
    {
      if (nsImageFormat::GetExactBitsPerPixel(a.m_sourceFormat) > nsImageFormat::GetExactBitsPerPixel(a.m_targetFormat) &&
          nsImageFormat::GetExactBitsPerPixel(b.m_sourceFormat) < nsImageFormat::GetExactBitsPerPixel(b.m_targetFormat))
      {
        // Disallow chaining conversions which first reduce to a smaller intermediate and then go back to a larger one, since
        // we end up throwing away information.
        return {};
      }

      TableEntry entry;
      entry.m_step = a.m_step;
      entry.m_cost = a.m_cost + b.m_cost;
      entry.m_sourceFormat = a.m_sourceFormat;
      entry.m_targetFormat = a.m_targetFormat;
      entry.m_flags = a.m_flags;
      entry.m_numChannels = nsMath::Min(a.m_numChannels, b.m_numChannels);
      return entry;
    }

    bool operator<(const TableEntry& other) const
    {
      if (m_numChannels > other.m_numChannels)
        return true;

      if (m_numChannels < other.m_numChannels)
        return false;

      return m_cost < other.m_cost;
    }

    bool isAdmissible() const
    {
      if (m_numChannels == 0)
        return false;

      return m_cost < nsMath::MaxValue<float>();
    }
  };

  nsMutex s_conversionTableLock;
  nsHashTable<nsUInt32, TableEntry> s_conversionTable;
  bool s_conversionTableValid = false;

  constexpr nsUInt32 MakeKey(nsImageFormat::Enum a, nsImageFormat::Enum b)
  {
    return a * nsImageFormat::NUM_FORMATS + b;
  }
  constexpr nsUInt32 MakeTypeKey(nsImageFormatType::Enum a, nsImageFormatType::Enum b)
  {
    return (a << 16) + b;
  }

  struct IntermediateBuffer
  {
    IntermediateBuffer(nsUInt32 uiBitsPerBlock)
      : m_bitsPerBlock(uiBitsPerBlock)
    {
    }
    nsUInt32 m_bitsPerBlock;
  };

  nsUInt32 allocateScratchBufferIndex(nsHybridArray<IntermediateBuffer, 16>& ref_scratchBuffers, nsUInt32 uiBitsPerBlock, nsUInt32 uiExcludedIndex)
  {
    int foundIndex = -1;

    for (nsUInt32 bufferIndex = 0; bufferIndex < nsUInt32(ref_scratchBuffers.GetCount()); ++bufferIndex)
    {
      if (bufferIndex == uiExcludedIndex)
      {
        continue;
      }

      if (ref_scratchBuffers[bufferIndex].m_bitsPerBlock == uiBitsPerBlock)
      {
        foundIndex = bufferIndex;
        break;
      }
    }

    if (foundIndex >= 0)
    {
      // Reuse existing scratch buffer
      return foundIndex;
    }
    else
    {
      // Allocate new scratch buffer
      ref_scratchBuffers.PushBack(IntermediateBuffer(uiBitsPerBlock));
      return ref_scratchBuffers.GetCount() - 1;
    }
  }
} // namespace

nsImageConversionStep::nsImageConversionStep()
{
  s_conversionTableValid = false;
}

nsImageConversionStep::~nsImageConversionStep()
{
  s_conversionTableValid = false;
}

nsResult nsImageConversion::BuildPath(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, bool bSourceEqualsTarget,
  nsHybridArray<nsImageConversion::ConversionPathNode, 16>& ref_path_out, nsUInt32& ref_uiNumScratchBuffers_out)
{
  NS_LOCK(s_conversionTableLock);

  ref_path_out.Clear();
  ref_uiNumScratchBuffers_out = 0;

  if (sourceFormat == targetFormat)
  {
    ConversionPathNode node;
    node.m_sourceFormat = sourceFormat;
    node.m_targetFormat = targetFormat;
    node.m_inPlace = bSourceEqualsTarget;
    node.m_sourceBufferIndex = 0;
    node.m_targetBufferIndex = 0;
    node.m_step = nullptr;
    ref_path_out.PushBack(node);
    return NS_SUCCESS;
  }

  if (!s_conversionTableValid)
  {
    RebuildConversionTable();
  }

  for (nsImageFormat::Enum current = sourceFormat; current != targetFormat;)
  {
    nsUInt32 currentTableIndex = MakeKey(current, targetFormat);

    TableEntry entry;

    if (!s_conversionTable.TryGetValue(currentTableIndex, entry))
    {
      return NS_FAILURE;
    }

    nsImageConversion::ConversionPathNode step;
    step.m_sourceFormat = entry.m_sourceFormat;
    step.m_targetFormat = entry.m_targetFormat;
    step.m_inPlace = entry.m_flags.IsAnySet(nsImageConversionFlags::InPlace);
    step.m_step = entry.m_step;

    current = entry.m_targetFormat;

    ref_path_out.PushBack(step);
  }

  nsHybridArray<IntermediateBuffer, 16> scratchBuffers;
  scratchBuffers.PushBack(IntermediateBuffer(nsImageFormat::GetBitsPerBlock(targetFormat)));

  for (int i = ref_path_out.GetCount() - 1; i >= 0; --i)
  {
    if (i == ref_path_out.GetCount() - 1)
      ref_path_out[i].m_targetBufferIndex = 0;
    else
      ref_path_out[i].m_targetBufferIndex = ref_path_out[i + 1].m_sourceBufferIndex;

    if (i > 0)
    {
      if (ref_path_out[i].m_inPlace)
      {
        ref_path_out[i].m_sourceBufferIndex = ref_path_out[i].m_targetBufferIndex;
      }
      else
      {
        nsUInt32 bitsPerBlock = nsImageFormat::GetBitsPerBlock(ref_path_out[i].m_sourceFormat);

        ref_path_out[i].m_sourceBufferIndex = allocateScratchBufferIndex(scratchBuffers, bitsPerBlock, ref_path_out[i].m_targetBufferIndex);
      }
    }
  }

  if (bSourceEqualsTarget)
  {
    // Enforce constraint that source == target
    ref_path_out[0].m_sourceBufferIndex = 0;

    // Did we accidentally break the in-place invariant?
    if (ref_path_out[0].m_sourceBufferIndex == ref_path_out[0].m_targetBufferIndex && !ref_path_out[0].m_inPlace)
    {
      if (ref_path_out.GetCount() == 1)
      {
        // Only a single step, so we need to add a copy step
        nsImageConversion::ConversionPathNode copy;
        copy.m_inPlace = false;
        copy.m_sourceFormat = sourceFormat;
        copy.m_targetFormat = sourceFormat;
        copy.m_sourceBufferIndex = ref_path_out[0].m_sourceBufferIndex;
        copy.m_targetBufferIndex =
          allocateScratchBufferIndex(scratchBuffers, nsImageFormat::GetBitsPerBlock(ref_path_out[0].m_sourceFormat), ref_path_out[0].m_sourceBufferIndex);
        ref_path_out[0].m_sourceBufferIndex = copy.m_targetBufferIndex;
        copy.m_step = nullptr;
        ref_path_out.InsertAt(0, copy);
      }
      else
      {
        // Turn second step to non-inplace
        ref_path_out[1].m_inPlace = false;
        ref_path_out[1].m_sourceBufferIndex =
          allocateScratchBufferIndex(scratchBuffers, nsImageFormat::GetBitsPerBlock(ref_path_out[1].m_sourceFormat), ref_path_out[0].m_sourceBufferIndex);
        ref_path_out[0].m_targetBufferIndex = ref_path_out[1].m_sourceBufferIndex;
      }
    }
  }
  else
  {
    ref_path_out[0].m_sourceBufferIndex = scratchBuffers.GetCount();
  }

  ref_uiNumScratchBuffers_out = scratchBuffers.GetCount() - 1;

  return NS_SUCCESS;
}

void nsImageConversion::RebuildConversionTable()
{
  NS_LOCK(s_conversionTableLock);

  s_conversionTable.Clear();

  // Prime conversion table with known conversions
  for (nsImageConversionStep* conversion = nsImageConversionStep::GetFirstInstance(); conversion; conversion = conversion->GetNextInstance())
  {
    nsArrayPtr<const nsImageConversionEntry> entries = conversion->GetSupportedConversions();

    for (nsUInt32 subIndex = 0; subIndex < (nsUInt32)entries.GetCount(); subIndex++)
    {
      const nsImageConversionEntry& subConversion = entries[subIndex];

      if (subConversion.m_flags.IsAnySet(nsImageConversionFlags::InPlace))
      {
        NS_ASSERT_DEV(nsImageFormat::IsCompressed(subConversion.m_sourceFormat) == nsImageFormat::IsCompressed(subConversion.m_targetFormat) &&
                        nsImageFormat::GetBitsPerBlock(subConversion.m_sourceFormat) == nsImageFormat::GetBitsPerBlock(subConversion.m_targetFormat),
          "In-place conversions are only allowed between formats of the same number of bits per pixel and compressedness");
      }

      if (nsImageFormat::GetType(subConversion.m_sourceFormat) == nsImageFormatType::PLANAR)
      {
        NS_ASSERT_DEV(nsImageFormat::GetType(subConversion.m_targetFormat) == nsImageFormatType::LINEAR, "Conversions from planar formats must target linear formats");
      }
      else if (nsImageFormat::GetType(subConversion.m_targetFormat) == nsImageFormatType::PLANAR)
      {
        NS_ASSERT_DEV(nsImageFormat::GetType(subConversion.m_sourceFormat) == nsImageFormatType::LINEAR, "Conversions to planar formats must sourced from linear formats");
      }

      nsUInt32 tableIndex = MakeKey(subConversion.m_sourceFormat, subConversion.m_targetFormat);

      // Use the cheapest known conversion for each combination in case there are multiple ones
      TableEntry candidate(conversion, subConversion);

      TableEntry existing;

      if (!s_conversionTable.TryGetValue(tableIndex, existing) || candidate < existing)
      {
        s_conversionTable.Insert(tableIndex, candidate);
      }
    }
  }

  for (nsUInt32 i = 0; i < nsImageFormat::NUM_FORMATS; i++)
  {
    const nsImageFormat::Enum format = static_cast<nsImageFormat::Enum>(i);
    // Add copy-conversion (from and to same format)
    s_conversionTable.Insert(
      MakeKey(format, format), TableEntry(nullptr, nsImageConversionEntry(nsImageConversionEntry(format, format, nsImageConversionFlags::InPlace))));
  }

  // Straight from http://en.wikipedia.org/wiki/Floyd-Warshall_algorithm
  for (nsUInt32 k = 1; k < nsImageFormat::NUM_FORMATS; k++)
  {
    for (nsUInt32 i = 1; i < nsImageFormat::NUM_FORMATS; i++)
    {
      if (k == i)
      {
        continue;
      }

      nsUInt32 tableIndexIK = MakeKey(static_cast<nsImageFormat::Enum>(i), static_cast<nsImageFormat::Enum>(k));

      TableEntry entryIK;
      if (!s_conversionTable.TryGetValue(tableIndexIK, entryIK))
      {
        continue;
      }

      for (nsUInt32 j = 1; j < nsImageFormat::NUM_FORMATS; j++)
      {
        if (j == i || j == k)
        {
          continue;
        }

        nsUInt32 tableIndexIJ = MakeKey(static_cast<nsImageFormat::Enum>(i), static_cast<nsImageFormat::Enum>(j));
        nsUInt32 tableIndexKJ = MakeKey(static_cast<nsImageFormat::Enum>(k), static_cast<nsImageFormat::Enum>(j));

        TableEntry entryKJ;
        if (!s_conversionTable.TryGetValue(tableIndexKJ, entryKJ))
        {
          continue;
        }

        TableEntry candidate = TableEntry::chain(entryIK, entryKJ);

        TableEntry existing;
        if (candidate.isAdmissible() && candidate < s_conversionTable[tableIndexIJ])
        {
          // To Convert from format I to format J, first Convert from I to K
          s_conversionTable[tableIndexIJ] = candidate;
        }
      }
    }
  }

  s_conversionTableValid = true;
}

nsResult nsImageConversion::Convert(const nsImageView& source, nsImage& ref_target, nsImageFormat::Enum targetFormat)
{
  NS_PROFILE_SCOPE("nsImageConversion::Convert");

  nsImageFormat::Enum sourceFormat = source.GetImageFormat();

  // Trivial copy
  if (sourceFormat == targetFormat)
  {
    if (&source != &ref_target)
    {
      // copy if not already the same
      ref_target.ResetAndCopy(source);
    }
    return NS_SUCCESS;
  }

  nsHybridArray<ConversionPathNode, 16> path;
  nsUInt32 numScratchBuffers = 0;
  if (BuildPath(sourceFormat, targetFormat, &source == &ref_target, path, numScratchBuffers).Failed())
  {
    return NS_FAILURE;
  }

  return Convert(source, ref_target, path, numScratchBuffers);
}

nsResult nsImageConversion::Convert(const nsImageView& source, nsImage& ref_target, nsArrayPtr<ConversionPathNode> path, nsUInt32 uiNumScratchBuffers)
{
  NS_ASSERT_DEV(path.GetCount() > 0, "Invalid conversion path");
  NS_ASSERT_DEV(path[0].m_sourceFormat == source.GetImageFormat(), "Invalid conversion path");

  nsHybridArray<nsImage, 16> intermediates;
  intermediates.SetCount(uiNumScratchBuffers);

  const nsImageView* pSource = &source;

  for (nsUInt32 i = 0; i < path.GetCount(); ++i)
  {
    nsUInt32 targetIndex = path[i].m_targetBufferIndex;

    nsImage* pTarget = targetIndex == 0 ? &ref_target : &intermediates[targetIndex - 1];

    if (ConvertSingleStep(path[i].m_step, *pSource, *pTarget, path[i].m_targetFormat).Failed())
    {
      return NS_FAILURE;
    }

    pSource = pTarget;
  }

  return NS_SUCCESS;
}

nsResult nsImageConversion::ConvertRaw(
  nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt32 uiNumElements, nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat)
{
  if (uiNumElements == 0)
  {
    return NS_SUCCESS;
  }

  // Trivial copy
  if (sourceFormat == targetFormat)
  {
    if (target.GetPtr() != source.GetPtr())
      memcpy(target.GetPtr(), source.GetPtr(), uiNumElements * nsUInt64(nsImageFormat::GetBitsPerPixel(sourceFormat)) / 8);
    return NS_SUCCESS;
  }

  if (nsImageFormat::IsCompressed(sourceFormat) || nsImageFormat::IsCompressed(targetFormat))
  {
    return NS_FAILURE;
  }

  nsHybridArray<ConversionPathNode, 16> path;
  nsUInt32 numScratchBuffers;
  if (BuildPath(sourceFormat, targetFormat, source.GetPtr() == target.GetPtr(), path, numScratchBuffers).Failed())
  {
    return NS_FAILURE;
  }

  return ConvertRaw(source, target, uiNumElements, path, numScratchBuffers);
}

nsResult nsImageConversion::ConvertRaw(
  nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt32 uiNumElements, nsArrayPtr<ConversionPathNode> path, nsUInt32 uiNumScratchBuffers)
{
  NS_ASSERT_DEV(path.GetCount() > 0, "Path of length 0 is invalid.");

  if (uiNumElements == 0)
  {
    return NS_SUCCESS;
  }

  if (nsImageFormat::IsCompressed(path.GetPtr()->m_sourceFormat) || nsImageFormat::IsCompressed((path.GetEndPtr() - 1)->m_targetFormat))
  {
    return NS_FAILURE;
  }

  nsHybridArray<nsBlob, 16> intermediates;
  intermediates.SetCount(uiNumScratchBuffers);

  for (nsUInt32 i = 0; i < path.GetCount(); ++i)
  {
    nsUInt32 targetIndex = path[i].m_targetBufferIndex;
    nsUInt32 targetBpp = nsImageFormat::GetBitsPerPixel(path[i].m_targetFormat);

    nsByteBlobPtr stepTarget;
    if (targetIndex == 0)
    {
      stepTarget = target;
    }
    else
    {
      nsUInt32 expectedSize = static_cast<nsUInt32>(targetBpp * uiNumElements / 8);
      intermediates[targetIndex - 1].SetCountUninitialized(expectedSize);
      stepTarget = intermediates[targetIndex - 1].GetByteBlobPtr();
    }

    if (path[i].m_step == nullptr)
    {
      memcpy(stepTarget.GetPtr(), source.GetPtr(), uiNumElements * targetBpp / 8);
    }
    else
    {
      if (static_cast<const nsImageConversionStepLinear*>(path[i].m_step)
            ->ConvertPixels(source, stepTarget, uiNumElements, path[i].m_sourceFormat, path[i].m_targetFormat)
            .Failed())
      {
        return NS_FAILURE;
      }
    }

    source = stepTarget;
  }

  return NS_SUCCESS;
}

nsResult nsImageConversion::ConvertSingleStep(
  const nsImageConversionStep* pStep, const nsImageView& source, nsImage& target, nsImageFormat::Enum targetFormat)
{
  if (!pStep)
  {
    target.ResetAndCopy(source);
    return NS_SUCCESS;
  }

  nsImageFormat::Enum sourceFormat = source.GetImageFormat();

  nsImageHeader header = source.GetHeader();
  header.SetImageFormat(targetFormat);
  target.ResetAndAlloc(header);

  switch (MakeTypeKey(nsImageFormat::GetType(sourceFormat), nsImageFormat::GetType(targetFormat)))
  {
    case MakeTypeKey(nsImageFormatType::LINEAR, nsImageFormatType::LINEAR):
    {
      // we have to do the computation in 64-bit otherwise it might overflow for very large textures (8k x 4k or bigger).
      nsUInt64 numElements = nsUInt64(8) * target.GetByteBlobPtr().GetCount() / (nsUInt64)nsImageFormat::GetBitsPerPixel(targetFormat);
      return static_cast<const nsImageConversionStepLinear*>(pStep)->ConvertPixels(
        source.GetByteBlobPtr(), target.GetByteBlobPtr(), (nsUInt32)numElements, sourceFormat, targetFormat);
    }

    case MakeTypeKey(nsImageFormatType::LINEAR, nsImageFormatType::BLOCK_COMPRESSED):
      return ConvertSingleStepCompress(source, target, sourceFormat, targetFormat, pStep);

    case MakeTypeKey(nsImageFormatType::LINEAR, nsImageFormatType::PLANAR):
      return ConvertSingleStepPlanarize(source, target, sourceFormat, targetFormat, pStep);

    case MakeTypeKey(nsImageFormatType::BLOCK_COMPRESSED, nsImageFormatType::LINEAR):
      return ConvertSingleStepDecompress(source, target, sourceFormat, targetFormat, pStep);

    case MakeTypeKey(nsImageFormatType::PLANAR, nsImageFormatType::LINEAR):
      return ConvertSingleStepDeplanarize(source, target, sourceFormat, targetFormat, pStep);

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      return NS_FAILURE;
  }
}

nsResult nsImageConversion::ConvertSingleStepDecompress(
  const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep)
{
  for (nsUInt32 arrayIndex = 0; arrayIndex < source.GetNumArrayIndices(); arrayIndex++)
  {
    for (nsUInt32 face = 0; face < source.GetNumFaces(); face++)
    {
      for (nsUInt32 mipLevel = 0; mipLevel < source.GetNumMipLevels(); mipLevel++)
      {
        const nsUInt32 width = target.GetWidth(mipLevel);
        const nsUInt32 height = target.GetHeight(mipLevel);

        const nsUInt32 blockSizeX = nsImageFormat::GetBlockWidth(sourceFormat);
        const nsUInt32 blockSizeY = nsImageFormat::GetBlockHeight(sourceFormat);

        const nsUInt32 numBlocksX = source.GetNumBlocksX(mipLevel);
        const nsUInt32 numBlocksY = source.GetNumBlocksY(mipLevel);

        const nsUInt64 targetRowPitch = target.GetRowPitch(mipLevel);
        const nsUInt32 targetBytesPerPixel = nsImageFormat::GetBitsPerPixel(targetFormat) / 8;

        // Decompress into a temp memory block so we don't have to explicitly handle the case where the image is not a multiple of the block
        // size
        nsHybridArray<nsUInt8, 256> tempBuffer;
        tempBuffer.SetCount(numBlocksX * blockSizeX * blockSizeY * targetBytesPerPixel);

        for (nsUInt32 slice = 0; slice < source.GetDepth(mipLevel); slice++)
        {
          for (nsUInt32 blockY = 0; blockY < numBlocksY; blockY++)
          {
            nsImageView sourceRowView = source.GetRowView(mipLevel, face, arrayIndex, blockY, slice);

            if (static_cast<const nsImageConversionStepDecompressBlocks*>(pStep)
                  ->DecompressBlocks(sourceRowView.GetByteBlobPtr(), nsByteBlobPtr(tempBuffer.GetData(), tempBuffer.GetCount()), numBlocksX,
                    sourceFormat, targetFormat)
                  .Failed())
            {
              return NS_FAILURE;
            }

            for (nsUInt32 blockX = 0; blockX < numBlocksX; blockX++)
            {
              nsUInt8* targetPointer = target.GetPixelPointer<nsUInt8>(mipLevel, face, arrayIndex, blockX * blockSizeX, blockY * blockSizeY, slice);

              // Copy into actual target, clamping to image dimensions
              nsUInt32 copyWidth = nsMath::Min(blockSizeX, width - blockX * blockSizeX);
              nsUInt32 copyHeight = nsMath::Min(blockSizeY, height - blockY * blockSizeY);
              for (nsUInt32 row = 0; row < copyHeight; row++)
              {
                memcpy(targetPointer, &tempBuffer[(blockX * blockSizeX + row) * blockSizeY * targetBytesPerPixel],
                  nsMath::SafeMultiply32(copyWidth, targetBytesPerPixel));
                targetPointer += targetRowPitch;
              }
            }
          }
        }
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsImageConversion::ConvertSingleStepCompress(
  const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep)
{
  for (nsUInt32 arrayIndex = 0; arrayIndex < source.GetNumArrayIndices(); arrayIndex++)
  {
    for (nsUInt32 face = 0; face < source.GetNumFaces(); face++)
    {
      for (nsUInt32 mipLevel = 0; mipLevel < source.GetNumMipLevels(); mipLevel++)
      {
        const nsUInt32 sourceWidth = source.GetWidth(mipLevel);
        const nsUInt32 sourceHeight = source.GetHeight(mipLevel);

        const nsUInt32 numBlocksX = target.GetNumBlocksX(mipLevel);
        const nsUInt32 numBlocksY = target.GetNumBlocksY(mipLevel);

        const nsUInt32 targetWidth = numBlocksX * nsImageFormat::GetBlockWidth(targetFormat);
        const nsUInt32 targetHeight = numBlocksY * nsImageFormat::GetBlockHeight(targetFormat);

        const nsUInt64 sourceRowPitch = source.GetRowPitch(mipLevel);
        const nsUInt32 sourceBytesPerPixel = nsImageFormat::GetBitsPerPixel(sourceFormat) / 8;

        // Pad image to multiple of block size for compression
        nsImageHeader paddedSliceHeader;
        paddedSliceHeader.SetWidth(targetWidth);
        paddedSliceHeader.SetHeight(targetHeight);
        paddedSliceHeader.SetImageFormat(sourceFormat);

        nsImage paddedSlice;
        paddedSlice.ResetAndAlloc(paddedSliceHeader);

        for (nsUInt32 slice = 0; slice < source.GetDepth(mipLevel); slice++)
        {
          for (nsUInt32 y = 0; y < targetHeight; ++y)
          {
            nsUInt32 sourceY = nsMath::Min(y, sourceHeight - 1);

            memcpy(paddedSlice.GetPixelPointer<void>(0, 0, 0, 0, y), source.GetPixelPointer<void>(mipLevel, face, arrayIndex, 0, sourceY, slice),
              static_cast<size_t>(sourceRowPitch));

            for (nsUInt32 x = sourceWidth; x < targetWidth; ++x)
            {
              memcpy(paddedSlice.GetPixelPointer<void>(0, 0, 0, x, y),
                source.GetPixelPointer<void>(mipLevel, face, arrayIndex, sourceWidth - 1, sourceY, slice), sourceBytesPerPixel);
            }
          }

          nsResult result = static_cast<const nsImageConversionStepCompressBlocks*>(pStep)->CompressBlocks(paddedSlice.GetByteBlobPtr(),
            target.GetSliceView(mipLevel, face, arrayIndex, slice).GetByteBlobPtr(), numBlocksX, numBlocksY, sourceFormat, targetFormat);

          if (result.Failed())
          {
            return NS_FAILURE;
          }
        }
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsImageConversion::ConvertSingleStepDeplanarize(
  const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep)
{
  for (nsUInt32 arrayIndex = 0; arrayIndex < source.GetNumArrayIndices(); arrayIndex++)
  {
    for (nsUInt32 face = 0; face < source.GetNumFaces(); face++)
    {
      for (nsUInt32 mipLevel = 0; mipLevel < source.GetNumMipLevels(); mipLevel++)
      {
        const nsUInt32 width = target.GetWidth(mipLevel);
        const nsUInt32 height = target.GetHeight(mipLevel);

        nsHybridArray<nsImageView, 2> sourcePlanes;
        for (nsUInt32 planeIndex = 0; planeIndex < source.GetPlaneCount(); ++planeIndex)
        {
          const nsUInt32 blockSizeX = nsImageFormat::GetBlockWidth(sourceFormat, planeIndex);
          const nsUInt32 blockSizeY = nsImageFormat::GetBlockHeight(sourceFormat, planeIndex);

          if (width % blockSizeX != 0 || height % blockSizeY != 0)
          {
            // Input image must be aligned to block dimensions already.
            return NS_FAILURE;
          }

          sourcePlanes.PushBack(source.GetPlaneView(mipLevel, face, arrayIndex, planeIndex));
        }

        if (static_cast<const nsImageConversionStepDeplanarize*>(pStep)
              ->ConvertPixels(sourcePlanes, target.GetSubImageView(mipLevel, face, arrayIndex), width, height, sourceFormat, targetFormat)
              .Failed())
        {
          return NS_FAILURE;
        }
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsImageConversion::ConvertSingleStepPlanarize(
  const nsImageView& source, nsImage& target, nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat, const nsImageConversionStep* pStep)
{
  for (nsUInt32 arrayIndex = 0; arrayIndex < source.GetNumArrayIndices(); arrayIndex++)
  {
    for (nsUInt32 face = 0; face < source.GetNumFaces(); face++)
    {
      for (nsUInt32 mipLevel = 0; mipLevel < source.GetNumMipLevels(); mipLevel++)
      {
        const nsUInt32 width = target.GetWidth(mipLevel);
        const nsUInt32 height = target.GetHeight(mipLevel);

        nsHybridArray<nsImage, 2> targetPlanes;
        for (nsUInt32 planeIndex = 0; planeIndex < target.GetPlaneCount(); ++planeIndex)
        {
          const nsUInt32 blockSizeX = nsImageFormat::GetBlockWidth(targetFormat, planeIndex);
          const nsUInt32 blockSizeY = nsImageFormat::GetBlockHeight(targetFormat, planeIndex);

          if (width % blockSizeX != 0 || height % blockSizeY != 0)
          {
            // Input image must be aligned to block dimensions already.
            return NS_FAILURE;
          }

          targetPlanes.PushBack(target.GetPlaneView(mipLevel, face, arrayIndex, planeIndex));
        }

        if (static_cast<const nsImageConversionStepPlanarize*>(pStep)
              ->ConvertPixels(source.GetSubImageView(mipLevel, face, arrayIndex), targetPlanes, width, height, sourceFormat, targetFormat)
              .Failed())
        {
          return NS_FAILURE;
        }
      }
    }
  }

  return NS_SUCCESS;
}

bool nsImageConversion::IsConvertible(nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat)
{
  NS_LOCK(s_conversionTableLock);

  if (!s_conversionTableValid)
  {
    RebuildConversionTable();
  }

  nsUInt32 tableIndex = MakeKey(sourceFormat, targetFormat);
  return s_conversionTable.Contains(tableIndex);
}

nsImageFormat::Enum nsImageConversion::FindClosestCompatibleFormat(
  nsImageFormat::Enum format, nsArrayPtr<const nsImageFormat::Enum> compatibleFormats)
{
  NS_LOCK(s_conversionTableLock);

  if (!s_conversionTableValid)
  {
    RebuildConversionTable();
  }

  TableEntry bestEntry;
  nsImageFormat::Enum bestFormat = nsImageFormat::UNKNOWN;

  for (nsUInt32 targetIndex = 0; targetIndex < nsUInt32(compatibleFormats.GetCount()); targetIndex++)
  {
    nsUInt32 tableIndex = MakeKey(format, compatibleFormats[targetIndex]);
    TableEntry candidate;
    if (s_conversionTable.TryGetValue(tableIndex, candidate) && candidate < bestEntry)
    {
      bestEntry = candidate;
      bestFormat = compatibleFormats[targetIndex];
    }
  }

  return bestFormat;
}
