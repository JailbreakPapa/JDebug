#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptComponent.h>
#include <Core/Scripting/ScriptWorldModule.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsScriptComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("UpdateInterval", GetUpdateInterval, SetUpdateInterval)->AddAttributes(new nsClampValueAttribute(nsTime::MakeZero(), nsVariant())),
    NS_ACCESSOR_PROPERTY("ScriptClass", GetScriptClassFile, SetScriptClassFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_ScriptClass")),
    NS_MAP_ACCESSOR_PROPERTY("Parameters", GetParameters, GetParameter, SetParameter, RemoveParameter)->AddAttributes(new nsExposedParametersAttribute("ScriptClass")),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(SetScriptVariable, In, "Name", In, "Value"),
    NS_SCRIPT_FUNCTION_PROPERTY(GetScriptVariable, In, "Name"),
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Scripting"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsScriptComponent::nsScriptComponent() = default;
nsScriptComponent::~nsScriptComponent() = default;

void nsScriptComponent::SerializeComponent(nsWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);
  auto& s = stream.GetStream();

  s << m_hScriptClass;
  s << m_UpdateInterval;

  nsUInt16 uiNumParams = static_cast<nsUInt16>(m_Parameters.GetCount());
  s << uiNumParams;

  for (nsUInt32 p = 0; p < uiNumParams; ++p)
  {
    s << m_Parameters.GetKey(p);
    s << m_Parameters.GetValue(p);
  }
}

void nsScriptComponent::DeserializeComponent(nsWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  auto& s = stream.GetStream();

  s >> m_hScriptClass;
  s >> m_UpdateInterval;

  nsUInt16 uiNumParams = 0;
  s >> uiNumParams;
  m_Parameters.Reserve(uiNumParams);

  nsHashedString key;
  nsVariant value;
  for (nsUInt32 p = 0; p < uiNumParams; ++p)
  {
    s >> key;
    s >> value;

    m_Parameters.Insert(key, value);
  }
}

void nsScriptComponent::Initialize()
{
  SUPER::Initialize();

  if (m_hScriptClass.IsValid())
  {
    InstantiateScript(false);
  }
}

void nsScriptComponent::Deinitialize()
{
  SUPER::Deinitialize();

  ClearInstance(false);
}

void nsScriptComponent::OnActivated()
{
  SUPER::OnActivated();

  CallScriptFunction(nsComponent_ScriptBaseClassFunctions::OnActivated);

  AddUpdateFunctionToSchedule();
}

void nsScriptComponent::OnDeactivated()
{
  SUPER::OnDeactivated();

  CallScriptFunction(nsComponent_ScriptBaseClassFunctions::OnDeactivated);

  RemoveUpdateFunctionToSchedule();
}

void nsScriptComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  CallScriptFunction(nsComponent_ScriptBaseClassFunctions::OnSimulationStarted);
}

void nsScriptComponent::SetScriptVariable(const nsHashedString& sName, const nsVariant& value)
{
  if (m_pInstance != nullptr)
  {
    m_pInstance->SetInstanceVariable(sName, value);
  }
}

nsVariant nsScriptComponent::GetScriptVariable(const nsHashedString& sName) const
{
  if (m_pInstance != nullptr)
  {
    return m_pInstance->GetInstanceVariable(sName);
  }

  return nsVariant();
}

void nsScriptComponent::SetScriptClass(const nsScriptClassResourceHandle& hScript)
{
  if (m_hScriptClass == hScript)
    return;

  if (IsInitialized())
  {
    ClearInstance(IsActiveAndInitialized());
  }

  m_hScriptClass = hScript;

  if (IsInitialized() && m_hScriptClass.IsValid())
  {
    InstantiateScript(IsActiveAndInitialized());
  }
}

void nsScriptComponent::SetScriptClassFile(const char* szFile)
{
  nsScriptClassResourceHandle hScript;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hScript = nsResourceManager::LoadResource<nsScriptClassResource>(szFile);
  }

  SetScriptClass(hScript);
}

const char* nsScriptComponent::GetScriptClassFile() const
{
  return m_hScriptClass.IsValid() ? m_hScriptClass.GetResourceID().GetData() : "";
}

void nsScriptComponent::SetUpdateInterval(nsTime interval)
{
  m_UpdateInterval = interval;

  AddUpdateFunctionToSchedule();
}

nsTime nsScriptComponent::GetUpdateInterval() const
{
  return m_UpdateInterval;
}

const nsRangeView<const char*, nsUInt32> nsScriptComponent::GetParameters() const
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

void nsScriptComponent::SetParameter(const char* szKey, const nsVariant& value)
{
  nsHashedString hs;
  hs.Assign(szKey);

  auto it = m_Parameters.Find(hs);
  if (it != nsInvalidIndex && m_Parameters.GetValue(it) == value)
    return;

  m_Parameters[hs] = value;

  if (IsInitialized() && m_hScriptClass.IsValid())
  {
    InstantiateScript(IsActiveAndInitialized());
  }
}

void nsScriptComponent::RemoveParameter(const char* szKey)
{
  if (m_Parameters.RemoveAndCopy(nsTempHashedString(szKey)))
  {
    if (IsInitialized() && m_hScriptClass.IsValid())
    {
      InstantiateScript(IsActiveAndInitialized());
    }
  }
}

bool nsScriptComponent::GetParameter(const char* szKey, nsVariant& out_value) const
{
  nsUInt32 it = m_Parameters.Find(szKey);

  if (it == nsInvalidIndex)
    return false;

  out_value = m_Parameters.GetValue(it);
  return true;
}

void nsScriptComponent::InstantiateScript(bool bActivate)
{
  ClearInstance(IsActiveAndInitialized());

  nsResourceLock<nsScriptClassResource> pScript(m_hScriptClass, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pScript.GetAcquireResult() != nsResourceAcquireResult::Final)
  {
    nsLog::Error("Failed to load script '{}'", GetScriptClassFile());
    return;
  }

  auto pScriptType = pScript->GetType();
  if (pScriptType == nullptr || pScriptType->IsDerivedFrom(nsGetStaticRTTI<nsComponent>()) == false)
  {
    nsLog::Error("Script type '{}' is not a component", pScriptType != nullptr ? pScriptType->GetTypeName() : "NULL");
    return;
  }

  m_pScriptType = pScriptType;
  m_pMessageDispatchType = pScriptType;

  m_pInstance = pScript->Instantiate(*this, GetWorld());
  if (m_pInstance != nullptr)
  {
    m_pInstance->SetInstanceVariables(m_Parameters);
  }

  GetWorld()->AddResourceReloadFunction(m_hScriptClass, GetHandle(), nullptr,
    [](const nsWorld::ResourceReloadContext& context)
    {
      nsStaticCast<nsScriptComponent*>(context.m_pComponent)->ReloadScript();
    });

  CallScriptFunction(nsComponent_ScriptBaseClassFunctions::Initialize);
  if (bActivate)
  {
    CallScriptFunction(nsComponent_ScriptBaseClassFunctions::OnActivated);

    if (GetWorld()->GetWorldSimulationEnabled())
    {
      CallScriptFunction(nsComponent_ScriptBaseClassFunctions::OnSimulationStarted);
    }
  }

  AddUpdateFunctionToSchedule();
}

void nsScriptComponent::ClearInstance(bool bDeactivate)
{
  if (bDeactivate)
  {
    CallScriptFunction(nsComponent_ScriptBaseClassFunctions::OnDeactivated);
  }
  CallScriptFunction(nsComponent_ScriptBaseClassFunctions::Deinitialize);

  RemoveUpdateFunctionToSchedule();

  auto pModule = GetWorld()->GetOrCreateModule<nsScriptWorldModule>();
  pModule->StopAndDeleteAllCoroutines(m_pInstance.Borrow());

  GetWorld()->RemoveResourceReloadFunction(m_hScriptClass, GetHandle(), nullptr);

  m_pInstance = nullptr;
  m_pScriptType = nullptr;

  m_pMessageDispatchType = GetDynamicRTTI();
}

void nsScriptComponent::AddUpdateFunctionToSchedule()
{
  if (IsActiveAndInitialized() == false)
    return;

  auto pModule = GetWorld()->GetOrCreateModule<nsScriptWorldModule>();
  if (auto pUpdateFunction = GetScriptFunction(nsComponent_ScriptBaseClassFunctions::Update))
  {
    const bool bOnlyWhenSimulating = true;
    pModule->AddUpdateFunctionToSchedule(pUpdateFunction, m_pInstance.Borrow(), m_UpdateInterval, bOnlyWhenSimulating);
  }
}

void nsScriptComponent::RemoveUpdateFunctionToSchedule()
{
  auto pModule = GetWorld()->GetOrCreateModule<nsScriptWorldModule>();
  if (auto pUpdateFunction = GetScriptFunction(nsComponent_ScriptBaseClassFunctions::Update))
  {
    pModule->RemoveUpdateFunctionToSchedule(pUpdateFunction, m_pInstance.Borrow());
  }
}

const nsAbstractFunctionProperty* nsScriptComponent::GetScriptFunction(nsUInt32 uiFunctionIndex)
{
  if (m_pScriptType != nullptr && m_pInstance != nullptr)
  {
    return m_pScriptType->GetFunctionByIndex(uiFunctionIndex);
  }

  return nullptr;
}

void nsScriptComponent::CallScriptFunction(nsUInt32 uiFunctionIndex)
{
  if (auto pFunction = GetScriptFunction(uiFunctionIndex))
  {
    nsVariant returnValue;
    pFunction->Execute(m_pInstance.Borrow(), nsArrayPtr<nsVariant>(), returnValue);
  }
}

void nsScriptComponent::ReloadScript()
{
  InstantiateScript(IsActiveAndInitialized());
}

NS_STATICLINK_FILE(Core, Core_Scripting_Implementation_ScriptComponent);
