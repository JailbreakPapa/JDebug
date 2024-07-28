
NS_ALWAYS_INLINE nsStringView nsWorld::GetName() const
{
  return m_Data.m_sName;
}

NS_ALWAYS_INLINE nsUInt32 nsWorld::GetIndex() const
{
  return m_uiIndex;
}

NS_FORCE_INLINE nsGameObjectHandle nsWorld::CreateObject(const nsGameObjectDesc& desc)
{
  nsGameObject* pNewObject;
  return CreateObject(desc, pNewObject);
}

NS_ALWAYS_INLINE const nsEvent<const nsGameObject*>& nsWorld::GetObjectDeletionEvent() const
{
  return m_Data.m_ObjectDeletionEvent;
}

NS_FORCE_INLINE bool nsWorld::IsValidObject(const nsGameObjectHandle& hObject) const
{
  CheckForReadAccess();
  NS_ASSERT_DEV(hObject.IsInvalidated() || hObject.m_InternalId.m_WorldIndex == m_uiIndex,
    "Object does not belong to this world. Expected world id {0} got id {1}", m_uiIndex, hObject.m_InternalId.m_WorldIndex);

  return m_Data.m_Objects.Contains(hObject);
}

NS_FORCE_INLINE bool nsWorld::TryGetObject(const nsGameObjectHandle& hObject, nsGameObject*& out_pObject)
{
  CheckForReadAccess();
  NS_ASSERT_DEV(hObject.IsInvalidated() || hObject.m_InternalId.m_WorldIndex == m_uiIndex,
    "Object does not belong to this world. Expected world id {0} got id {1}", m_uiIndex, hObject.m_InternalId.m_WorldIndex);

  return m_Data.m_Objects.TryGetValue(hObject, out_pObject);
}

NS_FORCE_INLINE bool nsWorld::TryGetObject(const nsGameObjectHandle& hObject, const nsGameObject*& out_pObject) const
{
  CheckForReadAccess();
  NS_ASSERT_DEV(hObject.IsInvalidated() || hObject.m_InternalId.m_WorldIndex == m_uiIndex,
    "Object does not belong to this world. Expected world id {0} got id {1}", m_uiIndex, hObject.m_InternalId.m_WorldIndex);

  nsGameObject* pObject = nullptr;
  bool bResult = m_Data.m_Objects.TryGetValue(hObject, pObject);
  out_pObject = pObject;
  return bResult;
}

NS_FORCE_INLINE bool nsWorld::TryGetObjectWithGlobalKey(const nsTempHashedString& sGlobalKey, nsGameObject*& out_pObject)
{
  CheckForReadAccess();
  nsGameObjectId id;
  if (m_Data.m_GlobalKeyToIdTable.TryGetValue(sGlobalKey.GetHash(), id))
  {
    out_pObject = m_Data.m_Objects[id];
    return true;
  }

  return false;
}

NS_FORCE_INLINE bool nsWorld::TryGetObjectWithGlobalKey(const nsTempHashedString& sGlobalKey, const nsGameObject*& out_pObject) const
{
  CheckForReadAccess();
  nsGameObjectId id;
  if (m_Data.m_GlobalKeyToIdTable.TryGetValue(sGlobalKey.GetHash(), id))
  {
    out_pObject = m_Data.m_Objects[id];
    return true;
  }

  return false;
}

NS_FORCE_INLINE nsUInt32 nsWorld::GetObjectCount() const
{
  CheckForReadAccess();
  // Subtract one to exclude dummy object with instance index 0
  return static_cast<nsUInt32>(m_Data.m_Objects.GetCount() - 1);
}

NS_FORCE_INLINE nsInternal::WorldData::ObjectIterator nsWorld::GetObjects()
{
  CheckForWriteAccess();
  return nsInternal::WorldData::ObjectIterator(m_Data.m_ObjectStorage.GetIterator(0));
}

NS_FORCE_INLINE nsInternal::WorldData::ConstObjectIterator nsWorld::GetObjects() const
{
  CheckForReadAccess();
  return nsInternal::WorldData::ConstObjectIterator(m_Data.m_ObjectStorage.GetIterator(0));
}

NS_FORCE_INLINE void nsWorld::Traverse(VisitorFunc visitorFunc, TraversalMethod method /*= DepthFirst*/)
{
  CheckForWriteAccess();

  if (method == DepthFirst)
  {
    m_Data.TraverseDepthFirst(visitorFunc);
  }
  else // method == BreadthFirst
  {
    m_Data.TraverseBreadthFirst(visitorFunc);
  }
}

template <typename ModuleType>
NS_ALWAYS_INLINE ModuleType* nsWorld::GetOrCreateModule()
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsWorldModule, ModuleType), "Not a valid module type");

  return nsStaticCast<ModuleType*>(GetOrCreateModule(nsGetStaticRTTI<ModuleType>()));
}

template <typename ModuleType>
NS_ALWAYS_INLINE void nsWorld::DeleteModule()
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsWorldModule, ModuleType), "Not a valid module type");

  DeleteModule(nsGetStaticRTTI<ModuleType>());
}

template <typename ModuleType>
NS_ALWAYS_INLINE ModuleType* nsWorld::GetModule()
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsWorldModule, ModuleType), "Not a valid module type");

  return nsStaticCast<ModuleType*>(GetModule(nsGetStaticRTTI<ModuleType>()));
}

template <typename ModuleType>
NS_ALWAYS_INLINE const ModuleType* nsWorld::GetModule() const
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsWorldModule, ModuleType), "Not a valid module type");

  return nsStaticCast<const ModuleType*>(GetModule(nsGetStaticRTTI<ModuleType>()));
}

template <typename ModuleType>
NS_ALWAYS_INLINE const ModuleType* nsWorld::GetModuleReadOnly() const
{
  return GetModule<ModuleType>();
}

template <typename ManagerType>
ManagerType* nsWorld::GetOrCreateComponentManager()
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsComponentManagerBase, ManagerType), "Not a valid component manager type");

  CheckForWriteAccess();

  const nsWorldModuleTypeId uiTypeId = ManagerType::TypeId();
  m_Data.m_Modules.EnsureCount(uiTypeId + 1);

  ManagerType* pModule = static_cast<ManagerType*>(m_Data.m_Modules[uiTypeId]);
  if (pModule == nullptr)
  {
    pModule = NS_NEW(&m_Data.m_Allocator, ManagerType, this);
    static_cast<nsWorldModule*>(pModule)->Initialize();

    m_Data.m_Modules[uiTypeId] = pModule;
    m_Data.m_ModulesToStartSimulation.PushBack(pModule);
  }

  return pModule;
}

NS_ALWAYS_INLINE nsComponentManagerBase* nsWorld::GetOrCreateManagerForComponentType(const nsRTTI* pComponentRtti)
{
  NS_ASSERT_DEV(pComponentRtti->IsDerivedFrom<nsComponent>(), "Invalid component type '%s'", pComponentRtti->GetTypeName());

  return nsStaticCast<nsComponentManagerBase*>(GetOrCreateModule(pComponentRtti));
}

template <typename ManagerType>
void nsWorld::DeleteComponentManager()
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsComponentManagerBase, ManagerType), "Not a valid component manager type");

  CheckForWriteAccess();

  const nsWorldModuleTypeId uiTypeId = ManagerType::TypeId();
  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    if (ManagerType* pModule = static_cast<ManagerType*>(m_Data.m_Modules[uiTypeId]))
    {
      m_Data.m_Modules[uiTypeId] = nullptr;

      static_cast<nsWorldModule*>(pModule)->Deinitialize();
      DeregisterUpdateFunctions(pModule);
      NS_DELETE(&m_Data.m_Allocator, pModule);
    }
  }
}

template <typename ManagerType>
NS_FORCE_INLINE ManagerType* nsWorld::GetComponentManager()
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsComponentManagerBase, ManagerType), "Not a valid component manager type");

  CheckForWriteAccess();

  const nsWorldModuleTypeId uiTypeId = ManagerType::TypeId();
  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    return nsStaticCast<ManagerType*>(m_Data.m_Modules[uiTypeId]);
  }

  return nullptr;
}

template <typename ManagerType>
NS_FORCE_INLINE const ManagerType* nsWorld::GetComponentManager() const
{
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsComponentManagerBase, ManagerType), "Not a valid component manager type");

  CheckForReadAccess();

  const nsWorldModuleTypeId uiTypeId = ManagerType::TypeId();
  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    return nsStaticCast<const ManagerType*>(m_Data.m_Modules[uiTypeId]);
  }

  return nullptr;
}

NS_ALWAYS_INLINE nsComponentManagerBase* nsWorld::GetManagerForComponentType(const nsRTTI* pComponentRtti)
{
  NS_ASSERT_DEV(pComponentRtti->IsDerivedFrom<nsComponent>(), "Invalid component type '{0}'", pComponentRtti->GetTypeName());

  return nsStaticCast<nsComponentManagerBase*>(GetModule(pComponentRtti));
}

NS_ALWAYS_INLINE const nsComponentManagerBase* nsWorld::GetManagerForComponentType(const nsRTTI* pComponentRtti) const
{
  NS_ASSERT_DEV(pComponentRtti->IsDerivedFrom<nsComponent>(), "Invalid component type '{0}'", pComponentRtti->GetTypeName());

  return nsStaticCast<const nsComponentManagerBase*>(GetModule(pComponentRtti));
}

inline bool nsWorld::IsValidComponent(const nsComponentHandle& hComponent) const
{
  CheckForReadAccess();
  const nsWorldModuleTypeId uiTypeId = hComponent.m_InternalId.m_TypeId;

  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    if (const nsWorldModule* pModule = m_Data.m_Modules[uiTypeId])
    {
      return static_cast<const nsComponentManagerBase*>(pModule)->IsValidComponent(hComponent);
    }
  }

  return false;
}

template <typename ComponentType>
inline bool nsWorld::TryGetComponent(const nsComponentHandle& hComponent, ComponentType*& out_pComponent)
{
  CheckForWriteAccess();
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsComponent, ComponentType), "Not a valid component type");

  const nsWorldModuleTypeId uiTypeId = hComponent.m_InternalId.m_TypeId;

  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    if (nsWorldModule* pModule = m_Data.m_Modules[uiTypeId])
    {
      nsComponent* pComponent = nullptr;
      bool bResult = static_cast<nsComponentManagerBase*>(pModule)->TryGetComponent(hComponent, pComponent);
      out_pComponent = nsDynamicCast<ComponentType*>(pComponent);
      return bResult && out_pComponent != nullptr;
    }
  }

  return false;
}

template <typename ComponentType>
inline bool nsWorld::TryGetComponent(const nsComponentHandle& hComponent, const ComponentType*& out_pComponent) const
{
  CheckForReadAccess();
  NS_CHECK_AT_COMPILETIME_MSG(NS_IS_DERIVED_FROM_STATIC(nsComponent, ComponentType), "Not a valid component type");

  const nsWorldModuleTypeId uiTypeId = hComponent.m_InternalId.m_TypeId;

  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    if (const nsWorldModule* pModule = m_Data.m_Modules[uiTypeId])
    {
      const nsComponent* pComponent = nullptr;
      bool bResult = static_cast<const nsComponentManagerBase*>(pModule)->TryGetComponent(hComponent, pComponent);
      out_pComponent = nsDynamicCast<const ComponentType*>(pComponent);
      return bResult && out_pComponent != nullptr;
    }
  }

  return false;
}

NS_FORCE_INLINE void nsWorld::SendMessage(const nsGameObjectHandle& hReceiverObject, nsMessage& ref_msg)
{
  CheckForWriteAccess();

  nsGameObject* pReceiverObject = nullptr;
  if (TryGetObject(hReceiverObject, pReceiverObject))
  {
    pReceiverObject->SendMessage(ref_msg);
  }
  else
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (ref_msg.GetDebugMessageRouting())
    {
      nsLog::Warning("nsWorld::SendMessage: The receiver nsGameObject for message of type '{0}' does not exist.", ref_msg.GetId());
    }
#endif
  }
}

NS_FORCE_INLINE void nsWorld::SendMessageRecursive(const nsGameObjectHandle& hReceiverObject, nsMessage& ref_msg)
{
  CheckForWriteAccess();

  nsGameObject* pReceiverObject = nullptr;
  if (TryGetObject(hReceiverObject, pReceiverObject))
  {
    pReceiverObject->SendMessageRecursive(ref_msg);
  }
  else
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (ref_msg.GetDebugMessageRouting())
    {
      nsLog::Warning("nsWorld::SendMessageRecursive: The receiver nsGameObject for message of type '{0}' does not exist.", ref_msg.GetId());
    }
#endif
  }
}

NS_ALWAYS_INLINE void nsWorld::PostMessage(
  const nsGameObjectHandle& hReceiverObject, const nsMessage& msg, nsTime delay, nsObjectMsgQueueType::Enum queueType) const
{
  // This method is allowed to be called from multiple threads.
  PostMessage(hReceiverObject, msg, queueType, delay, false);
}

NS_ALWAYS_INLINE void nsWorld::PostMessageRecursive(
  const nsGameObjectHandle& hReceiverObject, const nsMessage& msg, nsTime delay, nsObjectMsgQueueType::Enum queueType) const
{
  // This method is allowed to be called from multiple threads.
  PostMessage(hReceiverObject, msg, queueType, delay, true);
}

NS_FORCE_INLINE void nsWorld::SendMessage(const nsComponentHandle& hReceiverComponent, nsMessage& ref_msg)
{
  CheckForWriteAccess();

  nsComponent* pReceiverComponent = nullptr;
  if (TryGetComponent(hReceiverComponent, pReceiverComponent))
  {
    pReceiverComponent->SendMessage(ref_msg);
  }
  else
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (ref_msg.GetDebugMessageRouting())
    {
      nsLog::Warning("nsWorld::SendMessage: The receiver nsComponent for message of type '{0}' does not exist.", ref_msg.GetId());
    }
#endif
  }
}

NS_ALWAYS_INLINE void nsWorld::SetWorldSimulationEnabled(bool bEnable)
{
  m_Data.m_bSimulateWorld = bEnable;
}

NS_ALWAYS_INLINE bool nsWorld::GetWorldSimulationEnabled() const
{
  return m_Data.m_bSimulateWorld;
}

NS_ALWAYS_INLINE const nsSharedPtr<nsTask>& nsWorld::GetUpdateTask()
{
  return m_pUpdateTask;
}

NS_ALWAYS_INLINE nsUInt32 nsWorld::GetUpdateCounter() const
{
  return m_Data.m_uiUpdateCounter;
}

NS_FORCE_INLINE nsSpatialSystem* nsWorld::GetSpatialSystem()
{
  CheckForWriteAccess();

  return m_Data.m_pSpatialSystem.Borrow();
}

NS_FORCE_INLINE const nsSpatialSystem* nsWorld::GetSpatialSystem() const
{
  CheckForReadAccess();

  return m_Data.m_pSpatialSystem.Borrow();
}

NS_ALWAYS_INLINE void nsWorld::GetCoordinateSystem(const nsVec3& vGlobalPosition, nsCoordinateSystem& out_coordinateSystem) const
{
  m_Data.m_pCoordinateSystemProvider->GetCoordinateSystem(vGlobalPosition, out_coordinateSystem);
}

NS_ALWAYS_INLINE nsCoordinateSystemProvider& nsWorld::GetCoordinateSystemProvider()
{
  return *(m_Data.m_pCoordinateSystemProvider.Borrow());
}

NS_ALWAYS_INLINE const nsCoordinateSystemProvider& nsWorld::GetCoordinateSystemProvider() const
{
  return *(m_Data.m_pCoordinateSystemProvider.Borrow());
}

NS_ALWAYS_INLINE nsClock& nsWorld::GetClock()
{
  return m_Data.m_Clock;
}

NS_ALWAYS_INLINE const nsClock& nsWorld::GetClock() const
{
  return m_Data.m_Clock;
}

NS_ALWAYS_INLINE nsRandom& nsWorld::GetRandomNumberGenerator()
{
  return m_Data.m_Random;
}

NS_ALWAYS_INLINE nsAllocator* nsWorld::GetAllocator()
{
  return &m_Data.m_Allocator;
}

NS_ALWAYS_INLINE nsInternal::WorldLargeBlockAllocator* nsWorld::GetBlockAllocator()
{
  return &m_Data.m_BlockAllocator;
}

NS_ALWAYS_INLINE nsDoubleBufferedLinearAllocator* nsWorld::GetStackAllocator()
{
  return &m_Data.m_StackAllocator;
}

NS_ALWAYS_INLINE nsInternal::WorldData::ReadMarker& nsWorld::GetReadMarker() const
{
  return m_Data.m_ReadMarker;
}

NS_ALWAYS_INLINE nsInternal::WorldData::WriteMarker& nsWorld::GetWriteMarker()
{
  return m_Data.m_WriteMarker;
}

NS_FORCE_INLINE void nsWorld::SetUserData(void* pUserData)
{
  CheckForWriteAccess();

  m_Data.m_pUserData = pUserData;
}

NS_FORCE_INLINE void* nsWorld::GetUserData() const
{
  CheckForReadAccess();

  return m_Data.m_pUserData;
}

constexpr nsUInt64 nsWorld::GetMaxNumGameObjects()
{
  return nsGameObjectId::MAX_INSTANCES - 2;
}

constexpr nsUInt64 nsWorld::GetMaxNumHierarchyLevels()
{
  return 1 << (sizeof(nsGameObject::m_uiHierarchyLevel) * 8);
}

constexpr nsUInt64 nsWorld::GetMaxNumComponentsPerType()
{
  return nsComponentId::MAX_INSTANCES - 1;
}

constexpr nsUInt64 nsWorld::GetMaxNumWorldModules()
{
  return NS_MAX_WORLD_MODULE_TYPES;
}

constexpr nsUInt64 nsWorld::GetMaxNumComponentTypes()
{
  return NS_MAX_COMPONENT_TYPES;
}

constexpr nsUInt64 nsWorld::GetMaxNumWorlds()
{
  return NS_MAX_WORLDS;
}

// static
NS_ALWAYS_INLINE nsUInt32 nsWorld::GetWorldCount()
{
  return s_Worlds.GetCount();
}

// static
NS_ALWAYS_INLINE nsWorld* nsWorld::GetWorld(nsUInt32 uiIndex)
{
  return s_Worlds[uiIndex];
}

// static
NS_ALWAYS_INLINE nsWorld* nsWorld::GetWorld(const nsGameObjectHandle& hObject)
{
  return s_Worlds[hObject.GetInternalID().m_WorldIndex];
}

// static
NS_ALWAYS_INLINE nsWorld* nsWorld::GetWorld(const nsComponentHandle& hComponent)
{
  return s_Worlds[hComponent.GetInternalID().m_WorldIndex];
}

NS_ALWAYS_INLINE void nsWorld::CheckForReadAccess() const
{
  NS_ASSERT_DEV(m_Data.m_iReadCounter > 0, "Trying to read from World '{0}', but it is not marked for reading.", GetName());
}

NS_ALWAYS_INLINE void nsWorld::CheckForWriteAccess() const
{
  NS_ASSERT_DEV(
    m_Data.m_WriteThreadID == nsThreadUtils::GetCurrentThreadID(), "Trying to write to World '{0}', but it is not marked for writing.", GetName());
}

NS_ALWAYS_INLINE nsGameObject* nsWorld::GetObjectUnchecked(nsUInt32 uiIndex) const
{
  return m_Data.m_Objects.GetValueUnchecked(uiIndex);
}

NS_ALWAYS_INLINE bool nsWorld::ReportErrorWhenStaticObjectMoves() const
{
  return m_Data.m_bReportErrorWhenStaticObjectMoves;
}

NS_ALWAYS_INLINE float nsWorld::GetInvDeltaSeconds() const
{
  const float fDelta = (float)m_Data.m_Clock.GetTimeDiff().GetSeconds();
  if (fDelta > 0.0f)
  {
    return 1.0f / fDelta;
  }

  // when the clock is paused just use zero
  return 0.0f;
}
