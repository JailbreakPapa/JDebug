#include <Texture/TexturePCH.h>

#if NS_ENABLED(NS_PLATFORM_LINUX)

#  include <Texture/DirectXTex/BC.h>
#  include <Texture/Image/ImageConversion.h>

#  include <Foundation/Threading/TaskSystem.h>

  nsImageConversionEntry g_DXTexCpuConversions[] = {
  nsImageConversionEntry(nsImageFormat::R32G32B32A32_FLOAT, nsImageFormat::BC6H_UF16, nsImageConversionFlags::Default),

  nsImageConversionEntry(nsImageFormat::R8G8B8A8_UNORM, nsImageFormat::BC1_UNORM, nsImageConversionFlags::Default),
  nsImageConversionEntry(nsImageFormat::R8G8B8A8_UNORM, nsImageFormat::BC7_UNORM, nsImageConversionFlags::Default),

  nsImageConversionEntry(nsImageFormat::R8G8B8A8_UNORM_SRGB, nsImageFormat::BC1_UNORM_SRGB, nsImageConversionFlags::Default),
  nsImageConversionEntry(nsImageFormat::R8G8B8A8_UNORM_SRGB, nsImageFormat::BC7_UNORM_SRGB, nsImageConversionFlags::Default),
};

class nsImageConversion_CompressDxTexCpu : public nsImageConversionStepCompressBlocks
{
public:
  virtual nsArrayPtr<const nsImageConversionEntry> GetSupportedConversions() const override
  {
    return g_DXTexCpuConversions;
  }

  virtual nsResult CompressBlocks(nsConstByteBlobPtr source, nsByteBlobPtr target, nsUInt32 numBlocksX, nsUInt32 numBlocksY,
    nsImageFormat::Enum sourceFormat, nsImageFormat::Enum targetFormat) const override
  {
    if (targetFormat == nsImageFormat::BC7_UNORM || targetFormat == nsImageFormat::BC7_UNORM_SRGB)
    {
      const nsUInt32 srcStride = numBlocksX * 4 * 4;
      const nsUInt32 targetStride = numBlocksX * 16;

      nsTaskSystem::ParallelForIndexed(0, numBlocksY, [srcStride, targetStride, source, target, numBlocksX](nsUInt32 startIndex, nsUInt32 endIndex)
        {
        const nsUInt8* srcIt = source.GetPtr() + srcStride * startIndex * 4;
        nsUInt8* targetIt = target.GetPtr() + targetStride * startIndex;
        for (nsUInt32 blockY = startIndex; blockY < endIndex; ++blockY)
        {
          for (nsUInt32 blockX = 0; blockX < numBlocksX; ++blockX)
          {
            DirectX::XMVECTOR temp[16];
            for (nsUInt32 y = 0; y < 4; y++)
            {
              for (nsUInt32 x = 0; x < 4; x++)
              {
                const nsUInt8* pixel = srcIt + y * srcStride + x * 4;
                temp[y * 4 + x] = DirectX::XMVectorSet(pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f, pixel[3] / 255.0f);
              }
            }
            DirectX::D3DXEncodeBC7(targetIt, temp, 0);

            srcIt += 4 * 4;
            targetIt += 16;
          }
          srcIt += 3 * srcStride;
        } });

      return NS_SUCCESS;
    }
    else if (targetFormat == nsImageFormat::BC1_UNORM || targetFormat == nsImageFormat::BC1_UNORM_SRGB)
    {
      const nsUInt32 srcStride = numBlocksX * 4 * 4;
      const nsUInt32 targetStride = numBlocksX * 8;

      nsTaskSystem::ParallelForIndexed(0, numBlocksY, [srcStride, targetStride, source, target, numBlocksX](nsUInt32 startIndex, nsUInt32 endIndex)
        {
        const nsUInt8* srcIt = source.GetPtr() + srcStride * startIndex * 4;
        nsUInt8* targetIt = target.GetPtr() + targetStride * startIndex;
        for (nsUInt32 blockY = startIndex; blockY < endIndex; ++blockY)
        {
          for (nsUInt32 blockX = 0; blockX < numBlocksX; ++blockX)
          {
            DirectX::XMVECTOR temp[16];
            for (nsUInt32 y = 0; y < 4; y++)
            {
              for (nsUInt32 x = 0; x < 4; x++)
              {
                const nsUInt8* pixel = srcIt + y * srcStride + x * 4;
                temp[y * 4 + x] = DirectX::XMVectorSet(pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f, pixel[3] / 255.0f);
              }
            }
            DirectX::D3DXEncodeBC1(targetIt, temp, 1.0f, 0);

            srcIt += 4 * 4;
            targetIt += 8;
          }
          srcIt += 3 * srcStride;
        } });

      return NS_SUCCESS;
    }
    else if (targetFormat == nsImageFormat::BC6H_UF16)
    {
      const nsUInt32 srcStride = numBlocksX * 4 * 4 * sizeof(float);
      const nsUInt32 targetStride = numBlocksX * 16;

      nsTaskSystem::ParallelForIndexed(0, numBlocksY, [srcStride, targetStride, source, target, numBlocksX](nsUInt32 startIndex, nsUInt32 endIndex)
        {
        const nsUInt8* srcIt = source.GetPtr() + srcStride * startIndex * 4;
        nsUInt8* targetIt = target.GetPtr() + targetStride * startIndex;
        for (nsUInt32 blockY = startIndex; blockY < endIndex; ++blockY)
        {
          for (nsUInt32 blockX = 0; blockX < numBlocksX; ++blockX)
          {
            DirectX::XMVECTOR temp[16];
            for (nsUInt32 y = 0; y < 4; y++)
            {
              for (nsUInt32 x = 0; x < 4; x++)
              {
                const float* pixel = reinterpret_cast<const float*>(srcIt + y * srcStride + x * 4 * sizeof(float));
                temp[y * 4 + x] = DirectX::XMVectorSet(pixel[0], pixel[1], pixel[2], pixel[3]);
              }
            }
            DirectX::D3DXEncodeBC6HU(targetIt, temp, 0);

            srcIt += 4 * 4 * sizeof(float);
            targetIt += 16;
          }
          srcIt += 3 * srcStride;
        } });

      return NS_SUCCESS;
    }

    return NS_FAILURE;
  }
};

// NS_STATICLINK_FORCE
static nsImageConversion_CompressDxTexCpu s_conversion_compressDxTexCpu;

#endif

NS_STATICLINK_FILE(Texture, Texture_Image_Conversions_DXTexCpuConversions);
