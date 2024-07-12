#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

class NS_FOUNDATION_DLL nsApplicationFileSystemConfig
{
public:
  static constexpr const nsStringView s_sConfigFile = ":project/RuntimeConfigs/DataDirectories.ddl"_nssv;

  nsResult Save(nsStringView sPath = s_sConfigFile);
  void Load(nsStringView sPath = s_sConfigFile);

  /// \brief Sets up the data directories that were configured or loaded into this object
  void Apply();

  /// \brief Removes all data directories that were set up by any call to nsApplicationFileSystemConfig::Apply()
  static void Clear();

  nsResult CreateDataDirStubFiles();

  struct DataDirConfig
  {
    nsString m_sDataDirSpecialPath;
    nsString m_sRootName;
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

  bool operator==(const nsApplicationFileSystemConfig& rhs) const { return m_DataDirs == rhs.m_DataDirs; }

  nsHybridArray<DataDirConfig, 4> m_DataDirs;
};


using nsApplicationFileSystemConfig_DataDirConfig = nsApplicationFileSystemConfig::DataDirConfig;

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsApplicationFileSystemConfig);
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsApplicationFileSystemConfig_DataDirConfig);
