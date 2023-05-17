#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/VariantType.h>
#include <type_traits>

class wdRTTI;
class wdReflectedClass;
class wdVariant;

/// \brief Flags that describe a reflected type.
struct wdTypeFlags
{
  typedef wdUInt8 StorageType;

  enum Enum
  {
    StandardType = WD_BIT(0), ///< Anything that can be stored inside an wdVariant except for pointers and containers.
    IsEnum = WD_BIT(1),       ///< enum struct used for wdEnum.
    Bitflags = WD_BIT(2),     ///< bitflags struct used for wdBitflags.
    Class = WD_BIT(3),        ///< A class or struct. The above flags are mutually exclusive.

    Abstract = WD_BIT(4), ///< Type is abstract.
    Phantom = WD_BIT(5),  ///< De-serialized type information that cannot be created on this process.
    Minimal = WD_BIT(6),  ///< Does not contain any property, function or attribute information. Used only for versioning.
    Default = 0
  };

  struct Bits
  {
    StorageType StandardType : 1;
    StorageType IsEnum : 1;
    StorageType Bitflags : 1;
    StorageType Class : 1;
    StorageType Abstract : 1;
    StorageType Phantom : 1;
  };
};

WD_DECLARE_FLAGS_OPERATORS(wdTypeFlags)


// ****************************************************
// ***** Templates for accessing static RTTI data *****

namespace wdInternal
{
  /// \brief [internal] Helper struct for accessing static RTTI data.
  template <typename T>
  struct wdStaticRTTI
  {
  };

  // Special implementation for types that have no base
  template <>
  struct wdStaticRTTI<wdNoBase>
  {
    static const wdRTTI* GetRTTI() { return nullptr; }
  };

  // Special implementation for void to make function reflection compile void return values without further specialization.
  template <>
  struct wdStaticRTTI<void>
  {
    static const wdRTTI* GetRTTI() { return nullptr; }
  };

  template <typename T>
  WD_ALWAYS_INLINE const wdRTTI* GetStaticRTTI(wdTraitInt<1>) // class derived from wdReflectedClass
  {
    return T::GetStaticRTTI();
  }

  template <typename T>
  WD_ALWAYS_INLINE const wdRTTI* GetStaticRTTI(wdTraitInt<0>) // static rtti
  {
    // Since this is pure C++ and no preprocessor macro, calling it with types such as 'int' and 'wdInt32' will
    // actually return the same RTTI object, which would not be possible with a purely macro based solution

    return wdStaticRTTI<T>::GetRTTI();
  }

  template <typename Type>
  wdBitflags<wdTypeFlags> DetermineTypeFlags()
  {
    wdBitflags<wdTypeFlags> flags;
    wdVariantType::Enum type =
      static_cast<wdVariantType::Enum>(wdVariantTypeDeduction<typename wdTypeTraits<Type>::NonConstReferenceType>::value);
    if ((type >= wdVariantType::FirstStandardType && type <= wdVariantType::LastStandardType) || WD_IS_SAME_TYPE(wdVariant, Type))
      flags.Add(wdTypeFlags::StandardType);
    else
      flags.Add(wdTypeFlags::Class);

    if (std::is_abstract<Type>::value)
      flags.Add(wdTypeFlags::Abstract);

    return flags;
  }

  template <>
  WD_ALWAYS_INLINE wdBitflags<wdTypeFlags> DetermineTypeFlags<wdVariant>()
  {
    return wdTypeFlags::StandardType;
  }

  template <typename T>
  struct wdStaticRTTIWrapper
  {
    static_assert(sizeof(T) == 0, "Type has not been declared as reflectable (use WD_DECLARE_REFLECTABLE_TYPE macro)");
  };
} // namespace wdInternal

/// \brief Use this function, specialized with the type that you are interested in, to get the static RTTI data for some type.
template <typename T>
WD_ALWAYS_INLINE const wdRTTI* wdGetStaticRTTI()
{
  return wdInternal::GetStaticRTTI<T>(wdTraitInt<WD_IS_DERIVED_FROM_STATIC(wdReflectedClass, T)>());
}

// **************************************************
// ***** Macros for declaring types reflectable *****

#define WD_NO_LINKAGE

/// \brief Declares a type to be statically reflectable. Insert this into the header of a type to enable reflection on it.
/// This is not needed if the type is already dynamically reflectable.
#define WD_DECLARE_REFLECTABLE_TYPE(Linkage, TYPE)                    \
  namespace wdInternal                                                \
  {                                                                   \
    template <>                                                       \
    struct Linkage wdStaticRTTIWrapper<TYPE>                          \
    {                                                                 \
      static wdRTTI s_RTTI;                                           \
    };                                                                \
                                                                      \
    /* This specialization calls the function to get the RTTI data */ \
    /* This code might get duplicated in different DLLs, but all   */ \
    /* will call the same function, so the RTTI object is unique   */ \
    template <>                                                       \
    struct wdStaticRTTI<TYPE>                                         \
    {                                                                 \
      WD_ALWAYS_INLINE static const wdRTTI* GetRTTI()                 \
      {                                                               \
        return &wdStaticRTTIWrapper<TYPE>::s_RTTI;                    \
      }                                                               \
    };                                                                \
  }

/// \brief Insert this into a class/struct to enable properties that are private members.
/// All types that have dynamic reflection (\see WD_ADD_DYNAMIC_REFLECTION) already have this ability.
#define WD_ALLOW_PRIVATE_PROPERTIES(SELF) friend wdRTTI GetRTTI(SELF*)

/// \cond
// internal helper macro
#define WD_RTTIINFO_DECL(Type, BaseType, Version) \
                                                  \
  const char* GetTypeName(Type*)                  \
  {                                               \
    return #Type;                                 \
  }                                               \
  wdUInt32 GetTypeVersion(Type*)                  \
  {                                               \
    return Version;                               \
  }                                               \
                                                  \
  wdRTTI GetRTTI(Type*);

// internal helper macro
#define WD_RTTIINFO_GETRTTI_IMPL_BEGIN(Type, BaseType, AllocatorType)              \
  wdRTTI GetRTTI(Type*)                                                            \
  {                                                                                \
    using OwnType = Type;                                                          \
    using OwnBaseType = BaseType;                                                  \
    static AllocatorType Allocator;                                                \
    static wdBitflags<wdTypeFlags> flags = wdInternal::DetermineTypeFlags<Type>(); \
    static wdArrayPtr<wdAbstractProperty*> Properties;                             \
    static wdArrayPtr<wdAbstractProperty*> Functions;                              \
    static wdArrayPtr<wdPropertyAttribute*> Attributes;                            \
    static wdArrayPtr<wdAbstractMessageHandler*> MessageHandlers;                  \
    static wdArrayPtr<wdMessageSenderInfo> MessageSenders;

/// \endcond

/// \brief Implements the necessary functionality for a type to be statically reflectable.
///
/// \param Type
///   The type for which the reflection functionality should be implemented.
/// \param BaseType
///   The base class type of \a Type. If it has no base class, pass wdNoBase
/// \param Version
///   The version of \a Type. Must be increased when the class serialization changes.
/// \param AllocatorType
///   The type of an wdRTTIAllocator that can be used to create and destroy instances
///   of \a Type. Pass wdRTTINoAllocator for types that should not be created dynamically.
///   Pass wdRTTIDefaultAllocator<Type> for types that should be created on the default heap.
///   Pass a custom wdRTTIAllocator type to handle allocation differently.
#define WD_BEGIN_STATIC_REFLECTED_TYPE(Type, BaseType, Version, AllocatorType) \
  WD_RTTIINFO_DECL(Type, BaseType, Version)                                    \
  wdRTTI wdInternal::wdStaticRTTIWrapper<Type>::s_RTTI = GetRTTI((Type*)0);    \
  WD_RTTIINFO_GETRTTI_IMPL_BEGIN(Type, BaseType, AllocatorType)


/// \brief Ends the reflection code block that was opened with WD_BEGIN_STATIC_REFLECTED_TYPE.
#define WD_END_STATIC_REFLECTED_TYPE                                                                                                         \
  ;                                                                                                                                          \
  return wdRTTI(GetTypeName((OwnType*)0), wdGetStaticRTTI<OwnBaseType>(), sizeof(OwnType), GetTypeVersion((OwnType*)0),                      \
    wdVariantTypeDeduction<OwnType>::value, flags, &Allocator, Properties, Functions, Attributes, MessageHandlers, MessageSenders, nullptr); \
  }


/// \brief Within a WD_BEGIN_REFLECTED_TYPE / WD_END_REFLECTED_TYPE block, use this to start the block that declares all the properties.
#define WD_BEGIN_PROPERTIES static wdAbstractProperty* PropertyList[] =



/// \brief Ends the block to declare properties that was started with WD_BEGIN_PROPERTIES.
#define WD_END_PROPERTIES \
  ;                       \
  Properties = PropertyList

/// \brief Within a WD_BEGIN_REFLECTED_TYPE / WD_END_REFLECTED_TYPE block, use this to start the block that declares all the functions.
#define WD_BEGIN_FUNCTIONS static wdAbstractProperty* FunctionList[] =



/// \brief Ends the block to declare functions that was started with WD_BEGIN_FUNCTIONS.
#define WD_END_FUNCTIONS \
  ;                      \
  Functions = FunctionList

/// \brief Within a WD_BEGIN_REFLECTED_TYPE / WD_END_REFLECTED_TYPE block, use this to start the block that declares all the attributes.
#define WD_BEGIN_ATTRIBUTES static wdPropertyAttribute* AttributeList[] =



/// \brief Ends the block to declare attributes that was started with WD_BEGIN_ATTRIBUTES.
#define WD_END_ATTRIBUTES \
  ;                       \
  Attributes = AttributeList

/// \brief Within a WD_BEGIN_FUNCTIONS / WD_END_FUNCTIONS; block, this adds a member or static function property stored inside the RTTI
/// data.
///
/// \param Function
///   The function to be executed, must match the C++ function name.
#define WD_FUNCTION_PROPERTY(Function) (new wdFunctionProperty<decltype(&OwnType::Function)>(WD_STRINGIZE(Function), &OwnType::Function))

/// \brief Within a WD_BEGIN_FUNCTIONS / WD_END_FUNCTIONS; block, this adds a member or static function property stored inside the RTTI
/// data. Use this version if you need to change the name of the function or need to cast the function to one of its overload versions.
///
/// \param PropertyName
///   The name under which the property should be registered.
///
/// \param Function
///   The function to be executed, must match the C++ function name including the class name e.g. 'CLASS::NAME'.
#define WD_FUNCTION_PROPERTY_EX(PropertyName, Function) (new wdFunctionProperty<decltype(&Function)>(PropertyName, &Function))

/// \internal Used by WD_SCRIPT_FUNCTION_PROPERTY
#define _WD_SCRIPT_FUNCTION_PARAM(type, name) wdScriptableFunctionAttribute::ArgType::type, name

/// \brief Convenience macro to declare a function that can be called from scripts.
///
/// \param Function
///   The function to be executed, must match the C++ function name including the class name e.g. 'CLASS::NAME'.
///
/// Internally this calls WD_FUNCTION_PROPERTY and adds a wdScriptableFunctionAttribute.
/// Use the variadic arguments in pairs to configure how each function parameter gets exposed.
///   Use 'In', 'Out' or 'Inout' to specify whether a function parameter is only read, or also written back to.
///   Follow it with a string to specify the name under which the parameter should show up.
///
/// Example:
///   WD_SCRIPT_FUNCTION_PROPERTY(MyFunc1NoParams)
///   WD_SCRIPT_FUNCTION_PROPERTY(MyFunc2FloatInDoubleOut, In, "FloatValue", Out, "DoubleResult")
#define WD_SCRIPT_FUNCTION_PROPERTY(Function, ...) \
  WD_FUNCTION_PROPERTY(Function)->AddAttributes(new wdScriptableFunctionAttribute(WD_EXPAND_ARGS_PAIR_COMMA(_WD_SCRIPT_FUNCTION_PARAM, ##__VA_ARGS__)))

/// \brief Within a WD_BEGIN_FUNCTIONS / WD_END_FUNCTIONS; block, this adds a constructor function property stored inside the RTTI data.
///
/// \param Function
///   The function to be executed in the form of CLASS::FUNCTION_NAME.
#define WD_CONSTRUCTOR_PROPERTY(...) (new wdConstructorFunctionProperty<OwnType, ##__VA_ARGS__>())


// [internal] Helper macro to get the return type of a getter function.
#define WD_GETTER_TYPE(Class, GetterFunc) decltype(std::declval<Class>().GetterFunc())

/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that uses custom getter / setter functions.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param Getter
///   The getter function for this property.
/// \param Setter
///   The setter function for this property.
///
/// \note There does not actually need to be a variable for this type of properties, as all accesses go through functions.
/// Thus you can for example expose a 'vector' property that is actually stored as a column of a matrix.
#define WD_ACCESSOR_PROPERTY(PropertyName, Getter, Setter) \
  (new wdAccessorProperty<OwnType, WD_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, &OwnType::Setter))

/// \brief Same as WD_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define WD_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, Getter) \
  (new wdAccessorProperty<OwnType, WD_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, nullptr))

// [internal] Helper macro to get the return type of a array getter function.
#define WD_ARRAY_GETTER_TYPE(Class, GetterFunc) decltype(std::declval<Class>().GetterFunc(0))

/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that uses custom functions to access an array.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param GetCount
///   Function signature: wdUInt32 GetCount() const;
/// \param Getter
///   Function signature: Type GetValue(wdUInt32 uiIndex) const;
/// \param Setter
///   Function signature: void SetValue(wdUInt32 uiIndex, Type value);
/// \param Insert
///   Function signature: void Insert(wdUInt32 uiIndex, Type value);
/// \param Remove
///   Function signature: void Remove(wdUInt32 uiIndex);
#define WD_ARRAY_ACCESSOR_PROPERTY(PropertyName, GetCount, Getter, Setter, Insert, Remove) \
  (new wdAccessorArrayProperty<OwnType, WD_ARRAY_GETTER_TYPE(OwnType, OwnType::Getter)>(   \
    PropertyName, &OwnType::GetCount, &OwnType::Getter, &OwnType::Setter, &OwnType::Insert, &OwnType::Remove))

/// \brief Same as WD_ARRAY_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define WD_ARRAY_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, GetCount, Getter)             \
  (new wdAccessorArrayProperty<OwnType, WD_ARRAY_GETTER_TYPE(OwnType, OwnType::Getter)>( \
    PropertyName, &OwnType::GetCount, &OwnType::Getter, nullptr, nullptr, nullptr))

#define WD_SET_CONTAINER_TYPE(Class, GetterFunc) decltype(std::declval<Class>().GetterFunc())

#define WD_SET_CONTAINER_SUB_TYPE(Class, GetterFunc) \
  wdContainerSubTypeResolver<wdTypeTraits<decltype(std::declval<Class>().GetterFunc())>::NonConstReferenceType>::Type

/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that uses custom functions to access a set.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param GetValues
///   Function signature: Container<Type> GetValues() const;
/// \param Insert
///   Function signature: void Insert(Type value);
/// \param Remove
///   Function signature: void Remove(Type value);
///
/// \note Container<Type> can be any container that can be iterated via range based for loops.
#define WD_SET_ACCESSOR_PROPERTY(PropertyName, GetValues, Insert, Remove)                                            \
  (new wdAccessorSetProperty<OwnType, wdFunctionParameterTypeResolver<0, decltype(&OwnType::Insert)>::ParameterType, \
    WD_SET_CONTAINER_TYPE(OwnType, GetValues)>(PropertyName, &OwnType::GetValues, &OwnType::Insert, &OwnType::Remove))

/// \brief Same as WD_SET_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define WD_SET_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, GetValues)                                                              \
  (new wdAccessorSetProperty<OwnType, WD_SET_CONTAINER_SUB_TYPE(OwnType, GetValues), WD_SET_CONTAINER_TYPE(OwnType, GetValues)>( \
    PropertyName, &OwnType::GetValues, nullptr, nullptr))

/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that uses custom functions to for write access to a
/// map.
///   Use this if you have a wdHashTable or wdMap to expose directly and just want to be informed of write operations.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param GetContainer
///   Function signature: const Container<Key, Type>& GetValues() const;
/// \param Insert
///   Function signature: void Insert(const char* szKey, Type value);
/// \param Remove
///   Function signature: void Remove(const char* szKey);
///
/// \note Container can be wdMap or wdHashTable
#define WD_MAP_WRITE_ACCESSOR_PROPERTY(PropertyName, GetContainer, Insert, Remove)                                        \
  (new wdWriteAccessorMapProperty<OwnType, wdFunctionParameterTypeResolver<1, decltype(&OwnType::Insert)>::ParameterType, \
    WD_SET_CONTAINER_TYPE(OwnType, GetContainer)>(PropertyName, &OwnType::GetContainer, &OwnType::Insert, &OwnType::Remove))

/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that uses custom functions to access a map.
///   Use this if you you want to hide the implementation details of the map from the user.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param GetKeyRange
///   Function signature: const Range GetValues() const;
///   Range has to be an object that a ranged based for-loop can iterate over containing the keys
///   implicitly convertible to Type / wdString.
/// \param GetValue
///   Function signature: bool GetValue(const char* szKey, Type& value);
///   Returns whether the the key existed. value must be a non const ref as it is written to.
/// \param Insert
///   Function signature: void Insert(const char* szKey, Type value);
///   value can also be const and/or a reference.
/// \param Remove
///   Function signature: void Remove(const char* szKey);
///
/// \note Container can be wdMap or wdHashTable
#define WD_MAP_ACCESSOR_PROPERTY(PropertyName, GetKeyRange, GetValue, Insert, Remove)                                \
  (new wdAccessorMapProperty<OwnType, wdFunctionParameterTypeResolver<1, decltype(&OwnType::Insert)>::ParameterType, \
    WD_SET_CONTAINER_TYPE(OwnType, GetKeyRange)>(PropertyName, &OwnType::GetKeyRange, &OwnType::GetValue, &OwnType::Insert, &OwnType::Remove))

/// \brief Same as WD_MAP_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define WD_MAP_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, GetKeyRange, GetValue)                                           \
  (new wdAccessorMapProperty<OwnType,                                                                                     \
    wdTypeTraits<wdFunctionParameterTypeResolver<1, decltype(&OwnType::GetValue)>::ParameterType>::NonConstReferenceType, \
    WD_SET_CONTAINER_TYPE(OwnType, GetKeyRange)>(PropertyName, &OwnType::GetKeyRange, &OwnType::GetValue, nullptr, nullptr))



/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that uses custom getter / setter functions.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param EnumType
///   The name of the enum struct used by wdEnum.
/// \param Getter
///   The getter function for this property.
/// \param Setter
///   The setter function for this property.
#define WD_ENUM_ACCESSOR_PROPERTY(PropertyName, EnumType, Getter, Setter) \
  (new wdEnumAccessorProperty<OwnType, EnumType, WD_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, &OwnType::Setter))

/// \brief Same as WD_ENUM_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define WD_ENUM_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, EnumType, Getter) \
  (new wdEnumAccessorProperty<OwnType, EnumType, WD_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, nullptr))

/// \brief Same as WD_ENUM_ACCESSOR_PROPERTY, but for bitfields.
#define WD_BITFLAGS_ACCESSOR_PROPERTY(PropertyName, BitflagsType, Getter, Setter) \
  (new wdBitflagsAccessorProperty<OwnType, BitflagsType, WD_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, &OwnType::Setter))

/// \brief Same as WD_BITFLAGS_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define WD_BITFLAGS_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, BitflagsType, Getter) \
  (new wdBitflagsAccessorProperty<OwnType, BitflagsType, WD_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, nullptr))


// [internal] Helper macro to get the type of a class member.
#define WD_MEMBER_TYPE(Class, Member) decltype(std::declval<Class>().Member)

#define WD_MEMBER_CONTAINER_SUB_TYPE(Class, Member) \
  wdContainerSubTypeResolver<wdTypeTraits<decltype(std::declval<Class>().Member)>::NonConstReferenceType>::Type

/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that actually exists as a member.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param MemberName
///   The name of the member variable that should get exposed as a property.
///
/// \note Since the member is exposed directly, there is no way to know when the variable was modified. That also means
/// no custom limits to the values can be applied. If that becomes necessary, just add getter / setter functions and
/// expose the property as a WD_ENUM_ACCESSOR_PROPERTY instead.
#define WD_MEMBER_PROPERTY(PropertyName, MemberName)                                                   \
  (new wdMemberProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,                    \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::SetValue, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as WD_MEMBER_PROPERTY, but the property is read-only.
#define WD_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                  \
  (new wdMemberProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,                             \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, nullptr, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as WD_MEMBER_PROPERTY, but the property is an array (wdHybridArray, wdDynamicArray or wdDeque).
#define WD_ARRAY_MEMBER_PROPERTY(PropertyName, MemberName)                                                                                  \
  (new wdMemberArrayProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), WD_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>(PropertyName, \
    &wdArrayPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer,                        \
    &wdArrayPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetContainer))

/// \brief Same as WD_MEMBER_PROPERTY, but the property is a read-only array (wdArrayPtr, wdHybridArray, wdDynamicArray or wdDeque).
#define WD_ARRAY_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                                   \
  (new wdMemberArrayReadOnlyProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), WD_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>( \
    PropertyName, &wdArrayPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer))

/// \brief Same as WD_MEMBER_PROPERTY, but the property is a set (wdSet, wdHashSet).
#define WD_SET_MEMBER_PROPERTY(PropertyName, MemberName)                                                                                  \
  (new wdMemberSetProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), WD_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>(PropertyName, \
    &wdSetPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer,                        \
    &wdSetPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetContainer))

/// \brief Same as WD_MEMBER_PROPERTY, but the property is a read-only set (wdSet, wdHashSet).
#define WD_SET_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                           \
  (new wdMemberSetProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), WD_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>( \
    PropertyName, &wdSetPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer, nullptr))

/// \brief Same as WD_MEMBER_PROPERTY, but the property is a map (wdMap, wdHashTable).
#define WD_MAP_MEMBER_PROPERTY(PropertyName, MemberName)                                                                                  \
  (new wdMemberMapProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), WD_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>(PropertyName, \
    &wdMapPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer,                        \
    &wdMapPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetContainer))

/// \brief Same as WD_MEMBER_PROPERTY, but the property is a read-only map (wdMap, wdHashTable).
#define WD_MAP_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                           \
  (new wdMemberMapProperty<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), WD_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>( \
    PropertyName, &wdMapPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer, nullptr))

/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a property that actually exists as a member.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param EnumType
///   Name of the struct used by wdEnum.
/// \param MemberName
///   The name of the member variable that should get exposed as a property.
///
/// \note Since the member is exposed directly, there is no way to know when the variable was modified. That also means
/// no custom limits to the values can be applied. If that becomes necessary, just add getter / setter functions and
/// expose the property as a WD_ACCESSOR_PROPERTY instead.
#define WD_ENUM_MEMBER_PROPERTY(PropertyName, EnumType, MemberName)                                    \
  (new wdEnumMemberProperty<OwnType, EnumType, WD_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,      \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::SetValue, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as WD_ENUM_MEMBER_PROPERTY, but the property is read-only.
#define WD_ENUM_MEMBER_PROPERTY_READ_ONLY(PropertyName, EnumType, MemberName)                                   \
  (new wdEnumMemberProperty<OwnType, EnumType, WD_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,               \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, nullptr, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as WD_ENUM_MEMBER_PROPERTY, but for bitfields.
#define WD_BITFLAGS_MEMBER_PROPERTY(PropertyName, BitflagsType, MemberName)                               \
  (new wdBitflagsMemberProperty<OwnType, BitflagsType, WD_MEMBER_TYPE(OwnType, MemberName)>(PropertyName, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue,    \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::SetValue,    \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as WD_ENUM_MEMBER_PROPERTY_READ_ONLY, but for bitfields.
#define WD_BITFLAGS_MEMBER_PROPERTY_READ_ONLY(PropertyName, BitflagsType, MemberName)                           \
  (new wdBitflagsMemberProperty<OwnType, BitflagsType, WD_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,       \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, nullptr, \
    &wdPropertyAccessor<OwnType, WD_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))



/// \brief Within a WD_BEGIN_PROPERTIES / WD_END_PROPERTIES; block, this adds a constant property stored inside the RTTI data.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param Value
///   The constant value to be stored.
#define WD_CONSTANT_PROPERTY(PropertyName, Value) (new wdConstantProperty<decltype(Value)>(PropertyName, Value))



// [internal] Helper macro
#define WD_ENUM_VALUE_TO_CONSTANT_PROPERTY(name) WD_CONSTANT_PROPERTY(WD_STRINGIZE(name), (Storage)name),

/// \brief Within a WD_BEGIN_STATIC_REFLECTED_ENUM / WD_END_STATIC_REFLECTED_ENUM block, this converts a
/// list of enum values into constant RTTI properties.
#define WD_ENUM_CONSTANTS(...) WD_EXPAND_ARGS(WD_ENUM_VALUE_TO_CONSTANT_PROPERTY, ##__VA_ARGS__)

/// \brief Within a WD_BEGIN_STATIC_REFLECTED_ENUM / WD_END_STATIC_REFLECTED_ENUM block, this converts a
/// an enum value into a constant RTTI property.
#define WD_ENUM_CONSTANT(Value) WD_CONSTANT_PROPERTY(WD_STRINGIZE(Value), (Storage)Value)

/// \brief Within a WD_BEGIN_STATIC_REFLECTED_BITFLAGS / WD_END_STATIC_REFLECTED_BITFLAGS block, this converts a
/// list of bitflags into constant RTTI properties.
#define WD_BITFLAGS_CONSTANTS(...) WD_EXPAND_ARGS(WD_ENUM_VALUE_TO_CONSTANT_PROPERTY, ##__VA_ARGS__)

/// \brief Within a WD_BEGIN_STATIC_REFLECTED_BITFLAGS / WD_END_STATIC_REFLECTED_BITFLAGS block, this converts a
/// an bitflags into a constant RTTI property.
#define WD_BITFLAGS_CONSTANT(Value) WD_CONSTANT_PROPERTY(WD_STRINGIZE(Value), (Storage)Value)



/// \brief Implements the necessary functionality for an enum to be statically reflectable.
///
/// \param Type
///   The enum struct used by wdEnum for which reflection should be defined.
/// \param Version
///   The version of \a Type. Must be increased when the class changes.
#define WD_BEGIN_STATIC_REFLECTED_ENUM(Type, Version)                          \
  WD_BEGIN_STATIC_REFLECTED_TYPE(Type, wdEnumBase, Version, wdRTTINoAllocator) \
    ;                                                                          \
    typedef Type::StorageType Storage;                                         \
    WD_BEGIN_PROPERTIES                                                        \
      {                                                                        \
        WD_CONSTANT_PROPERTY(WD_STRINGIZE(Type::Default), (Storage)Type::Default),

#define WD_END_STATIC_REFLECTED_ENUM \
  }                                  \
  WD_END_PROPERTIES                  \
  ;                                  \
  flags |= wdTypeFlags::IsEnum;      \
  flags.Remove(wdTypeFlags::Class);  \
  WD_END_STATIC_REFLECTED_TYPE


/// \brief Implements the necessary functionality for bitflags to be statically reflectable.
///
/// \param Type
///   The bitflags struct used by wdBitflags for which reflection should be defined.
/// \param Version
///   The version of \a Type. Must be increased when the class changes.
#define WD_BEGIN_STATIC_REFLECTED_BITFLAGS(Type, Version)                          \
  WD_BEGIN_STATIC_REFLECTED_TYPE(Type, wdBitflagsBase, Version, wdRTTINoAllocator) \
    ;                                                                              \
    typedef Type::StorageType Storage;                                             \
    WD_BEGIN_PROPERTIES                                                            \
      {                                                                            \
        WD_CONSTANT_PROPERTY(WD_STRINGIZE(Type::Default), (Storage)Type::Default),

#define WD_END_STATIC_REFLECTED_BITFLAGS \
  }                                      \
  WD_END_PROPERTIES                      \
  ;                                      \
  flags |= wdTypeFlags::Bitflags;        \
  flags.Remove(wdTypeFlags::Class);      \
  WD_END_STATIC_REFLECTED_TYPE



/// \brief Within an WD_BEGIN_REFLECTED_TYPE / WD_END_REFLECTED_TYPE block, use this to start the block that declares all the message
/// handlers.
#define WD_BEGIN_MESSAGEHANDLERS static wdAbstractMessageHandler* HandlerList[] =


/// \brief Ends the block to declare message handlers that was started with WD_BEGIN_MESSAGEHANDLERS.
#define WD_END_MESSAGEHANDLERS \
  ;                            \
  MessageHandlers = HandlerList


/// \brief Within an WD_BEGIN_MESSAGEHANDLERS / WD_END_MESSAGEHANDLERS; block, this adds another message handler.
///
/// \param MessageType
///   The type of message that this handler function accepts. You may add 'const' in front of it.
/// \param FunctionName
///   The actual C++ name of the message handler function.
///
/// \note A message handler is a function that takes one parameter of type wdMessage (or a derived type) and returns void.
#define WD_MESSAGE_HANDLER(MessageType, FunctionName)                                                                                   \
  new wdInternal::MessageHandler<WD_IS_CONST_MESSAGE_HANDLER(OwnType, MessageType, &OwnType::FunctionName)>::Impl<OwnType, MessageType, \
    &OwnType::FunctionName>()


/// \brief Within an WD_BEGIN_REFLECTED_TYPE / WD_END_REFLECTED_TYPE block, use this to start the block that declares all the message
/// senders.
#define WD_BEGIN_MESSAGESENDERS static wdMessageSenderInfo SenderList[] =


/// \brief Ends the block to declare message senders that was started with WD_BEGIN_MESSAGESENDERS.
#define WD_END_MESSAGESENDERS \
  ;                           \
  MessageSenders = SenderList;

/// \brief Within an WD_BEGIN_MESSAGESENDERS / WD_END_MESSAGESENDERS block, this adds another message sender.
///
/// \param MemberName
///   The name of the member variable that should get exposed as a message sender.
///
/// \note A message sender must be derived from wdMessageSenderBase.
#define WD_MESSAGE_SENDER(MemberName)                                                  \
  {                                                                                    \
#    MemberName, wdGetStaticRTTI < WD_MEMBER_TYPE(OwnType, MemberName)::MessageType>() \
  }
