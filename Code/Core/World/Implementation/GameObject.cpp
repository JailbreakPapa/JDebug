#include <Core/CorePCH.h>

#include <Core/Messages/DeleteObjectMessage.h>
#include <Core/Messages/HierarchyChangedMessages.h>
#include <Core/Messages/TransformChangedMessage.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/World/EventMessageHandlerComponent.h>
#include <Core/World/World.h>

namespace
{
  static nsVariantArray GetDefaultTags()
  {
    nsVariantArray value(nsStaticsAllocatorWrapper::GetAllocator());
    value.PushBack("CastShadow");
    return value;
  }
} // namespace

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsGameObject, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Name", GetNameInternal, SetNameInternal),
    NS_ACCESSOR_PROPERTY("Active", GetActiveFlag, SetActiveFlag)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ACCESSOR_PROPERTY("GlobalKey", GetGlobalKeyInternal, SetGlobalKeyInternal),
    NS_ENUM_ACCESSOR_PROPERTY("Mode", nsObjectMode, Reflection_GetMode, Reflection_SetMode),
    NS_ACCESSOR_PROPERTY("LocalPosition", GetLocalPosition, SetLocalPosition)->AddAttributes(new nsSuffixAttribute(" m")),
    NS_ACCESSOR_PROPERTY("LocalRotation", GetLocalRotation, SetLocalRotation),
    NS_ACCESSOR_PROPERTY("LocalScaling", GetLocalScaling, SetLocalScaling)->AddAttributes(new nsDefaultValueAttribute(nsVec3(1.0f, 1.0f, 1.0f))),
    NS_ACCESSOR_PROPERTY("LocalUniformScaling", GetLocalUniformScaling, SetLocalUniformScaling)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
    NS_SET_MEMBER_PROPERTY("Tags", m_Tags)->AddAttributes(new nsTagSetWidgetAttribute("Default"), new nsDefaultValueAttribute(GetDefaultTags())),
    NS_SET_ACCESSOR_PROPERTY("Children", Reflection_GetChildren, Reflection_AddChild, Reflection_DetachChild)->AddFlags(nsPropertyFlags::PointerOwner | nsPropertyFlags::Hidden),
    NS_SET_ACCESSOR_PROPERTY("Components", Reflection_GetComponents, Reflection_AddComponent, Reflection_RemoveComponent)->AddFlags(nsPropertyFlags::PointerOwner),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(IsActive),
    NS_SCRIPT_FUNCTION_PROPERTY(SetCreatedByPrefab),
    NS_SCRIPT_FUNCTION_PROPERTY(WasCreatedByPrefab),

    NS_SCRIPT_FUNCTION_PROPERTY(HasName, In, "Name"),

    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_GetParent),
    NS_SCRIPT_FUNCTION_PROPERTY(FindChildByName, In, "Name", In, "Recursive")->AddFlags(nsPropertyFlags::Const),
    NS_SCRIPT_FUNCTION_PROPERTY(FindChildByPath, In, "Path")->AddFlags(nsPropertyFlags::Const),

    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_SetGlobalPosition, In, "Position"),
    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalPosition),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_SetGlobalRotation, In, "Rotation"),
    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalRotation),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_SetGlobalScaling, In, "Scaling"),
    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalScaling),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_SetGlobalTransform, In, "Transform"),
    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalTransform),

    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalDirForwards),
    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalDirRight),
    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalDirUp),

#if NS_ENABLED(NS_GAMEOBJECT_VELOCITY)
    NS_SCRIPT_FUNCTION_PROPERTY(GetLinearVelocity),
    NS_SCRIPT_FUNCTION_PROPERTY(GetAngularVelocity),
#endif

    NS_SCRIPT_FUNCTION_PROPERTY(SetTeamID, In, "Id"),
    NS_SCRIPT_FUNCTION_PROPERTY(GetTeamID),    
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgDeleteGameObject, OnMsgDeleteGameObject),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

void nsGameObject::Reflection_AddChild(nsGameObject* pChild)
{
  if (IsDynamic())
  {
    pChild->MakeDynamic();
  }

  AddChild(pChild->GetHandle(), TransformPreservation::PreserveLocal);

  // Check whether the child object was only dynamic because of its old parent
  // If that's the case make it static now.
  pChild->ConditionalMakeStatic();
}

void nsGameObject::Reflection_DetachChild(nsGameObject* pChild)
{
  DetachChild(pChild->GetHandle(), TransformPreservation::PreserveLocal);

  // The child object is now a top level object, check whether it should be static now.
  pChild->ConditionalMakeStatic();
}

nsHybridArray<nsGameObject*, 8> nsGameObject::Reflection_GetChildren() const
{
  ConstChildIterator it = GetChildren();

  nsHybridArray<nsGameObject*, 8> all;
  all.Reserve(GetChildCount());

  while (it.IsValid())
  {
    all.PushBack(it.m_pObject);
    ++it;
  }

  return all;
}

void nsGameObject::Reflection_AddComponent(nsComponent* pComponent)
{
  if (pComponent == nullptr)
    return;

  if (pComponent->IsDynamic())
  {
    MakeDynamic();
  }

  AddComponent(pComponent);
}

void nsGameObject::Reflection_RemoveComponent(nsComponent* pComponent)
{
  if (pComponent == nullptr)
    return;

  /*Don't call RemoveComponent here, Component is automatically removed when deleted.*/

  if (pComponent->IsDynamic())
  {
    ConditionalMakeStatic(pComponent);
  }
}

nsHybridArray<nsComponent*, nsGameObject::NUM_INPLACE_COMPONENTS> nsGameObject::Reflection_GetComponents() const
{
  return nsHybridArray<nsComponent*, nsGameObject::NUM_INPLACE_COMPONENTS>(m_Components);
}

nsObjectMode::Enum nsGameObject::Reflection_GetMode() const
{
  return m_Flags.IsSet(nsObjectFlags::ForceDynamic) ? nsObjectMode::ForceDynamic : nsObjectMode::Automatic;
}

void nsGameObject::Reflection_SetMode(nsObjectMode::Enum mode)
{
  if (Reflection_GetMode() == mode)
  {
    return;
  }

  if (mode == nsObjectMode::ForceDynamic)
  {
    m_Flags.Add(nsObjectFlags::ForceDynamic);
    MakeDynamic();
  }
  else
  {
    m_Flags.Remove(nsObjectFlags::ForceDynamic);
    ConditionalMakeStatic();
  }
}

nsGameObject* nsGameObject::Reflection_GetParent() const
{
  return GetWorld()->GetObjectUnchecked(m_uiParentIndex);
}

void nsGameObject::Reflection_SetGlobalPosition(const nsVec3& vPosition)
{
  SetGlobalPosition(vPosition);
}

void nsGameObject::Reflection_SetGlobalRotation(const nsQuat& qRotation)
{
  SetGlobalRotation(qRotation);
}

void nsGameObject::Reflection_SetGlobalScaling(const nsVec3& vScaling)
{
  SetGlobalScaling(vScaling);
}

void nsGameObject::Reflection_SetGlobalTransform(const nsTransform& transform)
{
  SetGlobalTransform(transform);
}

bool nsGameObject::DetermineDynamicMode(nsComponent* pComponentToIgnore /*= nullptr*/) const
{
  if (m_Flags.IsSet(nsObjectFlags::ForceDynamic))
  {
    return true;
  }

  const nsGameObject* pParent = GetParent();
  if (pParent != nullptr && pParent->IsDynamic())
  {
    return true;
  }

  for (auto pComponent : m_Components)
  {
    if (pComponent != pComponentToIgnore && pComponent->IsDynamic())
    {
      return true;
    }
  }

  return false;
}

void nsGameObject::ConditionalMakeStatic(nsComponent* pComponentToIgnore /*= nullptr*/)
{
  if (!DetermineDynamicMode(pComponentToIgnore))
  {
    MakeStaticInternal();

    for (auto it = GetChildren(); it.IsValid(); ++it)
    {
      it->ConditionalMakeStatic();
    }
  }
}

void nsGameObject::MakeStaticInternal()
{
  if (IsStatic())
  {
    return;
  }

  m_Flags.Remove(nsObjectFlags::Dynamic);

  GetWorld()->RecreateHierarchyData(this, true);
}

void nsGameObject::UpdateGlobalTransformAndBoundsRecursive()
{
  if (IsStatic() && GetWorld()->ReportErrorWhenStaticObjectMoves())
  {
    nsLog::Error("Static object '{0}' was moved during runtime.", GetName());
  }

  nsSimdTransform oldGlobalTransform = GetGlobalTransformSimd();

  m_pTransformationData->UpdateGlobalTransformNonRecursive(GetWorld()->GetUpdateCounter());

  if (nsSpatialSystem* pSpatialSystem = GetWorld()->GetSpatialSystem())
  {
    m_pTransformationData->UpdateGlobalBoundsAndSpatialData(*pSpatialSystem);
  }
  else
  {
    m_pTransformationData->UpdateGlobalBounds();
  }

  if (IsStatic() && m_Flags.IsSet(nsObjectFlags::StaticTransformChangesNotifications) && oldGlobalTransform != GetGlobalTransformSimd())
  {
    nsMsgTransformChanged msg;
    msg.m_OldGlobalTransform = nsSimdConversion::ToTransform(oldGlobalTransform);
    msg.m_NewGlobalTransform = GetGlobalTransform();

    SendMessage(msg);
  }

  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    it->UpdateGlobalTransformAndBoundsRecursive();
  }
}

void nsGameObject::UpdateLastGlobalTransform()
{
  m_pTransformationData->UpdateLastGlobalTransform(GetWorld()->GetUpdateCounter());
}

void nsGameObject::ConstChildIterator::Next()
{
  m_pObject = m_pWorld->GetObjectUnchecked(m_pObject->m_uiNextSiblingIndex);
}

nsGameObject::~nsGameObject()
{
  // Since we are using the small array base class for components we have to cleanup ourself with the correct allocator.
  m_Components.Clear();
  m_Components.Compact(GetWorld()->GetAllocator());
}

void nsGameObject::operator=(const nsGameObject& other)
{
  NS_ASSERT_DEV(m_InternalId.m_WorldIndex == other.m_InternalId.m_WorldIndex, "Cannot copy between worlds.");

  m_InternalId = other.m_InternalId;
  m_Flags = other.m_Flags;
  m_sName = other.m_sName;

  m_uiParentIndex = other.m_uiParentIndex;
  m_uiFirstChildIndex = other.m_uiFirstChildIndex;
  m_uiLastChildIndex = other.m_uiLastChildIndex;

  m_uiNextSiblingIndex = other.m_uiNextSiblingIndex;
  m_uiPrevSiblingIndex = other.m_uiPrevSiblingIndex;
  m_uiChildCount = other.m_uiChildCount;

  m_uiTeamID = other.m_uiTeamID;

  m_uiHierarchyLevel = other.m_uiHierarchyLevel;
  m_pTransformationData = other.m_pTransformationData;
  m_pTransformationData->m_pObject = this;

  if (!m_pTransformationData->m_hSpatialData.IsInvalidated())
  {
    nsSpatialSystem* pSpatialSystem = GetWorld()->GetSpatialSystem();
    pSpatialSystem->UpdateSpatialDataObject(m_pTransformationData->m_hSpatialData, this);
  }

  m_Components.CopyFrom(other.m_Components, GetWorld()->GetAllocator());
  for (nsComponent* pComponent : m_Components)
  {
    NS_ASSERT_DEV(pComponent->m_pOwner == &other, "");
    pComponent->m_pOwner = this;
  }

  m_Tags = other.m_Tags;
}

void nsGameObject::MakeDynamic()
{
  if (IsDynamic())
  {
    return;
  }

  m_Flags.Add(nsObjectFlags::Dynamic);

  GetWorld()->RecreateHierarchyData(this, false);

  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    it->MakeDynamic();
  }
}

void nsGameObject::MakeStatic()
{
  NS_ASSERT_DEV(!DetermineDynamicMode(), "This object can't be static because it has a dynamic parent or dynamic component(s) attached.");

  MakeStaticInternal();
}

void nsGameObject::SetActiveFlag(bool bEnabled)
{
  if (m_Flags.IsSet(nsObjectFlags::ActiveFlag) == bEnabled)
    return;

  m_Flags.AddOrRemove(nsObjectFlags::ActiveFlag, bEnabled);

  UpdateActiveState(GetParent() == nullptr ? true : GetParent()->IsActive());
}

void nsGameObject::UpdateActiveState(bool bParentActive)
{
  const bool bSelfActive = bParentActive && m_Flags.IsSet(nsObjectFlags::ActiveFlag);

  if (bSelfActive != m_Flags.IsSet(nsObjectFlags::ActiveState))
  {
    m_Flags.AddOrRemove(nsObjectFlags::ActiveState, bSelfActive);

    for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
    {
      m_Components[i]->UpdateActiveState(bSelfActive);
    }

    // recursively update all children
    for (auto it = GetChildren(); it.IsValid(); ++it)
    {
      it->UpdateActiveState(bSelfActive);
    }
  }
}

void nsGameObject::SetGlobalKey(const nsHashedString& sName)
{
  GetWorld()->SetObjectGlobalKey(this, sName);
}

nsStringView nsGameObject::GetGlobalKey() const
{
  return GetWorld()->GetObjectGlobalKey(this);
}

const char* nsGameObject::GetGlobalKeyInternal() const
{
  return GetWorld()->GetObjectGlobalKey(this).GetStartPointer(); // we know that it's zero terminated
}

void nsGameObject::SetParent(const nsGameObjectHandle& hParent, nsGameObject::TransformPreservation preserve)
{
  nsWorld* pWorld = GetWorld();

  nsGameObject* pParent = nullptr;
  bool _ = pWorld->TryGetObject(hParent, pParent);
  NS_IGNORE_UNUSED(_);
  pWorld->SetParent(this, pParent, preserve);
}

nsGameObject* nsGameObject::GetParent()
{
  return GetWorld()->GetObjectUnchecked(m_uiParentIndex);
}

const nsGameObject* nsGameObject::GetParent() const
{
  return GetWorld()->GetObjectUnchecked(m_uiParentIndex);
}

void nsGameObject::AddChild(const nsGameObjectHandle& hChild, nsGameObject::TransformPreservation preserve)
{
  nsWorld* pWorld = GetWorld();

  nsGameObject* pChild = nullptr;
  if (pWorld->TryGetObject(hChild, pChild))
  {
    pWorld->SetParent(pChild, this, preserve);
  }
}

void nsGameObject::DetachChild(const nsGameObjectHandle& hChild, nsGameObject::TransformPreservation preserve)
{
  nsWorld* pWorld = GetWorld();

  nsGameObject* pChild = nullptr;
  if (pWorld->TryGetObject(hChild, pChild))
  {
    if (pChild->GetParent() == this)
    {
      pWorld->SetParent(pChild, nullptr, preserve);
    }
  }
}

nsGameObject::ChildIterator nsGameObject::GetChildren()
{
  nsWorld* pWorld = GetWorld();
  return ChildIterator(pWorld->GetObjectUnchecked(m_uiFirstChildIndex), pWorld);
}

nsGameObject::ConstChildIterator nsGameObject::GetChildren() const
{
  const nsWorld* pWorld = GetWorld();
  return ConstChildIterator(pWorld->GetObjectUnchecked(m_uiFirstChildIndex), pWorld);
}

nsGameObject* nsGameObject::FindChildByName(const nsTempHashedString& sName, bool bRecursive /*= true*/)
{
  /// \test Needs a unit test

  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    if (it->m_sName == sName)
    {
      return &(*it);
    }
  }

  if (bRecursive)
  {
    for (auto it = GetChildren(); it.IsValid(); ++it)
    {
      nsGameObject* pChild = it->FindChildByName(sName, bRecursive);

      if (pChild != nullptr)
        return pChild;
    }
  }

  return nullptr;
}

nsGameObject* nsGameObject::FindChildByPath(nsStringView sPath)
{
  /// \test Needs a unit test

  if (sPath.IsEmpty())
    return this;

  const char* szSep = sPath.FindSubString("/");
  nsUInt64 uiNameHash = 0;

  if (szSep == nullptr)
    uiNameHash = nsHashingUtils::StringHash(sPath);
  else
    uiNameHash = nsHashingUtils::StringHash(nsStringView(sPath.GetStartPointer(), szSep));

  nsGameObject* pNextChild = FindChildByName(nsTempHashedString(uiNameHash), false);

  if (szSep == nullptr || pNextChild == nullptr)
    return pNextChild;

  return pNextChild->FindChildByPath(nsStringView(szSep + 1, sPath.GetEndPointer()));
}


nsGameObject* nsGameObject::SearchForChildByNameSequence(nsStringView sObjectSequence, const nsRTTI* pExpectedComponent /*= nullptr*/)
{
  /// \test Needs a unit test

  if (sObjectSequence.IsEmpty())
  {
    // in case we are searching for a specific component type, verify that it exists on this object
    if (pExpectedComponent != nullptr)
    {
      nsComponent* pComp = nullptr;
      if (!TryGetComponentOfBaseType(pExpectedComponent, pComp))
        return nullptr;
    }

    return this;
  }

  const char* szSep = sObjectSequence.FindSubString("/");
  nsStringView sNextSequence;
  nsUInt64 uiNameHash = 0;

  if (szSep == nullptr)
  {
    uiNameHash = nsHashingUtils::StringHash(sObjectSequence);
  }
  else
  {
    uiNameHash = nsHashingUtils::StringHash(nsStringView(sObjectSequence.GetStartPointer(), szSep));
    sNextSequence = nsStringView(szSep + 1, sObjectSequence.GetEndPointer());
  }

  const nsTempHashedString name(uiNameHash);

  // first go through all direct children an see if any of them actually matches the current name
  // if so, continue the recursion from there and give them the remaining search path to continue
  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    if (it->m_sName == name)
    {
      nsGameObject* res = it->SearchForChildByNameSequence(sNextSequence, pExpectedComponent);
      if (res != nullptr)
        return res;
    }
  }

  // if no direct child fulfilled the requirements, just recurse with the full name sequence
  // however, we can skip any child that already fulfilled the next sequence name,
  // because that's definitely a lost cause
  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    if (it->m_sName != name)
    {
      nsGameObject* res = it->SearchForChildByNameSequence(sObjectSequence, pExpectedComponent);
      if (res != nullptr)
        return res;
    }
  }

  return nullptr;
}


void nsGameObject::SearchForChildrenByNameSequence(nsStringView sObjectSequence, const nsRTTI* pExpectedComponent, nsHybridArray<nsGameObject*, 8>& out_objects)
{
  /// \test Needs a unit test

  if (sObjectSequence.IsEmpty())
  {
    // in case we are searching for a specific component type, verify that it exists on this object
    if (pExpectedComponent != nullptr)
    {
      nsComponent* pComp = nullptr;
      if (!TryGetComponentOfBaseType(pExpectedComponent, pComp))
        return;
    }

    out_objects.PushBack(this);
    return;
  }

  const char* szSep = sObjectSequence.FindSubString("/");
  nsStringView sNextSequence;
  nsUInt64 uiNameHash = 0;

  if (szSep == nullptr)
  {
    uiNameHash = nsHashingUtils::StringHash(sObjectSequence);
  }
  else
  {
    uiNameHash = nsHashingUtils::StringHash(nsStringView(sObjectSequence.GetStartPointer(), szSep));
    sNextSequence = nsStringView(szSep + 1, sObjectSequence.GetEndPointer());
  }

  const nsTempHashedString name(uiNameHash);

  // first go through all direct children an see if any of them actually matches the current name
  // if so, continue the recursion from there and give them the remaining search path to continue
  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    if (it->m_sName == name)
    {
      it->SearchForChildrenByNameSequence(sNextSequence, pExpectedComponent, out_objects);
    }
  }

  // if no direct child fulfilled the requirements, just recurse with the full name sequence
  // however, we can skip any child that already fulfilled the next sequence name,
  // because that's definitely a lost cause
  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    if (it->m_sName != name) // TODO: in this function it is actually debatable whether to skip these or not
    {
      it->SearchForChildrenByNameSequence(sObjectSequence, pExpectedComponent, out_objects);
    }
  }
}

nsWorld* nsGameObject::GetWorld()
{
  return nsWorld::GetWorld(m_InternalId.m_WorldIndex);
}

const nsWorld* nsGameObject::GetWorld() const
{
  return nsWorld::GetWorld(m_InternalId.m_WorldIndex);
}

nsVec3 nsGameObject::GetGlobalDirForwards() const
{
  nsCoordinateSystem coordinateSystem;
  GetWorld()->GetCoordinateSystem(GetGlobalPosition(), coordinateSystem);

  return GetGlobalRotation() * coordinateSystem.m_vForwardDir;
}

nsVec3 nsGameObject::GetGlobalDirRight() const
{
  nsCoordinateSystem coordinateSystem;
  GetWorld()->GetCoordinateSystem(GetGlobalPosition(), coordinateSystem);

  return GetGlobalRotation() * coordinateSystem.m_vRightDir;
}

nsVec3 nsGameObject::GetGlobalDirUp() const
{
  nsCoordinateSystem coordinateSystem;
  GetWorld()->GetCoordinateSystem(GetGlobalPosition(), coordinateSystem);

  return GetGlobalRotation() * coordinateSystem.m_vUpDir;
}

#if NS_ENABLED(NS_GAMEOBJECT_VELOCITY)
void nsGameObject::SetLastGlobalTransform(const nsSimdTransform& transform)
{
  m_pTransformationData->m_lastGlobalTransform = transform;
  m_pTransformationData->m_uiLastGlobalTransformUpdateCounter = GetWorld()->GetUpdateCounter();
}

nsVec3 nsGameObject::GetLinearVelocity() const
{
  const nsSimdFloat invDeltaSeconds = GetWorld()->GetInvDeltaSeconds();
  const nsSimdVec4f linearVelocity = (m_pTransformationData->m_globalTransform.m_Position - m_pTransformationData->m_lastGlobalTransform.m_Position) * invDeltaSeconds;
  return nsSimdConversion::ToVec3(linearVelocity);
}

nsVec3 nsGameObject::GetAngularVelocity() const
{
  const nsSimdFloat invDeltaSeconds = GetWorld()->GetInvDeltaSeconds();
  const nsSimdQuat q = m_pTransformationData->m_globalTransform.m_Rotation * -m_pTransformationData->m_lastGlobalTransform.m_Rotation;
  nsSimdVec4f angularVelocity = nsSimdVec4f::MakeZero();

  nsSimdVec4f axis;
  nsSimdFloat angle;
  if (q.GetRotationAxisAndAngle(axis, angle).Succeeded())
  {
    angularVelocity = axis * (angle * invDeltaSeconds);
  }
  return nsSimdConversion::ToVec3(angularVelocity);
}
#endif

void nsGameObject::UpdateGlobalTransform()
{
  m_pTransformationData->UpdateGlobalTransformRecursive(GetWorld()->GetUpdateCounter());
}

void nsGameObject::UpdateLocalBounds()
{
  nsMsgUpdateLocalBounds msg;
  msg.m_ResultingLocalBounds = nsBoundingBoxSphere::MakeInvalid();

  SendMessage(msg);

  const bool bIsAlwaysVisible = m_pTransformationData->m_localBounds.m_BoxHalfExtents.w() != nsSimdFloat::MakeZero();
  bool bRecreateSpatialData = false;

  if (m_pTransformationData->m_hSpatialData.IsInvalidated() == false)
  {
    // force spatial data re-creation if categories have changed
    bRecreateSpatialData |= m_pTransformationData->m_uiSpatialDataCategoryBitmask != msg.m_uiSpatialDataCategoryBitmask;

    // force spatial data re-creation if always visible flag has changed
    bRecreateSpatialData |= bIsAlwaysVisible != msg.m_bAlwaysVisible;

    // delete old spatial data if bounds are now invalid
    bRecreateSpatialData |= msg.m_bAlwaysVisible == false && msg.m_ResultingLocalBounds.IsValid() == false;
  }

  m_pTransformationData->m_localBounds = nsSimdConversion::ToBBoxSphere(msg.m_ResultingLocalBounds);
  m_pTransformationData->m_localBounds.m_BoxHalfExtents.SetW(msg.m_bAlwaysVisible ? 1.0f : 0.0f);
  m_pTransformationData->m_uiSpatialDataCategoryBitmask = msg.m_uiSpatialDataCategoryBitmask;

  nsSpatialSystem* pSpatialSystem = GetWorld()->GetSpatialSystem();
  if (pSpatialSystem != nullptr && (bRecreateSpatialData || m_pTransformationData->m_hSpatialData.IsInvalidated()))
  {
    m_pTransformationData->RecreateSpatialData(*pSpatialSystem);
  }

  if (IsStatic())
  {
    m_pTransformationData->UpdateGlobalBounds(pSpatialSystem);
  }
}

void nsGameObject::UpdateGlobalTransformAndBounds()
{
  m_pTransformationData->UpdateGlobalTransformRecursive(GetWorld()->GetUpdateCounter());
  m_pTransformationData->UpdateGlobalBounds(GetWorld()->GetSpatialSystem());
}

void nsGameObject::UpdateGlobalBounds()
{
  m_pTransformationData->UpdateGlobalBounds(GetWorld()->GetSpatialSystem());
}

bool nsGameObject::TryGetComponentOfBaseType(const nsRTTI* pType, nsComponent*& out_pComponent)
{
  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    if (pComponent->IsInstanceOf(pType))
    {
      out_pComponent = pComponent;
      return true;
    }
  }

  out_pComponent = nullptr;
  return false;
}

bool nsGameObject::TryGetComponentOfBaseType(const nsRTTI* pType, const nsComponent*& out_pComponent) const
{
  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    if (pComponent->IsInstanceOf(pType))
    {
      out_pComponent = pComponent;
      return true;
    }
  }

  out_pComponent = nullptr;
  return false;
}


void nsGameObject::TryGetComponentsOfBaseType(const nsRTTI* pType, nsDynamicArray<nsComponent*>& out_components)
{
  out_components.Clear();

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    if (pComponent->IsInstanceOf(pType))
    {
      out_components.PushBack(pComponent);
    }
  }
}

void nsGameObject::TryGetComponentsOfBaseType(const nsRTTI* pType, nsDynamicArray<const nsComponent*>& out_components) const
{
  out_components.Clear();

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    if (pComponent->IsInstanceOf(pType))
    {
      out_components.PushBack(pComponent);
    }
  }
}

void nsGameObject::SetTeamID(nsUInt16 uiId)
{
  m_uiTeamID = uiId;

  for (auto it = GetChildren(); it.IsValid(); ++it)
  {
    it->SetTeamID(uiId);
  }
}

nsVisibilityState nsGameObject::GetVisibilityState(nsUInt32 uiNumFramesBeforeInvisible) const
{
  if (!m_pTransformationData->m_hSpatialData.IsInvalidated())
  {
    const nsSpatialSystem* pSpatialSystem = GetWorld()->GetSpatialSystem();
    return pSpatialSystem->GetVisibilityState(m_pTransformationData->m_hSpatialData, uiNumFramesBeforeInvisible);
  }

  return nsVisibilityState::Direct;
}

void nsGameObject::OnMsgDeleteGameObject(nsMsgDeleteGameObject& msg)
{
  GetWorld()->DeleteObjectNow(GetHandle(), msg.m_bDeleteEmptyParents);
}

void nsGameObject::AddComponent(nsComponent* pComponent)
{
  NS_ASSERT_DEV(pComponent->m_pOwner == nullptr, "Component must not be added twice.");
  NS_ASSERT_DEV(IsDynamic() || !pComponent->IsDynamic(), "Cannot attach a dynamic component to a static object. Call MakeDynamic() first.");

  pComponent->m_pOwner = this;
  m_Components.PushBack(pComponent, GetWorld()->GetAllocator());
  m_Components.GetUserData<ComponentUserData>().m_uiVersion++;

  pComponent->UpdateActiveState(IsActive());

  if (m_Flags.IsSet(nsObjectFlags::ComponentChangesNotifications))
  {
    nsMsgComponentsChanged msg;
    msg.m_Type = nsMsgComponentsChanged::Type::ComponentAdded;
    msg.m_hOwner = GetHandle();
    msg.m_hComponent = pComponent->GetHandle();

    SendNotificationMessage(msg);
  }
}

void nsGameObject::RemoveComponent(nsComponent* pComponent)
{
  nsUInt32 uiIndex = m_Components.IndexOf(pComponent);
  NS_ASSERT_DEV(uiIndex != nsInvalidIndex, "Component not found");

  pComponent->m_pOwner = nullptr;
  m_Components.RemoveAtAndSwap(uiIndex);
  m_Components.GetUserData<ComponentUserData>().m_uiVersion++;

  if (m_Flags.IsSet(nsObjectFlags::ComponentChangesNotifications))
  {
    nsMsgComponentsChanged msg;
    msg.m_Type = nsMsgComponentsChanged::Type::ComponentRemoved;
    msg.m_hOwner = GetHandle();
    msg.m_hComponent = pComponent->GetHandle();

    SendNotificationMessage(msg);
  }
}

bool nsGameObject::SendMessageInternal(nsMessage& msg, bool bWasPostedMsg)
{
  bool bSentToAny = false;

  const nsRTTI* pRtti = nsGetStaticRTTI<nsGameObject>();
  bSentToAny |= pRtti->DispatchMessage(this, msg);

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    bSentToAny |= pComponent->SendMessageInternal(msg, bWasPostedMsg);
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (!bSentToAny && msg.GetDebugMessageRouting())
  {
    nsLog::Warning("nsGameObject::SendMessage: None of the target object's components had a handler for messages of type {0}.", msg.GetId());
  }
#endif

  return bSentToAny;
}

bool nsGameObject::SendMessageInternal(nsMessage& msg, bool bWasPostedMsg) const
{
  bool bSentToAny = false;

  const nsRTTI* pRtti = nsGetStaticRTTI<nsGameObject>();
  bSentToAny |= pRtti->DispatchMessage(this, msg);

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    // forward only to 'const' message handlers
    const nsComponent* pComponent = m_Components[i];
    bSentToAny |= pComponent->SendMessageInternal(msg, bWasPostedMsg);
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (!bSentToAny && msg.GetDebugMessageRouting())
  {
    nsLog::Warning("nsGameObject::SendMessage (const): None of the target object's components had a handler for messages of type {0}.", msg.GetId());
  }
#endif

  return bSentToAny;
}

bool nsGameObject::SendMessageRecursiveInternal(nsMessage& msg, bool bWasPostedMsg)
{
  bool bSentToAny = false;

  const nsRTTI* pRtti = nsGetStaticRTTI<nsGameObject>();
  bSentToAny |= pRtti->DispatchMessage(this, msg);

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    nsComponent* pComponent = m_Components[i];
    bSentToAny |= pComponent->SendMessageInternal(msg, bWasPostedMsg);
  }

  for (auto childIt = GetChildren(); childIt.IsValid(); ++childIt)
  {
    bSentToAny |= childIt->SendMessageRecursiveInternal(msg, bWasPostedMsg);
  }

  // should only be evaluated at the top function call
  // #if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  //  if (!bSentToAny && msg.GetDebugMessageRouting())
  //  {
  //    nsLog::Warning("nsGameObject::SendMessageRecursive: None of the target object's components had a handler for messages of type {0}.",
  //    msg.GetId());
  //  }
  // #endif
  // #
  return bSentToAny;
}

bool nsGameObject::SendMessageRecursiveInternal(nsMessage& msg, bool bWasPostedMsg) const
{
  bool bSentToAny = false;

  const nsRTTI* pRtti = nsGetStaticRTTI<nsGameObject>();
  bSentToAny |= pRtti->DispatchMessage(this, msg);

  for (nsUInt32 i = 0; i < m_Components.GetCount(); ++i)
  {
    // forward only to 'const' message handlers
    const nsComponent* pComponent = m_Components[i];
    bSentToAny |= pComponent->SendMessageInternal(msg, bWasPostedMsg);
  }

  for (auto childIt = GetChildren(); childIt.IsValid(); ++childIt)
  {
    bSentToAny |= childIt->SendMessageRecursiveInternal(msg, bWasPostedMsg);
  }

  // should only be evaluated at the top function call
  // #if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  //  if (!bSentToAny && msg.GetDebugMessageRouting())
  //  {
  //    nsLog::Warning("nsGameObject::SendMessageRecursive(const): None of the target object's components had a handler for messages of type
  //    {0}.", msg.GetId());
  //  }
  // #endif
  // #
  return bSentToAny;
}

void nsGameObject::PostMessage(const nsMessage& msg, nsTime delay, nsObjectMsgQueueType::Enum queueType) const
{
  GetWorld()->PostMessage(GetHandle(), msg, delay, queueType);
}

void nsGameObject::PostMessageRecursive(const nsMessage& msg, nsTime delay, nsObjectMsgQueueType::Enum queueType) const
{
  GetWorld()->PostMessageRecursive(GetHandle(), msg, delay, queueType);
}

bool nsGameObject::SendEventMessage(nsMessage& ref_msg, const nsComponent* pSenderComponent)
{
  if (auto pEventMsg = nsDynamicCast<nsEventMessage*>(&ref_msg))
  {
    pEventMsg->FillFromSenderComponent(pSenderComponent);
  }

  nsHybridArray<nsComponent*, 4> eventMsgHandlers;
  GetWorld()->FindEventMsgHandlers(ref_msg, this, eventMsgHandlers);

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (ref_msg.GetDebugMessageRouting())
  {
    if (eventMsgHandlers.IsEmpty())
    {
      nsLog::Warning("nsGameObject::SendEventMessage: None of the target object's components had a handler for messages of type {0}.", ref_msg.GetId());
    }
  }
#endif

  bool bResult = false;
  for (auto pEventMsgHandler : eventMsgHandlers)
  {
    bResult |= pEventMsgHandler->SendMessage(ref_msg);
  }
  return bResult;
}

bool nsGameObject::SendEventMessage(nsMessage& ref_msg, const nsComponent* pSenderComponent) const
{
  if (auto pEventMsg = nsDynamicCast<nsEventMessage*>(&ref_msg))
  {
    pEventMsg->FillFromSenderComponent(pSenderComponent);
  }

  nsHybridArray<const nsComponent*, 4> eventMsgHandlers;
  GetWorld()->FindEventMsgHandlers(ref_msg, this, eventMsgHandlers);

  bool bResult = false;
  for (auto pEventMsgHandler : eventMsgHandlers)
  {
    bResult |= pEventMsgHandler->SendMessage(ref_msg);
  }
  return bResult;
}

void nsGameObject::PostEventMessage(nsMessage& ref_msg, const nsComponent* pSenderComponent, nsTime delay, nsObjectMsgQueueType::Enum queueType) const
{
  if (auto pEventMsg = nsDynamicCast<nsEventMessage*>(&ref_msg))
  {
    pEventMsg->FillFromSenderComponent(pSenderComponent);
  }

  nsHybridArray<const nsComponent*, 4> eventMsgHandlers;
  GetWorld()->FindEventMsgHandlers(ref_msg, this, eventMsgHandlers);

  for (auto pEventMsgHandler : eventMsgHandlers)
  {
    pEventMsgHandler->PostMessage(ref_msg, delay, queueType);
  }
}

void nsGameObject::SetTags(const nsTagSet& tags)
{
  if (nsSpatialSystem* pSpatialSystem = GetWorld()->GetSpatialSystem())
  {
    if (m_Tags != tags)
    {
      m_Tags = tags;
      m_pTransformationData->RecreateSpatialData(*pSpatialSystem);
    }
  }
  else
  {
    m_Tags = tags;
  }
}

void nsGameObject::SetTag(const nsTag& tag)
{
  if (nsSpatialSystem* pSpatialSystem = GetWorld()->GetSpatialSystem())
  {
    if (m_Tags.IsSet(tag) == false)
    {
      m_Tags.Set(tag);
      m_pTransformationData->RecreateSpatialData(*pSpatialSystem);
    }
  }
  else
  {
    m_Tags.Set(tag);
  }
}

void nsGameObject::RemoveTag(const nsTag& tag)
{
  if (nsSpatialSystem* pSpatialSystem = GetWorld()->GetSpatialSystem())
  {
    if (m_Tags.IsSet(tag))
    {
      m_Tags.Remove(tag);
      m_pTransformationData->RecreateSpatialData(*pSpatialSystem);
    }
  }
  else
  {
    m_Tags.Remove(tag);
  }
}

void nsGameObject::FixComponentPointer(nsComponent* pOldPtr, nsComponent* pNewPtr)
{
  nsUInt32 uiIndex = m_Components.IndexOf(pOldPtr);
  NS_ASSERT_DEV(uiIndex != nsInvalidIndex, "Memory corruption?");
  m_Components[uiIndex] = pNewPtr;
}

void nsGameObject::SendNotificationMessage(nsMessage& msg)
{
  nsGameObject* pObject = this;
  while (pObject != nullptr)
  {
    pObject->SendMessage(msg);

    pObject = pObject->GetParent();
  }
}

//////////////////////////////////////////////////////////////////////////

void nsGameObject::TransformationData::UpdateLocalTransform()
{
  nsSimdTransform tLocal;

  if (m_pParentData != nullptr)
  {
    tLocal = nsSimdTransform::MakeLocalTransform(m_pParentData->m_globalTransform, m_globalTransform);
  }
  else
  {
    tLocal = m_globalTransform;
  }

  m_localPosition = tLocal.m_Position;
  m_localRotation = tLocal.m_Rotation;
  m_localScaling = tLocal.m_Scale;
  m_localScaling.SetW(1.0f);
}

void nsGameObject::TransformationData::UpdateGlobalTransformNonRecursive(nsUInt32 uiUpdateCounter)
{
  if (m_pParentData != nullptr)
  {
    UpdateGlobalTransformWithParent(uiUpdateCounter);
  }
  else
  {
    UpdateGlobalTransformWithoutParent(uiUpdateCounter);
  }
}

void nsGameObject::TransformationData::UpdateGlobalTransformRecursive(nsUInt32 uiUpdateCounter)
{
  if (m_pParentData != nullptr)
  {
    m_pParentData->UpdateGlobalTransformRecursive(uiUpdateCounter);
    UpdateGlobalTransformWithParent(uiUpdateCounter);
  }
  else
  {
    UpdateGlobalTransformWithoutParent(uiUpdateCounter);
  }
}

void nsGameObject::TransformationData::UpdateGlobalBounds(nsSpatialSystem* pSpatialSystem)
{
  if (pSpatialSystem == nullptr)
  {
    UpdateGlobalBounds();
  }
  else
  {
    UpdateGlobalBoundsAndSpatialData(*pSpatialSystem);
  }
}

void nsGameObject::TransformationData::UpdateGlobalBoundsAndSpatialData(nsSpatialSystem& ref_spatialSystem)
{
  nsSimdBBoxSphere oldGlobalBounds = m_globalBounds;

  UpdateGlobalBounds();

  const bool bIsAlwaysVisible = m_localBounds.m_BoxHalfExtents.w() != nsSimdFloat::MakeZero();
  if (m_hSpatialData.IsInvalidated() == false && bIsAlwaysVisible == false && m_globalBounds != oldGlobalBounds)
  {
    ref_spatialSystem.UpdateSpatialDataBounds(m_hSpatialData, m_globalBounds);
  }
}

void nsGameObject::TransformationData::RecreateSpatialData(nsSpatialSystem& ref_spatialSystem)
{
  if (m_hSpatialData.IsInvalidated() == false)
  {
    ref_spatialSystem.DeleteSpatialData(m_hSpatialData);
    m_hSpatialData.Invalidate();
  }

  const bool bIsAlwaysVisible = m_localBounds.m_BoxHalfExtents.w() != nsSimdFloat::MakeZero();
  if (bIsAlwaysVisible)
  {
    m_hSpatialData = ref_spatialSystem.CreateSpatialDataAlwaysVisible(m_pObject, m_uiSpatialDataCategoryBitmask, m_pObject->m_Tags);
  }
  else if (m_localBounds.IsValid())
  {
    UpdateGlobalBounds();
    m_hSpatialData = ref_spatialSystem.CreateSpatialData(m_globalBounds, m_pObject, m_uiSpatialDataCategoryBitmask, m_pObject->m_Tags);
  }
}

NS_STATICLINK_FILE(Core, Core_World_Implementation_GameObject);
