#include <Core/CorePCH.h>

#include <Core/Prefabs/PrefabReferenceComponent.h>
#include <Core/WorldSerializer/WorldWriter.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsPrefabReferenceComponent, 4, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Prefab", GetPrefabFile, SetPrefabFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Prefab")),
    NS_MAP_ACCESSOR_PROPERTY("Parameters", GetParameters, GetParameter, SetParameter, RemoveParameter)->AddAttributes(new nsExposedParametersAttribute("Prefab")),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Prefabs"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

enum PrefabComponentFlags
{
  SelfDeletion = 1, ///< the prefab component is currently deleting itself but does not want to remove the instantiated objects
};

nsPrefabReferenceComponent::nsPrefabReferenceComponent() = default;
nsPrefabReferenceComponent::~nsPrefabReferenceComponent() = default;

void nsPrefabReferenceComponent::SerializePrefabParameters(const nsWorld& world, nsWorldWriter& inout_stream, nsArrayMap<nsHashedString, nsVariant> parameters)
{
  // we need a copy of the parameters here, therefore we don't take it by reference

  auto& s = inout_stream.GetStream();
  const nsUInt32 numParams = parameters.GetCount();

  nsHybridArray<nsGameObjectHandle, 8> GoReferences;

  // Version 4
  {
    // to support game object references as exposed parameters (which are currently exposed as strings)
    // we need to remap the string from an 'editor uuid' to something that can be interpreted as a proper nsGameObjectHandle at runtime

    // so first we get the resolver and try to map any string parameter to a valid nsGameObjectHandle
    auto resolver = world.GetGameObjectReferenceResolver();

    if (resolver.IsValid())
    {
      nsStringBuilder tmp;

      for (nsUInt32 i = 0; i < numParams; ++i)
      {
        // if this is a string parameter
        nsVariant& var = parameters.GetValue(i);
        if (var.IsA<nsString>())
        {
          // and the resolver CAN map this string to a game object handle
          nsGameObjectHandle hObject = resolver(var.Get<nsString>().GetData(), nsComponentHandle(), nullptr);
          if (!hObject.IsInvalidated())
          {
            // write the handle properly to file (this enables correct remapping during deserialization)
            // and discard the string's value, and instead write a string that specifies the index of the serialized handle to use

            // local game object reference - index into GoReferences
            tmp.SetFormat("#!LGOR-{}", GoReferences.GetCount());
            var = tmp.GetData();

            GoReferences.PushBack(hObject);
          }
        }
      }
    }

    // now write all the nsGameObjectHandle's such that during deserialization the nsWorldReader will remap it as needed
    const nsUInt8 numRefs = static_cast<nsUInt8>(GoReferences.GetCount());
    s << numRefs;

    for (nsUInt8 i = 0; i < numRefs; ++i)
    {
      inout_stream.WriteGameObjectHandle(GoReferences[i]);
    }
  }

  // Version 2
  s << numParams;
  for (nsUInt32 i = 0; i < numParams; ++i)
  {
    s << parameters.GetKey(i);
    s << parameters.GetValue(i); // this may contain modified strings now, to map the game object handle references
  }
}

void nsPrefabReferenceComponent::DeserializePrefabParameters(nsArrayMap<nsHashedString, nsVariant>& out_parameters, nsWorldReader& inout_stream)
{
  out_parameters.Clear();

  // versioning of this stuff is tied to the version number of nsPrefabReferenceComponent
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(nsGetStaticRTTI<nsPrefabReferenceComponent>());
  auto& s = inout_stream.GetStream();

  // temp array to hold (and remap) the serialized game object handles
  nsHybridArray<nsGameObjectHandle, 8> GoReferences;

  if (uiVersion >= 4)
  {
    nsUInt8 numRefs = 0;
    s >> numRefs;
    GoReferences.SetCountUninitialized(numRefs);

    // just read them all, this will remap as necessary to the nsWorldReader
    for (nsUInt8 i = 0; i < numRefs; ++i)
    {
      GoReferences[i] = inout_stream.ReadGameObjectHandle();
    }
  }

  if (uiVersion >= 2)
  {
    nsUInt32 numParams = 0;
    s >> numParams;

    out_parameters.Reserve(numParams);

    nsHashedString key;
    nsVariant value;
    nsStringBuilder tmp;

    for (nsUInt32 i = 0; i < numParams; ++i)
    {
      s >> key;
      s >> value;

      if (value.IsA<nsString>())
      {
        // if we find a string parameter, check if it is a 'local game object reference'
        const nsString& str = value.Get<nsString>();
        if (str.StartsWith("#!LGOR-"))
        {
          // if so, extract the index into the GoReferences array
          nsInt32 idx;
          if (nsConversionUtils::StringToInt(str.GetData() + 7, idx).Succeeded())
          {
            // now we can lookup the remapped nsGameObjectHandle from our array
            const nsGameObjectHandle hObject = GoReferences[idx];

            // and stringify the handle into a 'global game object reference', ie. one that contains the internal integer data of the handle
            // a regular runtime world has a reference resolver that is capable to reverse this stringified format to a handle again
            // which will happen once 'InstantiatePrefab' passes the m_Parameters list to the newly created objects
            tmp.SetFormat("#!GGOR-{}", hObject.GetInternalID().m_Data);

            // map local game object reference to global game object reference
            value = tmp.GetData();
          }
        }
      }

      out_parameters.Insert(key, value);
    }
  }
}

void nsPrefabReferenceComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_hPrefab;

  nsPrefabReferenceComponent::SerializePrefabParameters(*GetWorld(), inout_stream, m_Parameters);
}

void nsPrefabReferenceComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  auto& s = inout_stream.GetStream();

  s >> m_hPrefab;

  if (uiVersion < 3)
  {
    bool bDummy;
    s >> bDummy;
  }

  nsPrefabReferenceComponent::DeserializePrefabParameters(m_Parameters, inout_stream);
}

void nsPrefabReferenceComponent::SetPrefabFile(const char* szFile)
{
  nsPrefabResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = nsResourceManager::LoadResource<nsPrefabResource>(szFile);
    nsResourceManager::PreloadResource(hResource);
  }

  SetPrefab(hResource);
}

const char* nsPrefabReferenceComponent::GetPrefabFile() const
{
  if (!m_hPrefab.IsValid())
    return "";

  return m_hPrefab.GetResourceID();
}

void nsPrefabReferenceComponent::SetPrefab(const nsPrefabResourceHandle& hPrefab)
{
  if (m_hPrefab == hPrefab)
    return;

  m_hPrefab = hPrefab;

  if (IsActiveAndInitialized())
  {
    // only add to update list, if not yet activated,
    // since OnActivate will do the instantiation anyway

    GetWorld()->GetComponentManager<nsPrefabReferenceComponentManager>()->AddToUpdateList(this);
  }
}

void nsPrefabReferenceComponent::InstantiatePrefab()
{
  // now instantiate the prefab
  if (m_hPrefab.IsValid())
  {
    nsResourceLock<nsPrefabResource> pResource(m_hPrefab, nsResourceAcquireMode::AllowLoadingFallback);

    nsTransform id;
    id.SetIdentity();

    nsPrefabInstantiationOptions options;
    options.m_hParent = GetOwner()->GetHandle();
    options.m_ReplaceNamedRootWithParent = "<Prefab-Root>";
    options.m_pOverrideTeamID = &GetOwner()->GetTeamID();

    // if this ID is valid, this prefab is instantiated at editor runtime
    // replicate the same ID across all instantiated sub components to get correct picking behavior
    if (GetUniqueID() != nsInvalidIndex)
    {
      nsHybridArray<nsGameObject*, 8> createdRootObjects;
      nsHybridArray<nsGameObject*, 16> createdChildObjects;

      options.m_pCreatedRootObjectsOut = &createdRootObjects;
      options.m_pCreatedChildObjectsOut = &createdChildObjects;

      nsUInt32 uiPrevCompCount = GetOwner()->GetComponents().GetCount();

      pResource->InstantiatePrefab(*GetWorld(), id, options, &m_Parameters);

      auto FixComponent = [](nsGameObject* pChild, nsUInt32 uiUniqueID)
      {
        // while exporting a scene all game objects with this flag are ignored and not exported
        // set this flag on all game objects that were created by instantiating this prefab
        // instead it should be instantiated at runtime again
        // only do this at editor time though, at regular runtime we do want to fully serialize the entire sub tree
        pChild->SetCreatedByPrefab();

        for (auto pComponent : pChild->GetComponents())
        {
          pComponent->SetUniqueID(uiUniqueID);
          pComponent->SetCreatedByPrefab();
        }
      };

      const nsUInt32 uiUniqueID = GetUniqueID();

      for (nsGameObject* pChild : createdRootObjects)
      {
        if (pChild == GetOwner())
          continue;

        FixComponent(pChild, uiUniqueID);
      }

      for (nsGameObject* pChild : createdChildObjects)
      {
        FixComponent(pChild, uiUniqueID);
      }

      for (; uiPrevCompCount < GetOwner()->GetComponents().GetCount(); ++uiPrevCompCount)
      {
        GetOwner()->GetComponents()[uiPrevCompCount]->SetUniqueID(GetUniqueID());
        GetOwner()->GetComponents()[uiPrevCompCount]->SetCreatedByPrefab();
      }
    }
    else
    {
      pResource->InstantiatePrefab(*GetWorld(), id, options, &m_Parameters);
    }
  }
}

void nsPrefabReferenceComponent::OnActivated()
{
  SUPER::OnActivated();

  // instantiate the prefab right away, such that game play code can access it as soon as possible
  // additionally the manager may update the instance later on, to properly enable editor work flows
  InstantiatePrefab();
}

void nsPrefabReferenceComponent::OnDeactivated()
{
  // if this was created procedurally during editor runtime, we do not need to clear specific nodes
  // after simulation, the scene is deleted anyway

  ClearPreviousInstances();

  SUPER::OnDeactivated();
}

void nsPrefabReferenceComponent::ClearPreviousInstances()
{
  if (GetUniqueID() != nsInvalidIndex)
  {
    // if this is in the editor, and the 'activate' flag is toggled,
    // get rid of all our created child objects

    nsArrayPtr<nsComponent* const> comps = GetOwner()->GetComponents();

    for (nsUInt32 ip1 = comps.GetCount(); ip1 > 0; ip1--)
    {
      const nsUInt32 i = ip1 - 1;

      if (comps[i] != this && // don't try to delete yourself
          comps[i]->WasCreatedByPrefab())
      {
        comps[i]->GetOwningManager()->DeleteComponent(comps[i]);
      }
    }

    for (auto it = GetOwner()->GetChildren(); it.IsValid(); ++it)
    {
      if (it->WasCreatedByPrefab())
      {
        GetWorld()->DeleteObjectNow(it->GetHandle());
      }
    }
  }
}

void nsPrefabReferenceComponent::Deinitialize()
{
  if (GetUserFlag(PrefabComponentFlags::SelfDeletion))
  {
    // do nothing, ie do not call OnDeactivated()
    // we do want to keep the created child objects around when this component gets destroyed during simulation
    // that's because the component actually deletes itself when simulation starts
    return;
  }

  // remove the children (through Deactivate)
  OnDeactivated();
}

void nsPrefabReferenceComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  if (GetUniqueID() == nsInvalidIndex)
  {
    SetUserFlag(PrefabComponentFlags::SelfDeletion, true);

    // remove the prefab reference component, to prevent issues after another serialization/deserialization
    // and also to save some memory
    DeleteComponent();
  }
}

const nsRangeView<const char*, nsUInt32> nsPrefabReferenceComponent::GetParameters() const
{
  return nsRangeView<const char*, nsUInt32>([]() -> nsUInt32
    { return 0; },
    [this]() -> nsUInt32
    { return m_Parameters.GetCount(); },
    [](nsUInt32& ref_uiIt)
    { ++ref_uiIt; },
    [this](const nsUInt32& uiIt) -> const char*
    { return m_Parameters.GetKey(uiIt).GetString().GetData(); });
}

void nsPrefabReferenceComponent::SetParameter(const char* szKey, const nsVariant& value)
{
  nsHashedString hs;
  hs.Assign(szKey);

  auto it = m_Parameters.Find(hs);
  if (it != nsInvalidIndex && m_Parameters.GetValue(it) == value)
    return;

  m_Parameters[hs] = value;

  if (IsActiveAndInitialized())
  {
    // only add to update list, if not yet activated,
    // since OnActivate will do the instantiation anyway
    GetWorld()->GetComponentManager<nsPrefabReferenceComponentManager>()->AddToUpdateList(this);
  }
}

void nsPrefabReferenceComponent::RemoveParameter(const char* szKey)
{
  if (m_Parameters.RemoveAndCopy(nsTempHashedString(szKey)))
  {
    if (IsActiveAndInitialized())
    {
      // only add to update list, if not yet activated,
      // since OnActivate will do the instantiation anyway
      GetWorld()->GetComponentManager<nsPrefabReferenceComponentManager>()->AddToUpdateList(this);
    }
  }
}

bool nsPrefabReferenceComponent::GetParameter(const char* szKey, nsVariant& out_value) const
{
  nsUInt32 it = m_Parameters.Find(szKey);

  if (it == nsInvalidIndex)
    return false;

  out_value = m_Parameters.GetValue(it);
  return true;
}

//////////////////////////////////////////////////////////////////////////

nsPrefabReferenceComponentManager::nsPrefabReferenceComponentManager(nsWorld* pWorld)
  : nsComponentManager<ComponentType, nsBlockStorageType::Compact>(pWorld)
{
  nsResourceManager::GetResourceEvents().AddEventHandler(nsMakeDelegate(&nsPrefabReferenceComponentManager::ResourceEventHandler, this));
}


nsPrefabReferenceComponentManager::~nsPrefabReferenceComponentManager()
{
  nsResourceManager::GetResourceEvents().RemoveEventHandler(nsMakeDelegate(&nsPrefabReferenceComponentManager::ResourceEventHandler, this));
}

void nsPrefabReferenceComponentManager::Initialize()
{
  auto desc = NS_CREATE_MODULE_UPDATE_FUNCTION_DESC(nsPrefabReferenceComponentManager::Update, this);

  RegisterUpdateFunction(desc);
}

void nsPrefabReferenceComponentManager::ResourceEventHandler(const nsResourceEvent& e)
{
  if (e.m_Type == nsResourceEvent::Type::ResourceContentUnloading && e.m_pResource->GetDynamicRTTI()->IsDerivedFrom<nsPrefabResource>())
  {
    nsPrefabResourceHandle hPrefab((nsPrefabResource*)(e.m_pResource));

    for (auto it = GetComponents(); it.IsValid(); it.Next())
    {
      if (it->m_hPrefab == hPrefab)
      {
        AddToUpdateList(it);
      }
    }
  }
}

void nsPrefabReferenceComponentManager::Update(const nsWorldModule::UpdateContext& context)
{
  for (auto hComp : m_ComponentsToUpdate)
  {
    nsPrefabReferenceComponent* pComponent;
    if (!TryGetComponent(hComp, pComponent))
      continue;

    pComponent->m_bInUpdateList = false;
    if (!pComponent->IsActive())
      continue;

    pComponent->ClearPreviousInstances();
    pComponent->InstantiatePrefab();
  }

  m_ComponentsToUpdate.Clear();
}

void nsPrefabReferenceComponentManager::AddToUpdateList(nsPrefabReferenceComponent* pComponent)
{
  if (!pComponent->m_bInUpdateList)
  {
    m_ComponentsToUpdate.PushBack(pComponent->GetHandle());
    pComponent->m_bInUpdateList = true;
  }
}



NS_STATICLINK_FILE(Core, Core_Prefabs_Implementation_PrefabReferenceComponent);
