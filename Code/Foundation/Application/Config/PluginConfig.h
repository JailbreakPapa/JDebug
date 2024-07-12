#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

class NS_FOUNDATION_DLL nsApplicationPluginConfig
{
public:
  nsApplicationPluginConfig();

  static constexpr const nsStringView s_sConfigFile = ":project/RuntimeConfigs/Plugins.ddl"_nssv;

  nsResult Save(nsStringView sConfigPath = s_sConfigFile) const;
  void Load(nsStringView sConfigPath = s_sConfigFile);
  void Apply();

  struct NS_FOUNDATION_DLL PluginConfig
  {
    bool operator<(const PluginConfig& rhs) const;

    nsString m_sAppDirRelativePath;
    bool m_bLoadCopy = false;
  };

  bool AddPlugin(const PluginConfig& cfg);
  bool RemovePlugin(const PluginConfig& cfg);

  mutable nsHybridArray<PluginConfig, 8> m_Plugins;
};


using nsApplicationPluginConfig_PluginConfig = nsApplicationPluginConfig::PluginConfig;

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsApplicationPluginConfig);
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsApplicationPluginConfig_PluginConfig);
