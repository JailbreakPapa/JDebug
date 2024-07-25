#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Project/ToolsProject.h>

NS_IMPLEMENT_SINGLETON(nsPrefabCache);

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, nsPrefabCache)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    NS_DEFAULT_NEW(nsPrefabCache);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsPrefabCache* pDummy = nsPrefabCache::GetSingleton();
    NS_DEFAULT_DELETE(pDummy);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsPrefabCache::nsPrefabCache()
  : m_SingletonRegistrar(this)
{
}

const nsStringBuilder& nsPrefabCache::GetCachedPrefabDocument(const nsUuid& documentGuid)
{
  PrefabData& data = nsPrefabCache::GetOrCreatePrefabCache(documentGuid);
  return data.m_sDocContent;
}

const nsAbstractObjectGraph* nsPrefabCache::GetCachedPrefabGraph(const nsUuid& documentGuid)
{
  PrefabData& data = nsPrefabCache::GetOrCreatePrefabCache(documentGuid);
  if (data.m_sAbsPath.IsEmpty())
    return nullptr;
  return &data.m_Graph;
}

void nsPrefabCache::LoadGraph(nsAbstractObjectGraph& out_graph, nsStringView sGraph)
{
  nsUInt64 uiHash = nsHashingUtils::xxHash64(sGraph.GetStartPointer(), sGraph.GetElementCount());
  auto it = m_CachedGraphs.Find(uiHash);
  if (!it.IsValid())
  {
    it = m_CachedGraphs.Insert(uiHash, nsUniquePtr<nsAbstractObjectGraph>(NS_DEFAULT_NEW(nsAbstractObjectGraph)));

    nsRawMemoryStreamReader stringReader(sGraph.GetStartPointer(), sGraph.GetElementCount());
    nsUniquePtr<nsAbstractObjectGraph> header;
    nsUniquePtr<nsAbstractObjectGraph> types;
    nsAbstractGraphDdlSerializer::ReadDocument(stringReader, header, it.Value(), types, true).IgnoreResult();
  }

  it.Value()->Clone(out_graph);
}

nsPrefabCache::PrefabData& nsPrefabCache::GetOrCreatePrefabCache(const nsUuid& documentGuid)
{
  auto it = m_PrefabData.Find(documentGuid);
  if (it.IsValid())
  {
    nsFileStats Stats;
    if (nsOSFile::GetFileStats(it.Value()->m_sAbsPath, Stats).Succeeded() && !Stats.m_LastModificationTime.Compare(it.Value()->m_fileModifiedTime, nsTimestamp::CompareMode::FileTimeEqual))
    {
      UpdatePrefabData(*it.Value().Borrow());
    }
  }
  else
  {
    it = m_PrefabData.Insert(documentGuid, nsUniquePtr<PrefabData>(NS_DEFAULT_NEW(PrefabData)));

    it.Value()->m_documentGuid = documentGuid;
    it.Value()->m_sAbsPath = nsToolsProject::GetSingleton()->GetPathForDocumentGuid(documentGuid);
    if (it.Value()->m_sAbsPath.IsEmpty())
    {
      nsStringBuilder sGuid;
      nsConversionUtils::ToString(documentGuid, sGuid);
      nsLog::Error("Can't resolve prefab document guid '{0}'. The resolved path is empty", sGuid);
    }
    else
      UpdatePrefabData(*it.Value().Borrow());
  }

  return *it.Value().Borrow();
}

void nsPrefabCache::UpdatePrefabData(PrefabData& data)
{
  if (data.m_sAbsPath.IsEmpty())
  {
    data.m_sAbsPath = nsToolsProject::GetSingleton()->GetPathForDocumentGuid(data.m_documentGuid);
    if (data.m_sAbsPath.IsEmpty())
    {
      nsStringBuilder sGuid;
      nsConversionUtils::ToString(data.m_documentGuid, sGuid);
      nsLog::Error("Can't resolve prefab document guid '{0}'. The resolved path is empty", sGuid);
      return;
    }
  }

  nsFileStats Stats;
  bool bStat = nsOSFile::GetFileStats(data.m_sAbsPath, Stats).Succeeded();

  if (!bStat)
  {
    nsLog::Error("Can't update prefab file '{0}', the file can't be opened.", data.m_sAbsPath);
    return;
  }

  data.m_sDocContent = nsPrefabUtils::ReadDocumentAsString(data.m_sAbsPath);

  if (data.m_sDocContent.IsEmpty())
    return;

  data.m_fileModifiedTime = Stats.m_LastModificationTime;
  data.m_Graph.Clear();
  nsPrefabUtils::LoadGraph(data.m_Graph, data.m_sDocContent);
}
