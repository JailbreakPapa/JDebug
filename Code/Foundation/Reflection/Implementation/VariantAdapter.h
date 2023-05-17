#pragma once

#include <Foundation/Types/Variant.h>

template <typename T>
struct wdCleanType2
{
  using Type = T;
  using RttiType = T;
};

template <typename T>
struct wdCleanType2<wdEnum<T>>
{
  using Type = wdEnum<T>;
  using RttiType = T;
};

template <typename T>
struct wdCleanType2<wdBitflags<T>>
{
  using Type = wdBitflags<T>;
  using RttiType = T;
};

template <typename T>
struct wdCleanType
{
  using Type = typename wdTypeTraits<T>::NonConstReferencePointerType;
  using RttiType = typename wdCleanType2<typename wdTypeTraits<T>::NonConstReferencePointerType>::RttiType;
};

template <>
struct wdCleanType<const char*>
{
  using Type = const char*;
  using RttiType = const char*;
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
struct wdIsOutParam
{
  enum
  {
    value = false,
  };
};

template <typename T>
struct wdIsOutParam<T&>
{
  enum
  {
    value = !std::is_const<typename wdTypeTraits<T>::NonReferencePointerType>::value,
  };
};

template <typename T>
struct wdIsOutParam<T*>
{
  enum
  {
    value = !std::is_const<typename wdTypeTraits<T>::NonReferencePointerType>::value,
  };
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to determine if the given type is a build-in standard variant type.
template <class T, class C = typename wdCleanType<T>::Type>
struct wdIsStandardType
{
  enum
  {
    value = wdVariant::TypeDeduction<C>::value >= wdVariantType::FirstStandardType && wdVariant::TypeDeduction<C>::value <= wdVariantType::LastStandardType,
  };
};

template <class T>
struct wdIsStandardType<T, wdVariant>
{
  enum
  {
    value = true,
  };
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to determine if the given type can be stored by value inside an wdVariant (either standard type or custom type).
template <class T, class C = typename wdCleanType<T>::Type>
struct wdIsValueType
{
  enum
  {
    value = (wdVariant::TypeDeduction<C>::value >= wdVariantType::FirstStandardType && wdVariant::TypeDeduction<C>::value <= wdVariantType::LastStandardType) || wdVariantTypeDeduction<C>::classification == wdVariantClass::CustomTypeCast,
  };
};

template <class T>
struct wdIsValueType<T, wdVariant>
{
  enum
  {
    value = true,
  };
};

//////////////////////////////////////////////////////////////////////////
/// \brief Used to automatically assign any value to an wdVariant using the assignment rules
/// outlined in wdAbstractFunctionProperty::Execute.
template <class T,                          ///< Only this parameter needs to be provided, the actual type of the value.
  class C = typename wdCleanType<T>::Type,  ///< Same as T but without the const&* fluff.
  int VALUE_TYPE = wdIsValueType<T>::value> ///< Is 1 if T is a wdTypeFlags::StandardType or a custom type
struct wdVariantAssignmentAdapter
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAssignmentAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  void operator=(RealType* rhs) { m_value = rhs; }
  void operator=(RealType&& rhs)
  {
    if (m_value.IsValid())
      *m_value.Get<RealType*>() = rhs;
  }
  wdVariant& m_value;
};

template <class T, class S>
struct wdVariantAssignmentAdapter<T, wdEnum<S>, 0>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAssignmentAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  void operator=(wdEnum<S>&& rhs) { m_value = static_cast<wdInt64>(rhs.GetValue()); }

  wdVariant& m_value;
};

template <class T, class S>
struct wdVariantAssignmentAdapter<T, wdBitflags<S>, 0>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAssignmentAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  void operator=(wdBitflags<S>&& rhs) { m_value = static_cast<wdInt64>(rhs.GetValue()); }

  wdVariant& m_value;
};

template <class T, class C>
struct wdVariantAssignmentAdapter<T, C, 1>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAssignmentAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  void operator=(T&& rhs) { m_value = rhs; }

  wdVariant& m_value;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to implicitly retrieve any value from an wdVariant to be used as a function argument
/// using the assignment rules outlined in wdAbstractFunctionProperty::Execute.
template <class T,                          ///< Only this parameter needs to be provided, the actual type of the argument. Rest is used to force specializations.
  class C = typename wdCleanType<T>::Type,  ///< Same as T but without the const&* fluff.
  int VALUE_TYPE = wdIsValueType<T>::value, ///< Is 1 if T is a wdTypeFlags::StandardType or a custom type
  int OUT_PARAM = wdIsOutParam<T>::value>   ///< Is 1 if T a non-const reference or pointer.
struct wdVariantAdapter
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;

  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  operator RealType&() { return *m_value.Get<RealType*>(); }

  operator RealType*() { return m_value.IsValid() ? m_value.Get<RealType*>() : nullptr; }

  wdVariant& m_value;
};

template <class T, class S>
struct wdVariantAdapter<T, wdEnum<S>, 0, 0>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue = static_cast<typename S::Enum>(m_value.ConvertTo<wdInt64>());
  }

  operator const wdEnum<S>&() { return m_realValue; }
  operator const wdEnum<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  wdVariant& m_value;
  wdEnum<S> m_realValue;
};

template <class T, class S>
struct wdVariantAdapter<T, wdEnum<S>, 0, 1>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue = static_cast<typename S::Enum>(m_value.ConvertTo<wdInt64>());
  }
  ~wdVariantAdapter()
  {
    if (m_value.IsValid())
      m_value = static_cast<wdInt64>(m_realValue.GetValue());
  }

  operator wdEnum<S>&() { return m_realValue; }
  operator wdEnum<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  wdVariant& m_value;
  wdEnum<S> m_realValue;
};

template <class T, class S>
struct wdVariantAdapter<T, wdBitflags<S>, 0, 0>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue.SetValue(static_cast<typename S::StorageType>(m_value.ConvertTo<wdInt64>()));
  }

  operator const wdBitflags<S>&() { return m_realValue; }
  operator const wdBitflags<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  wdVariant& m_value;
  wdBitflags<S> m_realValue;
};

template <class T, class S>
struct wdVariantAdapter<T, wdBitflags<S>, 0, 1>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue.SetValue(static_cast<typename S::StorageType>(m_value.ConvertTo<wdInt64>()));
  }
  ~wdVariantAdapter()
  {
    if (m_value.IsValid())
      m_value = static_cast<wdInt64>(m_realValue.GetValue());
  }

  operator wdBitflags<S>&() { return m_realValue; }
  operator wdBitflags<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  wdVariant& m_value;
  wdBitflags<S> m_realValue;
};

template <class T, class C>
struct wdVariantAdapter<T, C, 1, 0>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  operator const C&()
  {
    if constexpr (wdVariantTypeDeduction<C>::classification == wdVariantClass::CustomTypeCast)
    {
      if (m_value.GetType() == wdVariantType::TypedPointer)
        return *m_value.Get<RealType*>();
    }
    return m_value.Get<RealType>();
  }

  operator const C*()
  {
    if constexpr (wdVariantTypeDeduction<C>::classification == wdVariantClass::CustomTypeCast)
    {
      if (m_value.GetType() == wdVariantType::TypedPointer)
        return m_value.IsValid() ? m_value.Get<RealType*>() : nullptr;
    }
    return m_value.IsValid() ? &m_value.Get<RealType>() : nullptr;
  }

  wdVariant& m_value;
};

template <class T, class C>
struct wdVariantAdapter<T, C, 1, 1>
{
  using RealType = typename wdTypeTraits<T>::NonConstReferencePointerType;
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
    // We ignore the return value here instead const_cast the Get<> result to profit from the Get methods runtime type checks.
    m_value.GetWriteAccess();
  }

  operator C&()
  {
    if (m_value.GetType() == wdVariantType::TypedPointer)
      return *m_value.Get<RealType*>();
    else
      return const_cast<RealType&>(m_value.Get<RealType>());
  }
  operator C*()
  {
    if (m_value.GetType() == wdVariantType::TypedPointer)
      return m_value.IsValid() ? m_value.Get<RealType*>() : nullptr;
    else
      return m_value.IsValid() ? &const_cast<RealType&>(m_value.Get<RealType>()) : nullptr;
  }

  wdVariant& m_value;
};

template <class T>
struct wdVariantAdapter<T, wdVariant, 1, 0>
{
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  operator const wdVariant&() { return m_value; }
  operator const wdVariant*() { return &m_value; }

  wdVariant& m_value;
};

template <class T>
struct wdVariantAdapter<T, wdVariant, 1, 1>
{
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  operator wdVariant&() { return m_value; }
  operator wdVariant*() { return &m_value; }

  wdVariant& m_value;
};

template <>
struct wdVariantAdapter<const char*, const char*, 1, 0>
{
  wdVariantAdapter(wdVariant& value)
    : m_value(value)
  {
  }

  operator const char*() { return m_value.IsValid() ? m_value.Get<wdString>().GetData() : nullptr; }

  wdVariant& m_value;
};
