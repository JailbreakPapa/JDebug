#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Threading/ThreadUtils.h>

#include <GuiFoundation/UIServices/ImageCache.moc.h>
#include <QtConcurrent/qtconcurrentrun.h>

static nsQtImageCache* g_pImageCacheSingleton = nullptr;

NS_IMPLEMENT_SINGLETON(nsQtImageCache);

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, QtImageCache)

ON_CORESYSTEMS_STARTUP
{
  g_pImageCacheSingleton = NS_DEFAULT_NEW(nsQtImageCache);
}

ON_CORESYSTEMS_SHUTDOWN
{
  NS_DEFAULT_DELETE(g_pImageCacheSingleton);
}

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsQtImageCache::nsQtImageCache()
  : m_SingletonRegistrar(this)
{
  m_bCacheEnabled = true;
  m_bTaskRunning = false;
  m_iMemoryUsageThreshold = 128 * 1024 * 1024;
  m_iCurrentMemoryUsage = 0;
  m_pImageLoading = nullptr;
  m_pImageUnavailable = nullptr;
  m_uiCurImageID = 1;
}

void nsQtImageCache::SetFallbackImages(const char* szLoading, const char* szUnavailable)
{
  delete m_pImageLoading;
  if (nsStringUtils::EndsWith(szLoading, ".svg"))
  {
    m_pImageLoading = new QPixmap(nsSvgThumbnailToPixmap(szLoading));
  }
  else
  {
    m_pImageLoading = new QPixmap(szLoading);
  }

  delete m_pImageUnavailable;
  if (nsStringUtils::EndsWith(szUnavailable, ".svg"))
  {
    m_pImageUnavailable = new QPixmap(nsSvgThumbnailToPixmap(szUnavailable));
  }
  else
  {
    m_pImageUnavailable = new QPixmap(szUnavailable);
  }
}

void nsQtImageCache::InvalidateCache(const char* szAbsolutePath)
{
  nsStringBuilder sCleanPath = szAbsolutePath;
  sCleanPath.MakeCleanPath();

  const QString sPath = QString::fromUtf8(sCleanPath.GetData());

  NS_LOCK(m_Mutex);

  auto e = m_ImageCache.Find(sPath);

  if (!e.IsValid())
    return;

  nsUInt32 id = e.Value().m_uiImageID;
  m_ImageCache.Remove(e);

  Q_EMIT g_pImageCacheSingleton->ImageInvalidated(sPath, id);
}

const QPixmap* nsQtImageCache::QueryPixmap(
  const char* szAbsolutePath, QModelIndex index, QVariant userData1, QVariant userData2, nsUInt32* out_pImageID)
{
  if (out_pImageID)
    *out_pImageID = 0;

  if (m_pImageLoading == nullptr)
    SetFallbackImages(":/GuiFoundation/ThumbnailLoading.svg", ":/GuiFoundation/ThumbnailUnavailable.svg");

  nsStringBuilder sCleanPath = szAbsolutePath;
  sCleanPath.MakeCleanPath();

  const QString sPath = QString::fromUtf8(sCleanPath.GetData());

  NS_LOCK(m_Mutex);

  CleanupCache();

  auto itEntry = m_ImageCache.Find(sPath);

  if (itEntry.IsValid())
  {
    if (out_pImageID)
      *out_pImageID = itEntry.Value().m_uiImageID;

    itEntry.Value().m_LastAccess = nsTime::Now();
    return &itEntry.Value().m_Pixmap;
  }

  // do not queue any further requests, when the cache is disabled
  if (!m_bCacheEnabled)
    return m_pImageLoading;

  nsHashedString sHashed;
  sHashed.Assign(sCleanPath.GetData());

  Request r;
  r.m_sPath = sHashed;
  r.m_Index = index;
  r.m_UserData1 = userData1;
  r.m_UserData2 = userData2;

  // we could / should implement prioritization here
  m_Requests.Insert(r);

  RunLoadingTask();

  return m_pImageLoading;
}


const QPixmap* nsQtImageCache::QueryPixmapForType(const char* szType, const char* szAbsolutePath, QModelIndex index /*= QModelIndex()*/,
  QVariant userData1 /*= QVariant()*/, QVariant userData2 /*= QVariant()*/, nsUInt32* out_pImageID /*= nullptr*/)
{
  const QPixmap* pTypeImage = QueryTypeImage(szType);

  if (pTypeImage != nullptr)
    return pTypeImage;

  return QueryPixmap(szAbsolutePath, index, userData1, userData2, out_pImageID);
}

void nsQtImageCache::RunLoadingTask()
{
  NS_LOCK(m_Mutex);

  // if someone is already working
  if (m_bTaskRunning)
    return;

  // do not start another run, if the cache has been deactivated
  if (!m_bCacheEnabled)
    return;

  // if nothing is to do
  while (!m_Requests.IsEmpty())
  {
    auto it = m_Requests.GetIterator();
    Request req = it.Key();

    const QString sQtPath = QString::fromUtf8(req.m_sPath.GetData());

    // do not try to load something that has already been loaded in the mean time
    if (!m_ImageCache.Find(sQtPath).IsValid())
    {
      m_bTaskRunning = true;
      (void)QtConcurrent::run(LoadingTask, sQtPath, req.m_Index, req.m_UserData1, req.m_UserData2);
      return;
    }
    else
    {
      m_Requests.Remove(it);

      // inform the requester that his request has been fulfilled
      g_pImageCacheSingleton->EmitLoadedSignal(sQtPath, req.m_Index, req.m_UserData1, req.m_UserData2);
    }
  }

  // if we fall through, the queue is now empty
}

void nsQtImageCache::StopRequestProcessing(bool bPurgeExistingCache)
{
  bool bTaskRunning = false;

  {
    NS_LOCK(m_Mutex);

    bTaskRunning = m_bTaskRunning;

    m_bCacheEnabled = false;
    m_Requests.Clear();

    if (bPurgeExistingCache)
      m_ImageCache.Clear();
  }

  // make sure to wait till the loading task has stopped
  while (bTaskRunning)
  {
    {
      NS_LOCK(m_Mutex);
      bTaskRunning = m_bTaskRunning;
    }

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(100));
  }
}

void nsQtImageCache::EnableRequestProcessing()
{
  NS_LOCK(m_Mutex);

  m_bCacheEnabled = true;
  RunLoadingTask();
}


void nsQtImageCache::RegisterTypeImage(const char* szType, QPixmap pixmap)
{
  m_TypeImages[QString::fromUtf8(szType)] = pixmap;
}

const QPixmap* nsQtImageCache::QueryTypeImage(const char* szType) const
{
  auto it = m_TypeImages.Find(QString::fromUtf8(szType));

  if (it.IsValid())
    return &it.Value();

  return nullptr;
}

void nsQtImageCache::EmitLoadedSignal(QString sPath, QModelIndex index, QVariant UserData1, QVariant UserData2)
{
  Q_EMIT ImageLoaded(sPath, index, UserData1, UserData2);
}

void nsQtImageCache::LoadingTask(QString sPath, QModelIndex index, QVariant UserData1, QVariant UserData2)
{
  QImage Image;
  const bool bImageAvailable = Image.load(sPath);

  nsQtImageCache* pCache = nsQtImageCache::GetSingleton();

  NS_LOCK(pCache->m_Mutex);

  // remove the task from the queue
  {
    Request req;
    req.m_sPath.Assign(sPath.toUtf8().data());
    req.m_Index = index;
    req.m_UserData1 = UserData1;
    req.m_UserData2 = UserData2;

    pCache->m_Requests.Remove(req);
  }

  pCache->m_bTaskRunning = false;

  if (!pCache->m_bCacheEnabled)
    return;

  auto& entry = pCache->m_ImageCache[sPath];
  entry.m_uiImageID = ++pCache->m_uiCurImageID;

  pCache->m_iCurrentMemoryUsage -= nsMath::SafeMultiply64(entry.m_Pixmap.width(), entry.m_Pixmap.height(), 4);

  if (bImageAvailable)
    entry.m_Pixmap = QPixmap::fromImage(Image);
  else if (pCache->m_pImageUnavailable)
    entry.m_Pixmap = *pCache->m_pImageUnavailable;

  entry.m_LastAccess = nsTime::Now();

  pCache->m_iCurrentMemoryUsage += nsMath::SafeMultiply64(entry.m_Pixmap.width(), entry.m_Pixmap.height(), 4);

  // send event that something has been loaded
  g_pImageCacheSingleton->EmitLoadedSignal(sPath, index, UserData1, UserData2);

  // start the next task
  pCache->RunLoadingTask();
}

void nsQtImageCache::CleanupCache()
{
  NS_LOCK(m_Mutex);

  if (m_iCurrentMemoryUsage < m_iMemoryUsageThreshold)
    return;

  const nsTime tNow = nsTime::Now();

  // do not clean up too often
  if (tNow - m_LastCleanupTime < nsTime::MakeFromSeconds(10))
    return;

  m_LastCleanupTime = tNow;

  // purge everything older than 5 minutes, then 4 minutes, ...
  for (nsInt32 i = 5; i > 2; --i)
  {
    const nsTime tPurgeThreshold = nsTime::MakeFromSeconds(60) * i;

    // purge images that have not been accessed in a longer time
    for (auto it = m_ImageCache.GetIterator(); it.IsValid();)
    {
      if (tNow - it.Value().m_LastAccess > tPurgeThreshold)
      {
        // this image has not been accessed in a while, get rid of it

        m_iCurrentMemoryUsage -= nsMath::SafeMultiply64(it.Value().m_Pixmap.width(), it.Value().m_Pixmap.height(), 4);

        it = m_ImageCache.Remove(it);

        // if we have reached the threshold, stop further purging
        if (m_iCurrentMemoryUsage < m_iMemoryUsageThreshold)
          return;
      }
      else
        ++it;
    }
  }
}
