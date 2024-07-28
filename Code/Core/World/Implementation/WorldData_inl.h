
namespace nsInternal
{
  // static
  NS_ALWAYS_INLINE WorldData::HierarchyType::Enum WorldData::GetHierarchyType(bool bIsDynamic)
  {
    return bIsDynamic ? HierarchyType::Dynamic : HierarchyType::Static;
  }

  // static
  template <typename VISITOR>
  NS_FORCE_INLINE nsVisitorExecution::Enum WorldData::TraverseHierarchyLevel(Hierarchy::DataBlockArray& blocks, void* pUserData /* = nullptr*/)
  {
    for (WorldData::Hierarchy::DataBlock& block : blocks)
    {
      nsGameObject::TransformationData* pCurrentData = block.m_pData;
      nsGameObject::TransformationData* pEndData = block.m_pData + block.m_uiCount;

      while (pCurrentData < pEndData)
      {
        nsVisitorExecution::Enum execution = VISITOR::Visit(pCurrentData, pUserData);
        if (execution != nsVisitorExecution::Continue)
          return execution;

        ++pCurrentData;
      }
    }

    return nsVisitorExecution::Continue;
  }

  // static
  template <typename VISITOR>
  NS_FORCE_INLINE nsVisitorExecution::Enum WorldData::TraverseHierarchyLevelMultiThreaded(
    Hierarchy::DataBlockArray& blocks, void* pUserData /* = nullptr*/)
  {
    nsParallelForParams parallelForParams;
    parallelForParams.m_uiBinSize = 100;
    parallelForParams.m_uiMaxTasksPerThread = 2;
    parallelForParams.m_pTaskAllocator = m_StackAllocator.GetCurrentAllocator();

    nsTaskSystem::ParallelFor(
      blocks.GetArrayPtr(),
      [pUserData](nsArrayPtr<WorldData::Hierarchy::DataBlock> blocksSlice)
      {
        for (WorldData::Hierarchy::DataBlock& block : blocksSlice)
        {
          nsGameObject::TransformationData* pCurrentData = block.m_pData;
          nsGameObject::TransformationData* pEndData = block.m_pData + block.m_uiCount;

          while (pCurrentData < pEndData)
          {
            VISITOR::Visit(pCurrentData, pUserData);
            ++pCurrentData;
          }
        }
      },
      "World DataBlock Traversal Task", parallelForParams);

    return nsVisitorExecution::Continue;
  }

  // static
  NS_FORCE_INLINE void WorldData::UpdateGlobalTransform(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter)
  {
    pData->UpdateGlobalTransformWithoutParent(uiUpdateCounter);
    pData->UpdateGlobalBounds();
  }

  // static
  NS_FORCE_INLINE void WorldData::UpdateGlobalTransformWithParent(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter)
  {
    pData->UpdateGlobalTransformWithParent(uiUpdateCounter);
    pData->UpdateGlobalBounds();
  }

  // static
  NS_FORCE_INLINE void WorldData::UpdateGlobalTransformAndSpatialData(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter, nsSpatialSystem& spatialSystem)
  {
    pData->UpdateGlobalTransformWithoutParent(uiUpdateCounter);
    pData->UpdateGlobalBoundsAndSpatialData(spatialSystem);
  }

  // static
  NS_FORCE_INLINE void WorldData::UpdateGlobalTransformWithParentAndSpatialData(nsGameObject::TransformationData* pData, nsUInt32 uiUpdateCounter, nsSpatialSystem& spatialSystem)
  {
    pData->UpdateGlobalTransformWithParent(uiUpdateCounter);
    pData->UpdateGlobalBoundsAndSpatialData(spatialSystem);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////

  NS_ALWAYS_INLINE const nsGameObject& WorldData::ConstObjectIterator::operator*() const
  {
    return *m_Iterator;
  }

  NS_ALWAYS_INLINE const nsGameObject* WorldData::ConstObjectIterator::operator->() const
  {
    return m_Iterator;
  }

  NS_ALWAYS_INLINE WorldData::ConstObjectIterator::operator const nsGameObject*() const
  {
    return m_Iterator;
  }

  NS_ALWAYS_INLINE void WorldData::ConstObjectIterator::Next()
  {
    m_Iterator.Next();

    while (m_Iterator.IsValid() && m_Iterator->GetHandle().IsInvalidated())
    {
      m_Iterator.Next();
    }
  }

  NS_ALWAYS_INLINE bool WorldData::ConstObjectIterator::IsValid() const
  {
    return m_Iterator.IsValid();
  }

  NS_ALWAYS_INLINE void WorldData::ConstObjectIterator::operator++()
  {
    Next();
  }

  NS_ALWAYS_INLINE WorldData::ConstObjectIterator::ConstObjectIterator(ObjectStorage::ConstIterator iterator)
    : m_Iterator(iterator)
  {
    while (m_Iterator.IsValid() && m_Iterator->GetHandle().IsInvalidated())
    {
      m_Iterator.Next();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////

  NS_ALWAYS_INLINE nsGameObject& WorldData::ObjectIterator::operator*()
  {
    return *m_Iterator;
  }

  NS_ALWAYS_INLINE nsGameObject* WorldData::ObjectIterator::operator->()
  {
    return m_Iterator;
  }

  NS_ALWAYS_INLINE WorldData::ObjectIterator::operator nsGameObject*()
  {
    return m_Iterator;
  }

  NS_ALWAYS_INLINE void WorldData::ObjectIterator::Next()
  {
    m_Iterator.Next();

    while (m_Iterator.IsValid() && m_Iterator->GetHandle().IsInvalidated())
    {
      m_Iterator.Next();
    }
  }

  NS_ALWAYS_INLINE bool WorldData::ObjectIterator::IsValid() const
  {
    return m_Iterator.IsValid();
  }

  NS_ALWAYS_INLINE void WorldData::ObjectIterator::operator++()
  {
    Next();
  }

  NS_ALWAYS_INLINE WorldData::ObjectIterator::ObjectIterator(ObjectStorage::Iterator iterator)
    : m_Iterator(iterator)
  {
    while (m_Iterator.IsValid() && m_Iterator->GetHandle().IsInvalidated())
    {
      m_Iterator.Next();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////

  NS_FORCE_INLINE WorldData::InitBatch::InitBatch(nsAllocator* pAllocator, nsStringView sName, bool bMustFinishWithinOneFrame)
    : m_bMustFinishWithinOneFrame(bMustFinishWithinOneFrame)
    , m_ComponentsToInitialize(pAllocator)
    , m_ComponentsToStartSimulation(pAllocator)
  {
    m_sName.Assign(sName);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////

  NS_FORCE_INLINE void WorldData::RegisteredUpdateFunction::FillFromDesc(const nsWorldModule::UpdateFunctionDesc& desc)
  {
    m_Function = desc.m_Function;
    m_sFunctionName = desc.m_sFunctionName;
    m_fPriority = desc.m_fPriority;
    m_uiGranularity = desc.m_uiGranularity;
    m_bOnlyUpdateWhenSimulating = desc.m_bOnlyUpdateWhenSimulating;
  }

  NS_FORCE_INLINE bool WorldData::RegisteredUpdateFunction::operator<(const RegisteredUpdateFunction& other) const
  {
    // higher priority comes first
    if (m_fPriority != other.m_fPriority)
      return m_fPriority > other.m_fPriority;

    // sort by function name to ensure determinism
    nsInt32 iNameComp = nsStringUtils::Compare(m_sFunctionName, other.m_sFunctionName);
    NS_ASSERT_DEV(iNameComp != 0, "An update function with the same name and same priority is already registered. This breaks determinism.");
    return iNameComp < 0;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////

  NS_ALWAYS_INLINE WorldData::ReadMarker::ReadMarker(const WorldData& data)
    : m_Data(data)
  {
  }

  NS_FORCE_INLINE void WorldData::ReadMarker::Lock()
  {
    NS_ASSERT_DEV(m_Data.m_WriteThreadID == (nsThreadID)0 || m_Data.m_WriteThreadID == nsThreadUtils::GetCurrentThreadID(),
      "World '{0}' cannot be marked for reading because it is already marked for writing by another thread.", m_Data.m_sName);
    m_Data.m_iReadCounter.Increment();
  }

  NS_ALWAYS_INLINE void WorldData::ReadMarker::Unlock()
  {
    m_Data.m_iReadCounter.Decrement();
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////

  NS_ALWAYS_INLINE WorldData::WriteMarker::WriteMarker(WorldData& data)
    : m_Data(data)
  {
  }

  NS_FORCE_INLINE void WorldData::WriteMarker::Lock()
  {
    // already locked by this thread?
    if (m_Data.m_WriteThreadID != nsThreadUtils::GetCurrentThreadID())
    {
      NS_ASSERT_DEV(m_Data.m_iReadCounter == 0, "World '{0}' cannot be marked for writing because it is already marked for reading.", m_Data.m_sName);
      NS_ASSERT_DEV(m_Data.m_WriteThreadID == (nsThreadID)0,
        "World '{0}' cannot be marked for writing because it is already marked for writing by another thread.", m_Data.m_sName);

      m_Data.m_WriteThreadID = nsThreadUtils::GetCurrentThreadID();
      m_Data.m_iReadCounter.Increment(); // allow reading as well
    }

    m_Data.m_iWriteCounter++;
  }

  NS_FORCE_INLINE void WorldData::WriteMarker::Unlock()
  {
    m_Data.m_iWriteCounter--;

    if (m_Data.m_iWriteCounter == 0)
    {
      m_Data.m_iReadCounter.Decrement();
      m_Data.m_WriteThreadID = (nsThreadID)0;
    }
  }
} // namespace nsInternal
