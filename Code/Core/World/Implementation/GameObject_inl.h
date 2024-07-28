
NS_ALWAYS_INLINE nsGameObject::ConstChildIterator::ConstChildIterator(nsGameObject* pObject, const nsWorld* pWorld)
  : m_pObject(pObject)
  , m_pWorld(pWorld)
{
}

NS_ALWAYS_INLINE const nsGameObject& nsGameObject::ConstChildIterator::operator*() const
{
  return *m_pObject;
}

NS_ALWAYS_INLINE const nsGameObject* nsGameObject::ConstChildIterator::operator->() const
{
  return m_pObject;
}

NS_ALWAYS_INLINE nsGameObject::ConstChildIterator::operator const nsGameObject*() const
{
  return m_pObject;
}

NS_ALWAYS_INLINE bool nsGameObject::ConstChildIterator::IsValid() const
{
  return m_pObject != nullptr;
}

NS_ALWAYS_INLINE void nsGameObject::ConstChildIterator::operator++()
{
  Next();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

NS_ALWAYS_INLINE nsGameObject::ChildIterator::ChildIterator(nsGameObject* pObject, const nsWorld* pWorld)
  : ConstChildIterator(pObject, pWorld)
{
}

NS_ALWAYS_INLINE nsGameObject& nsGameObject::ChildIterator::operator*()
{
  return *m_pObject;
}

NS_ALWAYS_INLINE nsGameObject* nsGameObject::ChildIterator::operator->()
{
  return m_pObject;
}

NS_ALWAYS_INLINE nsGameObject::ChildIterator::operator nsGameObject*()
{
  return m_pObject;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline nsGameObject::nsGameObject() = default;

NS_ALWAYS_INLINE nsGameObject::nsGameObject(const nsGameObject& other)
{
  *this = other;
}

NS_ALWAYS_INLINE nsGameObjectHandle nsGameObject::GetHandle() const
{
  return nsGameObjectHandle(m_InternalId);
}

NS_ALWAYS_INLINE bool nsGameObject::IsDynamic() const
{
  return m_Flags.IsSet(nsObjectFlags::Dynamic);
}

NS_ALWAYS_INLINE bool nsGameObject::IsStatic() const
{
  return !m_Flags.IsSet(nsObjectFlags::Dynamic);
}

NS_ALWAYS_INLINE bool nsGameObject::GetActiveFlag() const
{
  return m_Flags.IsSet(nsObjectFlags::ActiveFlag);
}

NS_ALWAYS_INLINE bool nsGameObject::IsActive() const
{
  return m_Flags.IsSet(nsObjectFlags::ActiveState);
}

NS_ALWAYS_INLINE void nsGameObject::SetName(nsStringView sName)
{
  m_sName.Assign(sName);
}

NS_ALWAYS_INLINE void nsGameObject::SetName(const nsHashedString& sName)
{
  m_sName = sName;
}

NS_ALWAYS_INLINE void nsGameObject::SetGlobalKey(nsStringView sKey)
{
  nsHashedString sGlobalKey;
  sGlobalKey.Assign(sKey);
  SetGlobalKey(sGlobalKey);
}

NS_ALWAYS_INLINE nsStringView nsGameObject::GetName() const
{
  return m_sName.GetView();
}

NS_ALWAYS_INLINE void nsGameObject::SetNameInternal(const char* szName)
{
  m_sName.Assign(szName);
}

NS_ALWAYS_INLINE const char* nsGameObject::GetNameInternal() const
{
  return m_sName;
}

NS_ALWAYS_INLINE void nsGameObject::SetGlobalKeyInternal(const char* szName)
{
  SetGlobalKey(szName);
}

NS_ALWAYS_INLINE bool nsGameObject::HasName(const nsTempHashedString& sName) const
{
  return m_sName == sName;
}

NS_ALWAYS_INLINE void nsGameObject::EnableChildChangesNotifications()
{
  m_Flags.Add(nsObjectFlags::ChildChangesNotifications);
}

NS_ALWAYS_INLINE void nsGameObject::DisableChildChangesNotifications()
{
  m_Flags.Remove(nsObjectFlags::ChildChangesNotifications);
}

NS_ALWAYS_INLINE void nsGameObject::EnableParentChangesNotifications()
{
  m_Flags.Add(nsObjectFlags::ParentChangesNotifications);
}

NS_ALWAYS_INLINE void nsGameObject::DisableParentChangesNotifications()
{
  m_Flags.Remove(nsObjectFlags::ParentChangesNotifications);
}

NS_ALWAYS_INLINE void nsGameObject::AddChildren(const nsArrayPtr<const nsGameObjectHandle>& children, nsGameObject::TransformPreservation preserve)
{
  for (nsUInt32 i = 0; i < children.GetCount(); ++i)
  {
    AddChild(children[i], preserve);
  }
}

NS_ALWAYS_INLINE void nsGameObject::DetachChildren(const nsArrayPtr<const nsGameObjectHandle>& children, nsGameObject::TransformPreservation preserve)
{
  for (nsUInt32 i = 0; i < children.GetCount(); ++i)
  {
    DetachChild(children[i], preserve);
  }
}

NS_ALWAYS_INLINE nsUInt32 nsGameObject::GetChildCount() const
{
  return m_uiChildCount;
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalPosition(nsVec3 vPosition)
{
  SetLocalPosition(nsSimdConversion::ToVec3(vPosition));
}

NS_ALWAYS_INLINE nsVec3 nsGameObject::GetLocalPosition() const
{
  return nsSimdConversion::ToVec3(m_pTransformationData->m_localPosition);
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalRotation(nsQuat qRotation)
{
  SetLocalRotation(nsSimdConversion::ToQuat(qRotation));
}

NS_ALWAYS_INLINE nsQuat nsGameObject::GetLocalRotation() const
{
  return nsSimdConversion::ToQuat(m_pTransformationData->m_localRotation);
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalScaling(nsVec3 vScaling)
{
  SetLocalScaling(nsSimdConversion::ToVec3(vScaling));
}

NS_ALWAYS_INLINE nsVec3 nsGameObject::GetLocalScaling() const
{
  return nsSimdConversion::ToVec3(m_pTransformationData->m_localScaling);
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalUniformScaling(float fScaling)
{
  SetLocalUniformScaling(nsSimdFloat(fScaling));
}

NS_ALWAYS_INLINE float nsGameObject::GetLocalUniformScaling() const
{
  return m_pTransformationData->m_localScaling.w();
}

NS_ALWAYS_INLINE nsTransform nsGameObject::GetLocalTransform() const
{
  return nsSimdConversion::ToTransform(GetLocalTransformSimd());
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalPosition(const nsVec3& vPosition)
{
  SetGlobalPosition(nsSimdConversion::ToVec3(vPosition));
}

NS_ALWAYS_INLINE nsVec3 nsGameObject::GetGlobalPosition() const
{
  return nsSimdConversion::ToVec3(m_pTransformationData->m_globalTransform.m_Position);
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalRotation(const nsQuat& qRotation)
{
  SetGlobalRotation(nsSimdConversion::ToQuat(qRotation));
}

NS_ALWAYS_INLINE nsQuat nsGameObject::GetGlobalRotation() const
{
  return nsSimdConversion::ToQuat(m_pTransformationData->m_globalTransform.m_Rotation);
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalScaling(const nsVec3& vScaling)
{
  SetGlobalScaling(nsSimdConversion::ToVec3(vScaling));
}

NS_ALWAYS_INLINE nsVec3 nsGameObject::GetGlobalScaling() const
{
  return nsSimdConversion::ToVec3(m_pTransformationData->m_globalTransform.m_Scale);
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalTransform(const nsTransform& transform)
{
  SetGlobalTransform(nsSimdConversion::ToTransform(transform));
}

NS_ALWAYS_INLINE nsTransform nsGameObject::GetGlobalTransform() const
{
  return nsSimdConversion::ToTransform(m_pTransformationData->m_globalTransform);
}

NS_ALWAYS_INLINE nsTransform nsGameObject::GetLastGlobalTransform() const
{
  return nsSimdConversion::ToTransform(GetLastGlobalTransformSimd());
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalPosition(const nsSimdVec4f& vPosition, UpdateBehaviorIfStatic updateBehavior)
{
  m_pTransformationData->m_localPosition = vPosition;

  if (IsStatic() && updateBehavior == UpdateBehaviorIfStatic::UpdateImmediately)
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE const nsSimdVec4f& nsGameObject::GetLocalPositionSimd() const
{
  return m_pTransformationData->m_localPosition;
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalRotation(const nsSimdQuat& qRotation, UpdateBehaviorIfStatic updateBehavior)
{
  m_pTransformationData->m_localRotation = qRotation;

  if (IsStatic() && updateBehavior == UpdateBehaviorIfStatic::UpdateImmediately)
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE const nsSimdQuat& nsGameObject::GetLocalRotationSimd() const
{
  return m_pTransformationData->m_localRotation;
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalScaling(const nsSimdVec4f& vScaling, UpdateBehaviorIfStatic updateBehavior)
{
  nsSimdFloat uniformScale = m_pTransformationData->m_localScaling.w();
  m_pTransformationData->m_localScaling = vScaling;
  m_pTransformationData->m_localScaling.SetW(uniformScale);

  if (IsStatic() && updateBehavior == UpdateBehaviorIfStatic::UpdateImmediately)
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE const nsSimdVec4f& nsGameObject::GetLocalScalingSimd() const
{
  return m_pTransformationData->m_localScaling;
}


NS_ALWAYS_INLINE void nsGameObject::SetLocalUniformScaling(const nsSimdFloat& fScaling, UpdateBehaviorIfStatic updateBehavior)
{
  m_pTransformationData->m_localScaling.SetW(fScaling);

  if (IsStatic() && updateBehavior == UpdateBehaviorIfStatic::UpdateImmediately)
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE nsSimdFloat nsGameObject::GetLocalUniformScalingSimd() const
{
  return m_pTransformationData->m_localScaling.w();
}

NS_ALWAYS_INLINE nsSimdTransform nsGameObject::GetLocalTransformSimd() const
{
  const nsSimdVec4f vScale = m_pTransformationData->m_localScaling * m_pTransformationData->m_localScaling.w();
  return nsSimdTransform(m_pTransformationData->m_localPosition, m_pTransformationData->m_localRotation, vScale);
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalPosition(const nsSimdVec4f& vPosition)
{
  UpdateLastGlobalTransform();

  m_pTransformationData->m_globalTransform.m_Position = vPosition;

  m_pTransformationData->UpdateLocalTransform();

  if (IsStatic())
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE const nsSimdVec4f& nsGameObject::GetGlobalPositionSimd() const
{
  return m_pTransformationData->m_globalTransform.m_Position;
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalRotation(const nsSimdQuat& qRotation)
{
  UpdateLastGlobalTransform();

  m_pTransformationData->m_globalTransform.m_Rotation = qRotation;

  m_pTransformationData->UpdateLocalTransform();

  if (IsStatic())
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE const nsSimdQuat& nsGameObject::GetGlobalRotationSimd() const
{
  return m_pTransformationData->m_globalTransform.m_Rotation;
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalScaling(const nsSimdVec4f& vScaling)
{
  UpdateLastGlobalTransform();

  m_pTransformationData->m_globalTransform.m_Scale = vScaling;

  m_pTransformationData->UpdateLocalTransform();

  if (IsStatic())
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE const nsSimdVec4f& nsGameObject::GetGlobalScalingSimd() const
{
  return m_pTransformationData->m_globalTransform.m_Scale;
}


NS_ALWAYS_INLINE void nsGameObject::SetGlobalTransform(const nsSimdTransform& transform)
{
  UpdateLastGlobalTransform();

  m_pTransformationData->m_globalTransform = transform;

  // nsTransformTemplate<Type>::SetLocalTransform will produce NaNs in w components
  // of pos and scale if scale.w is not set to 1 here. This only affects builds that
  // use NS_SIMD_IMPLEMENTATION_FPU, e.g. arm atm.
  m_pTransformationData->m_globalTransform.m_Scale.SetW(1.0f);
  m_pTransformationData->UpdateLocalTransform();

  if (IsStatic())
  {
    UpdateGlobalTransformAndBoundsRecursive();
  }
}

NS_ALWAYS_INLINE const nsSimdTransform& nsGameObject::GetGlobalTransformSimd() const
{
  return m_pTransformationData->m_globalTransform;
}

NS_ALWAYS_INLINE const nsSimdTransform& nsGameObject::GetLastGlobalTransformSimd() const
{
#if NS_ENABLED(NS_GAMEOBJECT_VELOCITY)
  return m_pTransformationData->m_lastGlobalTransform;
#else
  return m_pTransformationData->m_globalTransform;
#endif
}

NS_ALWAYS_INLINE void nsGameObject::EnableStaticTransformChangesNotifications()
{
  m_Flags.Add(nsObjectFlags::StaticTransformChangesNotifications);
}

NS_ALWAYS_INLINE void nsGameObject::DisableStaticTransformChangesNotifications()
{
  m_Flags.Remove(nsObjectFlags::StaticTransformChangesNotifications);
}

NS_ALWAYS_INLINE nsBoundingBoxSphere nsGameObject::GetLocalBounds() const
{
  return nsSimdConversion::ToBBoxSphere(m_pTransformationData->m_localBounds);
}

NS_ALWAYS_INLINE nsBoundingBoxSphere nsGameObject::GetGlobalBounds() const
{
  return nsSimdConversion::ToBBoxSphere(m_pTransformationData->m_globalBounds);
}

NS_ALWAYS_INLINE const nsSimdBBoxSphere& nsGameObject::GetLocalBoundsSimd() const
{
  return m_pTransformationData->m_localBounds;
}

NS_ALWAYS_INLINE const nsSimdBBoxSphere& nsGameObject::GetGlobalBoundsSimd() const
{
  return m_pTransformationData->m_globalBounds;
}

NS_ALWAYS_INLINE nsSpatialDataHandle nsGameObject::GetSpatialData() const
{
  return m_pTransformationData->m_hSpatialData;
}

NS_ALWAYS_INLINE void nsGameObject::EnableComponentChangesNotifications()
{
  m_Flags.Add(nsObjectFlags::ComponentChangesNotifications);
}

NS_ALWAYS_INLINE void nsGameObject::DisableComponentChangesNotifications()
{
  m_Flags.Remove(nsObjectFlags::ComponentChangesNotifications);
}

template <typename T>
NS_ALWAYS_INLINE bool nsGameObject::TryGetComponentOfBaseType(T*& out_pComponent)
{
  return TryGetComponentOfBaseType(nsGetStaticRTTI<T>(), (nsComponent*&)out_pComponent);
}

template <typename T>
NS_ALWAYS_INLINE bool nsGameObject::TryGetComponentOfBaseType(const T*& out_pComponent) const
{
  return TryGetComponentOfBaseType(nsGetStaticRTTI<T>(), (const nsComponent*&)out_pComponent);
}

template <typename T>
void nsGameObject::TryGetComponentsOfBaseType(nsDynamicArray<T*>& out_components)
{
  out_components.Clear();

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    if (pComponent->IsInstanceOf<T>())
    {
      out_components.PushBack(static_cast<T*>(pComponent));
    }
  }
}

template <typename T>
void nsGameObject::TryGetComponentsOfBaseType(nsDynamicArray<const T*>& out_components) const
{
  out_components.Clear();

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    if (pComponent->IsInstanceOf<T>())
    {
      out_components.PushBack(static_cast<const T*>(pComponent));
    }
  }
}

NS_ALWAYS_INLINE nsArrayPtr<nsComponent* const> nsGameObject::GetComponents()
{
  return m_Components;
}

NS_ALWAYS_INLINE nsArrayPtr<const nsComponent* const> nsGameObject::GetComponents() const
{
  return nsMakeArrayPtr(const_cast<const nsComponent* const*>(m_Components.GetData()), m_Components.GetCount());
}

NS_ALWAYS_INLINE nsUInt16 nsGameObject::GetComponentVersion() const
{
  return m_Components.GetUserData<ComponentUserData>().m_uiVersion;
}

NS_ALWAYS_INLINE bool nsGameObject::SendMessage(nsMessage& ref_msg)
{
  return SendMessageInternal(ref_msg, false);
}

NS_ALWAYS_INLINE bool nsGameObject::SendMessage(nsMessage& ref_msg) const
{
  return SendMessageInternal(ref_msg, false);
}

NS_ALWAYS_INLINE bool nsGameObject::SendMessageRecursive(nsMessage& ref_msg)
{
  return SendMessageRecursiveInternal(ref_msg, false);
}

NS_ALWAYS_INLINE bool nsGameObject::SendMessageRecursive(nsMessage& ref_msg) const
{
  return SendMessageRecursiveInternal(ref_msg, false);
}

NS_ALWAYS_INLINE const nsTagSet& nsGameObject::GetTags() const
{
  return m_Tags;
}

NS_ALWAYS_INLINE nsUInt32 nsGameObject::GetStableRandomSeed() const
{
  return m_pTransformationData->m_uiStableRandomSeed;
}

NS_ALWAYS_INLINE void nsGameObject::SetStableRandomSeed(nsUInt32 uiSeed)
{
  m_pTransformationData->m_uiStableRandomSeed = uiSeed;
}

//////////////////////////////////////////////////////////////////////////

NS_ALWAYS_INLINE void nsGameObject::TransformationData::UpdateGlobalTransformWithoutParent(nsUInt32 uiUpdateCounter)
{
  UpdateLastGlobalTransform(uiUpdateCounter);

  m_globalTransform.m_Position = m_localPosition;
  m_globalTransform.m_Rotation = m_localRotation;
  m_globalTransform.m_Scale = m_localScaling * m_localScaling.w();
}

NS_ALWAYS_INLINE void nsGameObject::TransformationData::UpdateGlobalTransformWithParent(nsUInt32 uiUpdateCounter)
{
  UpdateLastGlobalTransform(uiUpdateCounter);

  const nsSimdVec4f vScale = m_localScaling * m_localScaling.w();
  const nsSimdTransform localTransform(m_localPosition, m_localRotation, vScale);
  m_globalTransform = nsSimdTransform::MakeGlobalTransform(m_pParentData->m_globalTransform, localTransform);
}

NS_FORCE_INLINE void nsGameObject::TransformationData::UpdateGlobalBounds()
{
  m_globalBounds = m_localBounds;
  m_globalBounds.Transform(m_globalTransform);
}

NS_ALWAYS_INLINE void nsGameObject::TransformationData::UpdateLastGlobalTransform(nsUInt32 uiUpdateCounter)
{
#if NS_ENABLED(NS_GAMEOBJECT_VELOCITY)
  if (m_uiLastGlobalTransformUpdateCounter != uiUpdateCounter)
  {
    m_lastGlobalTransform = m_globalTransform;
    m_uiLastGlobalTransformUpdateCounter = uiUpdateCounter;
  }
#endif
}
