#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptComponent.h>
#include <Core/Scripting/ScriptCoroutine.h>
#include <Core/Scripting/ScriptWorldModule.h>
#include <Foundation/Types/VariantTypeRegistry.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsScriptCoroutineHandle, nsNoBase, 1, nsRTTIDefaultAllocator<nsScriptCoroutineHandle>)
NS_END_STATIC_REFLECTED_TYPE;
NS_DEFINE_CUSTOM_VARIANT_TYPE(nsScriptCoroutineHandle);

NS_BEGIN_STATIC_REFLECTED_TYPE(nsScriptCoroutine, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY_READ_ONLY("Name", GetName),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_FUNCTION_PROPERTY(UpdateAndSchedule),
  }
  NS_END_FUNCTIONS;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsScriptCoroutine::nsScriptCoroutine() = default;

nsScriptCoroutine::~nsScriptCoroutine()
{
  NS_ASSERT_DEV(m_pOwnerModule == nullptr, "Deinitialize was not called");
}

void nsScriptCoroutine::UpdateAndSchedule(nsTime deltaTimeSinceLastUpdate)
{
  auto result = Update(deltaTimeSinceLastUpdate);

  // Has been deleted during update
  if (m_pOwnerModule == nullptr)
    return;

  if (result.m_State == Result::State::Running)
  {
    // We can safely pass false here since we would not end up here if the coroutine is used in a simulation only function
    // but the simulation is not running because then the outer function should not have been called.
    const bool bOnlyWhenSimulating = false;
    m_pOwnerModule->AddUpdateFunctionToSchedule(GetUpdateFunctionProperty(), this, result.m_MaxDelay, bOnlyWhenSimulating);
  }
  else
  {
    m_pOwnerModule->StopAndDeleteCoroutine(GetHandle());
  }
}

void nsScriptCoroutine::Initialize(nsScriptCoroutineId id, nsStringView sName, nsScriptInstance& inout_instance, nsScriptWorldModule& inout_ownerModule)
{
  m_Id = id;
  m_sName.Assign(sName);
  m_pInstance = &inout_instance;
  m_pOwnerModule = &inout_ownerModule;
}

void nsScriptCoroutine::Deinitialize()
{
  m_pOwnerModule->RemoveUpdateFunctionToSchedule(GetUpdateFunctionProperty(), this);
  m_pOwnerModule = nullptr;
}

// static
const nsAbstractFunctionProperty* nsScriptCoroutine::GetUpdateFunctionProperty()
{
  static const nsAbstractFunctionProperty* pUpdateFunctionProperty = []() -> const nsAbstractFunctionProperty*
  {
    const nsRTTI* pType = nsGetStaticRTTI<nsScriptCoroutine>();
    auto functions = pType->GetFunctions();
    for (auto pFunc : functions)
    {
      if (nsStringUtils::IsEqual(pFunc->GetPropertyName(), "UpdateAndSchedule"))
      {
        return pFunc;
      }
    }
    return nullptr;
  }();

  return pUpdateFunctionProperty;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsScriptCoroutineCreationMode, 1)
  NS_ENUM_CONSTANTS(nsScriptCoroutineCreationMode::StopOther, nsScriptCoroutineCreationMode::DontCreateNew, nsScriptCoroutineCreationMode::AllowOverlap)
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

//////////////////////////////////////////////////////////////////////////

nsScriptCoroutineRTTI::nsScriptCoroutineRTTI(nsStringView sName, nsUniquePtr<nsRTTIAllocator>&& pAllocator)
  : nsRTTI(nullptr, nsGetStaticRTTI<nsScriptCoroutine>(), 0, 1, nsVariantType::Invalid, nsTypeFlags::Class, nullptr, nsArrayPtr<const nsAbstractProperty*>(), nsArrayPtr<const nsAbstractFunctionProperty*>(), nsArrayPtr<const nsPropertyAttribute*>(), nsArrayPtr<nsAbstractMessageHandler*>(), nsArrayPtr<nsMessageSenderInfo>(), nullptr)
  , m_sTypeNameStorage(sName)
  , m_pAllocatorStorage(std::move(pAllocator))
{
  m_sTypeName = m_sTypeNameStorage;
  m_pAllocator = m_pAllocatorStorage.Borrow();

  RegisterType();

  SetupParentHierarchy();
}

nsScriptCoroutineRTTI::~nsScriptCoroutineRTTI()
{
  UnregisterType();
  m_sTypeName = nullptr;
}

//////////////////////////////////////////////////////////////////////////

nsScriptCoroutineFunctionProperty::nsScriptCoroutineFunctionProperty(nsStringView sName, const nsSharedPtr<nsScriptCoroutineRTTI>& pType, nsScriptCoroutineCreationMode::Enum creationMode)
  : nsScriptFunctionProperty(sName)
  , m_pType(pType)
  , m_CreationMode(creationMode)
{
}

nsScriptCoroutineFunctionProperty::~nsScriptCoroutineFunctionProperty() = default;

void nsScriptCoroutineFunctionProperty::Execute(void* pInstance, nsArrayPtr<nsVariant> arguments, nsVariant& out_returnValue) const
{
  NS_ASSERT_DEBUG(pInstance != nullptr, "Invalid instance");
  auto pScriptInstance = static_cast<nsScriptInstance*>(pInstance);

  nsWorld* pWorld = pScriptInstance->GetWorld();
  if (pWorld == nullptr)
  {
    nsLog::Error("Script coroutines need a script instance with a valid nsWorld");
    return;
  }

  auto pModule = pWorld->GetOrCreateModule<nsScriptWorldModule>();

  nsScriptCoroutine* pCoroutine = nullptr;
  auto hCoroutine = pModule->CreateCoroutine(m_pType.Borrow(), m_szPropertyName, *pScriptInstance, m_CreationMode, pCoroutine);

  if (pCoroutine != nullptr)
  {
    nsHybridArray<nsVariant, 8> finalArgs;
    finalArgs = arguments;
    finalArgs.PushBack(hCoroutine);

    pModule->StartCoroutine(hCoroutine, finalArgs);
  }
}

//////////////////////////////////////////////////////////////////////////

nsScriptCoroutineMessageHandler::nsScriptCoroutineMessageHandler(nsStringView sName, const nsScriptMessageDesc& desc, const nsSharedPtr<nsScriptCoroutineRTTI>& pType, nsScriptCoroutineCreationMode::Enum creationMode)
  : nsScriptMessageHandler(desc)
  , m_pType(pType)
  , m_CreationMode(creationMode)
{
  m_sName.Assign(sName);
  m_DispatchFunc = &Dispatch;
}

nsScriptCoroutineMessageHandler::~nsScriptCoroutineMessageHandler() = default;

// static
void nsScriptCoroutineMessageHandler::Dispatch(nsAbstractMessageHandler* pSelf, void* pInstance, nsMessage& ref_msg)
{
  NS_ASSERT_DEBUG(pInstance != nullptr, "Invalid instance");
  auto pHandler = static_cast<nsScriptCoroutineMessageHandler*>(pSelf);
  auto pComponent = static_cast<nsScriptComponent*>(pInstance);
  auto pScriptInstance = pComponent->GetScriptInstance();

  nsWorld* pWorld = pScriptInstance->GetWorld();
  if (pWorld == nullptr)
  {
    nsLog::Error("Script coroutines need a script instance with a valid nsWorld");
    return;
  }

  auto pModule = pWorld->GetOrCreateModule<nsScriptWorldModule>();

  nsScriptCoroutine* pCoroutine = nullptr;
  auto hCoroutine = pModule->CreateCoroutine(pHandler->m_pType.Borrow(), pHandler->m_sName, *pScriptInstance, pHandler->m_CreationMode, pCoroutine);

  if (pCoroutine != nullptr)
  {
    nsHybridArray<nsVariant, 8> arguments;
    pHandler->FillMessagePropertyValues(ref_msg, arguments);
    arguments.PushBack(hCoroutine);

    pModule->StartCoroutine(hCoroutine, arguments);
  }
}


NS_STATICLINK_FILE(Core, Core_Scripting_Implementation_ScriptCoroutine);
