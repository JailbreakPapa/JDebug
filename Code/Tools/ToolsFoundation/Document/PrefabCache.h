#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Types/UniquePtr.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdAbstractObjectGraph;

class WD_TOOLSFOUNDATION_DLL wdPrefabCache
{
  WD_DECLARE_SINGLETON(wdPrefabCache);

public:
  wdPrefabCache();

  const wdStringBuilder& GetCachedPrefabDocument(const wdUuid& documentGuid);
  const wdAbstractObjectGraph* GetCachedPrefabGraph(const wdUuid& documentGuid);
  void LoadGraph(wdAbstractObjectGraph& out_graph, wdStringView sGraph);

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, wdPrefabCache);

  struct PrefabData
  {
    PrefabData() {}

    wdUuid m_documentGuid;
    wdString m_sAbsPath;

    wdAbstractObjectGraph m_Graph;
    wdStringBuilder m_sDocContent;
    wdTimestamp m_fileModifiedTime;
  };
  PrefabData& GetOrCreatePrefabCache(const wdUuid& documentGuid);
  void UpdatePrefabData(PrefabData& data);

  wdMap<wdUInt64, wdUniquePtr<wdAbstractObjectGraph>> m_CachedGraphs;
  wdMap<wdUuid, wdUniquePtr<PrefabData>> m_PrefabData;
};
