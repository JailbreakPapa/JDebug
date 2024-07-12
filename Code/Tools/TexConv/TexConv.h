/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Application/Application.h>

class nsStreamWriter;

class nsTexConv : public nsApplication
{
public:
  using SUPER = nsApplication;

  struct KeyEnumValuePair
  {
    KeyEnumValuePair(nsStringView sKey, nsInt32 iVal)
      : m_sKey(sKey)
      , m_iEnumValue(iVal)
    {
    }

    nsStringView m_sKey;
    nsInt32 m_iEnumValue = -1;
  };

  nsTexConv();

public:
  virtual Execution Run() override;
  virtual nsResult BeforeCoreSystemsStartup() override;
  virtual void AfterCoreSystemsStartup() override;
  virtual void BeforeCoreSystemsShutdown() override;

  nsResult ParseCommandLine();
  nsResult ParseOutputType();
  nsResult DetectOutputFormat();
  nsResult ParseInputFiles();
  nsResult ParseOutputFiles();
  nsResult ParseChannelMappings();
  nsResult ParseChannelSliceMapping(nsInt32 iSlice);
  nsResult ParseChannelMappingConfig(nsTexConvChannelMapping& out_mapping, nsStringView sCfg, nsInt32 iChannelIndex, bool bSingleChannel);
  nsResult ParseUsage();
  nsResult ParseMipmapMode();
  nsResult ParseTargetPlatform();
  nsResult ParseCompressionMode();
  nsResult ParseWrapModes();
  nsResult ParseFilterModes();
  nsResult ParseResolutionModifiers();
  nsResult ParseMiscOptions();
  nsResult ParseAssetHeader();
  nsResult ParseBumpMapFilter();

  nsResult ParseUIntOption(nsStringView sOption, nsInt32 iMinValue, nsInt32 iMaxValue, nsUInt32& ref_uiResult) const;
  nsResult ParseStringOption(nsStringView sOption, const nsDynamicArray<KeyEnumValuePair>& allowed, nsInt32& ref_iResult) const;
  void PrintOptionValues(nsStringView sOption, const nsDynamicArray<KeyEnumValuePair>& allowed) const;
  void PrintOptionValuesHelp(nsStringView sOption, const nsDynamicArray<KeyEnumValuePair>& allowed) const;
  bool ParseFile(nsStringView sOption, nsString& ref_sResult) const;

  bool IsTexFormat() const;
  nsResult WriteTexFile(nsStreamWriter& inout_stream, const nsImage& image);
  nsResult WriteOutputFile(nsStringView sFile, const nsImage& image);

private:
  nsString m_sOutputFile;
  nsString m_sOutputThumbnailFile;
  nsString m_sOutputLowResFile;

  bool m_bOutputSupports2D = false;
  bool m_bOutputSupports3D = false;
  bool m_bOutputSupportsCube = false;
  bool m_bOutputSupportsAtlas = false;
  bool m_bOutputSupportsMipmaps = false;
  bool m_bOutputSupportsFiltering = false;
  bool m_bOutputSupportsCompression = false;

  nsTexConvProcessor m_Processor;
};
