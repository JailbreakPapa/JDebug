#pragma once

#include <Core/ResourceManager/Implementation/Declarations.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Types/UniquePtr.h>

/// \brief [internal] Worker task for loading resources (typically from disk).
class NS_CORE_DLL nsResourceManagerWorkerDataLoad final : public nsTask
{
public:
  ~nsResourceManagerWorkerDataLoad();

private:
  friend class nsResourceManager;
  friend class nsResourceManagerState;

  nsResourceManagerWorkerDataLoad();

  virtual void Execute() override;
};

/// \brief [internal] Worker task for uploading resource data.
/// Depending on the resource type, this may get scheduled to run on the main thread or on any thread.
class NS_CORE_DLL nsResourceManagerWorkerUpdateContent final : public nsTask
{
public:
  ~nsResourceManagerWorkerUpdateContent();

  nsResourceLoadData m_LoaderData;
  nsResource* m_pResourceToLoad = nullptr;
  nsResourceTypeLoader* m_pLoader = nullptr;
  // this is only used to clean up a custom loader at the right time, if one is used
  // m_pLoader is always set, no need to go through m_pCustomLoader
  nsUniquePtr<nsResourceTypeLoader> m_pCustomLoader;

private:
  friend class nsResourceManager;
  friend class nsResourceManagerState;
  friend class nsResourceManagerWorkerDataLoad;
  nsResourceManagerWorkerUpdateContent();

  virtual void Execute() override;
};
