#pragma once

#include <Foundation/Types/Variant.h>

template <typename T>
struct nsCleanType2
{
  using Type = T;
  using RttiType = T;
};

template <typename T>
struct nsCleanType2<nsEnum<T>>
{
  using Type = nsEnum<T>;
  using RttiType = T;
};

template <typename T>
struct nsCleanType2<nsBitflags<T>>
{
  using Type = nsBitflags<T>;
  using RttiType = T;
};

template <typename T>
struct nsCleanType
{
  using Type = typename nsTypeTraits<T>::NonConstReferencePointerType;
  using RttiType = typename nsCleanType2<typename nsTypeTraits<T>::NonConstReferencePointerType>::RttiType;
};

template <>
struct nsCleanType<const char*>
{
  using Type = const char*;
  using RttiType = const char*;
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
struct nsIsOutParam
{
  enum
  {
    value = false,
  };
};

template <typename T>
struct nsIsOutParam<T&>
{
  enum
  {
    value = !std::is_const<typename nsTypeTraits<T>::NonReferencePointerType>::value,
  };
};

template <typename T>
struct nsIsOutParam<T*>
{
  enum
  {
    value = !std::is_const<typename nsTypeTraits<T>::NonReferencePointerType>::value,
  };
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to determine if the given type is a build-in standard variant type.
template <class T, class C = typename nsCleanType<T>::Type>
struct nsIsStandardType
{
  enum
  {
    value = nsVariant::TypeDeduction<C>::value >= nsVariantType::FirstStandardType && nsVariant::TypeDeduction<C>::value <= nsVariantType::LastStandardType,
  };
};

template <class T>
struct nsIsStandardType<T, nsVariant>
{
  enum
  {
    value = true,
  };
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to determine if the given type can be stored by value inside an nsVariant (either standard type or custom type).
template <class T, class C = typename nsCleanType<T>::Type>
struct nsIsValueType
{
  enum
  {
    value = (nsVariant::TypeDeduction<C>::value >= nsVariantType::FirstStandardType && nsVariant::TypeDeduction<C>::value <= nsVariantType::LastStandardType) || nsVariantTypeDeduction<C>::classification == nsVariantClass::CustomTypeCast,
  };
};

template <class T>
struct nsIsValueType<T, nsVariant>
{
  enum
  {
    value = true,
  };
};

//////////////////////////////////////////////////////////////////////////
/// \brief Used to automatically assign any value to an nsVariant using the assignment rules
/// outlined in nsAbstractFunctionProperty::Execute.
template <class T,                          ///< Only this parameter needs to be provided, the actual type of the value.
  class C = typename nsCleanType<T>::Type,  ///< Same as T but without the const&* fluff.
  int VALUE_TYPE = nsIsValueType<T>::value> ///< Is 1 if T is a nsTypeFlags::StandardType or a custom type
struct nsVariantAssignmentAdapter
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAssignmentAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  void operator=(RealType* rhs) { m_value = rhs; }
  void operator=(RealType&& rhs)
  {
    if (m_value.IsValid())
      *m_value.Get<RealType*>() = rhs;
  }
  nsVariant& m_value;
};

template <class T, class S>
struct nsVariantAssignmentAdapter<T, nsEnum<S>, 0>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAssignmentAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  void operator=(nsEnum<S>&& rhs) { m_value = static_cast<nsInt64>(rhs.GetValue()); }

  nsVariant& m_value;
};

template <class T, class S>
struct nsVariantAssignmentAdapter<T, nsBitflags<S>, 0>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAssignmentAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  void operator=(nsBitflags<S>&& rhs) { m_value = static_cast<nsInt64>(rhs.GetValue()); }

  nsVariant& m_value;
};

template <class T, class C>
struct nsVariantAssignmentAdapter<T, C, 1>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAssignmentAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  void operator=(T&& rhs) { m_value = rhs; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAssignmentAdapter<T, nsVariantArray, 0>
{
  nsVariantAssignmentAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  void operator=(T&& rhs) { m_value = rhs; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAssignmentAdapter<T, nsVariantDictionary, 0>
{
  nsVariantAssignmentAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  void operator=(T&& rhs) { m_value = rhs; }

  nsVariant& m_value;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to implicitly retrieve any value from an nsVariant to be used as a function argument
/// using the assignment rules outlined in nsAbstractFunctionProperty::Execute.
template <class T,                          ///< Only this parameter needs to be provided, the actual type of the argument. Rest is used to force specializations.
  class C = typename nsCleanType<T>::Type,  ///< Same as T but without the const&* fluff.
  int VALUE_TYPE = nsIsValueType<T>::value, ///< Is 1 if T is a nsTypeFlags::StandardType or a custom type
  int OUT_PARAM = nsIsOutParam<T>::value>   ///< Is 1 if T a non-const reference or pointer.
struct nsVariantAdapter
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;

  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator RealType&() { return *m_value.Get<RealType*>(); }

  operator RealType*() { return m_value.IsValid() ? m_value.Get<RealType*>() : nullptr; }

  nsVariant& m_value;
};

template <class T, class S>
struct nsVariantAdapter<T, nsEnum<S>, 0, 0>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue = static_cast<typename S::Enum>(m_value.ConvertTo<nsInt64>());
  }

  operator const nsEnum<S>&() { return m_realValue; }
  operator const nsEnum<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  nsVariant& m_value;
  nsEnum<S> m_realValue;
};

template <class T, class S>
struct nsVariantAdapter<T, nsEnum<S>, 0, 1>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue = static_cast<typename S::Enum>(m_value.ConvertTo<nsInt64>());
  }
  ~nsVariantAdapter()
  {
    if (m_value.IsValid())
      m_value = static_cast<nsInt64>(m_realValue.GetValue());
  }

  operator nsEnum<S>&() { return m_realValue; }
  operator nsEnum<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  nsVariant& m_value;
  nsEnum<S> m_realValue;
};

template <class T, class S>
struct nsVariantAdapter<T, nsBitflags<S>, 0, 0>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue.SetValue(static_cast<typename S::StorageType>(m_value.ConvertTo<nsInt64>()));
  }

  operator const nsBitflags<S>&() { return m_realValue; }
  operator const nsBitflags<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  nsVariant& m_value;
  nsBitflags<S> m_realValue;
};

template <class T, class S>
struct nsVariantAdapter<T, nsBitflags<S>, 0, 1>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
    if (m_value.IsValid())
      m_realValue.SetValue(static_cast<typename S::StorageType>(m_value.ConvertTo<nsInt64>()));
  }
  ~nsVariantAdapter()
  {
    if (m_value.IsValid())
      m_value = static_cast<nsInt64>(m_realValue.GetValue());
  }

  operator nsBitflags<S>&() { return m_realValue; }
  operator nsBitflags<S>*() { return m_value.IsValid() ? &m_realValue : nullptr; }

  nsVariant& m_value;
  nsBitflags<S> m_realValue;
};

template <class T, class C>
struct nsVariantAdapter<T, C, 1, 0>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator const C&()
  {
    if constexpr (nsVariantTypeDeduction<C>::classification == nsVariantClass::CustomTypeCast)
    {
      if (m_value.GetType() == nsVariantType::TypedPointer)
        return *m_value.Get<RealType*>();
    }
    return m_value.Get<RealType>();
  }

  operator const C*()
  {
    if constexpr (nsVariantTypeDeduction<C>::classification == nsVariantClass::CustomTypeCast)
    {
      if (m_value.GetType() == nsVariantType::TypedPointer)
        return m_value.IsValid() ? m_value.Get<RealType*>() : nullptr;
    }
    return m_value.IsValid() ? &m_value.Get<RealType>() : nullptr;
  }

  nsVariant& m_value;
};

template <class T, class C>
struct nsVariantAdapter<T, C, 1, 1>
{
  using RealType = typename nsTypeTraits<T>::NonConstReferencePointerType;
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
    // We ignore the return value here instead const_cast the Get<> result to profit from the Get methods runtime type checks.
    m_value.GetWriteAccess();
  }

  operator C&()
  {
    if (m_value.GetType() == nsVariantType::TypedPointer)
      return *m_value.Get<RealType*>();
    else
      return const_cast<RealType&>(m_value.Get<RealType>());
  }
  operator C*()
  {
    if (m_value.GetType() == nsVariantType::TypedPointer)
      return m_value.IsValid() ? m_value.Get<RealType*>() : nullptr;
    else
      return m_value.IsValid() ? &const_cast<RealType&>(m_value.Get<RealType>()) : nullptr;
  }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAdapter<T, nsVariant, 1, 0>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator const nsVariant&() { return m_value; }
  operator const nsVariant*() { return &m_value; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAdapter<T, nsVariant, 1, 1>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator nsVariant&() { return m_value; }
  operator nsVariant*() { return &m_value; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAdapter<T, nsVariantArray, 0, 0>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator const nsVariantArray&() { return m_value.Get<nsVariantArray>(); }
  operator const nsVariantArray*() { return m_value.IsValid() ? &m_value.Get<nsVariantArray>() : nullptr; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAdapter<T, nsVariantArray, 0, 1>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator nsVariantArray&() { return m_value.GetWritable<nsVariantArray>(); }
  operator nsVariantArray*() { return m_value.IsValid() ? &m_value.GetWritable<nsVariantArray>() : nullptr; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAdapter<T, nsVariantDictionary, 0, 0>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator const nsVariantDictionary&() { return m_value.Get<nsVariantDictionary>(); }
  operator const nsVariantDictionary*() { return m_value.IsValid() ? &m_value.Get<nsVariantDictionary>() : nullptr; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAdapter<T, nsVariantDictionary, 0, 1>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator nsVariantDictionary&() { return m_value.GetWritable<nsVariantDictionary>(); }
  operator nsVariantDictionary*() { return m_value.IsValid() ? &m_value.GetWritable<nsVariantDictionary>() : nullptr; }

  nsVariant& m_value;
};

template <>
struct nsVariantAdapter<const char*, const char*, 1, 0>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator const char*() { return m_value.IsValid() ? m_value.Get<nsString>().GetData() : nullptr; }

  nsVariant& m_value;
};

template <class T>
struct nsVariantAdapter<T, nsStringView, 1, 0>
{
  nsVariantAdapter(nsVariant& value)
    : m_value(value)
  {
  }

  operator const nsStringView() { return m_value.IsA<nsStringView>() ? m_value.Get<nsStringView>() : m_value.Get<nsString>().GetView(); }

  nsVariant& m_value;
};
