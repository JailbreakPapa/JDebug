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

class wdRTTI;
class wdPropertyAttribute;

/// \brief Determines whether a type is wdIsBitflags.
template <typename T>
struct wdIsBitflags
{
  static constexpr bool value = false;
};

template <typename T>
struct wdIsBitflags<wdBitflags<T>>
{
  static constexpr bool value = true;
};

/// \brief Determines whether a type is wdIsBitflags.
template <typename T>
struct wdIsEnum
{
  static constexpr bool value = std::is_enum<T>::value;
};

template <typename T>
struct wdIsEnum<wdEnum<T>>
{
  static constexpr bool value = true;
};

/// \brief Flags used to describe a property and its type.
struct wdPropertyFlags
{
  typedef wdUInt16 StorageType;

  enum Enum : wdUInt16
  {
    StandardType = WD_BIT(0), ///< Anything that can be stored inside an wdVariant except for pointers and containers.
    IsEnum = WD_BIT(1),       ///< enum property, cast to wdAbstractEnumerationProperty.
    Bitflags = WD_BIT(2),     ///< Bitflags property, cast to wdAbstractEnumerationProperty.
    Class = WD_BIT(3),        ///< A struct or class. All of the above are mutually exclusive.

    Const = WD_BIT(4),     ///< Property value is const.
    Reference = WD_BIT(5), ///< Property value is a reference.
    Pointer = WD_BIT(6),   ///< Property value is a pointer.

    PointerOwner = WD_BIT(7), ///< This pointer property takes ownership of the passed pointer.
    ReadOnly = WD_BIT(8),     ///< Can only be read but not modified.
    Hidden = WD_BIT(9),       ///< This property should not appear in the UI.
    Phantom = WD_BIT(10),     ///< Phantom types are mirrored types on the editor side. Ie. they do not exist as actual classes in the process. Also used
                              ///< for data driven types, e.g. by the Visual Shader asset.

    VarOut = WD_BIT(11),   ///< Tag for non-const-ref function parameters to indicate usage 'out'
    VarInOut = WD_BIT(12), ///< Tag for non-const-ref function parameters to indicate usage 'inout'

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
  static wdBitflags<wdPropertyFlags> GetParameterFlags()
  {
    using CleanType = typename wdTypeTraits<Type>::NonConstReferencePointerType;
    wdBitflags<wdPropertyFlags> flags;
    wdVariantType::Enum type = static_cast<wdVariantType::Enum>(wdVariantTypeDeduction<CleanType>::value);
    if (std::is_same<CleanType, wdVariant>::value)
      flags.Add(wdPropertyFlags::StandardType);
    else if (std::is_same<Type, const char*>::value)
      // We treat const char* as a basic type and not a pointer.
      flags.Add(wdPropertyFlags::StandardType);
    else if ((type >= wdVariantType::FirstStandardType && type <= wdVariantType::LastStandardType) || WD_IS_SAME_TYPE(wdVariant, Type))
      flags.Add(wdPropertyFlags::StandardType);
    else if (wdIsEnum<CleanType>::value)
      flags.Add(wdPropertyFlags::IsEnum);
    else if (wdIsBitflags<CleanType>::value)
      flags.Add(wdPropertyFlags::Bitflags);
    else
      flags.Add(wdPropertyFlags::Class);

    if (std::is_const<typename wdTypeTraits<Type>::NonReferencePointerType>::value)
      flags.Add(wdPropertyFlags::Const);

    if (std::is_pointer<Type>::value && !std::is_same<Type, const char*>::value)
      flags.Add(wdPropertyFlags::Pointer);

    if (std::is_reference<Type>::value)
      flags.Add(wdPropertyFlags::Reference);

    return flags;
  }
};

template <>
inline wdBitflags<wdPropertyFlags> wdPropertyFlags::GetParameterFlags<void>()
{
  return wdBitflags<wdPropertyFlags>();
}

WD_DECLARE_FLAGS_OPERATORS(wdPropertyFlags)

/// \brief Describes what category a property belongs to.
struct wdPropertyCategory
{
  using StorageType = wdUInt8;

  enum Enum
  {
    Constant, ///< The property is a constant value that is stored inside the RTTI data.
    Member,   ///< The property is a 'member property', i.e. it represents some accessible value. Cast to wdAbstractMemberProperty.
    Function, ///< The property is a function which can be called. Cast to wdAbstractFunctionProperty.
    Array,    ///< The property is actually an array of values. The array dimensions might be changeable. Cast to wdAbstractArrayProperty.
    Set,      ///< The property is actually a set of values. Cast to wdAbstractArrayProperty.
    Map,      ///< The property is actually a map from string to values. Cast to wdAbstractArrayProperty.
    Default = Member
  };
};

/// \brief This is the base interface for all properties in the reflection system. It provides enough information to cast to the next better
/// base class.
class WD_FOUNDATION_DLL wdAbstractProperty
{
public:
  /// \brief The constructor must get the name of the property. The string must be a compile-time constant.
  wdAbstractProperty(const char* szPropertyName) { m_szPropertyName = szPropertyName; }

  virtual ~wdAbstractProperty() = default;

  /// \brief Returns the name of the property.
  const char* GetPropertyName() const { return m_szPropertyName; }

  /// \brief Returns the type information of the constant property. Use this to cast this property to a specific version of
  /// wdTypedConstantProperty.
  virtual const wdRTTI* GetSpecificType() const = 0;

  /// \brief Returns the category of this property. Cast this property to the next higher type for more information.
  virtual wdPropertyCategory::Enum GetCategory() const = 0; // [tested]

  /// \brief Returns the flags of the property.
  const wdBitflags<wdPropertyFlags>& GetFlags() const { return m_Flags; };

  /// \brief Adds flags to the property. Returns itself to allow to be called during initialization.
  wdAbstractProperty* AddFlags(wdBitflags<wdPropertyFlags> flags)
  {
    m_Flags.Add(flags);
    return this;
  };

  /// \brief Adds attributes to the property. Returns itself to allow to be called during initialization. Allocate an attribute using
  /// standard 'new'.
  wdAbstractProperty* AddAttributes(wdPropertyAttribute* pAttrib1, wdPropertyAttribute* pAttrib2 = nullptr, wdPropertyAttribute* pAttrib3 = nullptr,
    wdPropertyAttribute* pAttrib4 = nullptr, wdPropertyAttribute* pAttrib5 = nullptr, wdPropertyAttribute* pAttrib6 = nullptr)
  {
    WD_ASSERT_DEV(pAttrib1 != nullptr, "invalid attribute");

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
  const wdArrayPtr<wdPropertyAttribute* const> GetAttributes() const { return m_Attributes.GetArrayPtr(); }

  /// \brief Returns the first attribute that derives from the given type, or nullptr if nothing is found.
  template <typename Type>
  const Type* GetAttributeByType() const;

protected:
  wdBitflags<wdPropertyFlags> m_Flags;
  const char* m_szPropertyName;
  wdHybridArray<wdPropertyAttribute*, 2, wdStaticAllocatorWrapper> m_Attributes; // Do not track RTTI data.
};

/// \brief This is the base class for all constant properties that are stored inside the RTTI data.
class WD_FOUNDATION_DLL wdAbstractConstantProperty : public wdAbstractProperty
{
public:
  /// \brief Passes the property name through to wdAbstractProperty.
  wdAbstractConstantProperty(const char* szPropertyName)
    : wdAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns wdPropertyCategory::Constant.
  virtual wdPropertyCategory::Enum GetCategory() const override { return wdPropertyCategory::Constant; } // [tested]

  /// \brief Returns a pointer to the constant data or nullptr. See wdAbstractMemberProperty::GetPropertyPointer for more information.
  virtual void* GetPropertyPointer() const = 0;

  /// \brief Returns the constant value as an wdVariant
  virtual wdVariant GetConstant() const = 0;
};

/// \brief This is the base class for all properties that are members of a class. It provides more information about the actual type.
///
/// If wdPropertyFlags::Pointer is set as a flag, you must not cast this property to wdTypedMemberProperty, instead use GetValuePtr and
/// SetValuePtr. This is because reference and const-ness of the property are only fixed for the pointer but not the type, so the actual
/// property type cannot be derived.
class WD_FOUNDATION_DLL wdAbstractMemberProperty : public wdAbstractProperty
{
public:
  /// \brief Passes the property name through to wdAbstractProperty.
  wdAbstractMemberProperty(const char* szPropertyName)
    : wdAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns wdPropertyCategory::Member.
  virtual wdPropertyCategory::Enum GetCategory() const override { return wdPropertyCategory::Member; }

  /// \brief Returns a pointer to the property data or nullptr. If a valid pointer is returned, that pointer and the information from
  /// GetSpecificType() can be used to step deeper into the type (if required).
  ///
  /// You need to pass the pointer to an object on which you are operating. This function is mostly of interest when the property itself is
  /// a compound type (a struct or class). If it is a simple type (int, float, etc.) it doesn't make much sense to retrieve the pointer.
  ///
  /// For example GetSpecificType() might return that a property is of type wdVec3. In that case one might either stop and just use the code
  /// to handle wdVec3 types, or one might continue and enumerate all sub-properties (x, y and z) as well.
  ///
  /// \note There is no guarantee that this function returns a non-nullptr pointer, independent of the type. When a property uses custom
  /// 'accessors' (functions to get / set the property value), it is not possible (or useful) to get the property pointer.
  virtual void* GetPropertyPointer(const void* pInstance) const = 0;

  /// \brief Writes the value of this property in pInstance to pObject.
  /// pObject needs to point to an instance of this property's type.
  virtual void GetValuePtr(const void* pInstance, void* out_pObject) const = 0;

  /// \brief Sets the value of pObject to the property in pInstance.
  /// pObject needs to point to an instance of this property's type.
  virtual void SetValuePtr(void* pInstance, const void* pObject) = 0;
};


/// \brief The base class for a property that represents an array of values.
class WD_FOUNDATION_DLL wdAbstractArrayProperty : public wdAbstractProperty
{
public:
  /// \brief Passes the property name through to wdAbstractProperty.
  wdAbstractArrayProperty(const char* szPropertyName)
    : wdAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns wdPropertyCategory::Array.
  virtual wdPropertyCategory::Enum GetCategory() const override { return wdPropertyCategory::Array; }

  /// \brief Returns number of elements.
  virtual wdUInt32 GetCount(const void* pInstance) const = 0;

  /// \brief Writes element at index uiIndex to the target of pObject.
  virtual void GetValue(const void* pInstance, wdUInt32 uiIndex, void* pObject) const = 0;

  /// \brief Writes the target of pObject to the element at index uiIndex.
  virtual void SetValue(void* pInstance, wdUInt32 uiIndex, const void* pObject) = 0;

  /// \brief Inserts the target of pObject into the array at index uiIndex.
  virtual void Insert(void* pInstance, wdUInt32 uiIndex, const void* pObject) = 0;

  /// \brief Removes the element in the array at index uiIndex.
  virtual void Remove(void* pInstance, wdUInt32 uiIndex) = 0;

  /// \brief Clears the array.
  virtual void Clear(void* pInstance) = 0;

  /// \brief Resizes the array to uiCount.
  virtual void SetCount(void* pInstance, wdUInt32 uiCount) = 0;
};


/// \brief The base class for a property that represents a set of values.
///
/// The element type must either be a standard type or a pointer.
class WD_FOUNDATION_DLL wdAbstractSetProperty : public wdAbstractProperty
{
public:
  /// \brief Passes the property name through to wdAbstractProperty.
  wdAbstractSetProperty(const char* szPropertyName)
    : wdAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns wdPropertyCategory::Set.
  virtual wdPropertyCategory::Enum GetCategory() const override { return wdPropertyCategory::Set; }

  /// \brief Returns whether the set is empty.
  virtual bool IsEmpty(const void* pInstance) const = 0;

  /// \brief Clears the set.
  virtual void Clear(void* pInstance) = 0;

  /// \brief Inserts the target of pObject into the set.
  virtual void Insert(void* pInstance, const void* pObject) = 0;

  /// \brief Removes the target of pObject from the set.
  virtual void Remove(void* pInstance, const void* pObject) = 0;

  /// \brief Returns whether the target of pObject is in the set.
  virtual bool Contains(const void* pInstance, const void* pObject) const = 0;

  /// \brief Writes the content of the set to out_keys.
  virtual void GetValues(const void* pInstance, wdDynamicArray<wdVariant>& out_keys) const = 0;
};


/// \brief The base class for a property that represents a set of values.
///
/// The element type must either be a standard type or a pointer.
class WD_FOUNDATION_DLL wdAbstractMapProperty : public wdAbstractProperty
{
public:
  /// \brief Passes the property name through to wdAbstractProperty.
  wdAbstractMapProperty(const char* szPropertyName)
    : wdAbstractProperty(szPropertyName)
  {
  }

  /// \brief Returns wdPropertyCategory::Map.
  virtual wdPropertyCategory::Enum GetCategory() const override { return wdPropertyCategory::Map; }

  /// \brief Returns whether the set is empty.
  virtual bool IsEmpty(const void* pInstance) const = 0;

  /// \brief Clears the set.
  virtual void Clear(void* pInstance) = 0;

  /// \brief Inserts the target of pObject into the set.
  virtual void Insert(void* pInstance, const char* szKey, const void* pObject) = 0;

  /// \brief Removes the target of pObject from the set.
  virtual void Remove(void* pInstance, const char* szKey) = 0;

  /// \brief Returns whether the target of pObject is in the set.
  virtual bool Contains(const void* pInstance, const char* szKey) const = 0;

  /// \brief Writes element at index uiIndex to the target of pObject.
  virtual bool GetValue(const void* pInstance, const char* szKey, void* pObject) const = 0;

  /// \brief Writes the content of the set to out_keys.
  virtual void GetKeys(const void* pInstance, wdHybridArray<wdString, 16>& out_keys) const = 0;
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
struct wdFunctionParameterTypeResolver
{
};

template <int I, typename R, typename... P>
struct wdFunctionParameterTypeResolver<I, R (*)(P...)>
{
  enum Constants
  {
    Arguments = sizeof...(P),
  };
  WD_CHECK_AT_COMPILETIME_MSG(I < Arguments, "I needs to be smaller than the number of function parameters.");
  using ParameterType = typename getArgument<I, P...>::Type;
  using ReturnType = R;
};

template <int I, class Class, typename R, typename... P>
struct wdFunctionParameterTypeResolver<I, R (Class::*)(P...)>
{
  enum Constants
  {
    Arguments = sizeof...(P),
  };
  WD_CHECK_AT_COMPILETIME_MSG(I < Arguments, "I needs to be smaller than the number of function parameters.");
  using ParameterType = typename getArgument<I, P...>::Type;
  using ReturnType = R;
};

template <int I, class Class, typename R, typename... P>
struct wdFunctionParameterTypeResolver<I, R (Class::*)(P...) const>
{
  enum Constants
  {
    Arguments = sizeof...(P),
  };
  WD_CHECK_AT_COMPILETIME_MSG(I < Arguments, "I needs to be smaller than the number of function parameters.");
  using ParameterType = typename getArgument<I, P...>::Type;
  using ReturnType = R;
};

/// \brief Template that allows to probe a single parameter function for parameter and return type.
template <typename FUNC>
struct wdMemberFunctionParameterTypeResolver
{
};

template <class Class, typename R, typename P>
struct wdMemberFunctionParameterTypeResolver<R (Class::*)(P)>
{
  using ParameterType = P;
  using ReturnType = R;
};

/// \brief Template that allows to probe a container for its element type.
template <typename CONTAINER>
struct wdContainerSubTypeResolver
{
};

template <typename T>
struct wdContainerSubTypeResolver<wdArrayPtr<T>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct wdContainerSubTypeResolver<wdDynamicArray<T>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename T, wdUInt32 Size>
struct wdContainerSubTypeResolver<wdHybridArray<T, Size>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename T, wdUInt32 Size>
struct wdContainerSubTypeResolver<wdStaticArray<T, Size>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename T, wdUInt16 Size>
struct wdContainerSubTypeResolver<wdSmallArray<T, Size>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct wdContainerSubTypeResolver<wdDeque<T>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct wdContainerSubTypeResolver<wdSet<T>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename T>
struct wdContainerSubTypeResolver<wdHashSet<T>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename K, typename T>
struct wdContainerSubTypeResolver<wdHashTable<K, T>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};

template <typename K, typename T>
struct wdContainerSubTypeResolver<wdMap<K, T>>
{
  using Type = typename wdTypeTraits<T>::NonConstReferenceType;
};


/// \brief Describes what kind of function a property is.
struct wdFunctionType
{
  using StorageType = wdUInt8;

  enum Enum
  {
    Member,       ///< A normal member function, a valid instance pointer must be provided to call.
    StaticMember, ///< A static member function, instance pointer will be ignored.
    Constructor,  ///< A constructor. Return value is a void* pointing to the new instance allocated with the default allocator.
    Default = Member
  };
};

/// \brief The base class for a property that represents a function.
class wdAbstractFunctionProperty : public wdAbstractProperty
{
public:
  /// \brief Passes the property name through to wdAbstractProperty.
  wdAbstractFunctionProperty(const char* szPropertyName)
    : wdAbstractProperty(szPropertyName)
  {
  }

  virtual wdPropertyCategory::Enum GetCategory() const override { return wdPropertyCategory::Function; }
  /// \brief Returns the type of function, see wdFunctionPropertyType::Enum.
  virtual wdFunctionType::Enum GetFunctionType() const = 0;
  /// \brief Returns the type of the return value.
  virtual const wdRTTI* GetReturnType() const = 0;
  /// \brief Returns property flags of the return value.
  virtual wdBitflags<wdPropertyFlags> GetReturnFlags() const = 0;
  /// \brief Returns the number of arguments.
  virtual wdUInt32 GetArgumentCount() const = 0;
  /// \brief Returns the type of the given argument.
  virtual const wdRTTI* GetArgumentType(wdUInt32 uiParamIndex) const = 0;
  /// \brief Returns the property flags of the given argument.
  virtual wdBitflags<wdPropertyFlags> GetArgumentFlags(wdUInt32 uiParamIndex) const = 0;

  /// \brief Calls the function. Provide the instance on which the function is supposed to be called.
  ///
  /// arguments must be the size of GetArgumentCount, the following rules apply for both arguments and return value:
  /// Any standard type must be provided by value, even if it is a pointer to one. Types must match exactly, no ConvertTo is called.
  /// enum and bitflags are supported if wdEnum / wdBitflags is used, value must be provided as wdInt64.
  /// Out values (&, *) are written back to the variant they were read from.
  /// Any class is provided by pointer, regardless of whether it is a pointer or not.
  /// The returnValue must only be valid if the return value is a ref or by value class. In that case
  /// returnValue must be a ptr to a valid class instance of the returned type.
  /// An invalid variant is equal to a nullptr, except for if the argument is of type wdVariant, in which case
  /// it is impossible to pass along a nullptr.
  virtual void Execute(void* pInstance, wdArrayPtr<wdVariant> arguments, wdVariant& ref_returnValue) const = 0;

  virtual const wdRTTI* GetSpecificType() const override { return GetReturnType(); }
};
