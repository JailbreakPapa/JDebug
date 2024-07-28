#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererCore/Textures/Texture3DResource.h>
#include <RendererCore/Textures/TextureCubeResource.h>
#include <RendererCore/Textures/TextureLoader.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/ImageConversion.h>
#include <Texture/nsTexFormat/nsTexFormat.h>

static nsTextureResourceLoader s_TextureResourceLoader;

nsCVarFloat cvar_StreamingTextureLoadDelay("Streaming.TextureLoadDelay", 0.0f, nsCVarFlags::Save, "Artificial texture loading slowdown");

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, TextureResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsResourceManager::SetResourceTypeLoader<nsTexture2DResource>(&s_TextureResourceLoader);
    nsResourceManager::SetResourceTypeLoader<nsTexture3DResource>(&s_TextureResourceLoader);
    nsResourceManager::SetResourceTypeLoader<nsTextureCubeResource>(&s_TextureResourceLoader);
    nsResourceManager::SetResourceTypeLoader<nsRenderToTexture2DResource>(&s_TextureResourceLoader);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsResourceManager::SetResourceTypeLoader<nsTexture2DResource>(nullptr);
    nsResourceManager::SetResourceTypeLoader<nsTexture3DResource>(nullptr);
    nsResourceManager::SetResourceTypeLoader<nsTextureCubeResource>(nullptr);
    nsResourceManager::SetResourceTypeLoader<nsRenderToTexture2DResource>(nullptr);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsResourceLoadData nsTextureResourceLoader::OpenDataStream(const nsResource* pResource)
{
  LoadedData* pData = NS_DEFAULT_NEW(LoadedData);

  nsResourceLoadData res;

  // Solid Color Textures
  if (nsPathUtils::HasExtension(pResource->GetResourceID(), "color"))
  {
    nsStringBuilder sName = pResource->GetResourceID();
    sName.RemoveFileExtension();

    bool bValidColor = false;
    const nsColorGammaUB color = nsConversionUtils::GetColorByName(sName, &bValidColor);

    if (!bValidColor)
    {
      nsLog::Error("'{0}' is not a valid color name. Using 'RebeccaPurple' as fallback.", sName);
    }

    pData->m_TexFormat.m_bSRGB = true;

    nsImageHeader header;
    header.SetWidth(4);
    header.SetHeight(4);
    header.SetDepth(1);
    header.SetImageFormat(nsImageFormat::R8G8B8A8_UNORM_SRGB);
    header.SetNumMipLevels(1);
    header.SetNumFaces(1);
    pData->m_Image.ResetAndAlloc(header);
    nsUInt8* pPixels = pData->m_Image.GetPixelPointer<nsUInt8>();

    for (nsUInt32 px = 0; px < 4 * 4 * 4; px += 4)
    {
      pPixels[px + 0] = color.r;
      pPixels[px + 1] = color.g;
      pPixels[px + 2] = color.b;
      pPixels[px + 3] = color.a;
    }
  }
  else
  {
    nsFileReader File;
    if (File.Open(pResource->GetResourceID()).Failed())
      return res;

    const nsStringBuilder sAbsolutePath = File.GetFilePathAbsolute();
    res.m_sResourceDescription = File.GetFilePathRelative().GetView();

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
    {
      nsFileStats stat;
      if (nsFileSystem::GetFileStats(pResource->GetResourceID(), stat).Succeeded())
      {
        res.m_LoadedFileModificationDate = stat.m_LastModificationTime;
      }
    }
#endif

    /// In case this is not a proper asset (nsTextureXX format), this is a hack to get the SRGB information for the texture
    const nsStringBuilder sName = nsPathUtils::GetFileName(sAbsolutePath);
    pData->m_TexFormat.m_bSRGB = (sName.EndsWith_NoCase("_D") || sName.EndsWith_NoCase("_SRGB") || sName.EndsWith_NoCase("_diff"));

    if (sAbsolutePath.HasExtension("nsTexture2D") || sAbsolutePath.HasExtension("nsTexture3D") || sAbsolutePath.HasExtension("nsTextureCube") || sAbsolutePath.HasExtension("nsRenderTarget") || sAbsolutePath.HasExtension("nsLUT"))
    {
      if (LoadTexFile(File, *pData).Failed())
        return res;
    }
    else
    {
      // read whatever format, as long as nsImage supports it
      File.Close();

      if (pData->m_Image.LoadFrom(pResource->GetResourceID()).Failed())
        return res;

      if (pData->m_Image.GetImageFormat() == nsImageFormat::B8G8R8_UNORM)
      {
        /// \todo A conversion to B8G8R8X8_UNORM currently fails

        nsLog::Warning("Texture resource uses inefficient BGR format, converting to BGRX: '{0}'", sAbsolutePath);
        if (nsImageConversion::Convert(pData->m_Image, pData->m_Image, nsImageFormat::B8G8R8A8_UNORM).Failed())
          return res;
      }
    }
  }

  nsMemoryStreamWriter w(&pData->m_Storage);

  WriteTextureLoadStream(w, *pData);

  res.m_pDataStream = &pData->m_Reader;
  res.m_pCustomLoaderData = pData;

  if (cvar_StreamingTextureLoadDelay > 0)
  {
    nsThreadUtils::Sleep(nsTime::MakeFromSeconds(cvar_StreamingTextureLoadDelay));
  }

  return res;
}

void nsTextureResourceLoader::CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData)
{
  LoadedData* pData = (LoadedData*)loaderData.m_pCustomLoaderData;

  NS_DEFAULT_DELETE(pData);
}

bool nsTextureResourceLoader::IsResourceOutdated(const nsResource* pResource) const
{
  // solid color textures are never outdated
  if (nsPathUtils::HasExtension(pResource->GetResourceID(), "color"))
    return false;

  // don't try to reload a file that cannot be found
  nsStringBuilder sAbs;
  if (nsFileSystem::ResolvePath(pResource->GetResourceID(), &sAbs, nullptr).Failed())
    return false;

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)

  if (pResource->GetLoadedFileModificationTime().IsValid())
  {
    nsFileStats stat;
    if (nsFileSystem::GetFileStats(pResource->GetResourceID(), stat).Failed())
      return false;

    return !stat.m_LastModificationTime.Compare(pResource->GetLoadedFileModificationTime(), nsTimestamp::CompareMode::FileTimeEqual);
  }

#endif

  return true;
}

nsResult nsTextureResourceLoader::LoadTexFile(nsStreamReader& inout_stream, LoadedData& ref_data)
{
  // read the hash, ignore it
  nsAssetFileHeader AssetHash;
  NS_SUCCEED_OR_RETURN(AssetHash.Read(inout_stream));

  ref_data.m_TexFormat.ReadHeader(inout_stream);

  if (ref_data.m_TexFormat.m_iRenderTargetResolutionX == 0)
  {
    nsDdsFileFormat fmt;
    return fmt.ReadImage(inout_stream, ref_data.m_Image, "dds");
  }
  else
  {
    return NS_SUCCESS;
  }
}

void nsTextureResourceLoader::WriteTextureLoadStream(nsStreamWriter& w, const LoadedData& data)
{
  const nsImage* pImage = &data.m_Image;
  w.WriteBytes(&pImage, sizeof(nsImage*)).IgnoreResult();

  w << data.m_bIsFallback;
  data.m_TexFormat.WriteRenderTargetHeader(w);
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Textures_TextureLoader);
