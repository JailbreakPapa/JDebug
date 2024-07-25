#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Types/UniquePtr.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsAbstractObjectGraph;

class NS_TOOLSFOUNDATION_DLL nsPrefabCache
{
  NS_DECLARE_SINGLETON(nsPrefabCache);

public:
  nsPrefabCache();

  const nsStringBuilder& GetCachedPrefabDocument(const nsUuid& documentGuid);
  const nsAbstractObjectGraph* GetCachedPrefabGraph(const nsUuid& documentGuid);
  void LoadGraph(nsAbstractObjectGraph& out_graph, nsStringView sGraph);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(ToolsFoundation, nsPrefabCache);

  struct PrefabData
  {
    PrefabData() = default;

    nsUuid m_documentGuid;
    nsString m_sAbsPath;

    nsAbstractObjectGraph m_Graph;
    nsStringBuilder m_sDocContent;
    nsTimestamp m_fileModifiedTime;
  };
  PrefabData& GetOrCreatePrefabCache(const nsUuid& documentGuid);
  void UpdatePrefabData(PrefabData& data);

  nsMap<nsUInt64, nsUniquePtr<nsAbstractObjectGraph>> m_CachedGraphs;
  nsMap<nsUuid, nsUniquePtr<PrefabData>> m_PrefabData;
};
