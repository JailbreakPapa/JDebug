#pragma once

#include <Core/CoreDLL.h>

#include <Core/Scripting/ScriptRTTI.h>

class nsScriptWorldModule;

using nsScriptCoroutineId = nsGenericId<20, 12>;

/// \brief A handle to a script coroutine which can be used to determine whether a coroutine is still running
/// even after the underlying coroutine object has already been deleted.
///
/// \sa nsScriptWorldModule::CreateCoroutine, nsScriptWorldModule::IsCoroutineFinished
struct nsScriptCoroutineHandle
{
  NS_DECLARE_HANDLE_TYPE(nsScriptCoroutineHandle, nsScriptCoroutineId);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsScriptCoroutineHandle);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsScriptCoroutineHandle);

/// \brief Base class of script coroutines.
///
/// A coroutine is a function that can be distributed over multiple frames and behaves similar to a mini state machine.
/// That is why coroutines are actually individual objects that keep track of their state rather than simple functions.
/// At first Start() is called with the arguments of the coroutine followed by one or multiple calls to Update().
/// The return value of the Update() function determines whether the Update() function should be called again next frame
/// or at latest after the specified delay. If the Update() function returns completed the Stop() function is called and the
/// coroutine object is destroyed.
/// The nsScriptWorldModule is used to create and manage coroutine objects. The coroutine can then either be started and
/// scheduled automatically by calling nsScriptWorldModule::StartCoroutine or the
/// Start/Stop/Update function is called manually if the coroutine is embedded as a subroutine in another coroutine.
class NS_CORE_DLL nsScriptCoroutine
{
public:
  nsScriptCoroutine();
  virtual ~nsScriptCoroutine();

  nsScriptCoroutineHandle GetHandle() { return nsScriptCoroutineHandle(m_Id); }

  nsStringView GetName() const { return m_sName; }

  nsScriptInstance* GetScriptInstance() { return m_pInstance; }
  const nsScriptInstance* GetScriptInstance() const { return m_pInstance; }

  nsScriptWorldModule* GetScriptWorldModule() { return m_pOwnerModule; }
  const nsScriptWorldModule* GetScriptWorldModule() const { return m_pOwnerModule; }

  struct Result
  {
    struct State
    {
      using StorageType = nsUInt8;

      enum Enum
      {
        Invalid,
        Running,
        Completed,
        Failed,

        Default = Invalid,
      };
    };

    static NS_ALWAYS_INLINE Result Running(nsTime maxDelay = nsTime::MakeZero()) { return {State::Running, maxDelay}; }
    static NS_ALWAYS_INLINE Result Completed() { return {State::Completed}; }
    static NS_ALWAYS_INLINE Result Failed() { return {State::Failed}; }

    nsEnum<State> m_State;
    nsTime m_MaxDelay = nsTime::MakeZero();
  };

  virtual void StartWithVarargs(nsArrayPtr<nsVariant> arguments) = 0;
  virtual void Stop() {}
  virtual Result Update(nsTime deltaTimeSinceLastUpdate) = 0;

  void UpdateAndSchedule(nsTime deltaTimeSinceLastUpdate = nsTime::MakeZero());

private:
  friend class nsScriptWorldModule;
  void Initialize(nsScriptCoroutineId id, nsStringView sName, nsScriptInstance& inout_instance, nsScriptWorldModule& inout_ownerModule);
  void Deinitialize();

  static const nsAbstractFunctionProperty* GetUpdateFunctionProperty();

  nsScriptCoroutineId m_Id;
  nsHashedString m_sName;
  nsScriptInstance* m_pInstance = nullptr;
  nsScriptWorldModule* m_pOwnerModule = nullptr;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsScriptCoroutine);

/// \brief Base class of coroutines which are implemented in C++ to allow automatic unpacking of the arguments from variants
template <typename Derived, class... Args>
class nsTypedScriptCoroutine : public nsScriptCoroutine
{
private:
  template <std::size_t... I>
  NS_ALWAYS_INLINE void StartImpl(nsArrayPtr<nsVariant> arguments, std::index_sequence<I...>)
  {
    static_cast<Derived*>(this)->Start(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
  }

  virtual void StartWithVarargs(nsArrayPtr<nsVariant> arguments) override
  {
    StartImpl(arguments, std::make_index_sequence<sizeof...(Args)>{});
  }
};

/// \brief Mode that decides what should happen if a new coroutine is created while there is already another coroutine running with the same name
/// on a given instance.
///
/// \sa nsScriptWorldModule::CreateCoroutine
struct nsScriptCoroutineCreationMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    StopOther,     ///< Stop the other coroutine before creating a new one with the same name
    DontCreateNew, ///< Don't create a new coroutine if there is already one running with the same name
    AllowOverlap,  ///< Allow multiple overlapping coroutines with the same name

    Default = StopOther
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsScriptCoroutineCreationMode);

/// \brief A coroutine type that stores a custom allocator.
///
/// The custom allocator allows to pass more data to the created coroutine object than the default allocator.
/// E.g. this is used to pass the visual script graph to a visual script coroutine without the user needing to know
/// that the coroutine is actually implemented in visual script.
class NS_CORE_DLL nsScriptCoroutineRTTI : public nsRTTI, public nsRefCountingImpl
{
public:
  nsScriptCoroutineRTTI(nsStringView sName, nsUniquePtr<nsRTTIAllocator>&& pAllocator);
  ~nsScriptCoroutineRTTI();

private:
  nsString m_sTypeNameStorage;
  nsUniquePtr<nsRTTIAllocator> m_pAllocatorStorage;
};

/// \brief A function property that creates an instance of the given coroutine type and starts it immediately.
class NS_CORE_DLL nsScriptCoroutineFunctionProperty : public nsScriptFunctionProperty
{
public:
  nsScriptCoroutineFunctionProperty(nsStringView sName, const nsSharedPtr<nsScriptCoroutineRTTI>& pType, nsScriptCoroutineCreationMode::Enum creationMode);
  ~nsScriptCoroutineFunctionProperty();

  virtual nsFunctionType::Enum GetFunctionType() const override { return nsFunctionType::Member; }
  virtual const nsRTTI* GetReturnType() const override { return nullptr; }
  virtual nsBitflags<nsPropertyFlags> GetReturnFlags() const override { return nsPropertyFlags::Void; }
  virtual nsUInt32 GetArgumentCount() const override { return 0; }
  virtual const nsRTTI* GetArgumentType(nsUInt32 uiParamIndex) const override { return nullptr; }
  virtual nsBitflags<nsPropertyFlags> GetArgumentFlags(nsUInt32 uiParamIndex) const override { return nsPropertyFlags::Void; }

  virtual void Execute(void* pInstance, nsArrayPtr<nsVariant> arguments, nsVariant& out_returnValue) const override;

protected:
  nsSharedPtr<nsScriptCoroutineRTTI> m_pType;
  nsEnum<nsScriptCoroutineCreationMode> m_CreationMode;
};

/// \brief A message handler that creates an instance of the given coroutine type and starts it immediately.
class NS_CORE_DLL nsScriptCoroutineMessageHandler : public nsScriptMessageHandler
{
public:
  nsScriptCoroutineMessageHandler(nsStringView sName, const nsScriptMessageDesc& desc, const nsSharedPtr<nsScriptCoroutineRTTI>& pType, nsScriptCoroutineCreationMode::Enum creationMode);
  ~nsScriptCoroutineMessageHandler();

  static void Dispatch(nsAbstractMessageHandler* pSelf, void* pInstance, nsMessage& ref_msg);

protected:
  nsHashedString m_sName;
  nsSharedPtr<nsScriptCoroutineRTTI> m_pType;
  nsEnum<nsScriptCoroutineCreationMode> m_CreationMode;
};

/// \brief HashHelper implementation so coroutine handles can be used as key in a hash table. Also needed to store in a variant.
template <>
struct nsHashHelper<nsScriptCoroutineHandle>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsScriptCoroutineHandle value) { return nsHashHelper<nsUInt32>::Hash(value.GetInternalID().m_Data); }

  NS_ALWAYS_INLINE static bool Equal(nsScriptCoroutineHandle a, nsScriptCoroutineHandle b) { return a == b; }
};

/// \brief Currently not implemented as it is not needed for coroutine handles.
NS_ALWAYS_INLINE void operator<<(nsStreamWriter& inout_stream, const nsScriptCoroutineHandle& hValue) {}
NS_ALWAYS_INLINE void operator>>(nsStreamReader& inout_stream, nsScriptCoroutineHandle& ref_hValue) {}
