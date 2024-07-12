#pragma once

/// \file

#include <Foundation/Basics.h>

#include <Foundation/Containers/HashSet.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Containers/SmallArray.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Reflection/Implementation/RTTI.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Enum.h>

class nsRTTI;
class nsPropertyAttribute;

/// \brief Determines whether a type is nsIsBitflags.
template <typename T>
struct nsIsBitflags
{
  static constexpr bool value = false;
};

template <typename T>
struct nsIsBitflags<nsBitflags<T>>
{
  static constexpr bool value = true;
};

/// \brief Determines whether a type is nsIsBitflags.
template <typename T>
struct nsIsEnum
{
  static constexpr bool value = std::is_enum<T>::value;
};

template <typename T>
struct nsIsEnum<nsEnum<T>>
{
  static constexpr bool value = true;
};

/// \brief Flags used to describe a property and its type.
struct nsPropertyFlags
{
  using StorageType = nsUInt16;

  enum Enum : nsUInt16
  {
    StandardType = NS_BIT(0), ///< Anything that can be stored inside an nsVariant except for pointers and containers.
    IsEnum = NS_BIT(1),       ///< enum property, cast to nsAbstractEnumerationProperty.
    Bitflags = NS_BIT(2),     ///< Bitflags property, cast to nsAbstractEnumerationProperty.
    Class = NS_BIT(3),        ///< A struct or class. All of the above are mutually exclusive.

    Const = NS_BIT(4),        ///< Property value is const.
    Reference = NS_BIT(5),    ///< Property value is a reference.
    Pointer = NS_BIT(6),      ///< Property value is a pointer.

    PointerOwner = NS_BIT(7), ///< This pointer property takes ownership of the passed pointer.
    ReadOnly = NS_BIT(8),     ///< Can only be read but not modified.
    Hidden = NS_BIT(9),       ///< This property should not appear in the UI.
    Phantom = NS_BIT(10),     ///< Phantom types are mirrored types on the editor side. Ie. they do not exist as actual classes in the process. Also used
                              ///< for data driven types, e.g. by the Visual Shader asset.

    VarOut = NS_BIT(11),      ///< Tag for non-const-ref function parameters to indicate usage 'out'
    VarInOut = NS_BIT(12),    ///< Tag for non-const-ref function parameters to indicate usage 'inout'

    Default = 0,
    Void = 0
  };

  struct Bits
  {
    StorageType StandardType : 1;
    StorageType IsEnum : 1;
    StorageType Bitflags : 1;
    StorageType Class : 1;

    StorageType Const : 1;
    StorageType Reference : 1;
    StorageType Pointer : 1;

    StorageType PointerOwner : 1;
    StorageType ReadOnly : 1;
    StorageType Hidden : 1;
    StorageType Phantom : 1;
  };

  template <class Type>
  static nsBitflags<nsPropertyFlags> GetParameterFlags()
  {
    using CleanType = typename nsTypeTraits<Type>::NonConstReferencePointerType;
    nsBitflags<nsPropertyFlags> flags;
    constexpr nsVariantType::Enum type = static_cast<nsVariantType::Enum>(nsVariantTypeDeduction<CleanType>::value);
    if constexpr (std::is_same<CleanType, nsVariant>::value ||
                  std::is_same<Type, const char*>::value || // We treat const char* as a basic type and not a pointer.
                  (type >= nsVariantType::FirstStandardType && type <= nsVariantType::LastStandardType))
      flags.Add(nsPropertyFlags::StandardType);
    else if constexpr (nsIsEnum<CleanType>::value)
      flags.Add(nsPropertyFlags::IsEnum);
    else if constexpr (nsIsBitflags<CleanType>::value)
      flags.Add(nsPropertyFlags::Bitflags);
    else
      flags.Add(nsPropertyFlags::Class);

    if constexpr (std::is_const<typename nsTypeTraits<Type>::NonReferencePointerType>::value)
      flags.Add(nsPropertyFlags::Const);

    if constexpr (std::is_pointer<Type>::value && !std::is_same<Type, const char*>::value)
      flags.Add(nsPropertyFlags::Pointer);

    if constexpr (std::is_reference<Type>::value)
      flags.Add(nsPropertyFlags::Reference);

    return flags;
  }
};

template <>
inline nsBitflags<nsPropertyFlags> nsPropertyFlags::GetParameterFlags<void>()
{
  return nsBitflags<nsPropertyFlags>();
}

NS_DECLARE_FLAGS_OPERATORS(nsPropertyFlags)

/// \brief Describes what category a property belongs to.
struct nsPropertyCategory
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Constant, ///< The property is a constant value that is stored inside the RTTI data.
    Member,   ///< The property is a 'member property', i.e. it represents some accessible value. Cast to nsAbstractMemberProperty.
    Function, ///< The property is a function which can be called. Cast to nsAbstractFunctionProperty.
    Array,    ///< The property is actually an array of values. The array dimensions might be changeable. Cast to nsAbstractArrayProperty.
    Set,      ///< The property is actually a set of values. Cast to nsAbstractSetProperty.
    Map,      ///< The property is actually a map from string to values. Cast to nsAbstractMapProperty.
    Default = Member
  };
};

/// \brief This is the base interface for all properties in the reflection system. It provides enough information to cast to the next better
/// base class.
class NS_FOUNDATION_DLL nsAbstractProperty
{
public:
  /// \brief The constructor must get the name of the property. The string must be a compile-time constant.
  nsAbstractProperty(const char* szPropertyName) { m_szPropertyName = szPropertyName; }

  virtual ~nsAbstractProperty() = default;

  /// \brief Returns the name of the property.
  const char* GetPropertyName() const { return m_szPropertyName; }

  /// \brief Returns the type information of the constant property. Use this to cast this property to a specific version of
  /// nsTypedConstantProperty.
  virtual const nsRTTI* GetSpecificType() const = 0;

  /// \brief Returns the category of this property. Cast this property to the next higher type for more information.
  virtual nsPropertyCategory::Enum GetCategory() const = 0; // [tested]

  /// \brief Returns the flags of the property.
  const nsBitflags<nsPropertyFlags>& GetFlags() const { return m_Flags; };

  /// \brief Adds flags to the property. Returns itself to allow to be called during initialization.
  nsAbstractProperty* AddFlags(nsBitflags<nsPropertyFlags> flags)
  {
    m_Flags.Add(flags);
    return this;
  };

  /// \brief Adds attributes to the property. Returns itself to allow to be called during initialization. Allocate an attribute using
  /// standard 'new'.
  nsAbstractProperty* AddAttributes(nsPropertyAttribute* pAttrib1, nsPropertyAttribute* pAttrib2 = nullptr, nsPropertyAttribute* pAttrib3 = nullptr,
    nsPropertyAttribute* pAttrib4 = nullptr, nsPropertyAttribute* pAttrib5 = nullptr, nsPropertyAttribute* pAttrib6 = nullptr)
  {
    NS_ASSERT_DEV(pAttrib1 != nullptr, "invalid attribute");

    m_Attributes.PushBack(pAttrib1);
    if (pAttrib2)
      m_Attributes.PushBack(pAttrib2);
    if (pAttrib3)
      m_Attributes.PushBack(pAttrib3);
    if (pAttrib4)
      m_Attributes.PushBack(pAttrib4);
    if (pAttrib5)
      m_Attributes.PushBack(pAttrib5);
    if (pAttrib6)
      m_Attributes.PushBack(pAttrib6);
    return this;
  };

  /// \brief Returns the array of property attributes.
  nsArrayPtr<const nsPropertyAttribute* const> GetAttributes() const { return m_Attributes; }

  /// \brief Returns the first attribute that derives from the given type, or nullptr if nothing is found.
  template <typename Type>
  const Type* GetAttributeByType() const;

protected:
  nsBitflags<nsPropertyFlags> m_Flags;
  const char* m_szPropertyName;
  nsHybridArray<const nsPropertyAttribute*, 2, nsStaticsAllocatorWrapper> m_Attributes; // Do not track RTTI data.
};

/// \brief This is the base class for all constant properties that are stored inside the RTTI data.
class NS_FOUNDATION_DLL nsAbstractConstantProperty : public nsAbstractProperty
{
public:
  /// \brief Passes the property name through to nsAbstractProperty.
  nsAbstractConstantProperty(const char* szPropertyName)
    : nsAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns nsPropertyCategory::Constant.
  virtual nsPropertyCategory::Enum GetCategory() const override { return nsPropertyCategory::Constant; } // [tested]

  /// \brief Returns a pointer to the constant data or nullptr. See nsAbstractMemberProperty::GetPropertyPointer for more information.
  virtual void* GetPropertyPointer() const = 0;

  /// \brief Returns the constant value as an nsVariant
  virtual nsVariant GetConstant() const = 0;
};

/// \brief This is the base class for all properties that are members of a class. It provides more information about the actual type.
///
/// If nsPropertyFlags::Pointer is set as a flag, you must not cast this property to nsTypedMemberProperty, instead use GetValuePtr and
/// SetValuePtr. This is because reference and const-ness of the property are only fixed for the pointer but not the type, so the actual
/// property type cannot be derived.
class NS_FOUNDATION_DLL nsAbstractMemberProperty : public nsAbstractProperty
{
public:
  /// \brief Passes the property name through to nsAbstractProperty.
  nsAbstractMemberProperty(const char* szPropertyName)
    : nsAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns nsPropertyCategory::Member.
  virtual nsPropertyCategory::Enum GetCategory() const override { return nsPropertyCategory::Member; }

  /// \brief Returns a pointer to the property data or nullptr. If a valid pointer is returned, that pointer and the information from
  /// GetSpecificType() can be used to step deeper into the type (if required).
  ///
  /// You need to pass the pointer to an object on which you are operating. This function is mostly of interest when the property itself is
  /// a compound type (a struct or class). If it is a simple type (int, float, etc.) it doesn't make much sense to retrieve the pointer.
  ///
  /// For example GetSpecificType() might return that a property is of type nsVec3. In that case one might either stop and just use the code
  /// to handle nsVec3 types, or one might continue and enumerate all sub-properties (x, y and z) as well.
  ///
  /// \note There is no guarantee that this function returns a non-nullptr pointer, independent of the type. When a property uses custom
  /// 'accessors' (functions to get / set the property value), it is not possible (or useful) to get the property pointer.
  virtual void* GetPropertyPointer(const void* pInstance) const = 0;

  /// \brief Writes the value of this property in pInstance to pObject.
  /// pObject needs to point to an instance of this property's type.
  virtual void GetValuePtr(const void* pInstance, void* out_pObject) const = 0;

  /// \brief Sets the value of pObject to the property in pInstance.
  /// pObject needs to point to an instance of this property's type.
  virtual void SetValuePtr(void* pInstance, const void* pObject) const = 0;
};


/// \brief The base class for a property that represents an array of values.
class NS_FOUNDATION_DLL nsAbstractArrayProperty : public nsAbstractProperty
{
public:
  /// \brief Passes the property name through to nsAbstractProperty.
  nsAbstractArrayProperty(const char* szPropertyName)
    : nsAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns nsPropertyCategory::Array.
  virtual nsPropertyCategory::Enum GetCategory() const override { return nsPropertyCategory::Array; }

  /// \brief Returns number of elements.
  virtual nsUInt32 GetCount(const void* pInstance) const = 0;

  /// \brief Writes element at index uiIndex to the target of pObject.
  virtual void GetValue(const void* pInstance, nsUInt32 uiIndex, void* pObject) const = 0;

  /// \brief Writes the target of pObject to the element at index uiIndex.
  virtual void SetValue(void* pInstance, nsUInt32 uiIndex, const void* pObject) const = 0;

  /// \brief Inserts the target of pObject into the array at index uiIndex.
  virtual void Insert(void* pInstance, nsUInt32 uiIndex, const void* pObject) const = 0;

  /// \brief Removes the element in the array at index uiIndex.
  virtual void Remove(void* pInstance, nsUInt32 uiIndex) const = 0;

  /// \brief Clears the array.
  virtual void Clear(void* pInstance) const = 0;

  /// \brief Resizes the array to uiCount.
  virtual void SetCount(void* pInstance, nsUInt32 uiCount) const = 0;

  virtual void* GetValuePointer(void* pInstance, nsUInt32 uiIndex) const { return nullptr; }
};


/// \brief The base class for a property that represents a set of values.
///
/// The element type must either be a standard type or a pointer.
class NS_FOUNDATION_DLL nsAbstractSetProperty : public nsAbstractProperty
{
public:
  /// \brief Passes the property name through to nsAbstractProperty.
  nsAbstractSetProperty(const char* szPropertyName)
    : nsAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns nsPropertyCategory::Set.
  virtual nsPropertyCategory::Enum GetCategory() const override { return nsPropertyCategory::Set; }

  /// \brief Returns whether the set is empty.
  virtual bool IsEmpty(const void* pInstance) const = 0;

  /// \brief Clears the set.
  virtual void Clear(void* pInstance) const = 0;

  /// \brief Inserts the target of pObject into the set.
  virtual void Insert(void* pInstance, const void* pObject) const = 0;

  /// \brief Removes the target of pObject from the set.
  virtual void Remove(void* pInstance, const void* pObject) const = 0;

  /// \brief Returns whether the target of pObject is in the set.
  virtual bool Contains(const void* pInstance, const void* pObject) const = 0;

  /// \brief Writes the content of the set to out_keys.
  virtual void GetValues(const void* pInstance, nsDynamicArray<nsVariant>& out_keys) const = 0;
};


/// \brief The base class for a property that represents a set of values.
///
/// The element type must either be a standard type or a pointer.
class NS_FOUNDATION_DLL nsAbstractMapProperty : public nsAbstractProperty
{
public:
  /// \brief Passes the property name through to nsAbstractProperty.
  nsAbstractMapProperty(const char* szPropertyName)
    : nsAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns nsPropertyCategory::Map.
  virtual nsPropertyCategory::Enum GetCategory() const override { return nsPropertyCategory::Map; }

  /// \brief Returns whether the set is empty.
  virtual bool IsEmpty(const void* pInstance) const = 0;

  /// \brief Clears the set.
  virtual void Clear(void* pInstance) const = 0;

  /// \brief Inserts the target of pObject into the set.
  virtual void Insert(void* pInstance, const char* szKey, const void* pObject) const = 0;

  /// \brief Removes the target of pObject from the set.
  virtual void Remove(void* pInstance, const char* szKey) const = 0;

  /// \brief Returns whether the target of pObject is in the set.
  virtual bool Contains(const void* pInstance, const char* szKey) const = 0;

  /// \brief Writes element at index uiIndex to the target of pObject.
  virtual bool GetValue(const void* pInstance, const char* szKey, void* pObject) const = 0;

  /// \brief Writes the content of the set to out_keys.
  virtual void GetKeys(const void* pInstance, nsHybridArray<nsString, 16>& out_keys) const = 0;
};

/// \brief Use getArgument<N, Args...>::Type to get the type of the Nth argument in Args.
template <int _Index, class... Args>
struct getArgument;

template <class Head, class... Tail>
struct getArgument<0, Head, Tail...>
{
  using Type = Head;
};

template <int _Index, class Head, class... Tail>
struct getArgument<_Index, Head, Tail...>
{
  using Type = typename getArgument<_Index - 1, Tail...>::Type;
};

/// \brief Template that allows to probe a function for a parameter and return type.
template <int I, typename FUNC>
struct nsFunctionParameterTypeResolver
{
};

template <int I, typename R, typename... P>
struct nsFunctionParameterTypeResolver<I, R (*)(P...)>
{
  enum Constants
  {
    Arguments = sizeof...(P),
  };
  NS_CHECK_AT_COMPILETIME_MSG(I < Arguments, "I needs to be smaller than the number of function parameters.");
  using ParameterType = typename getArgument<I, P...>::Type;
  using ReturnType = R;
};

template <int I, class Class, typename R, typename... P>
struct nsFunctionParameterTypeResolver<I, R (Class::*)(P...)>
{
  enum Constants
  {
    Arguments = sizeof...(P),
  };
  NS_CHECK_AT_COMPILETIME_MSG(I < Arguments, "I needs to be smaller than the number of function parameters.");
  using ParameterType = typename getArgument<I, P...>::Type;
  using ReturnType = R;
};

template <int I, class Class, typename R, typename... P>
struct nsFunctionParameterTypeResolver<I, R (Class::*)(P...) const>
{
  enum Constants
  {
    Arguments = sizeof...(P),
  };
  NS_CHECK_AT_COMPILETIME_MSG(I < Arguments, "I needs to be smaller than the number of function parameters.");
  using ParameterType = typename getArgument<I, P...>::Type;
  using ReturnType = R;
};

/// \brief Template that allows to probe a single parameter function for parameter and return type.
template <typename FUNC>
struct nsMemberFunctionParameterTypeResolver
{
};

template <class Class, typename R, typename P>
struct nsMemberFunctionParameterTypeResolver<R (Class::*)(P)>
{
  using ParameterType = P;
  using ReturnType = R;
};

/// \brief Template that allows to probe a container for its element type.
template <typename CONTAINER>
struct nsContainerSubTypeResolver
{
};

template <typename T>
struct nsContainerSubTypeResolver<nsArrayPtr<T>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct nsContainerSubTypeResolver<nsDynamicArray<T>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename T, nsUInt32 Size>
struct nsContainerSubTypeResolver<nsHybridArray<T, Size>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename T, nsUInt32 Size>
struct nsContainerSubTypeResolver<nsStaticArray<T, Size>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename T, nsUInt16 Size>
struct nsContainerSubTypeResolver<nsSmallArray<T, Size>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct nsContainerSubTypeResolver<nsDeque<T>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct nsContainerSubTypeResolver<nsSet<T>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct nsContainerSubTypeResolver<nsHashSet<T>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename K, typename T>
struct nsContainerSubTypeResolver<nsHashTable<K, T>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};

template <typename K, typename T>
struct nsContainerSubTypeResolver<nsMap<K, T>>
{
  using Type = typename nsTypeTraits<T>::NonConstReferenceType;
};


/// \brief Describes what kind of function a property is.
struct nsFunctionType
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Member,       ///< A normal member function, a valid instance pointer must be provided to call.
    StaticMember, ///< A static member function, instance pointer will be ignored.
    Constructor,  ///< A constructor. Return value is a void* pointing to the new instance allocated with the default allocator.
    Default = Member
  };
};

/// \brief The base class for a property that represents a function.
class NS_FOUNDATION_DLL nsAbstractFunctionProperty : public nsAbstractProperty
{
public:
  /// \brief Passes the property name through to nsAbstractProperty.
  nsAbstractFunctionProperty(const char* szPropertyName)
    : nsAbstractProperty(szPropertyName)
  {
  }

  virtual nsPropertyCategory::Enum GetCategory() const override { return nsPropertyCategory::Function; }
  /// \brief Returns the type of function, see nsFunctionPropertyType::Enum.
  virtual nsFunctionType::Enum GetFunctionType() const = 0;
  /// \brief Returns the type of the return value.
  virtual const nsRTTI* GetReturnType() const = 0;
  /// \brief Returns property flags of the return value.
  virtual nsBitflags<nsPropertyFlags> GetReturnFlags() const = 0;
  /// \brief Returns the number of arguments.
  virtual nsUInt32 GetArgumentCount() const = 0;
  /// \brief Returns the type of the given argument.
  virtual const nsRTTI* GetArgumentType(nsUInt32 uiParamIndex) const = 0;
  /// \brief Returns the property flags of the given argument.
  virtual nsBitflags<nsPropertyFlags> GetArgumentFlags(nsUInt32 uiParamIndex) const = 0;

  /// \brief Calls the function. Provide the instance on which the function is supposed to be called.
  ///
  /// arguments must be the size of GetArgumentCount, the following rules apply for both arguments and return value:
  /// Any standard type must be provided by value, even if it is a pointer to one. Types must match exactly, no ConvertTo is called.
  /// enum and bitflags are supported if nsEnum / nsBitflags is used, value must be provided as nsInt64.
  /// Out values (&, *) are written back to the variant they were read from.
  /// Any class is provided by pointer, regardless of whether it is a pointer or not.
  /// The returnValue must only be valid if the return value is a ref or by value class. In that case
  /// returnValue must be a ptr to a valid class instance of the returned type.
  /// An invalid variant is equal to a nullptr, except for if the argument is of type nsVariant, in which case
  /// it is impossible to pass along a nullptr.
  virtual void Execute(void* pInstance, nsArrayPtr<nsVariant> arguments, nsVariant& out_returnValue) const = 0;

  virtual const nsRTTI* GetSpecificType() const override { return GetReturnType(); }

  /// \brief Adds flags to the property. Returns itself to allow to be called during initialization.
  nsAbstractFunctionProperty* AddFlags(nsBitflags<nsPropertyFlags> flags)
  {
    return static_cast<nsAbstractFunctionProperty*>(nsAbstractProperty::AddFlags(flags));
  }

  /// \brief Adds attributes to the property. Returns itself to allow to be called during initialization. Allocate an attribute using
  /// standard 'new'.
  nsAbstractFunctionProperty* AddAttributes(nsPropertyAttribute* pAttrib1, nsPropertyAttribute* pAttrib2 = nullptr, nsPropertyAttribute* pAttrib3 = nullptr,
    nsPropertyAttribute* pAttrib4 = nullptr, nsPropertyAttribute* pAttrib5 = nullptr, nsPropertyAttribute* pAttrib6 = nullptr)
  {
    return static_cast<nsAbstractFunctionProperty*>(nsAbstractProperty::AddAttributes(pAttrib1, pAttrib2, pAttrib3, pAttrib4, pAttrib5, pAttrib6));
  }
};
