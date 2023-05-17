#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

class WD_FOUNDATION_DLL wdApplicationPluginConfig
{
public:
  wdApplicationPluginConfig();

  static constexpr const wdStringView s_sConfigFile = ":project/RuntimeConfigs/Plugins.ddl"_wdsv;

  wdResult Save(wdStringView sConfigPath = s_sConfigFile) const;
  void Load(wdStringView sConfigPath = s_sConfigFile);
  void Apply();

  struct WD_FOUNDATION_DLL PluginConfig
  {
    bool operator<(const PluginConfig& rhs) const;

    wdString m_sAppDirRelativePath;
    bool m_bLoadCopy = false;
  };

  bool AddPlugin(const PluginConfig& cfg);
  bool RemovePlugin(const PluginConfig& cfg);

  mutable wdHybridArray<PluginConfig, 8> m_Plugins;
};


using wdApplicationPluginConfig_PluginConfig = wdApplicationPluginConfig::PluginConfig;

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdApplicationPluginConfig);
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdApplicationPluginConfig_PluginConfig);
