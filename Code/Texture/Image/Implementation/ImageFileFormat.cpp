#include <Texture/TexturePCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/Formats/ImageFileFormat.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsImageFileFormat);

nsImageFileFormat* nsImageFileFormat::GetReaderFormat(nsStringView sExtension)
{
  for (nsImageFileFormat* pFormat = nsImageFileFormat::GetFirstInstance(); pFormat; pFormat = pFormat->GetNextInstance())
  {
    if (pFormat->CanReadFileType(sExtension))
    {
      return pFormat;
    }
  }

  return nullptr;
}

nsImageFileFormat* nsImageFileFormat::GetWriterFormat(nsStringView sExtension)
{
  for (nsImageFileFormat* pFormat = nsImageFileFormat::GetFirstInstance(); pFormat; pFormat = pFormat->GetNextInstance())
  {
    if (pFormat->CanWriteFileType(sExtension))
    {
      return pFormat;
    }
  }

  return nullptr;
}

nsResult nsImageFileFormat::ReadImageHeader(nsStringView sFileName, nsImageHeader& ref_header)
{
  NS_LOG_BLOCK("Read Image Header", sFileName);

  NS_PROFILE_SCOPE(nsPathUtils::GetFileNameAndExtension(sFileName).GetStartPointer());

  nsFileReader reader;
  if (reader.Open(sFileName) == NS_FAILURE)
  {
    nsLog::Warning("Failed to open image file '{0}'", nsArgSensitive(sFileName, "File"));
    return NS_FAILURE;
  }

  nsStringView it = nsPathUtils::GetFileExtension(sFileName);

  if (nsImageFileFormat* pFormat = nsImageFileFormat::GetReaderFormat(it.GetStartPointer()))
  {
    if (pFormat->ReadImageHeader(reader, ref_header, it.GetStartPointer()) != NS_SUCCESS)
    {
      nsLog::Warning("Failed to read image file '{0}'", nsArgSensitive(sFileName, "File"));
      return NS_FAILURE;
    }

    return NS_SUCCESS;
  }

  nsLog::Warning("No known image file format for extension '{0}'", it);
  return NS_FAILURE;
}
