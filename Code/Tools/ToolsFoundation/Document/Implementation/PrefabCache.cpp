#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Project/ToolsProject.h>

WD_IMPLEMENT_SINGLETON(wdPrefabCache);

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, wdPrefabCache)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    WD_DEFAULT_NEW(wdPrefabCache);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdPrefabCache* pDummy = wdPrefabCache::GetSingleton();
    WD_DEFAULT_DELETE(pDummy);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdPrefabCache::wdPrefabCache()
  : m_SingletonRegistrar(this)
{
}

const wdStringBuilder& wdPrefabCache::GetCachedPrefabDocument(const wdUuid& documentGuid)
{
  PrefabData& data = wdPrefabCache::GetOrCreatePrefabCache(documentGuid);
  return data.m_sDocContent;
}

const wdAbstractObjectGraph* wdPrefabCache::GetCachedPrefabGraph(const wdUuid& documentGuid)
{
  PrefabData& data = wdPrefabCache::GetOrCreatePrefabCache(documentGuid);
  if (data.m_sAbsPath.IsEmpty())
    return nullptr;
  return &data.m_Graph;
}

void wdPrefabCache::LoadGraph(wdAbstractObjectGraph& out_graph, wdStringView sGraph)
{
  wdUInt64 uiHash = wdHashingUtils::xxHash64(sGraph.GetStartPointer(), sGraph.GetElementCount());
  auto it = m_CachedGraphs.Find(uiHash);
  if (!it.IsValid())
  {
    it = m_CachedGraphs.Insert(uiHash, wdUniquePtr<wdAbstractObjectGraph>(WD_DEFAULT_NEW(wdAbstractObjectGraph)));

    wdRawMemoryStreamReader stringReader(sGraph.GetStartPointer(), sGraph.GetElementCount());
    wdUniquePtr<wdAbstractObjectGraph> header;
    wdUniquePtr<wdAbstractObjectGraph> types;
    wdAbstractGraphDdlSerializer::ReadDocument(stringReader, header, it.Value(), types, true).IgnoreResult();
  }

  it.Value()->Clone(out_graph);
}

wdPrefabCache::PrefabData& wdPrefabCache::GetOrCreatePrefabCache(const wdUuid& documentGuid)
{
  auto it = m_PrefabData.Find(documentGuid);
  if (it.IsValid())
  {
    wdFileStats Stats;
    if (wdOSFile::GetFileStats(it.Value()->m_sAbsPath, Stats).Succeeded() && !Stats.m_LastModificationTime.Compare(it.Value()->m_fileModifiedTime, wdTimestamp::CompareMode::FileTimeEqual))
    {
      UpdatePrefabData(*it.Value().Borrow());
    }
  }
  else
  {
    it = m_PrefabData.Insert(documentGuid, wdUniquePtr<PrefabData>(WD_DEFAULT_NEW(PrefabData)));

    it.Value()->m_documentGuid = documentGuid;
    it.Value()->m_sAbsPath = wdToolsProject::GetSingleton()->GetPathForDocumentGuid(documentGuid);
    if (it.Value()->m_sAbsPath.IsEmpty())
    {
      wdStringBuilder sGuid;
      wdConversionUtils::ToString(documentGuid, sGuid);
      wdLog::Error("Can't resolve prefab document guid '{0}'. The resolved path is empty", sGuid);
    }
    else
      UpdatePrefabData(*it.Value().Borrow());
  }

  return *it.Value().Borrow();
}

void wdPrefabCache::UpdatePrefabData(PrefabData& data)
{
  if (data.m_sAbsPath.IsEmpty())
  {
    data.m_sAbsPath = wdToolsProject::GetSingleton()->GetPathForDocumentGuid(data.m_documentGuid);
    if (data.m_sAbsPath.IsEmpty())
    {
      wdStringBuilder sGuid;
      wdConversionUtils::ToString(data.m_documentGuid, sGuid);
      wdLog::Error("Can't resolve prefab document guid '{0}'. The resolved path is empty", sGuid);
      return;
    }
  }

  wdFileStats Stats;
  bool bStat = wdOSFile::GetFileStats(data.m_sAbsPath, Stats).Succeeded();

  if (!bStat)
  {
    wdLog::Error("Can't update prefab file '{0}', the file can't be opened.", data.m_sAbsPath);
    return;
  }

  data.m_sDocContent = wdPrefabUtils::ReadDocumentAsString(data.m_sAbsPath);

  if (data.m_sDocContent.IsEmpty())
    return;

  data.m_fileModifiedTime = Stats.m_LastModificationTime;
  data.m_Graph.Clear();
  wdPrefabUtils::LoadGraph(data.m_Graph, data.m_sDocContent);
}
