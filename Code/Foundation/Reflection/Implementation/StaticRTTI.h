#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/VariantType.h>
#include <type_traits>

class nsRTTI;
class nsReflectedClass;
class nsVariant;

/// \brief Flags that describe a reflected type.
struct nsTypeFlags
{
  using StorageType = nsUInt8;

  enum Enum
  {
    StandardType = NS_BIT(0), ///< Anything that can be stored inside an nsVariant except for pointers and containers.
    IsEnum = NS_BIT(1),       ///< enum struct used for nsEnum.
    Bitflags = NS_BIT(2),     ///< bitflags struct used for nsBitflags.
    Class = NS_BIT(3),        ///< A class or struct. The above flags are mutually exclusive.

    Abstract = NS_BIT(4),     ///< Type is abstract.
    Phantom = NS_BIT(5),      ///< De-serialized type information that cannot be created on this process.
    Minimal = NS_BIT(6),      ///< Does not contain any property, function or attribute information. Used only for versioning.
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

NS_DECLARE_FLAGS_OPERATORS(nsTypeFlags)


// ****************************************************
// ***** Templates for accessing static RTTI data *****

namespace nsInternal
{
  /// \brief [internal] Helper struct for accessing static RTTI data.
  template <typename T>
  struct nsStaticRTTI
  {
  };

  // Special implementation for types that have no base
  template <>
  struct nsStaticRTTI<nsNoBase>
  {
    static const nsRTTI* GetRTTI() { return nullptr; }
  };

  // Special implementation for void to make function reflection compile void return values without further specialization.
  template <>
  struct nsStaticRTTI<void>
  {
    static const nsRTTI* GetRTTI() { return nullptr; }
  };

  template <typename T>
  NS_ALWAYS_INLINE const nsRTTI* GetStaticRTTI(nsTraitInt<1>) // class derived from nsReflectedClass
  {
    return T::GetStaticRTTI();
  }

  template <typename T>
  NS_ALWAYS_INLINE const nsRTTI* GetStaticRTTI(nsTraitInt<0>) // static rtti
  {
    // Since this is pure C++ and no preprocessor macro, calling it with types such as 'int' and 'nsInt32' will
    // actually return the same RTTI object, which would not be possible with a purely macro based solution

    return nsStaticRTTI<T>::GetRTTI();
  }

  template <typename Type>
  nsBitflags<nsTypeFlags> DetermineTypeFlags()
  {
    nsBitflags<nsTypeFlags> flags;
    nsVariantType::Enum type =
      static_cast<nsVariantType::Enum>(nsVariantTypeDeduction<typename nsTypeTraits<Type>::NonConstReferenceType>::value);
    if ((type >= nsVariantType::FirstStandardType && type <= nsVariantType::LastStandardType) || NS_IS_SAME_TYPE(nsVariant, Type))
      flags.Add(nsTypeFlags::StandardType);
    else
      flags.Add(nsTypeFlags::Class);

    if (std::is_abstract<Type>::value)
      flags.Add(nsTypeFlags::Abstract);

    return flags;
  }

  template <>
  NS_ALWAYS_INLINE nsBitflags<nsTypeFlags> DetermineTypeFlags<nsVariant>()
  {
    return nsTypeFlags::StandardType;
  }

  template <typename T>
  struct nsStaticRTTIWrapper
  {
    static_assert(sizeof(T) == 0, "Type has not been declared as reflectable (use NS_DECLARE_REFLECTABLE_TYPE macro)");
  };
} // namespace nsInternal

/// \brief Use this function, specialized with the type that you are interested in, to get the static RTTI data for some type.
template <typename T>
NS_ALWAYS_INLINE const nsRTTI* nsGetStaticRTTI()
{
  return nsInternal::GetStaticRTTI<T>(nsTraitInt<NS_IS_DERIVED_FROM_STATIC(nsReflectedClass, T)>());
}

// **************************************************
// ***** Macros for declaring types reflectable *****

#define NS_NO_LINKAGE

/// \brief Declares a type to be statically reflectable. Insert this into the header of a type to enable reflection on it.
/// This is not needed if the type is already dynamically reflectable.
#define NS_DECLARE_REFLECTABLE_TYPE(Linkage, TYPE)                    \
  namespace nsInternal                                                \
  {                                                                   \
    template <>                                                       \
    struct Linkage nsStaticRTTIWrapper<TYPE>                          \
    {                                                                 \
      static nsRTTI s_RTTI;                                           \
    };                                                                \
                                                                      \
    /* This specialization calls the function to get the RTTI data */ \
    /* This code might get duplicated in different DLLs, but all   */ \
    /* will call the same function, so the RTTI object is unique   */ \
    template <>                                                       \
    struct nsStaticRTTI<TYPE>                                         \
    {                                                                 \
      NS_ALWAYS_INLINE static const nsRTTI* GetRTTI()                 \
      {                                                               \
        return &nsStaticRTTIWrapper<TYPE>::s_RTTI;                    \
      }                                                               \
    };                                                                \
  }

/// \brief Insert this into a class/struct to enable properties that are private members.
/// All types that have dynamic reflection (\see NS_ADD_DYNAMIC_REFLECTION) already have this ability.
#define NS_ALLOW_PRIVATE_PROPERTIES(SELF) friend nsRTTI GetRTTI(SELF*)

/// \cond
// internal helper macro
#define NS_RTTIINFO_DECL(Type, BaseType, Version) \
                                                  \
  nsStringView GetTypeName(Type*)                 \
  {                                               \
    return #Type;                                 \
  }                                               \
  nsUInt32 GetTypeVersion(Type*)                  \
  {                                               \
    return Version;                               \
  }                                               \
                                                  \
  nsRTTI GetRTTI(Type*);

// internal helper macro
#define NS_RTTIINFO_GETRTTI_IMPL_BEGIN(Type, BaseType, AllocatorType)              \
  nsRTTI GetRTTI(Type*)                                                            \
  {                                                                                \
    using OwnType = Type;                                                          \
    using OwnBaseType = BaseType;                                                  \
    static AllocatorType Allocator;                                                \
    static nsBitflags<nsTypeFlags> flags = nsInternal::DetermineTypeFlags<Type>(); \
    static nsArrayPtr<const nsAbstractProperty*> Properties;                       \
    static nsArrayPtr<const nsAbstractFunctionProperty*> Functions;                \
    static nsArrayPtr<const nsPropertyAttribute*> Attributes;                      \
    static nsArrayPtr<nsAbstractMessageHandler*> MessageHandlers;                  \
    static nsArrayPtr<nsMessageSenderInfo> MessageSenders;

/// \endcond

/// \brief Implements the necessary functionality for a type to be statically reflectable.
///
/// \param Type
///   The type for which the reflection functionality should be implemented.
/// \param BaseType
///   The base class type of \a Type. If it has no base class, pass nsNoBase
/// \param Version
///   The version of \a Type. Must be increased when the class serialization changes.
/// \param AllocatorType
///   The type of an nsRTTIAllocator that can be used to create and destroy instances
///   of \a Type. Pass nsRTTINoAllocator for types that should not be created dynamically.
///   Pass nsRTTIDefaultAllocator<Type> for types that should be created on the default heap.
///   Pass a custom nsRTTIAllocator type to handle allocation differently.
#define NS_BEGIN_STATIC_REFLECTED_TYPE(Type, BaseType, Version, AllocatorType) \
  NS_RTTIINFO_DECL(Type, BaseType, Version)                                    \
  nsRTTI nsInternal::nsStaticRTTIWrapper<Type>::s_RTTI = GetRTTI((Type*)0);    \
  NS_RTTIINFO_GETRTTI_IMPL_BEGIN(Type, BaseType, AllocatorType)


/// \brief Ends the reflection code block that was opened with NS_BEGIN_STATIC_REFLECTED_TYPE.
#define NS_END_STATIC_REFLECTED_TYPE                                                                                                         \
  ;                                                                                                                                          \
  return nsRTTI(GetTypeName((OwnType*)0), nsGetStaticRTTI<OwnBaseType>(), sizeof(OwnType), GetTypeVersion((OwnType*)0),                      \
    nsVariantTypeDeduction<OwnType>::value, flags, &Allocator, Properties, Functions, Attributes, MessageHandlers, MessageSenders, nullptr); \
  }


/// \brief Within a NS_BEGIN_REFLECTED_TYPE / NS_END_REFLECTED_TYPE block, use this to start the block that declares all the properties.
#define NS_BEGIN_PROPERTIES static const nsAbstractProperty* PropertyList[] =



/// \brief Ends the block to declare properties that was started with NS_BEGIN_PROPERTIES.
#define NS_END_PROPERTIES \
  ;                       \
  Properties = PropertyList

/// \brief Within a NS_BEGIN_REFLECTED_TYPE / NS_END_REFLECTED_TYPE block, use this to start the block that declares all the functions.
#define NS_BEGIN_FUNCTIONS static const nsAbstractFunctionProperty* FunctionList[] =



/// \brief Ends the block to declare functions that was started with NS_BEGIN_FUNCTIONS.
#define NS_END_FUNCTIONS \
  ;                      \
  Functions = FunctionList

/// \brief Within a NS_BEGIN_REFLECTED_TYPE / NS_END_REFLECTED_TYPE block, use this to start the block that declares all the attributes.
#define NS_BEGIN_ATTRIBUTES static const nsPropertyAttribute* AttributeList[] =



/// \brief Ends the block to declare attributes that was started with NS_BEGIN_ATTRIBUTES.
#define NS_END_ATTRIBUTES \
  ;                       \
  Attributes = AttributeList

/// \brief Within a NS_BEGIN_FUNCTIONS / NS_END_FUNCTIONS; block, this adds a member or static function property stored inside the RTTI
/// data.
///
/// \param Function
///   The function to be executed, must match the C++ function name.
#define NS_FUNCTION_PROPERTY(Function) (new nsFunctionProperty<decltype(&OwnType::Function)>(NS_STRINGIZE(Function), &OwnType::Function))

/// \brief Within a NS_BEGIN_FUNCTIONS / NS_END_FUNCTIONS; block, this adds a member or static function property stored inside the RTTI
/// data. Use this version if you need to change the name of the function or need to cast the function to one of its overload versions.
///
/// \param PropertyName
///   The name under which the property should be registered.
///
/// \param Function
///   The function to be executed, must match the C++ function name including the class name e.g. 'CLASS::NAME'.
#define NS_FUNCTION_PROPERTY_EX(PropertyName, Function) (new nsFunctionProperty<decltype(&Function)>(PropertyName, &Function))

/// \internal Used by NS_SCRIPT_FUNCTION_PROPERTY
#define _NS_SCRIPT_FUNCTION_PARAM(type, name) nsScriptableFunctionAttribute::ArgType::type, name

/// \brief Convenience macro to declare a function that can be called from scripts.
///
/// \param Function
///   The function to be executed, must match the C++ function name including the class name e.g. 'CLASS::NAME'.
///
/// Internally this calls NS_FUNCTION_PROPERTY and adds a nsScriptableFunctionAttribute.
/// Use the variadic arguments in pairs to configure how each function parameter gets exposed.
///   Use 'In', 'Out' or 'Inout' to specify whether a function parameter is only read, or also written back to.
///   Follow it with a string to specify the name under which the parameter should show up.
///
/// Example:
///   NS_SCRIPT_FUNCTION_PROPERTY(MyFunc1NoParams)
///   NS_SCRIPT_FUNCTION_PROPERTY(MyFunc2FloatInDoubleOut, In, "FloatValue", Out, "DoubleResult")
#define NS_SCRIPT_FUNCTION_PROPERTY(Function, ...) \
  NS_FUNCTION_PROPERTY(Function)->AddAttributes(new nsScriptableFunctionAttribute(NS_EXPAND_ARGS_PAIR_COMMA(_NS_SCRIPT_FUNCTION_PARAM, ##__VA_ARGS__)))

/// \brief Within a NS_BEGIN_FUNCTIONS / NS_END_FUNCTIONS; block, this adds a constructor function property stored inside the RTTI data.
///
/// \param Function
///   The function to be executed in the form of CLASS::FUNCTION_NAME.
#define NS_CONSTRUCTOR_PROPERTY(...) (new nsConstructorFunctionProperty<OwnType, ##__VA_ARGS__>())


// [internal] Helper macro to get the return type of a getter function.
#define NS_GETTER_TYPE(Class, GetterFunc) decltype(std::declval<Class>().GetterFunc())

/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that uses custom getter / setter functions.
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
#define NS_ACCESSOR_PROPERTY(PropertyName, Getter, Setter) \
  (new nsAccessorProperty<OwnType, NS_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, &OwnType::Setter))

/// \brief Same as NS_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define NS_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, Getter) \
  (new nsAccessorProperty<OwnType, NS_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, nullptr))

// [internal] Helper macro to get the return type of a array getter function.
#define NS_ARRAY_GETTER_TYPE(Class, GetterFunc) decltype(std::declval<Class>().GetterFunc(0))

/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that uses custom functions to access an array.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param GetCount
///   Function signature: nsUInt32 GetCount() const;
/// \param Getter
///   Function signature: Type GetValue(nsUInt32 uiIndex) const;
/// \param Setter
///   Function signature: void SetValue(nsUInt32 uiIndex, Type value);
/// \param Insert
///   Function signature: void Insert(nsUInt32 uiIndex, Type value);
/// \param Remove
///   Function signature: void Remove(nsUInt32 uiIndex);
#define NS_ARRAY_ACCESSOR_PROPERTY(PropertyName, GetCount, Getter, Setter, Insert, Remove) \
  (new nsAccessorArrayProperty<OwnType, NS_ARRAY_GETTER_TYPE(OwnType, OwnType::Getter)>(   \
    PropertyName, &OwnType::GetCount, &OwnType::Getter, &OwnType::Setter, &OwnType::Insert, &OwnType::Remove))

/// \brief Same as NS_ARRAY_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define NS_ARRAY_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, GetCount, Getter)             \
  (new nsAccessorArrayProperty<OwnType, NS_ARRAY_GETTER_TYPE(OwnType, OwnType::Getter)>( \
    PropertyName, &OwnType::GetCount, &OwnType::Getter, nullptr, nullptr, nullptr))

#define NS_SET_CONTAINER_TYPE(Class, GetterFunc) decltype(std::declval<Class>().GetterFunc())

#define NS_SET_CONTAINER_SUB_TYPE(Class, GetterFunc) \
  nsContainerSubTypeResolver<nsTypeTraits<decltype(std::declval<Class>().GetterFunc())>::NonConstReferenceType>::Type

/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that uses custom functions to access a set.
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
#define NS_SET_ACCESSOR_PROPERTY(PropertyName, GetValues, Insert, Remove)                                            \
  (new nsAccessorSetProperty<OwnType, nsFunctionParameterTypeResolver<0, decltype(&OwnType::Insert)>::ParameterType, \
    NS_SET_CONTAINER_TYPE(OwnType, GetValues)>(PropertyName, &OwnType::GetValues, &OwnType::Insert, &OwnType::Remove))

/// \brief Same as NS_SET_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define NS_SET_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, GetValues)                                                              \
  (new nsAccessorSetProperty<OwnType, NS_SET_CONTAINER_SUB_TYPE(OwnType, GetValues), NS_SET_CONTAINER_TYPE(OwnType, GetValues)>( \
    PropertyName, &OwnType::GetValues, nullptr, nullptr))

/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that uses custom functions to for write access to a
/// map.
///   Use this if you have a nsHashTable or nsMap to expose directly and just want to be informed of write operations.
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
/// \note Container can be nsMap or nsHashTable
#define NS_MAP_WRITE_ACCESSOR_PROPERTY(PropertyName, GetContainer, Insert, Remove)                                        \
  (new nsWriteAccessorMapProperty<OwnType, nsFunctionParameterTypeResolver<1, decltype(&OwnType::Insert)>::ParameterType, \
    NS_SET_CONTAINER_TYPE(OwnType, GetContainer)>(PropertyName, &OwnType::GetContainer, &OwnType::Insert, &OwnType::Remove))

/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that uses custom functions to access a map.
///   Use this if you you want to hide the implementation details of the map from the user.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param GetKeyRange
///   Function signature: const Range GetValues() const;
///   Range has to be an object that a ranged based for-loop can iterate over containing the keys
///   implicitly convertible to Type / nsString.
/// \param GetValue
///   Function signature: bool GetValue(const char* szKey, Type& value) const;
///   Returns whether the the key existed. value must be a non const ref as it is written to.
/// \param Insert
///   Function signature: void Insert(const char* szKey, Type value);
///   value can also be const and/or a reference.
/// \param Remove
///   Function signature: void Remove(const char* szKey);
///
/// \note Container can be nsMap or nsHashTable
#define NS_MAP_ACCESSOR_PROPERTY(PropertyName, GetKeyRange, GetValue, Insert, Remove)                                \
  (new nsAccessorMapProperty<OwnType, nsFunctionParameterTypeResolver<1, decltype(&OwnType::Insert)>::ParameterType, \
    NS_SET_CONTAINER_TYPE(OwnType, GetKeyRange)>(PropertyName, &OwnType::GetKeyRange, &OwnType::GetValue, &OwnType::Insert, &OwnType::Remove))

/// \brief Same as NS_MAP_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define NS_MAP_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, GetKeyRange, GetValue)                                           \
  (new nsAccessorMapProperty<OwnType,                                                                                     \
    nsTypeTraits<nsFunctionParameterTypeResolver<1, decltype(&OwnType::GetValue)>::ParameterType>::NonConstReferenceType, \
    NS_SET_CONTAINER_TYPE(OwnType, GetKeyRange)>(PropertyName, &OwnType::GetKeyRange, &OwnType::GetValue, nullptr, nullptr))



/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that uses custom getter / setter functions.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param EnumType
///   The name of the enum struct used by nsEnum.
/// \param Getter
///   The getter function for this property.
/// \param Setter
///   The setter function for this property.
#define NS_ENUM_ACCESSOR_PROPERTY(PropertyName, EnumType, Getter, Setter) \
  (new nsEnumAccessorProperty<OwnType, EnumType, NS_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, &OwnType::Setter))

/// \brief Same as NS_ENUM_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define NS_ENUM_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, EnumType, Getter) \
  (new nsEnumAccessorProperty<OwnType, EnumType, NS_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, nullptr))

/// \brief Same as NS_ENUM_ACCESSOR_PROPERTY, but for bitfields.
#define NS_BITFLAGS_ACCESSOR_PROPERTY(PropertyName, BitflagsType, Getter, Setter) \
  (new nsBitflagsAccessorProperty<OwnType, BitflagsType, NS_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, &OwnType::Setter))

/// \brief Same as NS_BITFLAGS_ACCESSOR_PROPERTY, but no setter is provided, thus making the property read-only.
#define NS_BITFLAGS_ACCESSOR_PROPERTY_READ_ONLY(PropertyName, BitflagsType, Getter) \
  (new nsBitflagsAccessorProperty<OwnType, BitflagsType, NS_GETTER_TYPE(OwnType, OwnType::Getter)>(PropertyName, &OwnType::Getter, nullptr))


// [internal] Helper macro to get the type of a class member.
#define NS_MEMBER_TYPE(Class, Member) decltype(std::declval<Class>().Member)

#define NS_MEMBER_CONTAINER_SUB_TYPE(Class, Member) \
  nsContainerSubTypeResolver<nsTypeTraits<decltype(std::declval<Class>().Member)>::NonConstReferenceType>::Type

/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that actually exists as a member.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param MemberName
///   The name of the member variable that should get exposed as a property.
///
/// \note Since the member is exposed directly, there is no way to know when the variable was modified. That also means
/// no custom limits to the values can be applied. If that becomes necessary, just add getter / setter functions and
/// expose the property as a NS_ENUM_ACCESSOR_PROPERTY instead.
#define NS_MEMBER_PROPERTY(PropertyName, MemberName)                                                   \
  (new nsMemberProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,                    \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::SetValue, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as NS_MEMBER_PROPERTY, but the property is read-only.
#define NS_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                  \
  (new nsMemberProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,                             \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, nullptr, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as NS_MEMBER_PROPERTY, but the property is an array (nsHybridArray, nsDynamicArray or nsDeque).
#define NS_ARRAY_MEMBER_PROPERTY(PropertyName, MemberName)                                                                                  \
  (new nsMemberArrayProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), NS_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>(PropertyName, \
    &nsArrayPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer,                        \
    &nsArrayPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetContainer))

/// \brief Same as NS_MEMBER_PROPERTY, but the property is a read-only array (nsArrayPtr, nsHybridArray, nsDynamicArray or nsDeque).
#define NS_ARRAY_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                                   \
  (new nsMemberArrayReadOnlyProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), NS_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>( \
    PropertyName, &nsArrayPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer))

/// \brief Same as NS_MEMBER_PROPERTY, but the property is a set (nsSet, nsHashSet).
#define NS_SET_MEMBER_PROPERTY(PropertyName, MemberName)                                                                                  \
  (new nsMemberSetProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), NS_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>(PropertyName, \
    &nsSetPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer,                        \
    &nsSetPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetContainer))

/// \brief Same as NS_MEMBER_PROPERTY, but the property is a read-only set (nsSet, nsHashSet).
#define NS_SET_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                           \
  (new nsMemberSetProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), NS_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>( \
    PropertyName, &nsSetPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer, nullptr))

/// \brief Same as NS_MEMBER_PROPERTY, but the property is a map (nsMap, nsHashTable).
#define NS_MAP_MEMBER_PROPERTY(PropertyName, MemberName)                                                                                  \
  (new nsMemberMapProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), NS_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>(PropertyName, \
    &nsMapPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer,                        \
    &nsMapPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetContainer))

/// \brief Same as NS_MEMBER_PROPERTY, but the property is a read-only map (nsMap, nsHashTable).
#define NS_MAP_MEMBER_PROPERTY_READ_ONLY(PropertyName, MemberName)                                                           \
  (new nsMemberMapProperty<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), NS_MEMBER_CONTAINER_SUB_TYPE(OwnType, MemberName)>( \
    PropertyName, &nsMapPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetConstContainer, nullptr))

/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a property that actually exists as a member.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param EnumType
///   Name of the struct used by nsEnum.
/// \param MemberName
///   The name of the member variable that should get exposed as a property.
///
/// \note Since the member is exposed directly, there is no way to know when the variable was modified. That also means
/// no custom limits to the values can be applied. If that becomes necessary, just add getter / setter functions and
/// expose the property as a NS_ACCESSOR_PROPERTY instead.
#define NS_ENUM_MEMBER_PROPERTY(PropertyName, EnumType, MemberName)                                    \
  (new nsEnumMemberProperty<OwnType, EnumType, NS_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,      \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::SetValue, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as NS_ENUM_MEMBER_PROPERTY, but the property is read-only.
#define NS_ENUM_MEMBER_PROPERTY_READ_ONLY(PropertyName, EnumType, MemberName)                                   \
  (new nsEnumMemberProperty<OwnType, EnumType, NS_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,               \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, nullptr, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as NS_ENUM_MEMBER_PROPERTY, but for bitfields.
#define NS_BITFLAGS_MEMBER_PROPERTY(PropertyName, BitflagsType, MemberName)                               \
  (new nsBitflagsMemberProperty<OwnType, BitflagsType, NS_MEMBER_TYPE(OwnType, MemberName)>(PropertyName, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue,    \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::SetValue,    \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))

/// \brief Same as NS_ENUM_MEMBER_PROPERTY_READ_ONLY, but for bitfields.
#define NS_BITFLAGS_MEMBER_PROPERTY_READ_ONLY(PropertyName, BitflagsType, MemberName)                           \
  (new nsBitflagsMemberProperty<OwnType, BitflagsType, NS_MEMBER_TYPE(OwnType, MemberName)>(PropertyName,       \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetValue, nullptr, \
    &nsPropertyAccessor<OwnType, NS_MEMBER_TYPE(OwnType, MemberName), &OwnType::MemberName>::GetPropertyPointer))



/// \brief Within a NS_BEGIN_PROPERTIES / NS_END_PROPERTIES; block, this adds a constant property stored inside the RTTI data.
///
/// \param PropertyName
///   The unique (in this class) name under which the property should be registered.
/// \param Value
///   The constant value to be stored.
#define NS_CONSTANT_PROPERTY(PropertyName, Value) (new nsConstantProperty<decltype(Value)>(PropertyName, Value))



// [internal] Helper macro
#define NS_ENUM_VALUE_TO_CONSTANT_PROPERTY(name) NS_CONSTANT_PROPERTY(NS_STRINGIZE(name), (Storage)name),

/// \brief Within a NS_BEGIN_STATIC_REFLECTED_ENUM / NS_END_STATIC_REFLECTED_ENUM block, this converts a
/// list of enum values into constant RTTI properties.
#define NS_ENUM_CONSTANTS(...) NS_EXPAND_ARGS(NS_ENUM_VALUE_TO_CONSTANT_PROPERTY, ##__VA_ARGS__)

/// \brief Within a NS_BEGIN_STATIC_REFLECTED_ENUM / NS_END_STATIC_REFLECTED_ENUM block, this converts a
/// an enum value into a constant RTTI property.
#define NS_ENUM_CONSTANT(Value) NS_CONSTANT_PROPERTY(NS_STRINGIZE(Value), (Storage)Value)

/// \brief Within a NS_BEGIN_STATIC_REFLECTED_BITFLAGS / NS_END_STATIC_REFLECTED_BITFLAGS block, this converts a
/// list of bitflags into constant RTTI properties.
#define NS_BITFLAGS_CONSTANTS(...) NS_EXPAND_ARGS(NS_ENUM_VALUE_TO_CONSTANT_PROPERTY, ##__VA_ARGS__)

/// \brief Within a NS_BEGIN_STATIC_REFLECTED_BITFLAGS / NS_END_STATIC_REFLECTED_BITFLAGS block, this converts a
/// an bitflags into a constant RTTI property.
#define NS_BITFLAGS_CONSTANT(Value) NS_CONSTANT_PROPERTY(NS_STRINGIZE(Value), (Storage)Value)



/// \brief Implements the necessary functionality for an enum to be statically reflectable.
///
/// \param Type
///   The enum struct used by nsEnum for which reflection should be defined.
/// \param Version
///   The version of \a Type. Must be increased when the class changes.
#define NS_BEGIN_STATIC_REFLECTED_ENUM(Type, Version)                          \
  NS_BEGIN_STATIC_REFLECTED_TYPE(Type, nsEnumBase, Version, nsRTTINoAllocator) \
    ;                                                                          \
    using Storage = Type::StorageType;                                         \
    NS_BEGIN_PROPERTIES                                                        \
      {                                                                        \
        NS_CONSTANT_PROPERTY(NS_STRINGIZE(Type::Default), (Storage)Type::Default),

#define NS_END_STATIC_REFLECTED_ENUM \
  }                                  \
  NS_END_PROPERTIES                  \
  ;                                  \
  flags |= nsTypeFlags::IsEnum;      \
  flags.Remove(nsTypeFlags::Class);  \
  NS_END_STATIC_REFLECTED_TYPE


/// \brief Implements the necessary functionality for bitflags to be statically reflectable.
///
/// \param Type
///   The bitflags struct used by nsBitflags for which reflection should be defined.
/// \param Version
///   The version of \a Type. Must be increased when the class changes.
#define NS_BEGIN_STATIC_REFLECTED_BITFLAGS(Type, Version)                          \
  NS_BEGIN_STATIC_REFLECTED_TYPE(Type, nsBitflagsBase, Version, nsRTTINoAllocator) \
    ;                                                                              \
    using Storage = Type::StorageType;                                             \
    NS_BEGIN_PROPERTIES                                                            \
      {                                                                            \
        NS_CONSTANT_PROPERTY(NS_STRINGIZE(Type::Default), (Storage)Type::Default),

#define NS_END_STATIC_REFLECTED_BITFLAGS \
  }                                      \
  NS_END_PROPERTIES                      \
  ;                                      \
  flags |= nsTypeFlags::Bitflags;        \
  flags.Remove(nsTypeFlags::Class);      \
  NS_END_STATIC_REFLECTED_TYPE



/// \brief Within an NS_BEGIN_REFLECTED_TYPE / NS_END_REFLECTED_TYPE block, use this to start the block that declares all the message
/// handlers.
#define NS_BEGIN_MESSAGEHANDLERS static nsAbstractMessageHandler* HandlerList[] =


/// \brief Ends the block to declare message handlers that was started with NS_BEGIN_MESSAGEHANDLERS.
#define NS_END_MESSAGEHANDLERS \
  ;                            \
  MessageHandlers = HandlerList


/// \brief Within an NS_BEGIN_MESSAGEHANDLERS / NS_END_MESSAGEHANDLERS; block, this adds another message handler.
///
/// \param MessageType
///   The type of message that this handler function accepts. You may add 'const' in front of it.
/// \param FunctionName
///   The actual C++ name of the message handler function.
///
/// \note A message handler is a function that takes one parameter of type nsMessage (or a derived type) and returns void.
#define NS_MESSAGE_HANDLER(MessageType, FunctionName)                                                                                   \
  new nsInternal::MessageHandler<NS_IS_CONST_MESSAGE_HANDLER(OwnType, MessageType, &OwnType::FunctionName)>::Impl<OwnType, MessageType, \
    &OwnType::FunctionName>()


/// \brief Within an NS_BEGIN_REFLECTED_TYPE / NS_END_REFLECTED_TYPE block, use this to start the block that declares all the message
/// senders.
#define NS_BEGIN_MESSAGESENDERS static nsMessageSenderInfo SenderList[] =


/// \brief Ends the block to declare message senders that was started with NS_BEGIN_MESSAGESENDERS.
#define NS_END_MESSAGESENDERS \
  ;                           \
  MessageSenders = SenderList;

/// \brief Within an NS_BEGIN_MESSAGESENDERS / NS_END_MESSAGESENDERS block, this adds another message sender.
///
/// \param MemberName
///   The name of the member variable that should get exposed as a message sender.
///
/// \note A message sender must be derived from nsMessageSenderBase.
#define NS_MESSAGE_SENDER(MemberName)                                                \
  {                                                                                  \
    #MemberName, nsGetStaticRTTI<NS_MEMBER_TYPE(OwnType, MemberName)::MessageType>() \
  }
