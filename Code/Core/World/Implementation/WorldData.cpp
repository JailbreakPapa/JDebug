#include <Core/CorePCH.h>

#include <Core/ResourceManager/ResourceManager.h>
#include <Core/World/Implementation/WorldData.h>
#include <Core/World/SpatialSystem_RegularGrid.h>
#include <Core/World/World.h>

#include <Foundation/Time/DefaultTimeStepSmoothing.h>

namespace nsInternal
{
  class DefaultCoordinateSystemProvider : public nsCoordinateSystemProvider
  {
  public:
    DefaultCoordinateSystemProvider()
      : nsCoordinateSystemProvider(nullptr)
    {
    }

    virtual void GetCoordinateSystem(const nsVec3& vGlobalPosition, nsCoordinateSystem& out_coordinateSystem) const override
    {
      out_coordinateSystem.m_vForwardDir = nsVec3(1.0f, 0.0f, 0.0f);
      out_coordinateSystem.m_vRightDir = nsVec3(0.0f, 1.0f, 0.0f);
      out_coordinateSystem.m_vUpDir = nsVec3(0.0f, 0.0f, 1.0f);
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  void WorldData::UpdateTask::Execute()
  {
    nsWorldModule::UpdateContext context;
    context.m_uiFirstComponentIndex = m_uiStartIndex;
    context.m_uiComponentCount = m_uiCount;

    m_Function(context);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////

  WorldData::WorldData(nsWorldDesc& desc)
    : m_sName(desc.m_sName)
    , m_Allocator(desc.m_sName, nsFoundation::GetDefaultAllocator())
    , m_AllocatorWrapper(&m_Allocator)
    , m_BlockAllocator(desc.m_sName, &m_Allocator)
    , m_StackAllocator(desc.m_sName, nsFoundation::GetAlignedAllocator())
    , m_ObjectStorage(&m_BlockAllocator, &m_Allocator)
    , m_MaxInitializationTimePerFrame(desc.m_MaxComponentInitializationTimePerFrame)
    , m_Clock(desc.m_sName)
    , m_WriteThreadID((nsThreadID)0)
    , m_bReportErrorWhenStaticObjectMoves(desc.m_bReportErrorWhenStaticObjectMoves)
    , m_ReadMarker(*this)
    , m_WriteMarker(*this)

  {
    m_AllocatorWrapper.Reset();

    if (desc.m_uiRandomNumberGeneratorSeed == 0)
    {
      m_Random.InitializeFromCurrentTime();
    }
    else
    {
      m_Random.Initialize(desc.m_uiRandomNumberGeneratorSeed);
    }

    // insert dummy entry to save some checks
    m_Objects.Insert(nullptr);

#if NS_ENABLED(NS_GAMEOBJECT_VELOCITY)
    NS_CHECK_AT_COMPILETIME(sizeof(nsGameObject::TransformationData) == 240);
#else
    NS_CHECK_AT_COMPILETIME(sizeof(nsGameObject::TransformationData) == 192);
#endif

    NS_CHECK_AT_COMPILETIME(sizeof(nsGameObject) == 128);
    NS_CHECK_AT_COMPILETIME(sizeof(QueuedMsgMetaData) == 16);
    NS_CHECK_AT_COMPILETIME(NS_COMPONENT_TYPE_INDEX_BITS <= sizeof(nsWorldModuleTypeId) * 8);

    auto pDefaultInitBatch = NS_NEW(&m_Allocator, InitBatch, &m_Allocator, "Default", true);
    pDefaultInitBatch->m_bIsReady = true;
    m_InitBatches.Insert(pDefaultInitBatch);
    m_pDefaultInitBatch = pDefaultInitBatch;
    m_pCurrentInitBatch = pDefaultInitBatch;

    m_pSpatialSystem = std::move(desc.m_pSpatialSystem);
    m_pCoordinateSystemProvider = desc.m_pCoordinateSystemProvider;

    if (m_pSpatialSystem == nullptr && desc.m_bAutoCreateSpatialSystem)
    {
      m_pSpatialSystem = NS_NEW(nsFoundation::GetAlignedAllocator(), nsSpatialSystem_RegularGrid);
    }

    if (m_pCoordinateSystemProvider == nullptr)
    {
      m_pCoordinateSystemProvider = NS_NEW(&m_Allocator, DefaultCoordinateSystemProvider);
    }

    if (m_pTimeStepSmoothing == nullptr)
    {
      m_pTimeStepSmoothing = NS_NEW(&m_Allocator, nsDefaultTimeStepSmoothing);
    }

    m_Clock.SetTimeStepSmoothing(m_pTimeStepSmoothing.Borrow());

    nsResourceManager::GetResourceEvents().AddEventHandler(nsMakeDelegate(&WorldData::ResourceEventHandler, this));
  }

  WorldData::~WorldData()
  {
    nsResourceManager::GetResourceEvents().RemoveEventHandler(nsMakeDelegate(&WorldData::ResourceEventHandler, this));
  }

  void WorldData::Clear()
  {
    // allow reading and writing during destruction
    m_WriteThreadID = nsThreadUtils::GetCurrentThreadID();
    m_iReadCounter.Increment();

    // deactivate all objects and components before destroying them
    for (auto it = m_ObjectStorage.GetIterator(); it.IsValid(); it.Next())
    {
      it->SetActiveFlag(false);
    }

    // deinitialize all modules before we invalidate the world. Components can still access the world during deinitialization.
    for (nsWorldModule* pModule : m_Modules)
    {
      if (pModule != nullptr)
      {
        pModule->Deinitialize();
      }
    }

    // now delete all modules
    for (nsWorldModule* pModule : m_Modules)
    {
      if (pModule != nullptr)
      {
        NS_DELETE(&m_Allocator, pModule);
      }
    }
    m_Modules.Clear();

    // this deletes the nsGameObject instances
    m_ObjectStorage.Clear();

    // delete all transformation data
    for (nsUInt32 uiHierarchyIndex = 0; uiHierarchyIndex < HierarchyType::COUNT; ++uiHierarchyIndex)
    {
      Hierarchy& hierarchy = m_Hierarchies[uiHierarchyIndex];

      for (nsUInt32 i = hierarchy.m_Data.GetCount(); i-- > 0;)
      {
        Hierarchy::DataBlockArray* blocks = hierarchy.m_Data[i];
        for (nsUInt32 j = blocks->GetCount(); j-- > 0;)
        {
          m_BlockAllocator.DeallocateBlock((*blocks)[j]);
        }
        NS_DELETE(&m_Allocator, blocks);
      }

      hierarchy.m_Data.Clear();
    }

    // delete task storage
    m_UpdateTasks.Clear();

    // delete queued messages
    for (nsUInt32 i = 0; i < nsObjectMsgQueueType::COUNT; ++i)
    {
      {
        MessageQueue& queue = m_MessageQueues[i];

        // The messages in this queue are allocated through a frame allocator and thus mustn't (and don't need to be) deallocated
        queue.Clear();
      }

      {
        MessageQueue& queue = m_TimedMessageQueues[i];
        while (!queue.IsEmpty())
        {
          MessageQueue::Entry& entry = queue.Peek();
          NS_DELETE(&m_Allocator, entry.m_pMessage);

          queue.Dequeue();
        }
      }
    }
  }

  nsGameObject::TransformationData* WorldData::CreateTransformationData(bool bDynamic, nsUInt32 uiHierarchyLevel)
  {
    Hierarchy& hierarchy = m_Hierarchies[GetHierarchyType(bDynamic)];

    while (uiHierarchyLevel >= hierarchy.m_Data.GetCount())
    {
      hierarchy.m_Data.PushBack(NS_NEW(&m_Allocator, Hierarchy::DataBlockArray, &m_Allocator));
    }

    Hierarchy::DataBlockArray& blocks = *hierarchy.m_Data[uiHierarchyLevel];
    Hierarchy::DataBlock* pBlock = nullptr;

    if (!blocks.IsEmpty())
    {
      pBlock = &blocks.PeekBack();
    }

    if (pBlock == nullptr || pBlock->IsFull())
    {
      blocks.PushBack(m_BlockAllocator.AllocateBlock<nsGameObject::TransformationData>());
      pBlock = &blocks.PeekBack();
    }

    return pBlock->ReserveBack();
  }

  void WorldData::DeleteTransformationData(bool bDynamic, nsUInt32 uiHierarchyLevel, nsGameObject::TransformationData* pData)
  {
    Hierarchy& hierarchy = m_Hierarchies[GetHierarchyType(bDynamic)];
    Hierarchy::DataBlockArray& blocks = *hierarchy.m_Data[uiHierarchyLevel];

    Hierarchy::DataBlock& lastBlock = blocks.PeekBack();
    const nsGameObject::TransformationData* pLast = lastBlock.PopBack();

    if (pData != pLast)
    {
      nsMemoryUtils::Copy(pData, pLast, 1);
      pData->m_pObject->m_pTransformationData = pData;

      // fix parent transform data for children as well
      auto it = pData->m_pObject->GetChildren();
      while (it.IsValid())
      {
        auto pTransformData = it->m_pTransformationData;
        pTransformData->m_pParentData = pData;
        it.Next();
      }
    }

    if (lastBlock.IsEmpty())
    {
      m_BlockAllocator.DeallocateBlock(lastBlock);
      blocks.PopBack();
    }
  }

  void WorldData::TraverseBreadthFirst(VisitorFunc& func)
  {
    struct Helper
    {
      NS_ALWAYS_INLINE static nsVisitorExecution::Enum Visit(nsGameObject::TransformationData* pData, void* pUserData) { return (*static_cast<VisitorFunc*>(pUserData))(pData->m_pObject); }
    };

    const nsUInt32 uiMaxHierarchyLevel = nsMath::Max(m_Hierarchies[HierarchyType::Static].m_Data.GetCount(), m_Hierarchies[HierarchyType::Dynamic].m_Data.GetCount());

    for (nsUInt32 uiHierarchyLevel = 0; uiHierarchyLevel < uiMaxHierarchyLevel; ++uiHierarchyLevel)
    {
      for (nsUInt32 uiHierarchyIndex = 0; uiHierarchyIndex < HierarchyType::COUNT; ++uiHierarchyIndex)
      {
        Hierarchy& hierarchy = m_Hierarchies[uiHierarchyIndex];
        if (uiHierarchyLevel < hierarchy.m_Data.GetCount())
        {
          nsVisitorExecution::Enum execution = TraverseHierarchyLevel<Helper>(*hierarchy.m_Data[uiHierarchyLevel], &func);
          NS_ASSERT_DEV(execution != nsVisitorExecution::Skip, "Skip is not supported when using breadth first traversal");
          if (execution == nsVisitorExecution::Stop)
            return;
        }
      }
    }
  }

  void WorldData::TraverseDepthFirst(VisitorFunc& func)
  {
    struct Helper
    {
      NS_ALWAYS_INLINE static nsVisitorExecution::Enum Visit(nsGameObject::TransformationData* pData, void* pUserData) { return WorldData::TraverseObjectDepthFirst(pData->m_pObject, *static_cast<VisitorFunc*>(pUserData)); }
    };

    for (nsUInt32 uiHierarchyIndex = 0; uiHierarchyIndex < HierarchyType::COUNT; ++uiHierarchyIndex)
    {
      Hierarchy& hierarchy = m_Hierarchies[uiHierarchyIndex];
      if (!hierarchy.m_Data.IsEmpty())
      {
        if (TraverseHierarchyLevel<Helper>(*hierarchy.m_Data[0], &func) == nsVisitorExecution::Stop)
          return;
      }
    }
  }

  // static
  nsVisitorExecution::Enum WorldData::TraverseObjectDepthFirst(nsGameObject* pObject, VisitorFunc& func)
  {
    nsVisitorExecution::Enum execution = func(pObject);
    if (execution == nsVisitorExecution::Stop)
      return nsVisitorExecution::Stop;

    if (execution != nsVisitorExecution::Skip) // skip all children
    {
      for (auto it = pObject->GetChildren(); it.IsValid(); ++it)
      {
        if (TraverseObjectDepthFirst(it, func) == nsVisitorExecution::Stop)
          return nsVisitorExecution::Stop;
      }
    }

    return nsVisitorExecution::Continue;
  }

  void WorldData::UpdateGlobalTransforms()
  {
    struct UserData
    {
      nsSpatialSystem* m_pSpatialSystem;
      nsUInt32 m_uiUpdateCounter;
    };

    UserData userData;
    userData.m_pSpatialSystem = m_pSpatialSystem.Borrow();
    userData.m_uiUpdateCounter = m_uiUpdateCounter;

    struct RootLevel
    {
      NS_ALWAYS_INLINE static nsVisitorExecution::Enum Visit(nsGameObject::TransformationData* pData, void* pUserData0)
      {
        auto pUserData = static_cast<const UserData*>(pUserData0);
        WorldData::UpdateGlobalTransform(pData, pUserData->m_uiUpdateCounter);
        return nsVisitorExecution::Continue;
      }
    };

    struct WithParent
    {
      NS_ALWAYS_INLINE static nsVisitorExecution::Enum Visit(nsGameObject::TransformationData* pData, void* pUserData0)
      {
        auto pUserData = static_cast<const UserData*>(pUserData0);
        WorldData::UpdateGlobalTransformWithParent(pData, pUserData->m_uiUpdateCounter);
        return nsVisitorExecution::Continue;
      }
    };

    struct RootLevelWithSpatialData
    {
      NS_ALWAYS_INLINE static nsVisitorExecution::Enum Visit(nsGameObject::TransformationData* pData, void* pUserData0)
      {
        auto pUserData = static_cast<UserData*>(pUserData0);
        WorldData::UpdateGlobalTransformAndSpatialData(pData, pUserData->m_uiUpdateCounter, *pUserData->m_pSpatialSystem);
        return nsVisitorExecution::Continue;
      }
    };

    struct WithParentWithSpatialData
    {
      NS_ALWAYS_INLINE static nsVisitorExecution::Enum Visit(nsGameObject::TransformationData* pData, void* pUserData0)
      {
        auto pUserData = static_cast<UserData*>(pUserData0);
        WorldData::UpdateGlobalTransformWithParentAndSpatialData(pData, pUserData->m_uiUpdateCounter, *pUserData->m_pSpatialSystem);
        return nsVisitorExecution::Continue;
      }
    };

    Hierarchy& hierarchy = m_Hierarchies[HierarchyType::Dynamic];
    if (!hierarchy.m_Data.IsEmpty())
    {
      auto dataPtr = hierarchy.m_Data.GetData();

      // If we have no spatial system, we perform multi-threaded update as we do not
      // have to acquire a write lock in the process.
      if (m_pSpatialSystem == nullptr)
      {
        TraverseHierarchyLevelMultiThreaded<RootLevel>(*dataPtr[0], &userData);

        for (nsUInt32 i = 1; i < hierarchy.m_Data.GetCount(); ++i)
        {
          TraverseHierarchyLevelMultiThreaded<WithParent>(*dataPtr[i], &userData);
        }
      }
      else
      {
        TraverseHierarchyLevel<RootLevelWithSpatialData>(*dataPtr[0], &userData);

        for (nsUInt32 i = 1; i < hierarchy.m_Data.GetCount(); ++i)
        {
          TraverseHierarchyLevel<WithParentWithSpatialData>(*dataPtr[i], &userData);
        }
      }
    }
  }

  void WorldData::ResourceEventHandler(const nsResourceEvent& e)
  {
    if (e.m_Type != nsResourceEvent::Type::ResourceContentUnloading || e.m_pResource->GetReferenceCount() == 0)
      return;

    nsTypelessResourceHandle hResource(e.m_pResource);
    if (m_ReloadFunctions.Contains(hResource))
    {
      m_NeedReload.Insert(hResource);
    }
  }

} // namespace nsInternal
