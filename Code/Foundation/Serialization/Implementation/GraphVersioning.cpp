#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>
#include <Foundation/Serialization/GraphVersioning.h>
#include <Foundation/Serialization/RttiConverter.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsTypeVersionInfo, nsNoBase, 1, nsRTTIDefaultAllocator<nsTypeVersionInfo>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("TypeName", GetTypeName, SetTypeName),
    NS_ACCESSOR_PROPERTY("ParentTypeName", GetParentTypeName, SetParentTypeName),
    NS_MEMBER_PROPERTY("TypeVersion", m_uiTypeVersion),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

const char* nsTypeVersionInfo::GetTypeName() const
{
  return m_sTypeName.GetData();
}

void nsTypeVersionInfo::SetTypeName(const char* szName)
{
  m_sTypeName.Assign(szName);
}

const char* nsTypeVersionInfo::GetParentTypeName() const
{
  return m_sParentTypeName.GetData();
}

void nsTypeVersionInfo::SetParentTypeName(const char* szName)
{
  m_sParentTypeName.Assign(szName);
}

void nsGraphPatchContext::PatchBaseClass(const char* szType, nsUInt32 uiTypeVersion, bool bForcePatch)
{
  nsHashedString sType;
  sType.Assign(szType);
  for (nsUInt32 uiBaseClassIndex = m_uiBaseClassIndex; uiBaseClassIndex < m_BaseClasses.GetCount(); ++uiBaseClassIndex)
  {
    if (m_BaseClasses[uiBaseClassIndex].m_sType == sType)
    {
      Patch(uiBaseClassIndex, uiTypeVersion, bForcePatch);
      return;
    }
  }
  NS_REPORT_FAILURE("Base class of name '{0}' not found in parent types of '{1}'", sType.GetData(), m_pNode->GetType());
}

void nsGraphPatchContext::RenameClass(const char* szTypeName)
{
  m_pNode->SetType(m_pGraph->RegisterString(szTypeName));
  m_BaseClasses[m_uiBaseClassIndex].m_sType.Assign(szTypeName);
}


void nsGraphPatchContext::RenameClass(const char* szTypeName, nsUInt32 uiVersion)
{
  m_pNode->SetType(m_pGraph->RegisterString(szTypeName));
  m_BaseClasses[m_uiBaseClassIndex].m_sType.Assign(szTypeName);
  // After a Patch is applied, the version is always increased. So if we want to change the version we need to reduce it by one so that in the next patch loop the requested version is not skipped.
  NS_ASSERT_DEV(uiVersion > 0, "Cannot change the version of a class to 0, target version must be at least 1.");
  m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion = uiVersion - 1;
}

void nsGraphPatchContext::ChangeBaseClass(nsArrayPtr<nsVersionKey> baseClasses)
{
  m_BaseClasses.SetCount(m_uiBaseClassIndex + 1 + baseClasses.GetCount());
  for (nsUInt32 i = 0; i < baseClasses.GetCount(); i++)
  {
    m_BaseClasses[m_uiBaseClassIndex + 1 + i] = baseClasses[i];
  }
}

//////////////////////////////////////////////////////////////////////////

nsGraphPatchContext::nsGraphPatchContext(nsGraphVersioning* pParent, nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph)
{
  NS_PROFILE_SCOPE("nsGraphPatchContext");
  m_pParent = pParent;
  m_pGraph = pGraph;
  if (pTypesGraph)
  {
    nsRttiConverterContext context;
    nsRttiConverterReader rttiConverter(pTypesGraph, &context);
    nsString sDescTypeName = "nsReflectedTypeDescriptor";
    auto& nodes = pTypesGraph->GetAllNodes();
    m_TypeToInfo.Reserve(nodes.GetCount());
    for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
    {
      if (it.Value()->GetType() == sDescTypeName)
      {
        nsTypeVersionInfo info;
        rttiConverter.ApplyPropertiesToObject(it.Value(), nsGetStaticRTTI<nsTypeVersionInfo>(), &info);
        m_TypeToInfo.Insert(info.m_sTypeName, info);
      }
    }
  }
}

void nsGraphPatchContext::Patch(nsAbstractObjectNode* pNode)
{
  m_pNode = pNode;
  // Build version hierarchy.
  m_BaseClasses.Clear();
  nsVersionKey key;
  key.m_sType.Assign(m_pNode->GetType());
  key.m_uiTypeVersion = m_pNode->GetTypeVersion();

  m_BaseClasses.PushBack(key);
  UpdateBaseClasses();

  // Patch
  for (m_uiBaseClassIndex = 0; m_uiBaseClassIndex < m_BaseClasses.GetCount(); ++m_uiBaseClassIndex)
  {
    const nsUInt32 uiMaxVersion = m_pParent->GetMaxPatchVersion(m_BaseClasses[m_uiBaseClassIndex].m_sType);
    Patch(m_uiBaseClassIndex, uiMaxVersion, false);
  }
  m_pNode->SetTypeVersion(m_BaseClasses[0].m_uiTypeVersion);
}


void nsGraphPatchContext::Patch(nsUInt32 uiBaseClassIndex, nsUInt32 uiTypeVersion, bool bForcePatch)
{
  if (bForcePatch)
  {
    m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion = nsMath::Min(m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion, uiTypeVersion - 1);
  }
  while (m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion < uiTypeVersion)
  {
    // Don't move this out of the loop, needed to support renaming a class which will change the key.
    nsVersionKey key = m_BaseClasses[uiBaseClassIndex];
    key.m_uiTypeVersion += 1;
    const nsGraphPatch* pPatch = nullptr;
    if (m_pParent->m_NodePatches.TryGetValue(key, pPatch))
    {
      pPatch->Patch(*this, m_pGraph, m_pNode);
      uiTypeVersion = m_pParent->GetMaxPatchVersion(m_BaseClasses[m_uiBaseClassIndex].m_sType);
    }
    // Don't use a ref to the key as the array might get resized during patching.
    // Patch function can change the type and version so we need to read m_uiTypeVersion again instead of just writing key.m_uiTypeVersion;
    m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion++;
  }
}

void nsGraphPatchContext::UpdateBaseClasses()
{
  for (;;)
  {
    nsHashedString sParentType;
    if (nsTypeVersionInfo* pInfo = m_TypeToInfo.GetValue(m_BaseClasses.PeekBack().m_sType))
    {
      m_BaseClasses.PeekBack().m_uiTypeVersion = pInfo->m_uiTypeVersion;
      sParentType = pInfo->m_sParentTypeName;
    }
    else if (const nsRTTI* pType = nsRTTI::FindTypeByName(m_BaseClasses.PeekBack().m_sType.GetData()))
    {
      m_BaseClasses.PeekBack().m_uiTypeVersion = pType->GetTypeVersion();
      if (pType->GetParentType())
      {
        sParentType.Assign(pType->GetParentType()->GetTypeName());
      }
      else
        sParentType = nsHashedString();
    }
    else
    {
      nsLog::Error("Can't patch base class, parent type of '{0}' unknown.", m_BaseClasses.PeekBack().m_sType.GetData());
      break;
    }

    if (sParentType.IsEmpty())
      break;

    nsVersionKey key;
    key.m_sType = std::move(sParentType);
    key.m_uiTypeVersion = 0;
    m_BaseClasses.PushBack(key);
  }
}

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_SINGLETON(nsGraphVersioning);

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, GraphVersioning)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Reflection"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    NS_DEFAULT_NEW(nsGraphVersioning);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsGraphVersioning* pDummy = nsGraphVersioning::GetSingleton();
    NS_DEFAULT_DELETE(pDummy);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsGraphVersioning::nsGraphVersioning()
  : m_SingletonRegistrar(this)
{
  nsPlugin::Events().AddEventHandler(nsMakeDelegate(&nsGraphVersioning::PluginEventHandler, this));

  UpdatePatches();
}

nsGraphVersioning::~nsGraphVersioning()
{
  nsPlugin::Events().RemoveEventHandler(nsMakeDelegate(&nsGraphVersioning::PluginEventHandler, this));
}

void nsGraphVersioning::PatchGraph(nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph)
{
  NS_PROFILE_SCOPE("PatchGraph");

  nsGraphPatchContext context(this, pGraph, pTypesGraph);
  for (const nsGraphPatch* pPatch : m_GraphPatches)
  {
    pPatch->Patch(context, pGraph, nullptr);
  }

  auto& nodes = pGraph->GetAllNodes();
  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    nsAbstractObjectNode* pNode = it.Value();
    context.Patch(pNode);
  }
}

void nsGraphVersioning::PluginEventHandler(const nsPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case nsPluginEvent::AfterLoadingBeforeInit:
    case nsPluginEvent::AfterUnloading:
      UpdatePatches();
      break;
    default:
      break;
  }
}

void nsGraphVersioning::UpdatePatches()
{
  m_GraphPatches.Clear();
  m_NodePatches.Clear();
  m_MaxPatchVersion.Clear();

  nsVersionKey key;
  nsGraphPatch* pInstance = nsGraphPatch::GetFirstInstance();

  while (pInstance)
  {
    switch (pInstance->GetPatchType())
    {
      case nsGraphPatch::PatchType::NodePatch:
      {
        key.m_sType.Assign(pInstance->GetType());
        key.m_uiTypeVersion = pInstance->GetTypeVersion();
        m_NodePatches.Insert(key, pInstance);

        if (nsUInt32* pMax = m_MaxPatchVersion.GetValue(key.m_sType))
        {
          *pMax = nsMath::Max(*pMax, key.m_uiTypeVersion);
        }
        else
        {
          m_MaxPatchVersion[key.m_sType] = key.m_uiTypeVersion;
        }
      }
      break;
      case nsGraphPatch::PatchType::GraphPatch:
      {
        m_GraphPatches.PushBack(pInstance);
      }
      break;
    }
    pInstance = pInstance->GetNextInstance();
  }

  m_GraphPatches.Sort([](const nsGraphPatch* a, const nsGraphPatch* b) -> bool
    { return a->GetTypeVersion() < b->GetTypeVersion(); });
}

nsUInt32 nsGraphVersioning::GetMaxPatchVersion(const nsHashedString& sType) const
{
  if (const nsUInt32* pMax = m_MaxPatchVersion.GetValue(sType))
  {
    return *pMax;
  }
  return 0;
}

NS_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_GraphVersioning);
