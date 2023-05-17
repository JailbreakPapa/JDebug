
#define WD_CHECK_CLASS(T)                                 \
  WD_CHECK_AT_COMPILETIME_MSG(!std::is_trivial<T>::value, \
    "POD type is treated as class. Use WD_DECLARE_POD_TYPE(YourClass) or WD_DEFINE_AS_POD_TYPE(ExternalClass) to mark it as POD.")

// public methods: redirect to implementation
template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Construct(T* pDestination, size_t uiCount)
{
  // Default constructor is always called, so that debug helper initializations (e.g. wdVec3 initializes to NaN) take place.
  // Note that destructor is ONLY called for class types.
  // Special case for c++11 to prevent default construction of "real" Pod types, also avoids warnings on msvc
  Construct(pDestination, uiCount, wdTraitInt < wdIsPodType<T>::value && std::is_trivial<T>::value > ());
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::ConstructorFunction wdMemoryUtils::MakeConstructorFunction()
{
  return MakeConstructorFunction<T>(wdTraitInt < wdIsPodType<T>::value && std::is_trivial<T>::value > ());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::DefaultConstruct(T* pDestination, size_t uiCount)
{
  for (size_t i = 0; i < uiCount; i++)
  {
    ::new (pDestination + i) T();
  }
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::ConstructorFunction wdMemoryUtils::MakeDefaultConstructorFunction()
{
  struct Helper
  {
    static void DefaultConstruct(void* pDestination) { wdMemoryUtils::DefaultConstruct(static_cast<T*>(pDestination), 1); }
  };

  return &Helper::DefaultConstruct;
}

template <typename Destination, typename Source>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyConstruct(Destination* pDestination, const Source& copy, size_t uiCount)
{
  CopyConstruct<Destination, Source>(pDestination, copy, uiCount, wdIsPodType<Destination>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyConstructArray(T* pDestination, const T* pSource, size_t uiCount)
{
  WD_ASSERT_DEV(pDestination < pSource || pSource + uiCount <= pDestination, "Memory regions must not overlap when using CopyConstruct.");
  CopyConstructArray<T>(pDestination, pSource, uiCount, wdIsPodType<T>());
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::CopyConstructorFunction wdMemoryUtils::MakeCopyConstructorFunction()
{
  struct Helper
  {
    static void CopyConstruct(void* pDestination, const void* pSource)
    {
      wdMemoryUtils::CopyConstruct(static_cast<T*>(pDestination), *static_cast<const T*>(pSource), 1);
    }
  };

  return &Helper::CopyConstruct;
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::MoveConstruct(T* pDestination, T&& source)
{
  // Make sure source is actually an rvalue reference (T&& is a universal reference).
  static_assert(std::is_rvalue_reference<decltype(source)>::value, "'source' parameter is not an rvalue reference.");
  ::new (pDestination) T(std::forward<T>(source));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::MoveConstruct(T* pDestination, T* pSource, size_t uiCount)
{
  WD_ASSERT_DEV(pDestination < pSource || pSource + uiCount <= pDestination, "Memory regions must not overlap when using MoveConstruct.");

  // Enforce move construction.
  static_assert(std::is_move_constructible<T>::value, "Type is not move constructible!");

  for (size_t i = 0; i < uiCount; ++i)
  {
    ::new (pDestination + i) T(std::move(pSource[i]));
  }
}

template <typename Destination, typename Source>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyOrMoveConstruct(Destination* pDestination, Source&& source)
{
  typedef typename std::is_rvalue_reference<decltype(source)>::type IsRValueRef;
  CopyOrMoveConstruct<Destination, Source>(pDestination, std::forward<Source>(source), IsRValueRef());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::RelocateConstruct(T* pDestination, T* pSource, size_t uiCount)
{
  WD_ASSERT_DEV(pDestination < pSource || pSource + uiCount <= pDestination, "Memory regions must not overlap when using RelocateConstruct.");
  RelocateConstruct(pDestination, pSource, uiCount, wdGetTypeClass<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Destruct(T* pDestination, size_t uiCount)
{
  Destruct(pDestination, uiCount, wdIsPodType<T>());
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::DestructorFunction wdMemoryUtils::MakeDestructorFunction()
{
  return MakeDestructorFunction<T>(wdIsPodType<T>());
}

WD_ALWAYS_INLINE void wdMemoryUtils::RawByteCopy(void* pDestination, const void* pSource, size_t uiNumBytesToCopy)
{
  memcpy(pDestination, pSource, uiNumBytesToCopy);
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Copy(T* pDestination, const T* pSource, size_t uiCount)
{
  WD_ASSERT_DEV(
    pDestination < pSource || pSource + uiCount <= pDestination, "Memory regions must not overlap when using Copy. Use CopyOverlapped instead.");
  Copy(pDestination, pSource, uiCount, wdIsPodType<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyOverlapped(T* pDestination, const T* pSource, size_t uiCount)
{
  CopyOverlapped(pDestination, pSource, uiCount, wdIsPodType<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Relocate(T* pDestination, T* pSource, size_t uiCount)
{
  WD_ASSERT_DEV(pDestination < pSource || pSource + uiCount <= pDestination, "Memory regions must not overlap when using Relocate.");
  Relocate(pDestination, pSource, uiCount, wdGetTypeClass<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::RelocateOverlapped(T* pDestination, T* pSource, size_t uiCount)
{
  RelocateOverlapped(pDestination, pSource, uiCount, wdGetTypeClass<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, const T& source, size_t uiCount)
{
  Prepend(pDestination, source, uiCount, wdGetTypeClass<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, T&& source, size_t uiCount)
{
  Prepend(pDestination, std::move(source), uiCount, wdGetTypeClass<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, const T* pSource, size_t uiSourceCount, size_t uiCount)
{
  Prepend(pDestination, pSource, uiSourceCount, uiCount, wdGetTypeClass<T>());
}

template <typename T>
WD_ALWAYS_INLINE bool wdMemoryUtils::IsEqual(const T* a, const T* b, size_t uiCount /*= 1*/)
{
  return IsEqual(a, b, uiCount, wdIsPodType<T>());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::ZeroFill(T* pDestination, size_t uiCount)
{
  memset(pDestination, 0, uiCount * sizeof(T));
}

template <typename T, size_t N>
WD_ALWAYS_INLINE void wdMemoryUtils::ZeroFillArray(T (&destination)[N])
{
  return ZeroFill(destination, N);
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::PatternFill(T* pDestination, wdUInt8 uiBytePattern, size_t uiCount)
{
  memset(pDestination, uiBytePattern, uiCount * sizeof(T));
}

template <typename T, size_t N>
WD_ALWAYS_INLINE void wdMemoryUtils::PatternFillArray(T (&destination)[N], wdUInt8 uiBytePattern)
{
  return PatternFill(destination, uiBytePattern, N);
}

template <typename T>
WD_ALWAYS_INLINE wdInt32 wdMemoryUtils::Compare(const T* a, const T* b, size_t uiCount /*= 1*/)
{
  return memcmp(a, b, uiCount * sizeof(T));
}

WD_ALWAYS_INLINE wdInt32 wdMemoryUtils::RawByteCompare(const void* a, const void* b, size_t uiNumBytesToCompare)
{
  return memcmp(a, b, uiNumBytesToCompare);
}

template <typename T>
WD_ALWAYS_INLINE T* wdMemoryUtils::AddByteOffset(T* pPtr, ptrdiff_t iOffset)
{
  return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(pPtr) + iOffset);
}

template <typename T>
WD_ALWAYS_INLINE T* wdMemoryUtils::AlignBackwards(T* pPtr, size_t uiAlignment)
{
  return reinterpret_cast<T*>(reinterpret_cast<size_t>(pPtr) & ~(uiAlignment - 1));
}

template <typename T>
WD_ALWAYS_INLINE T* wdMemoryUtils::AlignForwards(T* pPtr, size_t uiAlignment)
{
  return reinterpret_cast<T*>((reinterpret_cast<size_t>(pPtr) + uiAlignment - 1) & ~(uiAlignment - 1));
}

template <typename T>
WD_ALWAYS_INLINE T wdMemoryUtils::AlignSize(T uiSize, T uiAlignment)
{
  return ((uiSize + (uiAlignment - 1)) & ~(uiAlignment - 1));
}

template <typename T>
WD_ALWAYS_INLINE bool wdMemoryUtils::IsAligned(const T* pPtr, size_t uiAlignment)
{
  return (reinterpret_cast<size_t>(pPtr) & (uiAlignment - 1)) == 0;
}

template <typename T>
WD_ALWAYS_INLINE bool wdMemoryUtils::IsSizeAligned(T uiSize, T uiAlignment)
{
  return (uiSize & (uiAlignment - 1)) == 0;
}

// private methods

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Construct(T* pDestination, size_t uiCount, wdTypeIsPod)
{
  WD_CHECK_AT_COMPILETIME_MSG(std::is_trivial<T>::value, "This method should only be called for 'real' pod aka trivial types");
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Construct(T* pDestination, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

#define WD_GCC_WARNING_NAME "-Wstringop-overflow"
#include <Foundation/Basics/Compiler/GCC/DisableWarning_GCC.h>

  for (size_t i = 0; i < uiCount; i++)
  {
    ::new (pDestination + i) T();
  }

#include <Foundation/Basics/Compiler/GCC/RestoreWarning_GCC.h>
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::ConstructorFunction wdMemoryUtils::MakeConstructorFunction(wdTypeIsPod)
{
  WD_CHECK_AT_COMPILETIME_MSG(std::is_trivial<T>::value, "This method should only be called for 'real' pod aka trivial types");
  return nullptr;
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::ConstructorFunction wdMemoryUtils::MakeConstructorFunction(wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  struct Helper
  {
    static void Construct(void* pDestination) { wdMemoryUtils::Construct(static_cast<T*>(pDestination), 1, wdTypeIsClass()); }
  };

  return &Helper::Construct;
}

template <typename Destination, typename Source>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyConstruct(Destination* pDestination, const Source& copy, size_t uiCount, wdTypeIsPod)
{
  static_assert(std::is_same<Destination, Source>::value ||
                  (std::is_base_of<Destination, Source>::value == false && std::is_base_of<Source, Destination>::value == false),
    "Can't copy POD types that are derived from each other. Are you certain any of these types should be POD?");

  const Destination& copyConverted = copy;
  for (size_t i = 0; i < uiCount; i++)
  {
    memcpy(pDestination + i, &copyConverted, sizeof(Destination));
  }
}

template <typename Destination, typename Source>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyConstruct(Destination* pDestination, const Source& copy, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(Destination);

  for (size_t i = 0; i < uiCount; i++)
  {
    ::new (pDestination + i) Destination(copy); // Note that until now copy has not been converted to Destination. This allows for calling
                                                // specialized constructors if available.
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyConstructArray(T* pDestination, const T* pSource, size_t uiCount, wdTypeIsPod)
{
  memcpy(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyConstructArray(T* pDestination, const T* pSource, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  for (size_t i = 0; i < uiCount; i++)
  {
    ::new (pDestination + i) T(pSource[i]);
  }
}

template <typename Destination, typename Source>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyOrMoveConstruct(Destination* pDestination, const Source& source, NotRValueReference)
{
  CopyConstruct<Destination, Source>(pDestination, source, 1);
}

template <typename Destination, typename Source>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyOrMoveConstruct(Destination* pDestination, Source&& source, IsRValueReference)
{
  static_assert(std::is_rvalue_reference<decltype(source)>::value,
    "Implementation Error: This version of CopyOrMoveConstruct should only be called with a rvalue reference!");
  ::new (pDestination) Destination(std::move(source));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::RelocateConstruct(T* pDestination, T* pSource, size_t uiCount, wdTypeIsPod)
{
  memcpy(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::RelocateConstruct(T* pDestination, T* pSource, size_t uiCount, wdTypeIsMemRelocatable)
{
  memcpy(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::RelocateConstruct(T* pDestination, T* pSource, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  for (size_t i = 0; i < uiCount; i++)
  {
    // Note that this calls the move constructor only if available and will copy otherwise.
    ::new (pDestination + i) T(std::move(pSource[i]));
  }

  Destruct(pSource, uiCount, wdTypeIsClass());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Destruct(T* pDestination, size_t uiCount, wdTypeIsPod)
{
  // Nothing to do here. See Construct of for more info.
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Destruct(T* pDestination, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

#define WD_GCC_WARNING_NAME "-Waggressive-loop-optimizations"
#include <Foundation/Basics/Compiler/GCC/DisableWarning_GCC.h>

  for (size_t i = uiCount; i > 0; --i)
  {
    pDestination[i - 1].~T();
  }

#include <Foundation/Basics/Compiler/GCC/RestoreWarning_GCC.h>
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::DestructorFunction wdMemoryUtils::MakeDestructorFunction(wdTypeIsPod)
{
  return nullptr;
}

template <typename T>
WD_ALWAYS_INLINE wdMemoryUtils::DestructorFunction wdMemoryUtils::MakeDestructorFunction(wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  struct Helper
  {
    static void Destruct(void* pDestination) { wdMemoryUtils::Destruct(static_cast<T*>(pDestination), 1, wdTypeIsClass()); }
  };

  return &Helper::Destruct;
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Copy(T* pDestination, const T* pSource, size_t uiCount, wdTypeIsPod)
{
  memcpy(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Copy(T* pDestination, const T* pSource, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  for (size_t i = 0; i < uiCount; i++)
  {
    pDestination[i] = pSource[i];
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::CopyOverlapped(T* pDestination, const T* pSource, size_t uiCount, wdTypeIsPod)
{
  memmove(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
inline void wdMemoryUtils::CopyOverlapped(T* pDestination, const T* pSource, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  if (pDestination == pSource)
    return;

  if (pDestination < pSource)
  {
    for (size_t i = 0; i < uiCount; i++)
    {
      pDestination[i] = pSource[i];
    }
  }
  else
  {
    for (size_t i = uiCount; i > 0; --i)
    {
      pDestination[i - 1] = pSource[i - 1];
    }
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Relocate(T* pDestination, T* pSource, size_t uiCount, wdTypeIsPod)
{
  memcpy(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Relocate(T* pDestination, T* pSource, size_t uiCount, wdTypeIsMemRelocatable)
{
  memcpy(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Relocate(T* pDestination, T* pSource, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  for (size_t i = 0; i < uiCount; i++)
  {
    // Note that this calls the move constructor only if available and will copy otherwise.
    pDestination[i] = std::move(pSource[i]);
  }

  Destruct(pSource, uiCount, wdTypeIsClass());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::RelocateOverlapped(T* pDestination, T* pSource, size_t uiCount, wdTypeIsPod)
{
  memmove(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::RelocateOverlapped(T* pDestination, T* pSource, size_t uiCount, wdTypeIsMemRelocatable)
{
  if (pDestination < pSource)
  {
    size_t uiDestructCount = pSource - pDestination;
    Destruct(pDestination, uiDestructCount, wdTypeIsClass());
  }
  else
  {
    size_t uiDestructCount = pDestination - pSource;
    Destruct(pSource + uiCount, uiDestructCount, wdTypeIsClass());
  }
  memmove(pDestination, pSource, uiCount * sizeof(T));
}

template <typename T>
inline void wdMemoryUtils::RelocateOverlapped(T* pDestination, T* pSource, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  if (pDestination == pSource)
    return;

  if (pDestination < pSource)
  {
    for (size_t i = 0; i < uiCount; i++)
    {
      pDestination[i] = std::move(pSource[i]);
    }

    size_t uiDestructCount = pSource - pDestination;
    Destruct(pSource + uiCount - uiDestructCount, uiDestructCount, wdTypeIsClass());
  }
  else
  {
    for (size_t i = uiCount; i > 0; --i)
    {
      pDestination[i - 1] = std::move(pSource[i - 1]);
    }

    size_t uiDestructCount = pDestination - pSource;
    Destruct(pSource, uiDestructCount, wdTypeIsClass());
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, const T& source, size_t uiCount, wdTypeIsPod)
{
  memmove(pDestination + 1, pDestination, uiCount * sizeof(T));
  CopyConstruct(pDestination, source, 1, wdTypeIsPod());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, const T& source, size_t uiCount, wdTypeIsMemRelocatable)
{
  memmove(pDestination + 1, pDestination, uiCount * sizeof(T));
  CopyConstruct(pDestination, source, 1, wdTypeIsClass());
}

template <typename T>
inline void wdMemoryUtils::Prepend(T* pDestination, const T& source, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  if (uiCount > 0)
  {
    MoveConstruct(pDestination + uiCount, std::move(pDestination[uiCount - 1]));

    for (size_t i = uiCount - 1; i > 0; --i)
    {
      pDestination[i] = std::move(pDestination[i - 1]);
    }

    *pDestination = source;
  }
  else
  {
    CopyConstruct(pDestination, source, 1, wdTypeIsClass());
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, T&& source, size_t uiCount, wdTypeIsPod)
{
  memmove(pDestination + 1, pDestination, uiCount * sizeof(T));
  MoveConstruct(pDestination, std::move(source));
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, T&& source, size_t uiCount, wdTypeIsMemRelocatable)
{
  memmove(pDestination + 1, pDestination, uiCount * sizeof(T));
  MoveConstruct(pDestination, std::move(source));
}

template <typename T>
inline void wdMemoryUtils::Prepend(T* pDestination, T&& source, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  if (uiCount > 0)
  {
    MoveConstruct(pDestination + uiCount, std::move(pDestination[uiCount - 1]));

    for (size_t i = uiCount - 1; i > 0; --i)
    {
      pDestination[i] = std::move(pDestination[i - 1]);
    }

    *pDestination = std::move(source);
  }
  else
  {
    MoveConstruct(pDestination, std::move(source));
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, const T* pSource, size_t uiSourceCount, size_t uiCount, wdTypeIsPod)
{
  memmove(pDestination + uiSourceCount, pDestination, uiCount * sizeof(T));
  CopyConstructArray(pDestination, pSource, uiSourceCount, wdTypeIsPod());
}

template <typename T>
WD_ALWAYS_INLINE void wdMemoryUtils::Prepend(T* pDestination, const T* pSource, size_t uiSourceCount, size_t uiCount, wdTypeIsMemRelocatable)
{
  memmove(pDestination + uiSourceCount, pDestination, uiCount * sizeof(T));
  CopyConstructArray(pDestination, pSource, uiSourceCount, wdTypeIsClass());
}

template <typename T>
inline void wdMemoryUtils::Prepend(T* pDestination, const T* pSource, size_t uiSourceCount, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  if (uiCount > 0)
  {
    MoveConstruct(pDestination + uiSourceCount, pDestination, uiCount);
    CopyConstructArray(pDestination, pSource, uiSourceCount, wdTypeIsClass());
  }
  else
  {
    CopyConstructArray(pDestination, pSource, uiSourceCount, wdTypeIsClass());
  }
}

template <typename T>
WD_ALWAYS_INLINE bool wdMemoryUtils::IsEqual(const T* a, const T* b, size_t uiCount, wdTypeIsPod)
{
  return memcmp(a, b, uiCount * sizeof(T)) == 0;
}

template <typename T>
WD_ALWAYS_INLINE bool wdMemoryUtils::IsEqual(const T* a, const T* b, size_t uiCount, wdTypeIsClass)
{
  WD_CHECK_CLASS(T);

  for (size_t i = 0; i < uiCount; i++)
  {
    if (!(a[i] == b[i]))
      return false;
  }
  return true;
}


#undef WD_CHECK_CLASS
