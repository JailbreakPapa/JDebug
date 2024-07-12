#pragma once

// ***** Definition of types *****

#include <cstdint>

using nsUInt8 = uint8_t;
using nsUInt16 = uint16_t;
using nsUInt32 = uint32_t;
using nsUInt64 = unsigned long long;

using nsInt8 = int8_t;
using nsInt16 = int16_t;
using nsInt32 = int32_t;
using nsInt64 = long long;

// no float-types, since those are well portable

// Do some compile-time checks on the types
NS_CHECK_AT_COMPILETIME(sizeof(bool) == 1);
NS_CHECK_AT_COMPILETIME(sizeof(char) == 1);
NS_CHECK_AT_COMPILETIME(sizeof(float) == 4);
NS_CHECK_AT_COMPILETIME(sizeof(double) == 8);
NS_CHECK_AT_COMPILETIME(sizeof(nsInt8) == 1);
NS_CHECK_AT_COMPILETIME(sizeof(nsInt16) == 2);
NS_CHECK_AT_COMPILETIME(sizeof(nsInt32) == 4);
NS_CHECK_AT_COMPILETIME(sizeof(nsInt64) == 8); // must be defined in the specific compiler header
NS_CHECK_AT_COMPILETIME(sizeof(nsUInt8) == 1);
NS_CHECK_AT_COMPILETIME(sizeof(nsUInt16) == 2);
NS_CHECK_AT_COMPILETIME(sizeof(nsUInt32) == 4);
NS_CHECK_AT_COMPILETIME(sizeof(nsUInt64) == 8); // must be defined in the specific compiler header
NS_CHECK_AT_COMPILETIME(sizeof(long long int) == 8);

#if NS_ENABLED(NS_PLATFORM_64BIT)
#  define NS_ALIGNMENT_MINIMUM 8
#elif NS_ENABLED(NS_PLATFORM_32BIT)
#  define NS_ALIGNMENT_MINIMUM 4
#else
#  error "Unknown pointer size."
#endif

NS_CHECK_AT_COMPILETIME(sizeof(void*) == NS_ALIGNMENT_MINIMUM);
NS_CHECK_AT_COMPILETIME(alignof(void*) == NS_ALIGNMENT_MINIMUM);

/// \brief Enum values for success and failure. To be used by functions as return values mostly, instead of bool.
enum nsResultEnum
{
  NS_FAILURE,
  NS_SUCCESS
};

/// \brief Default enum for returning failure or success, instead of using a bool.
struct [[nodiscard]] NS_FOUNDATION_DLL nsResult
{
public:
  nsResult(nsResultEnum res)
    : m_E(res)
  {
  }

  void operator=(nsResultEnum rhs) { m_E = rhs; }
  bool operator==(nsResultEnum cmp) const { return m_E == cmp; }
  bool operator!=(nsResultEnum cmp) const { return m_E != cmp; }

  [[nodiscard]] NS_ALWAYS_INLINE bool Succeeded() const { return m_E == NS_SUCCESS; }
  [[nodiscard]] NS_ALWAYS_INLINE bool Failed() const { return m_E == NS_FAILURE; }

  /// \brief Used to silence compiler warnings, when success or failure doesn't matter.
  NS_ALWAYS_INLINE void IgnoreResult()
  {
    /* dummy to be called when a return value is [[nodiscard]] but the result is not needed */
  }

  /// \brief Asserts that the function succeeded. In case of failure, the program will terminate.
  ///
  /// If \a msg is given, this will be the assert message. If \a details is provided, \a msg should contain a formatting element ({}), e.g. "Error: {}".
  void AssertSuccess(const char* szMsg = nullptr, const char* szDetails = nullptr) const;

private:
  nsResultEnum m_E;
};

/// \brief Explicit conversion to nsResult, can be overloaded for arbitrary types.
///
/// This is intentionally not done via casting operator overload (or even additional constructors) since this usually comes with a
/// considerable data loss.
NS_ALWAYS_INLINE nsResult nsToResult(nsResult result)
{
  return result;
}

/// \brief Helper macro to call functions that return nsStatus or nsResult in a function that returns nsStatus (or nsResult) as well.
/// If the called function fails, its return value is returned from the calling scope.
#define NS_SUCCEED_OR_RETURN(code) \
  do                               \
  {                                \
    auto s = (code);               \
    if (nsToResult(s).Failed())    \
      return s;                    \
  } while (false)

/// \brief Like NS_SUCCEED_OR_RETURN, but with error logging.
#define NS_SUCCEED_OR_RETURN_LOG(code)                                    \
  do                                                                      \
  {                                                                       \
    auto s = (code);                                                      \
    if (nsToResult(s).Failed())                                           \
    {                                                                     \
      nsLog::Error("Call '{0}' failed with: {1}", NS_STRINGIZE(code), s); \
      return s;                                                           \
    }                                                                     \
  } while (false)

/// \brief Like NS_SUCCEED_OR_RETURN, but with custom error logging.
#define NS_SUCCEED_OR_RETURN_CUSTOM_LOG(code, log)                          \
  do                                                                        \
  {                                                                         \
    auto s = (code);                                                        \
    if (nsToResult(s).Failed())                                             \
    {                                                                       \
      nsLog::Error("Call '{0}' failed with: {1}", NS_STRINGIZE(code), log); \
      return s;                                                             \
    }                                                                       \
  } while (false)

//////////////////////////////////////////////////////////////////////////

class nsRTTI;

/// \brief Dummy type to pass to templates and macros that expect a base type for a class that has no base.
class nsNoBase
{
public:
  static const nsRTTI* GetStaticRTTI() { return nullptr; }
};

/// \brief Dummy type to pass to templates and macros that expect a base type for an enum class.
class nsEnumBase
{
};

/// \brief Dummy type to pass to templates and macros that expect a base type for an bitflags class.
class nsBitflagsBase
{
};

/// \brief Helper struct to get a storage type from a size in byte.
template <size_t SizeInByte>
struct nsSizeToType;
/// \cond
template <>
struct nsSizeToType<1>
{
  using Type = nsUInt8;
};
template <>
struct nsSizeToType<2>
{
  using Type = nsUInt16;
};
template <>
struct nsSizeToType<3>
{
  using Type = nsUInt32;
};
template <>
struct nsSizeToType<4>
{
  using Type = nsUInt32;
};
template <>
struct nsSizeToType<5>
{
  using Type = nsUInt64;
};
template <>
struct nsSizeToType<6>
{
  using Type = nsUInt64;
};
template <>
struct nsSizeToType<7>
{
  using Type = nsUInt64;
};
template <>
struct nsSizeToType<8>
{
  using Type = nsUInt64;
};
/// \endcond
