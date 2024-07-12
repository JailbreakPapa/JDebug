/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <TexConv/TexConvPCH.h>

#include <TexConv/TexConv.h>

#include <Foundation/Utilities/CommandLineOptions.h>

nsCommandLineOptionPath opt_Out("_TexConv", "-out",
  "Absolute path to main output file.\n\
   ext = tga, dds, nsTexture2D, nsTexture3D, nsTextureCube or nsTextureAtlas.",
  "");


nsCommandLineOptionDoc opt_In("_TexConv", "-inX", "\"File\"",
  "Specifies input image X.\n\
   X = 0 .. 63, e.g. -in0, -in1, etc.\n\
   If X is not given, X equals 0.",
  "");

nsCommandLineOptionDoc opt_Channels("_TexConv", "-r;-rg;-rgb;-rgba", "inX.rgba",
  "\
  Specifies how many output channels are used (1 - 4) and from which input image to take the data.\n\
  Examples:\n\
  -rgba in0 -> Output has 4 channels, all taken from input image 0.\n\
  -rgb in0 -> Output has 3 channels, all taken from input image 0.\n\
  -rgb in0 -a in1.r -> Output has 4 channels, RGB taken from input image 0 (RGB) Alpha taken from input 1 (Red).\n\
  -rgb in0.bgr -> Output has 3 channels, taken from image 0 and swapped blue and red.\n\
  -r in0.r -g in1.r -b in2.r -a in3.r -> Output has 4 channels, each one taken from another input image (Red).\n\
  -rgb0 in0 -rgb1 in1 -rgb2 in2 -rgb3 in3 -rgb4 in4 -rgb5 in5 -> Output has 3 channels and six faces (-type Cubemap), built from 6 images.\n\
",
  "");

nsCommandLineOptionBool opt_MipsPreserveCoverage("_TexConv", "-mipsPreserveCoverage", "Whether to preserve alpha-coverage in mipmaps for alpha-tested geometry.", false);

nsCommandLineOptionBool opt_FlipHorz("_TexConv", "-flip_horz", "Whether to flip the output horizontally.", false);

nsCommandLineOptionBool opt_Dilate("_TexConv", "-dilate", "Dilate/smear color from opaque areas into transparent areas.", false);

nsCommandLineOptionInt opt_DilateStrength("_TexConv", "-dilateStrength", "How many pixels to smear the image, if -dilate is enabled.", 8, 1, 255);

nsCommandLineOptionBool opt_Premulalpha("_TexConv", "-premulalpha", "Whether to multiply the alpha channel into the RGB channels.", false);

nsCommandLineOptionInt opt_ThumbnailRes("_TexConv", "-thumbnailRes", "Thumbnail resolution. Should be a power-of-two.", 0, 32, 1024);

nsCommandLineOptionPath opt_ThumbnailOut("_TexConv", "-thumbnailOut",
  "\
  Path to 2D thumbnail file.\n\
  ext = tga, jpg, png\n\
",
  "");

nsCommandLineOptionPath opt_LowOut("_TexConv", "-lowOut",
  "\
  Path to low-resolution output file.\n\
  ext = Same as main output\n\
",
  "");

nsCommandLineOptionInt opt_LowMips("_TexConv", "-lowMips", "Number of mipmaps to use from main result as low-res data.", 0, 0, 8);

nsCommandLineOptionInt opt_MinRes("_TexConv", "-minRes", "The minimum resolution allowed for the output.", 16, 4, 8 * 1024);

nsCommandLineOptionInt opt_MaxRes("_TexConv", "-maxRes", "The maximum resolution allowed for the output.", 1024 * 8, 4, 16 * 1024);

nsCommandLineOptionInt opt_Downscale("_TexConv", "-downscale", "How often to half the input texture resolution.", 0, 0, 10);

nsCommandLineOptionFloat opt_MipsAlphaThreshold("_TexConv", "-mipsAlphaThreshold", "Alpha threshold used by renderer for alpha-testing, when alpha-coverage should be preserved.", 0.5f, 0.01f, 0.99f);

nsCommandLineOptionFloat opt_HdrExposure("_TexConv", "-hdrExposure", "For scaling HDR image brightness up or down.", 0.0f, -20.0f, +20.0f);

nsCommandLineOptionFloat opt_Clamp("_TexConv", "-clamp", "Input values will be clamped to [-value ; +value].", 64000.0f, -64000.0f, 64000.0f);

nsCommandLineOptionInt opt_AssetVersion("_TexConv", "-assetVersion", "Asset version number to embed in ns specific output formats", 0, 1, 0xFFFF);

nsCommandLineOptionString opt_AssetHashLow("_TexConv", "-assetHashLow", "Low part of a 64 bit asset hash value.\n\
Has to be specified as a HEX value.\n\
Required to be non-zero when using ns specific output formats.\n\
Example: -assetHashLow 0xABCDABCD",
  "");

nsCommandLineOptionString opt_AssetHashHigh("_TexConv", "-assetHashHigh", "High part of a 64 bit asset hash value.\n\
Has to be specified as a HEX value.\n\
Required to be non-zero when using ns specific output formats.\n\
Example: -assetHashHigh 0xABCDABCD",
  "");

nsCommandLineOptionEnum opt_Type("_TexConv", "-type", "The type of output to generate.", "2D = 1 | Volume = 2 | Cubemap = 3 | Atlas = 4", 1);

nsCommandLineOptionEnum opt_Compression("_TexConv", "-compression", "Compression strength for output format.", "Medium = 1 | High = 2 | None = 0", 1);

nsCommandLineOptionEnum opt_Usage("_TexConv", "-usage", "What type of data the image contains. Affects which final output format is used and how mipmaps are generated.", "Auto = 0 | Color = 1 | Linear = 2 | HDR = 3 | NormalMap = 4 | NormalMap_Inverted = 5 | BumpMap = 6", 0);

nsCommandLineOptionEnum opt_Mipmaps("_TexConv", "-mipmaps", "Whether to generate mipmaps and with which algorithm.", "None = 0 |Linear = 1 | Kaiser = 2", 1);

nsCommandLineOptionEnum opt_AddressU("_TexConv", "-addressU", "Which texture address mode to use along U. Only supported by ns-specific output formats.", "Repeat = 0 | Clamp = 1 | ClampBorder = 2 | Mirror = 3", 0);
nsCommandLineOptionEnum opt_AddressV("_TexConv", "-addressV", "Which texture address mode to use along V. Only supported by ns-specific output formats.", "Repeat = 0 | Clamp = 1 | ClampBorder = 2 | Mirror = 3", 0);
nsCommandLineOptionEnum opt_AddressW("_TexConv", "-addressW", "Which texture address mode to use along W. Only supported by ns-specific output formats.", "Repeat = 0 | Clamp = 1 | ClampBorder = 2 | Mirror = 3", 0);

nsCommandLineOptionEnum opt_Filter("_TexConv", "-filter", "Which texture filter mode to use at runtime. Only supported by ns-specific output formats.", "Default = 9 | Lowest = 7 | Low = 8 | High = 10 | Highest = 11 | Nearest = 0 | Linear = 1 | Trilinear = 2 | Aniso2x = 3 | Aniso4x = 4 | Aniso8x = 5 | Aniso16x = 6", 9);

nsCommandLineOptionEnum opt_BumpMapFilter("_TexConv", "-bumpMapFilter", "Filter used to approximate the x/y bump map gradients.", "Finite = 0 | Sobel = 1 | Scharr = 2", 0);

nsCommandLineOptionEnum opt_Platform("_TexConv", "-platform", "What platform to generate the textures for.", "PC | Android", 0);

nsResult nsTexConv::ParseCommandLine()
{
  if (nsCommandLineOption::LogAvailableOptions(nsCommandLineOption::LogAvailableModes::IfHelpRequested, "_TexConv"))
    return NS_FAILURE;

  NS_SUCCEED_OR_RETURN(ParseOutputFiles());
  NS_SUCCEED_OR_RETURN(DetectOutputFormat());

  NS_SUCCEED_OR_RETURN(ParseOutputType());
  NS_SUCCEED_OR_RETURN(ParseAssetHeader());
  NS_SUCCEED_OR_RETURN(ParseTargetPlatform());
  NS_SUCCEED_OR_RETURN(ParseCompressionMode());
  NS_SUCCEED_OR_RETURN(ParseUsage());
  NS_SUCCEED_OR_RETURN(ParseMipmapMode());
  NS_SUCCEED_OR_RETURN(ParseWrapModes());
  NS_SUCCEED_OR_RETURN(ParseFilterModes());
  NS_SUCCEED_OR_RETURN(ParseResolutionModifiers());
  NS_SUCCEED_OR_RETURN(ParseMiscOptions());
  NS_SUCCEED_OR_RETURN(ParseInputFiles());
  NS_SUCCEED_OR_RETURN(ParseChannelMappings());
  NS_SUCCEED_OR_RETURN(ParseBumpMapFilter());

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseOutputType()
{
  if (m_sOutputFile.IsEmpty())
  {
    m_Processor.m_Descriptor.m_OutputType = nsTexConvOutputType::None;
    return NS_SUCCESS;
  }

  nsInt32 value = opt_Type.GetOptionValue(nsCommandLineOption::LogMode::Always);

  m_Processor.m_Descriptor.m_OutputType = static_cast<nsTexConvOutputType::Enum>(value);

  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Texture2D)
  {
    if (!m_bOutputSupports2D)
    {
      nsLog::Error("2D textures are not supported by the chosen output file format.");
      return NS_FAILURE;
    }
  }
  else if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Cubemap)
  {
    if (!m_bOutputSupportsCube)
    {
      nsLog::Error("Cubemap textures are not supported by the chosen output file format.");
      return NS_FAILURE;
    }
  }
  else if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Atlas)
  {
    if (!m_bOutputSupportsAtlas)
    {
      nsLog::Error("Atlas textures are not supported by the chosen output file format.");
      return NS_FAILURE;
    }

    if (!ParseFile("-atlasDesc", m_Processor.m_Descriptor.m_sTextureAtlasDescFile))
      return NS_FAILURE;
  }
  else if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Volume)
  {
    if (!m_bOutputSupports3D)
    {
      nsLog::Error("Volume textures are not supported by the chosen output file format.");
      return NS_FAILURE;
    }
  }
  else
  {
    NS_ASSERT_NOT_IMPLEMENTED;
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseInputFiles()
{
  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Atlas)
    return NS_SUCCESS;

  nsStringBuilder tmp, res;
  const auto pCmd = nsCommandLineUtils::GetGlobalInstance();

  auto& files = m_Processor.m_Descriptor.m_InputFiles;

  for (nsUInt32 i = 0; i < 64; ++i)
  {
    tmp.Format("-in{0}", i);

    res = pCmd->GetAbsolutePathOption(tmp);

    // stop once an option was not found
    if (res.IsEmpty())
      break;

    files.EnsureCount(i + 1);
    files[i] = res;
  }

  // if no numbered inputs were given, try '-in', ignore it otherwise
  if (files.IsEmpty())
  {
    // short version for -in1
    res = pCmd->GetAbsolutePathOption("-in");

    if (!res.IsEmpty())
    {
      files.PushBack(res);
    }
  }

  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Cubemap)
  {
    // 0 = +X = Right
    // 1 = -X = Left
    // 2 = +Y = Top
    // 3 = -Y = Bottom
    // 4 = +Z = Front
    // 5 = -Z = Back

    if (files.IsEmpty() && (pCmd->GetOptionIndex("-right") != -1 || pCmd->GetOptionIndex("-px") != -1))
    {
      files.SetCount(6);

      files[0] = pCmd->GetAbsolutePathOption("-right", 0, files[0]);
      files[1] = pCmd->GetAbsolutePathOption("-left", 0, files[1]);
      files[2] = pCmd->GetAbsolutePathOption("-top", 0, files[2]);
      files[3] = pCmd->GetAbsolutePathOption("-bottom", 0, files[3]);
      files[4] = pCmd->GetAbsolutePathOption("-front", 0, files[4]);
      files[5] = pCmd->GetAbsolutePathOption("-back", 0, files[5]);

      files[0] = pCmd->GetAbsolutePathOption("-px", 0, files[0]);
      files[1] = pCmd->GetAbsolutePathOption("-nx", 0, files[1]);
      files[2] = pCmd->GetAbsolutePathOption("-py", 0, files[2]);
      files[3] = pCmd->GetAbsolutePathOption("-ny", 0, files[3]);
      files[4] = pCmd->GetAbsolutePathOption("-pz", 0, files[4]);
      files[5] = pCmd->GetAbsolutePathOption("-nz", 0, files[5]);
    }
  }

  for (nsUInt32 i = 0; i < files.GetCount(); ++i)
  {
    if (files[i].IsEmpty())
    {
      nsLog::Error("Input file {} is not specified", i);
      return NS_FAILURE;
    }

    nsLog::Info("Input file {}: '{}'", i, files[i]);
  }

  if (m_Processor.m_Descriptor.m_InputFiles.IsEmpty())
  {
    nsLog::Error("No input files were specified. Use \'-in \"path/to/file\"' to specify an input file. Use '-in0', '-in1' etc. to specify "
                 "multiple input files.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseOutputFiles()
{
  m_sOutputFile = opt_Out.GetOptionValue(nsCommandLineOption::LogMode::Always);

  m_sOutputThumbnailFile = opt_ThumbnailOut.GetOptionValue(nsCommandLineOption::LogMode::Always);

  if (!m_sOutputThumbnailFile.IsEmpty())
  {
    m_Processor.m_Descriptor.m_uiThumbnailOutputResolution = opt_ThumbnailRes.GetOptionValue(nsCommandLineOption::LogMode::Always);
  }

  m_sOutputLowResFile = opt_LowOut.GetOptionValue(nsCommandLineOption::LogMode::Always);

  if (!m_sOutputLowResFile.IsEmpty())
  {
    m_Processor.m_Descriptor.m_uiLowResMipmaps = opt_LowMips.GetOptionValue(nsCommandLineOption::LogMode::Always);
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseUsage()
{
  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Atlas)
    return NS_SUCCESS;

  const nsInt32 value = opt_Usage.GetOptionValue(nsCommandLineOption::LogMode::Always);

  m_Processor.m_Descriptor.m_Usage = static_cast<nsTexConvUsage::Enum>(value);
  return NS_SUCCESS;
}

nsResult nsTexConv::ParseMipmapMode()
{
  if (!m_bOutputSupportsMipmaps)
  {
    nsLog::Info("Selected output format does not support -mipmap options.");

    m_Processor.m_Descriptor.m_MipmapMode = nsTexConvMipmapMode::None;
    return NS_SUCCESS;
  }

  const nsInt32 value = opt_Mipmaps.GetOptionValue(nsCommandLineOption::LogMode::Always);

  m_Processor.m_Descriptor.m_MipmapMode = static_cast<nsTexConvMipmapMode::Enum>(value);

  m_Processor.m_Descriptor.m_bPreserveMipmapCoverage = opt_MipsPreserveCoverage.GetOptionValue(nsCommandLineOption::LogMode::Always);

  if (m_Processor.m_Descriptor.m_bPreserveMipmapCoverage)
  {
    m_Processor.m_Descriptor.m_fMipmapAlphaThreshold = opt_MipsAlphaThreshold.GetOptionValue(nsCommandLineOption::LogMode::Always);
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseTargetPlatform()
{
  nsInt32 value = opt_Platform.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified);

  m_Processor.m_Descriptor.m_TargetPlatform = static_cast<nsTexConvTargetPlatform::Enum>(value);
  return NS_SUCCESS;
}

nsResult nsTexConv::ParseCompressionMode()
{
  if (!m_bOutputSupportsCompression)
  {
    nsLog::Info("Selected output format does not support -compression options.");

    m_Processor.m_Descriptor.m_CompressionMode = nsTexConvCompressionMode::None;
    return NS_SUCCESS;
  }

  const nsInt32 value = opt_Compression.GetOptionValue(nsCommandLineOption::LogMode::Always);

  m_Processor.m_Descriptor.m_CompressionMode = static_cast<nsTexConvCompressionMode::Enum>(value);
  return NS_SUCCESS;
}

nsResult nsTexConv::ParseWrapModes()
{
  // cubemaps do not require any wrap mode settings
  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Cubemap || m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Atlas || m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::None)
    return NS_SUCCESS;

  {
    nsInt32 value = opt_AddressU.GetOptionValue(nsCommandLineOption::LogMode::Always);
    m_Processor.m_Descriptor.m_AddressModeU = static_cast<nsImageAddressMode::Enum>(value);
  }
  {
    nsInt32 value = opt_AddressV.GetOptionValue(nsCommandLineOption::LogMode::Always);
    m_Processor.m_Descriptor.m_AddressModeV = static_cast<nsImageAddressMode::Enum>(value);
  }

  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Volume)
  {
    nsInt32 value = opt_AddressW.GetOptionValue(nsCommandLineOption::LogMode::AlwaysIfSpecified);
    m_Processor.m_Descriptor.m_AddressModeW = static_cast<nsImageAddressMode::Enum>(value);
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseFilterModes()
{
  if (!m_bOutputSupportsFiltering)
  {
    nsLog::Info("Selected output format does not support -filter options.");
    return NS_SUCCESS;
  }

  nsInt32 value = opt_Filter.GetOptionValue(nsCommandLineOption::LogMode::Always);

  m_Processor.m_Descriptor.m_FilterMode = static_cast<nsTextureFilterSetting::Enum>(value);
  return NS_SUCCESS;
}

nsResult nsTexConv::ParseResolutionModifiers()
{
  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::None)
    return NS_SUCCESS;

  m_Processor.m_Descriptor.m_uiMinResolution = opt_MinRes.GetOptionValue(nsCommandLineOption::LogMode::Always);
  m_Processor.m_Descriptor.m_uiMaxResolution = opt_MaxRes.GetOptionValue(nsCommandLineOption::LogMode::Always);
  m_Processor.m_Descriptor.m_uiDownscaleSteps = opt_Downscale.GetOptionValue(nsCommandLineOption::LogMode::Always);

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseMiscOptions()
{
  if (m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::Texture2D || m_Processor.m_Descriptor.m_OutputType == nsTexConvOutputType::None)
  {
    m_Processor.m_Descriptor.m_bFlipHorizontal = opt_FlipHorz.GetOptionValue(nsCommandLineOption::LogMode::Always);

    m_Processor.m_Descriptor.m_bPremultiplyAlpha = opt_Premulalpha.GetOptionValue(nsCommandLineOption::LogMode::Always);

    if (opt_Dilate.GetOptionValue(nsCommandLineOption::LogMode::Always))
    {
      m_Processor.m_Descriptor.m_uiDilateColor = static_cast<nsUInt8>(opt_DilateStrength.GetOptionValue(nsCommandLineOption::LogMode::Always));
    }
  }

  if (m_Processor.m_Descriptor.m_Usage == nsTexConvUsage::Hdr)
  {
    m_Processor.m_Descriptor.m_fHdrExposureBias = opt_HdrExposure.GetOptionValue(nsCommandLineOption::LogMode::Always);
  }

  m_Processor.m_Descriptor.m_fMaxValue = opt_Clamp.GetOptionValue(nsCommandLineOption::LogMode::Always);

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseAssetHeader()
{
  const nsStringView ext = nsPathUtils::GetFileExtension(m_sOutputFile);

  if (!ext.StartsWith_NoCase("ns"))
    return NS_SUCCESS;

  m_Processor.m_Descriptor.m_uiAssetVersion = (nsUInt16)opt_AssetVersion.GetOptionValue(nsCommandLineOption::LogMode::Always);

  nsUInt32 uiHashLow = 0;
  nsUInt32 uiHashHigh = 0;
  if (nsConversionUtils::ConvertHexStringToUInt32(opt_AssetHashLow.GetOptionValue(nsCommandLineOption::LogMode::Always), uiHashLow).Failed() ||
      nsConversionUtils::ConvertHexStringToUInt32(opt_AssetHashHigh.GetOptionValue(nsCommandLineOption::LogMode::Always), uiHashHigh).Failed())
  {
    nsLog::Error("'-assetHashLow 0xHEX32' and '-assetHashHigh 0xHEX32' have not been specified correctly.");
    return NS_FAILURE;
  }

  m_Processor.m_Descriptor.m_uiAssetHash = (static_cast<nsUInt64>(uiHashHigh) << 32) | static_cast<nsUInt64>(uiHashLow);

  if (m_Processor.m_Descriptor.m_uiAssetHash == 0)
  {
    nsLog::Error("'-assetHashLow 0xHEX32' and '-assetHashHigh 0xHEX32' have not been specified correctly.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsTexConv::ParseBumpMapFilter()
{
  const nsInt32 value = opt_BumpMapFilter.GetOptionValue(nsCommandLineOption::LogMode::Always);

  m_Processor.m_Descriptor.m_BumpMapFilter = static_cast<nsTexConvBumpMapFilter::Enum>(value);
  return NS_SUCCESS;
}
