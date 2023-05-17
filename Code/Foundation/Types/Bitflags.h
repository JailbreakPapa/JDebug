#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Types/Enum.h>

/// \brief The wdBitflags class allows you to work with type-safe bitflags.
///
/// wdBitflags takes a struct as its template parameter, which contains an enum for the available flag values.
/// wdBitflags wraps this type in a way which enables the compiler to do type-checks. This makes it very easy
/// to document and enforce what flags are to be used in an interface.
/// For example, in traditional C++ code, you usually need to have an integer as a function parameter type,
/// when that parameter is supposed to take flags. However, WHICH flags (e.g. from which enum) cannot be enforced
/// through compile time checks. Thus it is difficult for the user to see whether he used the correct type, and
/// it is impossible for the compiler to help find such bugs.
/// wdBitflags solves this problem. However the flag type used to instantiate wdBitflags must fulfill some requirements.
///
/// There are two ways to define your bitflags type, that can be used with wdBitflags.
///
/// The easier, less powerful way: Use the WD_DECLARE_FLAGS() macro.\n
/// Example:\n
/// \code{.cpp}
///   WD_DECLARE_FLAGS(wdUInt8, SimpleRenderFlags, EnableEffects, EnableLighting, EnableShadows);
/// \endcode
/// This will declare a type 'SimpleRenderFlags' which contains three different flags.
/// You can then create a function which takes flags like this:
/// \code{.cpp}
///   void RenderScene(wdBitflags<SimpleRenderFlags> Flags);
/// \endcode
/// And this function can be called like this:\n
/// \code{.cpp}
///   RenderScene(EnableEffects | EnableLighting | EnableShadows);
/// \endcode
/// However it will refuse to compile with anything else, for example this will not work:\n
/// \code{.cpp}
///   RenderScene(1);
/// \endcode
///
/// The second way to declare your bitflags type allows even more flexibility. Here you need to declare your bitflag type manually:
/// \code{.cpp}
///   struct SimpleRenderFlags
///   {
///     typedef wdUInt32 StorageType;
///
///     enum Enum
///     {
///       EnableEffects   = WD_BIT(0),
///       EnableLighting  = WD_BIT(1),
///       EnableShadows   = WD_BIT(2),
///       FullLighting    = EnableLighting | EnableShadows,
///       AllFeatures     = FullLighting | EnableEffects,
///       Default = AllFeatures
///     };
///
///     struct Bits
///     {
///       StorageType EnableEffects   : 1;
///       StorageType EnableLighting  : 1;
///       StorageType EnableShadows   : 1;
///     };
///   };
///
///   WD_DECLARE_FLAGS_OPERATORS(SimpleRenderFlags);
/// \endcode
///
/// Here we declare a struct which contains our enum that contains all the flags that we want to have. This enum can contain
/// flags that are combinations of other flags. Note also the 'Default' flag, which is mandatory.\n
/// The 'Bits' struct enables debuggers to show exactly which flags are enabled (with nice names) when you inspect an wdBitflags
/// instance. You could leave this struct empty, but then your debugger can not show helpful information about the flags anymore.
/// The Bits struct should contain one named entry for each individual bit. E.g. here only the flags 'EnableEffects', 'EnableLighting'
/// and 'EnableShadows' actually map to single bits, the other flags are combinations of those. Therefore the Bits struct only
/// specifies names for those first three Bits.\n
/// The typedef 'StorageType' is also mandatory, such that wdBitflags can access it.\n
/// Finally the macro WD_DECLARE_FLAGS_OPERATORS will define the required operator to be able to combine bitflags of your type.
/// I.e. it enables to write wdBitflags<SimpleRenderFlags> f = EnableEffects | EnableLighting;\n
///
/// For a real world usage example, see wdCVarFlags.
template <typename T>
struct wdBitflags
{
private:
  typedef typename T::Enum Enum;
  using Bits = typename T::Bits;
  using StorageType = typename T::StorageType;

public:
  /// \brief Constructor. Initializes the flags to the default value.
  WD_ALWAYS_INLINE wdBitflags()
    : m_Value(T::Default) // [tested]
  {
  }

  /// \brief Converts the incoming type to wdBitflags<T>
  WD_ALWAYS_INLINE wdBitflags(Enum flag1) // [tested]
  {
    m_Value = (StorageType)flag1;
  }

  WD_ALWAYS_INLINE void operator=(Enum flag1) { m_Value = (StorageType)flag1; }

  /// \brief Comparison operator.
  WD_ALWAYS_INLINE bool operator==(const StorageType rhs) const // [tested]
  {
    return m_Value == rhs;
  }

  /// \brief Comparison operator.
  WD_ALWAYS_INLINE bool operator!=(const StorageType rhs) const { return m_Value != rhs; }

  /// \brief Comparison operator.
  WD_ALWAYS_INLINE bool operator==(const wdBitflags<T>& rhs) const { return m_Value == rhs.m_Value; }

  /// \brief Comparison operator.
  WD_ALWAYS_INLINE bool operator!=(const wdBitflags<T>& rhs) const { return m_Value != rhs.m_Value; }

  /// \brief Clears all flags
  WD_ALWAYS_INLINE void Clear() // [tested]
  {
    m_Value = 0;
  }

  /// \brief Checks if certain flags are set within the bitfield.
  WD_ALWAYS_INLINE bool IsSet(Enum flag) const // [tested]
  {
    return (m_Value & flag) != 0;
  }

  /// \brief Returns whether all the given flags are set.
  WD_ALWAYS_INLINE bool AreAllSet(const wdBitflags<T>& rhs) const // [tested]
  {
    return (m_Value & rhs.m_Value) == rhs.m_Value;
  }

  /// \brief Returns whether none of the given flags is set.
  WD_ALWAYS_INLINE bool AreNoneSet(const wdBitflags<T>& rhs) const // [tested]
  {
    return (m_Value & rhs.m_Value) == 0;
  }

  /// \brief  Returns whether any of the given flags is set.
  WD_ALWAYS_INLINE bool IsAnySet(const wdBitflags<T>& rhs) const // [tested]
  {
    return (m_Value & rhs.m_Value) != 0;
  }

  /// \brief Sets the given flag.
  WD_ALWAYS_INLINE void Add(const wdBitflags<T>& rhs) // [tested]
  {
    m_Value |= rhs.m_Value;
  }

  /// \brief Removes the given flag.
  WD_ALWAYS_INLINE void Remove(const wdBitflags<T>& rhs) // [tested]
  {
    m_Value &= (~rhs.m_Value);
  }

  /// \brief Toggles the state of the given flag.
  WD_ALWAYS_INLINE void Toggle(const wdBitflags<T>& rhs) // [tested]
  {
    m_Value ^= rhs.m_Value;
  }

  /// \brief Sets or clears the given flag.
  WD_ALWAYS_INLINE void AddOrRemove(const wdBitflags<T>& rhs, bool bState) // [tested]
  {
    m_Value = (bState) ? m_Value | rhs.m_Value : m_Value & (~rhs.m_Value);
  }

  /// \brief Returns an object that has the flags of \a this and \a rhs combined.
  WD_ALWAYS_INLINE wdBitflags<T> operator|(const wdBitflags<T>& rhs) const // [tested]
  {
    return wdBitflags<T>(m_Value | rhs.m_Value);
  }

  /// \brief Returns an object that has the flags that were set both in \a this and \a rhs.
  WD_ALWAYS_INLINE wdBitflags<T> operator&(const wdBitflags<T>& rhs) const // [tested]
  {
    return wdBitflags<T>(m_Value & rhs.m_Value);
  }

  /// \brief Modifies \a this to also contain the bits from \a rhs.
  WD_ALWAYS_INLINE void operator|=(const wdBitflags<T>& rhs) // [tested]
  {
    m_Value |= rhs.m_Value;
  }

  /// \brief Modifies \a this to only contain the bits that were set in \a this and \a rhs.
  WD_ALWAYS_INLINE void operator&=(const wdBitflags<T>& rhs) // [tested]
  {
    m_Value &= rhs.m_Value;
  }

  /// \brief Returns the stored value as the underlying integer type.
  WD_ALWAYS_INLINE StorageType GetValue() const // [tested]
  {
    return m_Value;
  }

  /// \brief Overwrites the flags with a new value.
  WD_ALWAYS_INLINE void SetValue(StorageType value) // [tested]
  {
    m_Value = value;
  }

  /// \brief Returns true if not a single bit is set.
  WD_ALWAYS_INLINE bool IsNoFlagSet() const // [tested]
  {
    return m_Value == 0;
  }

  /// \brief Returns true if any bitflag is set.
  WD_ALWAYS_INLINE bool IsAnyFlagSet() const // [tested]
  {
    return m_Value != 0;
  }

private:
  WD_ALWAYS_INLINE explicit wdBitflags(StorageType flags)
    : m_Value(flags)
  {
  }

  union
  {
    StorageType m_Value;
    Bits m_bits;
  };
};


/// \brief This macro will define the operator| and operator& function that is required for class \a FlagsType to work with wdBitflags.
/// See class wdBitflags for more information.
#define WD_DECLARE_FLAGS_OPERATORS(FlagsType)                                      \
  inline wdBitflags<FlagsType> operator|(FlagsType::Enum lhs, FlagsType::Enum rhs) \
  {                                                                                \
    return (wdBitflags<FlagsType>(lhs) | wdBitflags<FlagsType>(rhs));              \
  }                                                                                \
                                                                                   \
  inline wdBitflags<FlagsType> operator&(FlagsType::Enum lhs, FlagsType::Enum rhs) \
  {                                                                                \
    return (wdBitflags<FlagsType>(lhs) & wdBitflags<FlagsType>(rhs));              \
  }



/// \brief This macro allows to conveniently declare a bitflag type that can be used with the wdBitflags class.
///
/// Usage: WD_DECLARE_FLAGS(wdUInt32, FlagsTypeName, Flag1Name, Flag2Name, Flag3Name, Flag4Name, ...)
///
/// This macro will define a simple type of with the name that is given as the second parameter,
/// which can be used as type-safe bitflags. Everything that is necessary to work with the wdBitflags
/// class, will be set up automatically.
/// The bitflag type will use the integer type that is given as the first parameter for its internal
/// storage. So if you pass wdUInt32 as the first parameter, your bitflag type will take up 4 bytes
/// and will support up to 32 flags. You can also pass any other integer type to adjust the required
/// storage space, if you don't need that many flags.
///
/// The third parameter and onwards declare the names of the flags that the type should contain.
/// Each flag will use a different bit. If you need to define flags that are combinations of several
/// other flags, you need to declare the bitflag struct manually. See the wdBitflags class for more
/// information on how to do that.
#define WD_DECLARE_FLAGS_WITH_DEFAULT(InternalStorageType, BitflagsTypeName, DefaultValue, ...) \
  struct BitflagsTypeName                                                                       \
  {                                                                                             \
    static const wdUInt32 Count = WD_VA_NUM_ARGS(__VA_ARGS__);                                  \
    typedef InternalStorageType StorageType;                                                    \
    enum Enum                                                                                   \
    {                                                                                           \
      WD_EXPAND_ARGS_WITH_INDEX(WD_DECLARE_FLAGS_ENUM, ##__VA_ARGS__) Default = DefaultValue    \
    };                                                                                          \
    struct Bits                                                                                 \
    {                                                                                           \
      WD_EXPAND_ARGS(WD_DECLARE_FLAGS_BITS, ##__VA_ARGS__)                                      \
    };                                                                                          \
    WD_ENUM_TO_STRING(__VA_ARGS__)                                                              \
  };                                                                                            \
  WD_DECLARE_FLAGS_OPERATORS(BitflagsTypeName)

#define WD_DECLARE_FLAGS(InternalStorageType, BitflagsTypeName, ...) \
  WD_DECLARE_FLAGS_WITH_DEFAULT(InternalStorageType, BitflagsTypeName, 0, ##__VA_ARGS__)
/// \cond

/// Internal Do not use.
#define WD_DECLARE_FLAGS_ENUM(name, n) name = WD_BIT(n),

/// Internal Do not use.
#define WD_DECLARE_FLAGS_BITS(name) StorageType name : 1;

/// \endcond

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// \brief Similar to wdBitflags but without type-safety.
///
/// This class is intended for use cases where type-safety can get in the way,
/// for example when it is intended for the user to extend a flags enum with custom flags a separate file / enum.
template <typename T>
struct wdTypelessBitflags
{
public:
  using StorageType = T;

  /// \brief Initializes the flags zero.
  WD_ALWAYS_INLINE wdTypelessBitflags() // [tested]
    : m_Value(0)
  {
  }

  WD_ALWAYS_INLINE wdTypelessBitflags(StorageType flags) // [tested]
  {
    m_Value = flags;
  }

  WD_ALWAYS_INLINE void operator=(StorageType flags) // [tested]
  {
    m_Value = flags;
  }

  WD_ALWAYS_INLINE bool operator==(const StorageType rhs) const { return m_Value == rhs; }                              // [tested]
  WD_ALWAYS_INLINE bool operator!=(const StorageType rhs) const { return m_Value != rhs; }                              // [tested]
  WD_ALWAYS_INLINE bool operator==(const wdTypelessBitflags<StorageType>& rhs) const { return m_Value == rhs.m_Value; } // [tested]
  WD_ALWAYS_INLINE bool operator!=(const wdTypelessBitflags<StorageType>& rhs) const { return m_Value != rhs.m_Value; } // [tested]

  /// \brief Clears all flags
  WD_ALWAYS_INLINE void Clear() { m_Value = 0; } // [tested]

  /// \brief Returns whether all the given flags are set.
  WD_ALWAYS_INLINE bool AreAllSet(const wdTypelessBitflags<StorageType>& rhs) const // [tested]
  {
    return (m_Value & rhs.m_Value) == rhs.m_Value;
  }

  /// \brief Returns whether none of the given flags is set.
  WD_ALWAYS_INLINE bool AreNoneSet(const wdTypelessBitflags<StorageType>& rhs) const // [tested]
  {
    return (m_Value & rhs.m_Value) == 0;
  }

  /// \brief  Returns whether any of the given flags is set.
  WD_ALWAYS_INLINE bool IsAnySet(const wdTypelessBitflags<StorageType>& rhs) const // [tested]
  {
    return (m_Value & rhs.m_Value) != 0;
  }

  /// \brief Sets the given flag.
  WD_ALWAYS_INLINE void Add(const wdTypelessBitflags<StorageType>& rhs) // [tested]
  {
    m_Value |= rhs.m_Value;
  }

  /// \brief Removes the given flag.
  WD_ALWAYS_INLINE void Remove(const wdTypelessBitflags<StorageType>& rhs) // [tested]
  {
    m_Value &= (~rhs.m_Value);
  }

  /// \brief Toggles the state of the given flag.
  WD_ALWAYS_INLINE void Toggle(const wdTypelessBitflags<StorageType>& rhs) // [tested]
  {
    m_Value ^= rhs.m_Value;
  }

  /// \brief Sets or clears the given flag.
  WD_ALWAYS_INLINE void AddOrRemove(const wdTypelessBitflags<StorageType>& rhs, bool bState) // [tested]
  {
    m_Value = (bState) ? m_Value | rhs.m_Value : m_Value & (~rhs.m_Value);
  }

  /// \brief Returns an object that has the flags of \a this and \a rhs combined.
  WD_ALWAYS_INLINE wdTypelessBitflags<StorageType> operator|(const wdTypelessBitflags<StorageType>& rhs) const // [tested]
  {
    return wdTypelessBitflags<StorageType>(m_Value | rhs.m_Value);
  }

  /// \brief Returns an object that has the flags that were set both in \a this and \a rhs.
  WD_ALWAYS_INLINE wdTypelessBitflags<StorageType> operator&(const wdTypelessBitflags<StorageType>& rhs) const // [tested]
  {
    return wdTypelessBitflags<StorageType>(m_Value & rhs.m_Value);
  }

  /// \brief Modifies \a this to also contain the bits from \a rhs.
  WD_ALWAYS_INLINE void operator|=(const wdTypelessBitflags<StorageType>& rhs) // [tested]
  {
    m_Value |= rhs.m_Value;
  }

  /// \brief Modifies \a this to only contain the bits that were set in \a this and \a rhs.
  WD_ALWAYS_INLINE void operator&=(const wdTypelessBitflags<StorageType>& rhs) // [tested]
  {
    m_Value &= rhs.m_Value;
  }

  /// \brief Returns the stored value as the underlying integer type.
  WD_ALWAYS_INLINE StorageType GetValue() const // [tested]
  {
    return m_Value;
  }

  /// \brief Overwrites the flags with a new value.
  WD_ALWAYS_INLINE void SetValue(StorageType value) // [tested]
  {
    m_Value = value;
  }

  /// \brief Returns true if not a single bit is set.
  WD_ALWAYS_INLINE bool IsNoFlagSet() const // [tested]
  {
    return m_Value == 0;
  }

  /// \brief Returns true if any bitflag is set.
  WD_ALWAYS_INLINE bool IsAnyFlagSet() const // [tested]
  {
    return m_Value != 0;
  }

private:
  StorageType m_Value;
};
