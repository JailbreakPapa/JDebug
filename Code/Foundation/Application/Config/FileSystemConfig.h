#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

class WD_FOUNDATION_DLL wdApplicationFileSystemConfig
{
public:
  static constexpr const wdStringView s_sConfigFile = ":project/RuntimeConfigs/DataDirectories.ddl"_wdsv;

  wdResult Save(wdStringView sPath = s_sConfigFile);
  void Load(wdStringView sPath = s_sConfigFile);

  /// \brief Sets up the data directories that were configured or loaded into this object
  void Apply();

  /// \brief Removes all data directories that were set up by any call to wdApplicationFileSystemConfig::Apply()
  static void Clear();

  wdResult CreateDataDirStubFiles();

  struct DataDirConfig
  {
    wdString m_sDataDirSpecialPath;
    wdString m_sRootName;
    bool m_bWritable;            ///< Whether the directory is going to be mounted for writing
    bool m_bHardCodedDependency; ///< If set to true, this indicates that it may not be removed by the user (in a config dialog)

    DataDirConfig()
    {
      m_bWritable = false;
      m_bHardCodedDependency = false;
    }

    bool operator==(const DataDirConfig& rhs) const
    {
      return m_bWritable == rhs.m_bWritable && m_sDataDirSpecialPath == rhs.m_sDataDirSpecialPath && m_sRootName == rhs.m_sRootName;
    }
  };

  bool operator==(const wdApplicationFileSystemConfig& rhs) const { return m_DataDirs == rhs.m_DataDirs; }

  wdHybridArray<DataDirConfig, 4> m_DataDirs;
};


typedef wdApplicationFileSystemConfig::DataDirConfig wdApplicationFileSystemConfig_DataDirConfig;

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdApplicationFileSystemConfig);
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdApplicationFileSystemConfig_DataDirConfig);
