#pragma once

#include <Foundation/Communication/MessageQueue.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Math/Random.h>
#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Threading/DelegateTask.h>
#include <Foundation/Time/Clock.h>
#include <Foundation/Types/SharedPtr.h>

#include <Core/ResourceManager/ResourceHandle.h>
#include <Core/World/GameObject.h>
#include <Core/World/WorldDesc.h>

namespace nsInternal
{
  class NS_CORE_DLL WorldData
  {
  private:
    friend class ::nsWorld;
    friend class ::nsComponentManagerBase;

    WorldData(nsWorldDesc& desc);
    ~WorldData();

    void Clear();

    nsHashedString m_sName;
    mutable nsProxyAllocator m_Allocator;
    nsLocalAllocatorWrapper m_AllocatorWrapper;
    nsInternal::WorldLargeBlockAllocator m_BlockAllocator;
    nsDoubleBufferedLinearAllocator m_StackAllocator;

    enum
    {
      GAME_OBJECTS_PER_BLOCK = nsDataBlock<nsGameObject, nsInternal::DEFAULT_BLOCK_SIZE>::CAPACITY,
      TRANSFORMATION_DATA_PER_BLOCK = nsDataBlock<nsGameObject::TransformationData, nsInternal::DEFAULT_BLOCK_SIZE>::CAPACITY
    };

    // object storage
    using ObjectStorage = nsBlockStorage<nsGameObject, nsInternal::DEFAULT_BLOCK_SIZE, nsBlockStorageType::Compact>;
    nsIdTable<nsGameObjectId, nsGameObject*, nsLocalAllocatorWrapper> m_Objects;
    ObjectStorage m_ObjectStorage;

    nsSet<nsGameObject*, nsCompareHelper<nsGameObject*>, nsLocalAllocatorWrapper> m_DeadObjects;
    nsEvent<const nsGameObject*> m_ObjectDeletionEvent;

  public:
    class NS_CORE_DLL ConstObjectIterator
    {
    public:
      const nsGameObject& operator*() const;
      const nsGameObject* operator->() const;

      operator const nsGameObject*() const;

      /// \brief Advances the iterator to the next object. The iterator will not be valid anymore, if the last object is reached.
      void Next();

      /// \brief Checks whether this iterator points to a valid object.
      bool IsValid() const;

      /// \brief Shorthand for 'Next'
      void operator++();

    private:
      friend class ::nsWorld;

      ConstObjectIterator(ObjectStorage::ConstIterator iterator);

      ObjectStorage::ConstIterator m_Iterator;
    };

    class NS_CORE_DLL ObjectIterator
    {
    public:
      nsGameObject& operator*();
      nsGameObject* operator->();

      operator nsGameObject*();

      /// \brief Advances the iterator to the next object. The iterator will not be valid anymore, if the last object is reached.
      void Next();

      /// \brief Checks whether this iterator points to a valid object.
      bool IsValid() const;

      /// \brief Shorthand for 'Next'
      void operator++();

    private:
      friend class ::nsWorld;

      ObjectIterator(ObjectStorage::Iterator iterator);

      ObjectStorage::Iterator m_Iterator;
    };

  private:
    // hierarchy structures
    struct Hierarchy
    {
      using DataBlock = nsDataBlock<nsGameObject::TransformationData, nsInternal::DEFAULT_BLOCK_SIZE>;
      using DataBlockArray = nsDynamicArray<DataBlock>;

      nsHybridArray<DataBlockArray*, 8, nsLocalAllocatorWrapper> m_Data;
    };

    struct HierarchyType
    {
      enum Enum
      {
        Static,
        Dynamic,
        COUNT
      };
    };

    Hierarchy m_Hierarchies[HierarchyType::COUNT];

    static HierarchyType::Enum GetHierarchyType(bool bDynamic);

    nsGameObject::TransformationData* CreateTransformationData(bool bDynamic, nsUInt32 uiHierarchyLevel);

    void DeleteTransformationData(bool bDynamic, nsUInt32 uiHierarchyLevel, nsGameObject::TransformationData* pData);

    template <typename VISITOR>
    static nsVisitorExecution::Enum TraverseHierarchyLevel(Hierarchy::DataBlockArray& blocks, void* pUserData = nullptr);
    template <typename VISITOR>
    nsVisitorExecution::Enum TraverseHierarchyLevelMultiThreaded(Hierarchy::DataBlockArray& blocks, void* pUserData = nullptr);

    using VisitorFunc = nsDelegate<nsVisitorExecution::Enum(nsGameObject*)>;
    void TraverseBreadthFirst(VisitorFunc& func);
    void TraverseDepthFirst(VisitorFunc& func);
    static nsVisitorExecution::Enum TraverseObjectDepthFirst(nsGameObject* pObject, VisitorFunc& func);

    static void UpdateGlobalTransform(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter);
    static void UpdateGlobalTransformWithParent(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter);

    static void UpdateGlobalTransformAndSpatialData(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter, nsSpatialSystem& spatialSystem);
    static void UpdateGlobalTransformWithParentAndSpatialData(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter, nsSpatialSystem& spatialSystem);

    void UpdateGlobalTransforms();

    void ResourceEventHandler(const nsResourceEvent& e);

    // game object lookups
    nsHashTable<nsUInt64, nsGameObjectId, nsHashHelper<nsUInt64>, nsLocalAllocatorWrapper> m_GlobalKeyToIdTable;
    nsHashTable<nsUInt64, nsHashedString, nsHashHelper<nsUInt64>, nsLocalAllocatorWrapper> m_IdToGlobalKeyTable;

    // modules
    nsDynamicArray<nsWorldModule*, nsLocalAllocatorWrapper> m_Modules;
    nsDynamicArray<nsWorldModule*, nsLocalAllocatorWrapper> m_ModulesToStartSimulation;

    // component management
    nsSet<nsComponent*, nsCompareHelper<nsComponent*>, nsLocalAllocatorWrapper> m_DeadComponents;

    struct InitBatch
    {
      InitBatch(nsAllocator* pAllocator, nsStringView sName, bool bMustFinishWithinOneFrame);

      nsHashedString m_sName;
      bool m_bMustFinishWithinOneFrame = true;
      bool m_bIsReady = false;

      nsUInt32 m_uiNextComponentToInitialize = 0;
      nsUInt32 m_uiNextComponentToStartSimulation = 0;
      nsDynamicArray<nsComponentHandle> m_ComponentsToInitialize;
      nsDynamicArray<nsComponentHandle> m_ComponentsToStartSimulation;
    };

    nsTime m_MaxInitializationTimePerFrame;
    nsIdTable<nsComponentInitBatchId, nsUniquePtr<InitBatch>, nsLocalAllocatorWrapper> m_InitBatches;
    InitBatch* m_pDefaultInitBatch = nullptr;
    InitBatch* m_pCurrentInitBatch = nullptr;

    struct RegisteredUpdateFunction
    {
      nsWorldModule::UpdateFunction m_Function;
      nsHashedString m_sFunctionName;
      float m_fPriority;
      nsUInt16 m_uiGranularity;
      bool m_bOnlyUpdateWhenSimulating;

      void FillFromDesc(const nsWorldModule::UpdateFunctionDesc& desc);
      bool operator<(const RegisteredUpdateFunction& other) const;
    };

    struct UpdateTask final : public nsTask
    {
      virtual void Execute() override;

      nsWorldModule::UpdateFunction m_Function;
      nsUInt32 m_uiStartIndex;
      nsUInt32 m_uiCount;
    };

    nsDynamicArray<RegisteredUpdateFunction, nsLocalAllocatorWrapper> m_UpdateFunctions[nsWorldModule::UpdateFunctionDesc::Phase::COUNT];
    nsDynamicArray<nsWorldModule::UpdateFunctionDesc, nsLocalAllocatorWrapper> m_UpdateFunctionsToRegister;

    nsDynamicArray<nsSharedPtr<UpdateTask>, nsLocalAllocatorWrapper> m_UpdateTasks;

    nsUniquePtr<nsSpatialSystem> m_pSpatialSystem;
    nsSharedPtr<nsCoordinateSystemProvider> m_pCoordinateSystemProvider;
    nsUniquePtr<nsTimeStepSmoothing> m_pTimeStepSmoothing;

    nsClock m_Clock;
    nsRandom m_Random;

    struct QueuedMsgMetaData
    {
      NS_DECLARE_POD_TYPE();

      NS_ALWAYS_INLINE QueuedMsgMetaData()
        : m_uiReceiverData(0)
      {
      }

      union
      {
        struct
        {
          nsUInt64 m_uiReceiverObjectOrComponent : 62;
          nsUInt64 m_uiReceiverIsComponent : 1;
          nsUInt64 m_uiRecursive : 1;
        };

        nsUInt64 m_uiReceiverData;
      };

      nsTime m_Due;
    };

    using MessageQueue = nsMessageQueue<QueuedMsgMetaData, nsLocalAllocatorWrapper>;
    mutable MessageQueue m_MessageQueues[nsObjectMsgQueueType::COUNT];
    mutable MessageQueue m_TimedMessageQueues[nsObjectMsgQueueType::COUNT];
    nsObjectMsgQueueType::Enum m_ProcessingMessageQueue = nsObjectMsgQueueType::COUNT;

    nsThreadID m_WriteThreadID;
    nsInt32 m_iWriteCounter = 0;
    mutable nsAtomicInteger32 m_iReadCounter;

    nsUInt32 m_uiUpdateCounter = 0;
    bool m_bSimulateWorld = true;
    bool m_bReportErrorWhenStaticObjectMoves = true;

    /// \brief Maps some data (given as void*) to an nsGameObjectHandle. Only available in special situations (e.g. editor use cases).
    nsDelegate<nsGameObjectHandle(const void*, nsComponentHandle, nsStringView)> m_GameObjectReferenceResolver;

    struct ResourceReloadContext
    {
      nsWorld* m_pWorld = nullptr;
      nsComponent* m_pComponent = nullptr;
      void* m_pUserData = nullptr;
    };

    using ResourceReloadFunc = nsDelegate<void(ResourceReloadContext&)>;

    struct ResourceReloadFunctionData
    {
      nsComponentHandle m_hComponent;
      void* m_pUserData = nullptr;
      ResourceReloadFunc m_Func;
    };

    using ReloadFunctionList = nsHybridArray<ResourceReloadFunctionData, 8>;
    nsHashTable<nsTypelessResourceHandle, ReloadFunctionList> m_ReloadFunctions;
    nsHashSet<nsTypelessResourceHandle> m_NeedReload;
    ReloadFunctionList m_TempReloadFunctions;

  public:
    class ReadMarker
    {
    public:
      void Lock();
      void Unlock();

    private:
      friend class ::nsInternal::WorldData;

      ReadMarker(const WorldData& data);
      const WorldData& m_Data;
    };

    class WriteMarker
    {
    public:
      void Lock();
      void Unlock();

    private:
      friend class ::nsInternal::WorldData;

      WriteMarker(WorldData& data);
      WorldData& m_Data;
    };

  private:
    mutable ReadMarker m_ReadMarker;
    WriteMarker m_WriteMarker;

    void* m_pUserData = nullptr;
  };
} // namespace nsInternal

#include <Core/World/Implementation/WorldData_inl.h>
