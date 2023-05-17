#pragma once

// ***** Definition of types *****

using wdUInt8 = unsigned char;
using wdUInt16 = unsigned short;
using wdUInt32 = unsigned int;
using wdUInt64 = unsigned long long;

using wdInt8 = signed char;
using wdInt16 = short;
using wdInt32 = int;
using wdInt64 = long long;

// no float-types, since those are well portable

// Do some compile-time checks on the types
WD_CHECK_AT_COMPILETIME(sizeof(bool) == 1);
WD_CHECK_AT_COMPILETIME(sizeof(char) == 1);
WD_CHECK_AT_COMPILETIME(sizeof(float) == 4);
WD_CHECK_AT_COMPILETIME(sizeof(double) == 8);
WD_CHECK_AT_COMPILETIME(sizeof(wdInt8) == 1);
WD_CHECK_AT_COMPILETIME(sizeof(wdInt16) == 2);
WD_CHECK_AT_COMPILETIME(sizeof(wdInt32) == 4);
WD_CHECK_AT_COMPILETIME(sizeof(wdInt64) == 8); // must be defined in the specific compiler header
WD_CHECK_AT_COMPILETIME(sizeof(wdUInt8) == 1);
WD_CHECK_AT_COMPILETIME(sizeof(wdUInt16) == 2);
WD_CHECK_AT_COMPILETIME(sizeof(wdUInt32) == 4);
WD_CHECK_AT_COMPILETIME(sizeof(wdUInt64) == 8); // must be defined in the specific compiler header
WD_CHECK_AT_COMPILETIME(sizeof(long long int) == 8);

#if WD_ENABLED(WD_PLATFORM_64BIT)
#  define WD_ALIGNMENT_MINIMUM 8
#elif WD_ENABLED(WD_PLATFORM_32BIT)
#  define WD_ALIGNMENT_MINIMUM 4
#else
#  error "Unknown pointer size."
#endif

WD_CHECK_AT_COMPILETIME(sizeof(void*) == WD_ALIGNMENT_MINIMUM);

/// \brief Enum values for success and failure. To be used by functions as return values mostly, instead of bool.
enum wdResultEnum
{
  WD_FAILURE,
  WD_SUCCESS
};

/// \brief Default enum for returning failure or success, instead of using a bool.
struct [[nodiscard]] WD_FOUNDATION_DLL wdResult
{
public:
  wdResult(wdResultEnum res)
    : m_E(res)
  {
  }

  void operator=(wdResultEnum rhs) { m_E = rhs; }
  bool operator==(wdResultEnum cmp) const { return m_E == cmp; }
  bool operator!=(wdResultEnum cmp) const { return m_E != cmp; }

  [[nodiscard]] WD_ALWAYS_INLINE bool Succeeded() const { return m_E == WD_SUCCESS; }
  [[nodiscard]] WD_ALWAYS_INLINE bool Failed() const { return m_E == WD_FAILURE; }

  /// \brief Used to silence compiler warnings, when success or failure doesn't matter.
  WD_ALWAYS_INLINE void IgnoreResult()
  { /* dummy to be called when a return value is [[nodiscard]] but the result is not needed */
  }

  /// \brief Asserts that the function succeeded. In case of failure, the program will terminate.
  ///
  /// If \a msg is given, this will be the assert message. If \a details is provided, \a msg should contain a formatting element ({}), e.g. "Error: {}".
  void AssertSuccess(const char* szMsg = nullptr, const char* szDetails = nullptr) const;

  /// \brief Same as 'Succeeded()'.
  ///
  /// Allows wdResult to be used in if statements:
  ///  - if (r)
  ///  - if (!r)
  ///  - if (r1 && r2)
  ///  - if (r1 || r2)
  ///
  /// Disallows anything else implicitly, e.g. all these won't compile:
  ///   - if (r == true)
  ///   - bool b = r;
  ///   - void* p = r;
  ///   - return r; // with bool return type
  explicit operator bool() const { return m_E == WD_SUCCESS; }

  /// \brief Special case to prevent this from working: "bool b = !r"
  wdResult operator!() const { return wdResult((m_E == WD_SUCCESS) ? WD_FAILURE : WD_SUCCESS); }

private:
  wdResultEnum m_E;
};

/// \brief Explicit conversion to wdResult, can be overloaded for arbitrary types.
///
/// This is intentionally not done via casting operator overload (or even additional constructors) since this usually comes with a
/// considerable data loss.
WD_ALWAYS_INLINE wdResult wdToResult(wdResult result)
{
  return result;
}

/// \brief Helper macro to call functions that return wdStatus or wdResult in a function that returns wdStatus (or wdResult) as well.
/// If the called function fails, its return value is returned from the calling scope.
#define WD_SUCCEED_OR_RETURN(code) \
  do                               \
  {                                \
    auto s = (code);               \
    if (wdToResult(s).Failed())    \
      return s;                    \
  } while (false)

/// \brief Like WD_SUCCEED_OR_RETURN, but with error logging.
#define WD_SUCCEED_OR_RETURN_LOG(code)                                    \
  do                                                                      \
  {                                                                       \
    auto s = (code);                                                      \
    if (wdToResult(s).Failed())                                           \
    {                                                                     \
      wdLog::Error("Call '{0}' failed with: {1}", WD_STRINGIZE(code), s); \
      return s;                                                           \
    }                                                                     \
  } while (false)

/// \brief Like WD_SUCCEED_OR_RETURN, but with custom error logging.
#define WD_SUCCEED_OR_RETURN_CUSTOM_LOG(code, log)                          \
  do                                                                        \
  {                                                                         \
    auto s = (code);                                                        \
    if (wdToResult(s).Failed())                                             \
    {                                                                       \
      wdLog::Error("Call '{0}' failed with: {1}", WD_STRINGIZE(code), log); \
      return s;                                                             \
    }                                                                       \
  } while (false)

//////////////////////////////////////////////////////////////////////////

class wdRTTI;

/// \brief Dummy type to pass to templates and macros that expect a base type for a class that has no base.
class wdNoBase
{
public:
  static const wdRTTI* GetStaticRTTI() { return nullptr; }
};

/// \brief Dummy type to pass to templates and macros that expect a base type for an enum class.
class wdEnumBase
{
};

/// \brief Dummy type to pass to templates and macros that expect a base type for an bitflags class.
class wdBitflagsBase
{
};

/// \brief Helper struct to get a storage type from a size in byte.
template <size_t SizeInByte>
struct wdSizeToType;
/// \cond
template <>
struct wdSizeToType<1>
{
  using Type = wdUInt8;
};
template <>
struct wdSizeToType<2>
{
  using Type = wdUInt16;
};
template <>
struct wdSizeToType<3>
{
  using Type = wdUInt32;
};
template <>
struct wdSizeToType<4>
{
  using Type = wdUInt32;
};
template <>
struct wdSizeToType<5>
{
  using Type = wdUInt64;
};
template <>
struct wdSizeToType<6>
{
  using Type = wdUInt64;
};
template <>
struct wdSizeToType<7>
{
  using Type = wdUInt64;
};
template <>
struct wdSizeToType<8>
{
  using Type = wdUInt64;
};
/// \endcond
