#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/VariantAdapter.h>


template <class R, class... Args>
class nsTypedFunctionProperty : public nsAbstractFunctionProperty
{
public:
  nsTypedFunctionProperty(const char* szPropertyName)
    : nsAbstractFunctionProperty(szPropertyName)
  {
  }

  virtual const nsRTTI* GetReturnType() const override { return nsGetStaticRTTI<typename nsCleanType<R>::RttiType>(); }
  virtual nsBitflags<nsPropertyFlags> GetReturnFlags() const override { return nsPropertyFlags::GetParameterFlags<R>(); }

  virtual nsUInt32 GetArgumentCount() const override { return sizeof...(Args); }

  template <std::size_t... I>
  const nsRTTI* GetParameterTypeImpl(nsUInt32 uiParamIndex, std::index_sequence<I...>) const
  {
    // There is a dummy entry at the end to support zero parameter functions (can't have zero-size arrays).
    static const nsRTTI* params[] = {nsGetStaticRTTI<typename nsCleanType<typename getArgument<I, Args...>::Type>::RttiType>()..., nullptr};
    return params[uiParamIndex];
  }

  virtual const nsRTTI* GetArgumentType(nsUInt32 uiParamIndex) const override
  {
    return GetParameterTypeImpl(uiParamIndex, std::make_index_sequence<sizeof...(Args)>{});
  }

  template <std::size_t... I>
  nsBitflags<nsPropertyFlags> GetParameterFlagsImpl(nsUInt32 uiParamIndex, std::index_sequence<I...>) const
  {
    // There is a dummy entry at the end to support zero parameter functions (can't have zero-size arrays).
    static nsBitflags<nsPropertyFlags> params[] = {
      nsPropertyFlags::GetParameterFlags<typename getArgument<I, Args...>::Type>()..., nsPropertyFlags::Void};
    return params[uiParamIndex];
  }

  virtual nsBitflags<nsPropertyFlags> GetArgumentFlags(nsUInt32 uiParamIndex) const override
  {
    return GetParameterFlagsImpl(uiParamIndex, std::make_index_sequence<sizeof...(Args)>{});
  }
};

template <typename FUNC>
class nsFunctionProperty
{
};

template <class CLASS, class R, class... Args>
class nsFunctionProperty<R (CLASS::*)(Args...)> : public nsTypedFunctionProperty<R, Args...>
{
public:
  using TargetFunction = R (CLASS::*)(Args...);

  nsFunctionProperty(const char* szPropertyName, TargetFunction func)
    : nsTypedFunctionProperty<R, Args...>(szPropertyName)
  {
    m_Function = func;
  }

  virtual nsFunctionType::Enum GetFunctionType() const override
  {
    return nsFunctionType::Member;
  }

  template <std::size_t... I>
  NS_FORCE_INLINE void ExecuteImpl(void* pInstance, nsVariant& out_returnValue, nsArrayPtr<nsVariant> arguments, std::index_sequence<I...>) const
  {
    CLASS* pTargetInstance = static_cast<CLASS*>(pInstance);
    if constexpr (std::is_same<R, void>::value)
    {
      (pTargetInstance->*m_Function)(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
      out_returnValue = nsVariant();
    }
    else
    {
      nsVariantAssignmentAdapter<R> returnWrapper(out_returnValue);
      returnWrapper = (pTargetInstance->*m_Function)(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    }
  }

  virtual void Execute(void* pInstance, nsArrayPtr<nsVariant> arguments, nsVariant& out_returnValue) const override
  {
    ExecuteImpl(pInstance, out_returnValue, arguments, std::make_index_sequence<sizeof...(Args)>{});
  }

private:
  TargetFunction m_Function;
};

template <class CLASS, class R, class... Args>
class nsFunctionProperty<R (CLASS::*)(Args...) const> : public nsTypedFunctionProperty<R, Args...>
{
public:
  using TargetFunction = R (CLASS::*)(Args...) const;

  nsFunctionProperty(const char* szPropertyName, TargetFunction func)
    : nsTypedFunctionProperty<R, Args...>(szPropertyName)
  {
    m_Function = func;
    this->AddFlags(nsPropertyFlags::Const);
  }

  virtual nsFunctionType::Enum GetFunctionType() const override
  {
    return nsFunctionType::Member;
  }

  template <std::size_t... I>
  NS_FORCE_INLINE void ExecuteImpl(const void* pInstance, nsVariant& out_returnValue, nsArrayPtr<nsVariant> arguments, std::index_sequence<I...>) const
  {
    const CLASS* pTargetInstance = static_cast<const CLASS*>(pInstance);
    if constexpr (std::is_same<R, void>::value)
    {
      (pTargetInstance->*m_Function)(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
      out_returnValue = nsVariant();
    }
    else
    {
      nsVariantAssignmentAdapter<R> returnWrapper(out_returnValue);
      returnWrapper = (pTargetInstance->*m_Function)(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    }
  }

  virtual void Execute(void* pInstance, nsArrayPtr<nsVariant> arguments, nsVariant& out_returnValue) const override
  {
    ExecuteImpl(pInstance, out_returnValue, arguments, std::make_index_sequence<sizeof...(Args)>{});
  }

private:
  TargetFunction m_Function;
};

template <class R, class... Args>
class nsFunctionProperty<R (*)(Args...)> : public nsTypedFunctionProperty<R, Args...>
{
public:
  using TargetFunction = R (*)(Args...);

  nsFunctionProperty(const char* szPropertyName, TargetFunction func)
    : nsTypedFunctionProperty<R, Args...>(szPropertyName)
  {
    m_Function = func;
  }

  virtual nsFunctionType::Enum GetFunctionType() const override { return nsFunctionType::StaticMember; }

  template <std::size_t... I>
  void ExecuteImpl(nsTraitInt<1>, nsVariant& out_returnValue, nsArrayPtr<nsVariant> arguments, std::index_sequence<I...>) const
  {
    (*m_Function)(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    out_returnValue = nsVariant();
  }

  template <std::size_t... I>
  void ExecuteImpl(nsTraitInt<0>, nsVariant& out_returnValue, nsArrayPtr<nsVariant> arguments, std::index_sequence<I...>) const
  {
    nsVariantAssignmentAdapter<R> returnWrapper(out_returnValue);
    returnWrapper = (*m_Function)(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
  }

  virtual void Execute(void* pInstance, nsArrayPtr<nsVariant> arguments, nsVariant& out_returnValue) const override
  {
    ExecuteImpl(nsTraitInt<std::is_same<R, void>::value>(), out_returnValue, arguments, std::make_index_sequence<sizeof...(Args)>{});
  }

private:
  TargetFunction m_Function;
};


template <class CLASS, class... Args>
class nsConstructorFunctionProperty : public nsTypedFunctionProperty<CLASS*, Args...>
{
public:
  nsConstructorFunctionProperty()
    : nsTypedFunctionProperty<CLASS*, Args...>("Constructor")
  {
  }

  virtual nsFunctionType::Enum GetFunctionType() const override { return nsFunctionType::Constructor; }

  template <std::size_t... I>
  void ExecuteImpl(nsTraitInt<1>, nsVariant& out_returnValue, nsArrayPtr<nsVariant> arguments, std::index_sequence<I...>) const
  {
    out_returnValue = CLASS(nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    // returnValue = CLASS(static_cast<typename getArgument<I, Args...>::Type>(nsVariantAdapter<typename getArgument<I,
    // Args...>::Type>(arguments[I]))...);
  }

  template <std::size_t... I>
  void ExecuteImpl(nsTraitInt<0>, nsVariant& out_returnValue, nsArrayPtr<nsVariant> arguments, std::index_sequence<I...>) const
  {
    CLASS* pInstance = NS_DEFAULT_NEW(CLASS, nsVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    // CLASS* pInstance = NS_DEFAULT_NEW(CLASS, static_cast<typename getArgument<I, Args...>::Type>(nsVariantAdapter<typename getArgument<I,
    // Args...>::Type>(arguments[I]))...);
    out_returnValue = pInstance;
  }

  virtual void Execute(void* pInstance, nsArrayPtr<nsVariant> arguments, nsVariant& out_returnValue) const override
  {
    ExecuteImpl(nsTraitInt<nsIsStandardType<CLASS>::value>(), out_returnValue, arguments, std::make_index_sequence<sizeof...(Args)>{});
  }
};
