#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>
#include <Foundation/Serialization/GraphVersioning.h>
#include <Foundation/Serialization/RttiConverter.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdTypeVersionInfo, wdNoBase, 1, wdRTTIDefaultAllocator<wdTypeVersionInfo>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ACCESSOR_PROPERTY("TypeName", GetTypeName, SetTypeName),
    WD_ACCESSOR_PROPERTY("ParentTypeName", GetParentTypeName, SetParentTypeName),
    WD_MEMBER_PROPERTY("TypeVersion", m_uiTypeVersion),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

const char* wdTypeVersionInfo::GetTypeName() const
{
  return m_sTypeName.GetData();
}

void wdTypeVersionInfo::SetTypeName(const char* szName)
{
  m_sTypeName.Assign(szName);
}

const char* wdTypeVersionInfo::GetParentTypeName() const
{
  return m_sParentTypeName.GetData();
}

void wdTypeVersionInfo::SetParentTypeName(const char* szName)
{
  m_sParentTypeName.Assign(szName);
}

void wdGraphPatchContext::PatchBaseClass(const char* szType, wdUInt32 uiTypeVersion, bool bForcePatch)
{
  wdHashedString sType;
  sType.Assign(szType);
  for (wdUInt32 uiBaseClassIndex = m_uiBaseClassIndex; uiBaseClassIndex < m_BaseClasses.GetCount(); ++uiBaseClassIndex)
  {
    if (m_BaseClasses[uiBaseClassIndex].m_sType == sType)
    {
      Patch(uiBaseClassIndex, uiTypeVersion, bForcePatch);
      return;
    }
  }
  WD_REPORT_FAILURE("Base class of name '{0}' not found in parent types of '{1}'", sType.GetData(), m_pNode->GetType());
}

void wdGraphPatchContext::RenameClass(const char* szTypeName)
{
  m_pNode->SetType(m_pGraph->RegisterString(szTypeName));
  m_BaseClasses[m_uiBaseClassIndex].m_sType.Assign(szTypeName);
}


void wdGraphPatchContext::RenameClass(const char* szTypeName, wdUInt32 uiVersion)
{
  m_pNode->SetType(m_pGraph->RegisterString(szTypeName));
  m_BaseClasses[m_uiBaseClassIndex].m_sType.Assign(szTypeName);
  // After a Patch is applied, the version is always increased. So if we want to change the version we need to reduce it by one so that in the next patch loop the requested version is not skipped.
  WD_ASSERT_DEV(uiVersion > 0, "Cannot change the version of a class to 0, target version must be at least 1.");
  m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion = uiVersion - 1;
}

void wdGraphPatchContext::ChangeBaseClass(wdArrayPtr<wdVersionKey> baseClasses)
{
  m_BaseClasses.SetCount(m_uiBaseClassIndex + 1 + baseClasses.GetCount());
  for (wdUInt32 i = 0; i < baseClasses.GetCount(); i++)
  {
    m_BaseClasses[m_uiBaseClassIndex + 1 + i] = baseClasses[i];
  }
}

//////////////////////////////////////////////////////////////////////////

wdGraphPatchContext::wdGraphPatchContext(wdGraphVersioning* pParent, wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph)
{
  WD_PROFILE_SCOPE("wdGraphPatchContext");
  m_pParent = pParent;
  m_pGraph = pGraph;
  if (pTypesGraph)
  {
    wdRttiConverterContext context;
    wdRttiConverterReader rttiConverter(pTypesGraph, &context);
    wdString sDescTypeName = "wdReflectedTypeDescriptor";
    auto& nodes = pTypesGraph->GetAllNodes();
    m_TypeToInfo.Reserve(nodes.GetCount());
    for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
    {
      if (it.Value()->GetType() == sDescTypeName)
      {
        wdTypeVersionInfo info;
        rttiConverter.ApplyPropertiesToObject(it.Value(), wdGetStaticRTTI<wdTypeVersionInfo>(), &info);
        m_TypeToInfo.Insert(info.m_sTypeName, info);
      }
    }
  }
}

void wdGraphPatchContext::Patch(wdAbstractObjectNode* pNode)
{
  m_pNode = pNode;
  // Build version hierarchy.
  m_BaseClasses.Clear();
  wdVersionKey key;
  key.m_sType.Assign(m_pNode->GetType());
  key.m_uiTypeVersion = m_pNode->GetTypeVersion();

  m_BaseClasses.PushBack(key);
  UpdateBaseClasses();

  // Patch
  for (m_uiBaseClassIndex = 0; m_uiBaseClassIndex < m_BaseClasses.GetCount(); ++m_uiBaseClassIndex)
  {
    const wdUInt32 uiMaxVersion = m_pParent->GetMaxPatchVersion(m_BaseClasses[m_uiBaseClassIndex].m_sType);
    Patch(m_uiBaseClassIndex, uiMaxVersion, false);
  }
  m_pNode->SetTypeVersion(m_BaseClasses[0].m_uiTypeVersion);
}


void wdGraphPatchContext::Patch(wdUInt32 uiBaseClassIndex, wdUInt32 uiTypeVersion, bool bForcePatch)
{
  if (bForcePatch)
  {
    m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion = wdMath::Min(m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion, uiTypeVersion - 1);
  }
  while (m_BaseClasses[m_uiBaseClassIndex].m_uiTypeVersion < uiTypeVersion)
  {
    // Don't move this out of the loop, needed to support renaming a class which will change the key.
    wdVersionKey key = m_BaseClasses[uiBaseClassIndex];
    key.m_uiTypeVersion += 1;
    const wdGraphPatch* pPatch = nullptr;
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

void wdGraphPatchContext::UpdateBaseClasses()
{
  for (;;)
  {
    wdHashedString sParentType;
    if (wdTypeVersionInfo* pInfo = m_TypeToInfo.GetValue(m_BaseClasses.PeekBack().m_sType))
    {
      m_BaseClasses.PeekBack().m_uiTypeVersion = pInfo->m_uiTypeVersion;
      sParentType = pInfo->m_sParentTypeName;
    }
    else if (const wdRTTI* pType = wdRTTI::FindTypeByName(m_BaseClasses.PeekBack().m_sType.GetData()))
    {
      m_BaseClasses.PeekBack().m_uiTypeVersion = pType->GetTypeVersion();
      if (pType->GetParentType())
      {
        sParentType.Assign(pType->GetParentType()->GetTypeName());
      }
      else
        sParentType = wdHashedString();
    }
    else
    {
      wdLog::Error("Can't patch base class, parent type of '{0}' unknown.", m_BaseClasses.PeekBack().m_sType.GetData());
      break;
    }

    if (sParentType.IsEmpty())
      break;

    wdVersionKey key;
    key.m_sType = sParentType;
    key.m_uiTypeVersion = 0;
    m_BaseClasses.PushBack(key);
  }
}

//////////////////////////////////////////////////////////////////////////

WD_IMPLEMENT_SINGLETON(wdGraphVersioning);

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, GraphVersioning)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Reflection"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    WD_DEFAULT_NEW(wdGraphVersioning);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdGraphVersioning* pDummy = wdGraphVersioning::GetSingleton();
    WD_DEFAULT_DELETE(pDummy);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdGraphVersioning::wdGraphVersioning()
  : m_SingletonRegistrar(this)
{
  wdPlugin::Events().AddEventHandler(wdMakeDelegate(&wdGraphVersioning::PluginEventHandler, this));

  UpdatePatches();
}

wdGraphVersioning::~wdGraphVersioning()
{
  wdPlugin::Events().RemoveEventHandler(wdMakeDelegate(&wdGraphVersioning::PluginEventHandler, this));
}

void wdGraphVersioning::PatchGraph(wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph)
{
  WD_PROFILE_SCOPE("PatchGraph");

  wdGraphPatchContext context(this, pGraph, pTypesGraph);
  for (const wdGraphPatch* pPatch : m_GraphPatches)
  {
    pPatch->Patch(context, pGraph, nullptr);
  }

  auto& nodes = pGraph->GetAllNodes();
  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    wdAbstractObjectNode* pNode = it.Value();
    context.Patch(pNode);
  }
}

void wdGraphVersioning::PluginEventHandler(const wdPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case wdPluginEvent::AfterLoadingBeforeInit:
    case wdPluginEvent::AfterUnloading:
      UpdatePatches();
      break;
    default:
      break;
  }
}

void wdGraphVersioning::UpdatePatches()
{
  m_GraphPatches.Clear();
  m_NodePatches.Clear();
  m_MaxPatchVersion.Clear();

  wdVersionKey key;
  wdGraphPatch* pInstance = wdGraphPatch::GetFirstInstance();

  while (pInstance)
  {
    switch (pInstance->GetPatchType())
    {
      case wdGraphPatch::PatchType::NodePatch:
      {
        key.m_sType.Assign(pInstance->GetType());
        key.m_uiTypeVersion = pInstance->GetTypeVersion();
        m_NodePatches.Insert(key, pInstance);

        if (wdUInt32* pMax = m_MaxPatchVersion.GetValue(key.m_sType))
        {
          *pMax = wdMath::Max(*pMax, key.m_uiTypeVersion);
        }
        else
        {
          m_MaxPatchVersion[key.m_sType] = key.m_uiTypeVersion;
        }
      }
      break;
      case wdGraphPatch::PatchType::GraphPatch:
      {
        m_GraphPatches.PushBack(pInstance);
      }
      break;
    }
    pInstance = pInstance->GetNextInstance();
  }

  m_GraphPatches.Sort([](const wdGraphPatch* a, const wdGraphPatch* b) -> bool { return a->GetTypeVersion() < b->GetTypeVersion(); });
}

wdUInt32 wdGraphVersioning::GetMaxPatchVersion(const wdHashedString& sType) const
{
  if (const wdUInt32* pMax = m_MaxPatchVersion.GetValue(sType))
  {
    return *pMax;
  }
  return 0;
}

WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_GraphVersioning);
