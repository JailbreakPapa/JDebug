/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <TexConv/TexConvPCH.h>

#include <Core/Assets/AssetFileHeader.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <TexConv/TexConv.h>
#include <Texture/Image/Formats/DdsFileFormat.h>
#include <Texture/Image/Formats/StbImageFileFormats.h>
#include <Texture/ezTexFormat/ezTexFormat.h>

nsTexConv::nsTexConv()
  : nsApplication("TexConv")
{
}

nsResult nsTexConv::BeforeCoreSystemsStartup()
{
  nsStartup::AddApplicationTag("tool");
  nsStartup::AddApplicationTag("texconv");

  return SUPER::BeforeCoreSystemsStartup();
}

void nsTexConv::AfterCoreSystemsStartup()
{
  nsFileSystem::AddDataDirectory("", "App", ":", nsFileSystem::AllowWrites).IgnoreResult();

  nsGlobalLog::AddLogWriter(nsLogWriter::Console::LogMessageHandler);
  nsGlobalLog::AddLogWriter(nsLogWriter::VisualStudio::LogMessageHandler);
}

void nsTexConv::BeforeCoreSystemsShutdown()
{
  nsGlobalLog::RemoveLogWriter(nsLogWriter::Console::LogMessageHandler);
  nsGlobalLog::RemoveLogWriter(nsLogWriter::VisualStudio::LogMessageHandler);

  SUPER::BeforeCoreSystemsShutdown();
}

nsResult nsTexConv::DetectOutputFormat()
{
  if (m_sOutputFile.IsEmpty())
  {
    m_Processor.m_Descriptor.m_OutputType = nsTexConvOutputType::None;
    return NS_SUCCESS;
  }

  nsStringBuilder sExt = nsPathUtils::GetFileExtension(m_sOutputFile);
  sExt.ToUpper();

  if (sExt == "DDS")
  {
    m_bOutputSupports2D = true;
    m_bOutputSupports3D = true;
    m_bOutputSupportsCube = true;
    m_bOutputSupportsAtlas = false;
    m_bOutputSupportsMipmaps = true;
    m_bOutputSupportsFiltering = false;
    m_bOutputSupportsCompression = true;
    return NS_SUCCESS;
  }
  if (sExt == "TGA" || sExt == "PNG")
  {
    m_bOutputSupports2D = true;
    m_bOutputSupports3D = false;
    m_bOutputSupportsCube = false;
    m_bOutputSupportsAtlas = false;
    m_bOutputSupportsMipmaps = false;
    m_bOutputSupportsFiltering = false;
    m_bOutputSupportsCompression = false;
    return NS_SUCCESS;
  }
  if (sExt == "NSTEXTURE2D" || sExt == "nsTexture2D")
  {
    m_bOutputSupports2D = true;
    m_bOutputSupports3D = false;
    m_bOutputSupportsCube = false;
    m_bOutputSupportsAtlas = false;
    m_bOutputSupportsMipmaps = true;
    m_bOutputSupportsFiltering = true;
    m_bOutputSupportsCompression = true;
    return NS_SUCCESS;
  }
  if (sExt == "NSTEXTURE3D")
  {
    m_bOutputSupports2D = false;
    m_bOutputSupports3D = true;
    m_bOutputSupportsCube = false;
    m_bOutputSupportsAtlas = false;
    m_bOutputSupportsMipmaps = true;
    m_bOutputSupportsFiltering = true;
    m_bOutputSupportsCompression = true;
    return NS_SUCCESS;
  }
  if (sExt == "NSTEXTURECUBE")
  {
    m_bOutputSupports2D = false;
    m_bOutputSupports3D = false;
    m_bOutputSupportsCube = true;
    m_bOutputSupportsAtlas = false;
    m_bOutputSupportsMipmaps = true;
    m_bOutputSupportsFiltering = true;
    m_bOutputSupportsCompression = true;
    return NS_SUCCESS;
  }
  if (sExt == "NSTEXTUREATLAS")
  {
    m_bOutputSupports2D = false;
    m_bOutputSupports3D = false;
    m_bOutputSupportsCube = false;
    m_bOutputSupportsAtlas = true;
    m_bOutputSupportsMipmaps = true;
    m_bOutputSupportsFiltering = true;
    m_bOutputSupportsCompression = true;
    return NS_SUCCESS;
  }
  if (sExt == "NSIMAGEDATA")
  {
    m_bOutputSupports2D = true;
    m_bOutputSupports3D = false;
    m_bOutputSupportsCube = false;
    m_bOutputSupportsAtlas = false;
    m_bOutputSupportsMipmaps = false;
    m_bOutputSupportsFiltering = false;
    m_bOutputSupportsCompression = false;
    return NS_SUCCESS;
  }

  nsLog::Error("Output file uses unsupported file format '{}'", sExt);
  return NS_FAILURE;
}

bool nsTexConv::IsTexFormat() const
{
  const nsStringView ext = nsPathUtils::GetFileExtension(m_sOutputFile);

  return ext.StartsWith_NoCase("ns");
}

nsResult nsTexConv::WriteTexFile(nsStreamWriter& inout_stream, const nsImage& image)
{
  nsAssetFileHeader asset;
  asset.SetFileHashAndVersion(m_Processor.m_Descriptor.m_uiAssetHash, m_Processor.m_Descriptor.m_uiAssetVersion);

  NS_SUCCEED_OR_RETURN(asset.Write(inout_stream));

  nsTexFormat texFormat;
  texFormat.m_bSRGB = nsImageFormat::IsSrgb(image.GetImageFormat());
  texFormat.m_AddressModeU = m_Processor.m_Descriptor.m_AddressModeU;
  texFormat.m_AddressModeV = m_Processor.m_Descriptor.m_AddressModeV;
  texFormat.m_AddressModeW = m_Processor.m_Descriptor.m_AddressModeW;
  texFormat.m_TextureFilter = m_Processor.m_Descriptor.m_FilterMode;

  texFormat.WriteTextureHeader(inout_stream);

  nsDdsFileFormat ddsWriter;
  if (ddsWriter.WriteImage(inout_stream, image, "dds").Failed())
  {
    nsLog::Error("Failed to write DDS image chunk to nsTex file.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::WriteOutputFile(nsStringView sFile, const nsImage& image)
{
  if (sFile.HasExtension("nsImageData"))
  {
    nsDeferredFileWriter file;
    file.SetOutput(sFile);

    nsAssetFileHeader asset;
    asset.SetFileHashAndVersion(m_Processor.m_Descriptor.m_uiAssetHash, m_Processor.m_Descriptor.m_uiAssetVersion);

    if (asset.Write(file).Failed())
    {
      nsLog::Error("Failed to write asset header to file.");
      return NS_FAILURE;
    }

    nsUInt8 uiVersion = 1;
    file << uiVersion;

    nsUInt8 uiFormat = 1; // 1 == PNG
    file << uiFormat;

    nsStbImageFileFormats pngWriter;
    if (pngWriter.WriteImage(file, image, "png").Failed())
    {
      nsLog::Error("Failed to write data as PNG to nsImageData file.");
      return NS_FAILURE;
    }

    return file.Close();
  }
  else if (IsTexFormat())
  {
    nsDeferredFileWriter file;
    file.SetOutput(sFile);

    NS_SUCCEED_OR_RETURN(WriteTexFile(file, image));

    return file.Close();
  }
  else
  {
    return image.SaveTo(sFile);
  }
}

nsApplication::Execution nsTexConv::Run()
{
  SetReturnCode(-1);

  if (ParseCommandLine().Failed())
    return nsApplication::Execution::Quit;

  if (m_Processor.Process().Failed())
    return nsApplication::Execution::Quit;

  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Atlas)
  {
    nsDeferredFileWriter file;
    file.SetOutput(m_sOutputFile);

    nsAssetFileHeader header;
    header.SetFileHashAndVersion(m_Processor.m_Descriptor.m_uiAssetHash, m_Processor.m_Descriptor.m_uiAssetVersion);

    header.Write(file).IgnoreResult();

    m_Processor.m_TextureAtlas.CopyToStream(file).IgnoreResult();

    if (file.Close().Succeeded())
    {
      SetReturnCode(0);
    }
    else
    {
      nsLog::Error("Failed to write atlas output image.");
    }

    return nsApplication::Execution::Quit;
  }

  if (!m_sOutputFile.IsEmpty() && m_Processor.m_OutputImage.IsValid())
  {
    if (WriteOutputFile(m_sOutputFile, m_Processor.m_OutputImage).Failed())
    {
      nsLog::Error("Failed to write main result to '{}'", m_sOutputFile);
      return nsApplication::Execution::Quit;
    }

    nsLog::Success("Wrote main result to '{}'", m_sOutputFile);
  }

  if (!m_sOutputThumbnailFile.IsEmpty() && m_Processor.m_ThumbnailOutputImage.IsValid())
  {
    if (m_Processor.m_ThumbnailOutputImage.SaveTo(m_sOutputThumbnailFile).Failed())
    {
      nsLog::Error("Failed to write thumbnail result to '{}'", m_sOutputThumbnailFile);
      return nsApplication::Execution::Quit;
    }

    nsLog::Success("Wrote thumbnail to '{}'", m_sOutputThumbnailFile);
  }

  if (!m_sOutputLowResFile.IsEmpty())
  {
    // the image may not exist, if we do not have enough mips, so make sure any old low-res file is cleaned up
    nsOSFile::DeleteFile(m_sOutputLowResFile).IgnoreResult();

    if (m_Processor.m_LowResOutputImage.IsValid())
    {
      if (WriteOutputFile(m_sOutputLowResFile, m_Processor.m_LowResOutputImage).Failed())
      {
        nsLog::Error("Failed to write low-res result to '{}'", m_sOutputLowResFile);
        return nsApplication::Execution::Quit;
      }

      nsLog::Success("Wrote low-res result to '{}'", m_sOutputLowResFile);
    }
  }

  SetReturnCode(0);
  return nsApplication::Execution::Quit;
}

NS_CONSOLEAPP_ENTRY_POINT(nsTexConv);
