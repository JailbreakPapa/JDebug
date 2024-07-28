#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClassResource.h>
#include <Core/Scripting/ScriptWorldModule.h>

// clang-format off
NS_IMPLEMENT_WORLD_MODULE(nsScriptWorldModule);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsScriptWorldModule, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsScriptWorldModule::nsScriptWorldModule(nsWorld* pWorld)
  : nsWorldModule(pWorld)
{
}

nsScriptWorldModule::~nsScriptWorldModule() = default;

void nsScriptWorldModule::Initialize()
{
  SUPER::Initialize();

  {
    auto updateDesc = NS_CREATE_MODULE_UPDATE_FUNCTION_DESC(nsScriptWorldModule::CallUpdateFunctions, this);
    updateDesc.m_Phase = nsWorldModule::UpdateFunctionDesc::Phase::PreAsync;

    RegisterUpdateFunction(updateDesc);
  }
}

void nsScriptWorldModule::WorldClear()
{
  m_Scheduler.Clear();
}

void nsScriptWorldModule::AddUpdateFunctionToSchedule(const nsAbstractFunctionProperty* pFunction, void* pInstance, nsTime updateInterval, bool bOnlyWhenSimulating)
{
  FunctionContext context;
  context.m_pFunctionAndFlags.SetPtrAndFlags(pFunction, bOnlyWhenSimulating ? FunctionContext::Flags::OnlyWhenSimulating : FunctionContext::Flags::None);
  context.m_pInstance = pInstance;

  m_Scheduler.AddOrUpdateWork(context, updateInterval);
}

void nsScriptWorldModule::RemoveUpdateFunctionToSchedule(const nsAbstractFunctionProperty* pFunction, void* pInstance)
{
  FunctionContext context;
  context.m_pFunctionAndFlags.SetPtr(pFunction);
  context.m_pInstance = pInstance;

  m_Scheduler.RemoveWork(context);
}

nsScriptCoroutineHandle nsScriptWorldModule::CreateCoroutine(const nsRTTI* pCoroutineType, nsStringView sName, nsScriptInstance& inout_instance, nsScriptCoroutineCreationMode::Enum creationMode, nsScriptCoroutine*& out_pCoroutine)
{
  if (creationMode != nsScriptCoroutineCreationMode::AllowOverlap)
  {
    nsScriptCoroutine* pOverlappingCoroutine = nullptr;

    auto& runningCoroutines = m_InstanceToScriptCoroutines[&inout_instance];
    for (auto& hCoroutine : runningCoroutines)
    {
      nsUniquePtr<nsScriptCoroutine>* pCoroutine = nullptr;
      if (m_RunningScriptCoroutines.TryGetValue(hCoroutine.GetInternalID(), pCoroutine) && (*pCoroutine)->GetName() == sName)
      {
        pOverlappingCoroutine = pCoroutine->Borrow();
        break;
      }
    }

    if (pOverlappingCoroutine != nullptr)
    {
      if (creationMode == nsScriptCoroutineCreationMode::StopOther)
      {
        StopAndDeleteCoroutine(pOverlappingCoroutine->GetHandle());
      }
      else if (creationMode == nsScriptCoroutineCreationMode::DontCreateNew)
      {
        out_pCoroutine = nullptr;
        return nsScriptCoroutineHandle();
      }
      else
      {
        NS_ASSERT_NOT_IMPLEMENTED;
      }
    }
  }

  auto pCoroutine = pCoroutineType->GetAllocator()->Allocate<nsScriptCoroutine>(nsScriptAllocator::GetAllocator());

  nsScriptCoroutineId id = m_RunningScriptCoroutines.Insert(pCoroutine);
  pCoroutine->Initialize(id, sName, inout_instance, *this);

  m_InstanceToScriptCoroutines[&inout_instance].PushBack(nsScriptCoroutineHandle(id));

  out_pCoroutine = pCoroutine;
  return nsScriptCoroutineHandle(id);
}

void nsScriptWorldModule::StartCoroutine(nsScriptCoroutineHandle hCoroutine, nsArrayPtr<nsVariant> arguments)
{
  nsUniquePtr<nsScriptCoroutine>* pCoroutine = nullptr;
  if (m_RunningScriptCoroutines.TryGetValue(hCoroutine.GetInternalID(), pCoroutine))
  {
    (*pCoroutine)->StartWithVarargs(arguments);
    (*pCoroutine)->UpdateAndSchedule();
  }
}

void nsScriptWorldModule::StopAndDeleteCoroutine(nsScriptCoroutineHandle hCoroutine)
{
  nsUniquePtr<nsScriptCoroutine> pCoroutine;
  if (m_RunningScriptCoroutines.Remove(hCoroutine.GetInternalID(), &pCoroutine) == false)
    return;

  pCoroutine->Stop();
  pCoroutine->Deinitialize();
  m_DeadScriptCoroutines.PushBack(std::move(pCoroutine));
}

void nsScriptWorldModule::StopAndDeleteCoroutine(nsStringView sName, nsScriptInstance* pInstance)
{
  if (auto pCoroutines = m_InstanceToScriptCoroutines.GetValue(pInstance))
  {
    for (nsUInt32 i = 0; i < pCoroutines->GetCount();)
    {
      auto hCoroutine = (*pCoroutines)[i];

      nsUniquePtr<nsScriptCoroutine>* pCoroutine = nullptr;
      if (m_RunningScriptCoroutines.TryGetValue(hCoroutine.GetInternalID(), pCoroutine) && (*pCoroutine)->GetName() == sName)
      {
        StopAndDeleteCoroutine(hCoroutine);
      }
      else
      {
        ++i;
      }
    }
  }
}

void nsScriptWorldModule::StopAndDeleteAllCoroutines(nsScriptInstance* pInstance)
{
  if (auto pCoroutines = m_InstanceToScriptCoroutines.GetValue(pInstance))
  {
    for (auto hCoroutine : *pCoroutines)
    {
      StopAndDeleteCoroutine(hCoroutine);
    }
  }
}

bool nsScriptWorldModule::IsCoroutineFinished(nsScriptCoroutineHandle hCoroutine) const
{
  return m_RunningScriptCoroutines.Contains(hCoroutine.GetInternalID()) == false;
}

void nsScriptWorldModule::CallUpdateFunctions(const nsWorldModule::UpdateContext& context)
{
  nsWorld* pWorld = GetWorld();

  nsTime deltaTime;
  if (pWorld->GetWorldSimulationEnabled())
  {
    deltaTime = pWorld->GetClock().GetTimeDiff();
  }
  else
  {
    deltaTime = nsClock::GetGlobalClock()->GetTimeDiff();
  }

  m_Scheduler.Update(deltaTime,
    [this](const FunctionContext& context, nsTime deltaTime)
    {
      if (GetWorld()->GetWorldSimulationEnabled() || context.m_pFunctionAndFlags.GetFlags() == FunctionContext::Flags::None)
      {
        nsVariant args[] = {deltaTime};
        nsVariant returnValue;
        context.m_pFunctionAndFlags->Execute(context.m_pInstance, nsMakeArrayPtr(args), returnValue);
      }
    });

  // Delete dead coroutines
  for (nsUInt32 i = 0; i < m_DeadScriptCoroutines.GetCount(); ++i)
  {
    auto& pCoroutine = m_DeadScriptCoroutines[i];
    nsScriptInstance* pInstance = pCoroutine->GetScriptInstance();
    auto pCoroutines = m_InstanceToScriptCoroutines.GetValue(pInstance);
    NS_ASSERT_DEV(pCoroutines != nullptr, "Implementation error");

    pCoroutines->RemoveAndSwap(pCoroutine->GetHandle());
    if (pCoroutines->IsEmpty())
    {
      m_InstanceToScriptCoroutines.Remove(pInstance);
    }

    pCoroutine = nullptr;
  }
  m_DeadScriptCoroutines.Clear();
}


NS_STATICLINK_FILE(Core, Core_Scripting_Implementation_ScriptWorldModule);
