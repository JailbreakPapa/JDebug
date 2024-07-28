#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <Core/Scripting/ScriptCoroutine.h>
#include <Core/Utils/IntervalScheduler.h>
#include <Core/World/World.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>

using nsScriptClassResourceHandle = nsTypedResourceHandle<class nsScriptClassResource>;
class nsScriptInstance;

class NS_CORE_DLL nsScriptWorldModule : public nsWorldModule
{
  NS_DECLARE_WORLD_MODULE();
  NS_ADD_DYNAMIC_REFLECTION(nsScriptWorldModule, nsWorldModule);
  NS_DISALLOW_COPY_AND_ASSIGN(nsScriptWorldModule);

public:
  nsScriptWorldModule(nsWorld* pWorld);
  ~nsScriptWorldModule();

  virtual void Initialize() override;
  virtual void WorldClear() override;

  void AddUpdateFunctionToSchedule(const nsAbstractFunctionProperty* pFunction, void* pInstance, nsTime updateInterval, bool bOnlyWhenSimulating);
  void RemoveUpdateFunctionToSchedule(const nsAbstractFunctionProperty* pFunction, void* pInstance);

  /// \name Coroutine Functions
  ///@{

  /// \brief Creates a new coroutine of pCoroutineType with the given name. If the creationMode prevents creating a new coroutine,
  /// this function will return an invalid handle and a nullptr in out_pCoroutine if there is already a coroutine running
  /// with the same name on the given instance.
  nsScriptCoroutineHandle CreateCoroutine(const nsRTTI* pCoroutineType, nsStringView sName, nsScriptInstance& inout_instance, nsScriptCoroutineCreationMode::Enum creationMode, nsScriptCoroutine*& out_pCoroutine);

  /// \brief Starts the coroutine with the given arguments. This will call the Start() function and then UpdateAndSchedule() once on the coroutine object.
  void StartCoroutine(nsScriptCoroutineHandle hCoroutine, nsArrayPtr<nsVariant> arguments);

  /// \brief Stops and deletes the coroutine. This will call the Stop() function and will delete the coroutine on next update of the script world module.
  void StopAndDeleteCoroutine(nsScriptCoroutineHandle hCoroutine);

  /// \brief Stops and deletes all coroutines with the given name on pInstance.
  void StopAndDeleteCoroutine(nsStringView sName, nsScriptInstance* pInstance);

  /// \brief Stops and deletes all coroutines on pInstance.
  void StopAndDeleteAllCoroutines(nsScriptInstance* pInstance);

  /// \brief Returns whether the coroutine has already finished or has been stopped.
  bool IsCoroutineFinished(nsScriptCoroutineHandle hCoroutine) const;

  ///@}

  /// \brief Returns a expression vm that can be used in custom script implementations.
  /// Make sure to only execute one expression at a time, the VM is NOT thread safe.
  nsExpressionVM& GetSharedExpressionVM() { return m_SharedExpressionVM; }

  struct FunctionContext
  {
    enum Flags : nsUInt8
    {
      None,
      OnlyWhenSimulating
    };

    nsPointerWithFlags<const nsAbstractFunctionProperty, 1> m_pFunctionAndFlags;
    void* m_pInstance = nullptr;

    bool operator==(const FunctionContext& other) const
    {
      return m_pFunctionAndFlags == other.m_pFunctionAndFlags && m_pInstance == other.m_pInstance;
    }
  };

private:
  void CallUpdateFunctions(const nsWorldModule::UpdateContext& context);

  nsIntervalScheduler<FunctionContext> m_Scheduler;

  nsIdTable<nsScriptCoroutineId, nsUniquePtr<nsScriptCoroutine>> m_RunningScriptCoroutines;
  nsHashTable<nsScriptInstance*, nsSmallArray<nsScriptCoroutineHandle, 8>> m_InstanceToScriptCoroutines;
  nsDynamicArray<nsUniquePtr<nsScriptCoroutine>> m_DeadScriptCoroutines;

  nsExpressionVM m_SharedExpressionVM;
};

//////////////////////////////////////////////////////////////////////////

template <>
struct nsHashHelper<nsScriptWorldModule::FunctionContext>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsScriptWorldModule::FunctionContext& value)
  {
    nsUInt32 hash = nsHashHelper<const void*>::Hash(value.m_pFunctionAndFlags);
    hash = nsHashingUtils::CombineHashValues32(hash, nsHashHelper<void*>::Hash(value.m_pInstance));
    return hash;
  }

  NS_ALWAYS_INLINE static bool Equal(const nsScriptWorldModule::FunctionContext& a, const nsScriptWorldModule::FunctionContext& b) { return a == b; }
};
