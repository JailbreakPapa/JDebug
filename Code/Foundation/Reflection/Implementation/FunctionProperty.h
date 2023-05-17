#pragma once

/// \file

#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/VariantAdapter.h>


template <class R, class... Args>
class wdTypedFunctionProperty : public wdAbstractFunctionProperty
{
public:
  wdTypedFunctionProperty(const char* szPropertyName)
    : wdAbstractFunctionProperty(szPropertyName)
  {
  }

  virtual const wdRTTI* GetReturnType() const override { return wdGetStaticRTTI<typename wdCleanType<R>::RttiType>(); }
  virtual wdBitflags<wdPropertyFlags> GetReturnFlags() const override { return wdPropertyFlags::GetParameterFlags<R>(); }

  virtual wdUInt32 GetArgumentCount() const override { return sizeof...(Args); }

  template <std::size_t... I>
  const wdRTTI* GetParameterTypeImpl(wdUInt32 uiParamIndex, std::index_sequence<I...>) const
  {
    // There is a dummy entry at the end to support zero parameter functions (can't have zero-size arrays).
    static const wdRTTI* params[] = {wdGetStaticRTTI<typename wdCleanType<typename getArgument<I, Args...>::Type>::RttiType>()..., nullptr};
    return params[uiParamIndex];
  }

  virtual const wdRTTI* GetArgumentType(wdUInt32 uiParamIndex) const override
  {
    return GetParameterTypeImpl(uiParamIndex, std::make_index_sequence<sizeof...(Args)>{});
  }

  template <std::size_t... I>
  wdBitflags<wdPropertyFlags> GetParameterFlagsImpl(wdUInt32 uiParamIndex, std::index_sequence<I...>) const
  {
    // There is a dummy entry at the end to support zero parameter functions (can't have zero-size arrays).
    static wdBitflags<wdPropertyFlags> params[] = {
      wdPropertyFlags::GetParameterFlags<typename getArgument<I, Args...>::Type>()..., wdPropertyFlags::Void};
    return params[uiParamIndex];
  }

  virtual wdBitflags<wdPropertyFlags> GetArgumentFlags(wdUInt32 uiParamIndex) const override
  {
    return GetParameterFlagsImpl(uiParamIndex, std::make_index_sequence<sizeof...(Args)>{});
  }
};

template <typename FUNC>
class wdFunctionProperty
{
};

#define wdFunctionPropertyCode(CONSTNESS)                                                                                                      \
  template <class CLASS, class R, class... Args>                                                                                               \
  class wdFunctionProperty<R (CLASS::*)(Args...) CONSTNESS> : public wdTypedFunctionProperty<R, Args...>                                       \
  {                                                                                                                                            \
  public:                                                                                                                                      \
    typedef R (CLASS::*TargetFunction)(Args...) CONSTNESS;                                                                                     \
                                                                                                                                               \
    wdFunctionProperty(const char* szPropertyName, TargetFunction func)                                                                        \
      : wdTypedFunctionProperty<R, Args...>(szPropertyName)                                                                                    \
    {                                                                                                                                          \
      m_Function = func;                                                                                                                       \
    }                                                                                                                                          \
                                                                                                                                               \
    virtual wdFunctionType::Enum GetFunctionType() const override { return wdFunctionType::Member; }                                           \
                                                                                                                                               \
    template <std::size_t... I>                                                                                                                \
    void ExecuteImpl(                                                                                                                          \
      wdTraitInt<1>, CONSTNESS void* pInstance, wdVariant& returnValue, wdArrayPtr<wdVariant> arguments, std::index_sequence<I...>) const      \
    {                                                                                                                                          \
      CONSTNESS CLASS* pTargetInstance = (CONSTNESS CLASS*)pInstance;                                                                          \
      (pTargetInstance->*m_Function)(wdVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);                               \
      returnValue = wdVariant();                                                                                                               \
    }                                                                                                                                          \
                                                                                                                                               \
    template <std::size_t... I>                                                                                                                \
    void ExecuteImpl(                                                                                                                          \
      wdTraitInt<0>, CONSTNESS void* pInstance, wdVariant& returnValue, wdArrayPtr<wdVariant> arguments, std::index_sequence<I...>) const      \
    {                                                                                                                                          \
      CONSTNESS CLASS* pTargetInstance = (CONSTNESS CLASS*)pInstance;                                                                          \
      wdVariantAssignmentAdapter<R> returnWrapper(returnValue);                                                                                \
      returnWrapper = (pTargetInstance->*m_Function)(wdVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);               \
    }                                                                                                                                          \
                                                                                                                                               \
    virtual void Execute(void* pInstance, wdArrayPtr<wdVariant> arguments, wdVariant& returnValue) const override                              \
    {                                                                                                                                          \
      ExecuteImpl(wdTraitInt<std::is_same<R, void>::value>(), pInstance, returnValue, arguments, std::make_index_sequence<sizeof...(Args)>{}); \
    }                                                                                                                                          \
                                                                                                                                               \
  private:                                                                                                                                     \
    TargetFunction m_Function;                                                                                                                 \
  }

// just need an empty token to call wdFunctionPropertyCode
#define NON_CONST
wdFunctionPropertyCode(NON_CONST);
#undef NON_CONST

wdFunctionPropertyCode(const);

template <class R, class... Args>
class wdFunctionProperty<R (*)(Args...)> : public wdTypedFunctionProperty<R, Args...>
{
public:
  typedef R (*TargetFunction)(Args...);

  wdFunctionProperty(const char* szPropertyName, TargetFunction func)
    : wdTypedFunctionProperty<R, Args...>(szPropertyName)
  {
    m_Function = func;
  }

  virtual wdFunctionType::Enum GetFunctionType() const override { return wdFunctionType::StaticMember; }

  template <std::size_t... I>
  void ExecuteImpl(wdTraitInt<1>, wdVariant& ref_returnValue, wdArrayPtr<wdVariant> arguments, std::index_sequence<I...>) const
  {
    (*m_Function)(wdVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    ref_returnValue = wdVariant();
  }

  template <std::size_t... I>
  void ExecuteImpl(wdTraitInt<0>, wdVariant& ref_returnValue, wdArrayPtr<wdVariant> arguments, std::index_sequence<I...>) const
  {
    wdVariantAssignmentAdapter<R> returnWrapper(ref_returnValue);
    returnWrapper = (*m_Function)(wdVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
  }

  virtual void Execute(void* pInstance, wdArrayPtr<wdVariant> arguments, wdVariant& ref_returnValue) const override
  {
    ExecuteImpl(wdTraitInt<std::is_same<R, void>::value>(), ref_returnValue, arguments, std::make_index_sequence<sizeof...(Args)>{});
  }

private:
  TargetFunction m_Function;
};


template <class CLASS, class... Args>
class wdConstructorFunctionProperty : public wdTypedFunctionProperty<CLASS*, Args...>
{
public:
  wdConstructorFunctionProperty()
    : wdTypedFunctionProperty<CLASS*, Args...>("Constructor")
  {
  }

  virtual wdFunctionType::Enum GetFunctionType() const override { return wdFunctionType::Constructor; }

  template <std::size_t... I>
  void ExecuteImpl(wdTraitInt<1>, wdVariant& ref_returnValue, wdArrayPtr<wdVariant> arguments, std::index_sequence<I...>) const
  {
    ref_returnValue = CLASS(wdVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    // returnValue = CLASS(static_cast<typename getArgument<I, Args...>::Type>(wdVariantAdapter<typename getArgument<I,
    // Args...>::Type>(arguments[I]))...);
  }

  template <std::size_t... I>
  void ExecuteImpl(wdTraitInt<0>, wdVariant& ref_returnValue, wdArrayPtr<wdVariant> arguments, std::index_sequence<I...>) const
  {
    CLASS* pInstance = WD_DEFAULT_NEW(CLASS, wdVariantAdapter<typename getArgument<I, Args...>::Type>(arguments[I])...);
    // CLASS* pInstance = WD_DEFAULT_NEW(CLASS, static_cast<typename getArgument<I, Args...>::Type>(wdVariantAdapter<typename getArgument<I,
    // Args...>::Type>(arguments[I]))...);
    ref_returnValue = pInstance;
  }

  virtual void Execute(void* pInstance, wdArrayPtr<wdVariant> arguments, wdVariant& ref_returnValue) const override
  {
    ExecuteImpl(wdTraitInt<wdIsStandardType<CLASS>::value>(), ref_returnValue, arguments, std::make_index_sequence<sizeof...(Args)>{});
  }
};
