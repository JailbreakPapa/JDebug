#pragma once

#include <Foundation/Basics.h>

/// \brief A wrapper around a raw pointer that allows to use the lower N bits for flags
///
/// When accessing the pointer, the lower N bits are masked off.
/// Typically one can safely store 3 bits in the lower bits of a pointer as most data is 8 byte aligned,
/// especially when it was heap allocated.
template <typename PtrType, nsUInt8 NumFlagBits = 2>
class nsPointerWithFlags
{
private:
  enum : size_t
  {
    AllOnes = (std::size_t)(-1),
    PtrBits = sizeof(void*) * 8,
    FlagsMask = (AllOnes >> (PtrBits - NumFlagBits)),
    PtrMask = ~FlagsMask,
  };

  void* m_pPtr = nullptr;

public:
  /// \brief Initializes the pointer and flags with zero
  nsPointerWithFlags() = default;

  /// \brief Initializes the pointer and flags
  explicit nsPointerWithFlags(PtrType* pPtr, nsUInt8 uiFlags = 0) { SetPtrAndFlags(pPtr, uiFlags); }

  /// \brief Changes the pointer and flags
  void SetPtrAndFlags(PtrType* pPtr, nsUInt8 uiFlags)
  {
    const std::uintptr_t isrc = *reinterpret_cast<std::uintptr_t*>(&pPtr);
    std::uintptr_t& iptr = *reinterpret_cast<std::uintptr_t*>(&m_pPtr);

    iptr = (isrc & PtrMask) | (uiFlags & FlagsMask);
  }

  /// \brief Returns the masked off pointer value
  const PtrType* GetPtr() const
  {
    const std::uintptr_t& iptr = *reinterpret_cast<const std::uintptr_t*>(&m_pPtr);
    return reinterpret_cast<const PtrType*>(iptr & PtrMask); // mask off lower N bits
  }

  /// \brief Returns the masked off pointer value
  PtrType* GetPtr()
  {
    std::uintptr_t& iptr = *reinterpret_cast<std::uintptr_t*>(&m_pPtr);
    return reinterpret_cast<PtrType*>(iptr & PtrMask); // mask off lower N bits
  }

  /// \brief Changes the pointer value only. Flags stay unchanged.
  void SetPtr(PtrType* pPtr)
  {
    const std::uintptr_t isrc = *reinterpret_cast<std::uintptr_t*>(&pPtr);
    NS_ASSERT_DEBUG(
      (isrc & FlagsMask) == 0, "The given pointer does not have an {} byte alignment and thus cannot be stored lossless.", 1u << NumFlagBits);

    std::uintptr_t& iptr = *reinterpret_cast<std::uintptr_t*>(&m_pPtr);

    iptr = (isrc & PtrMask) | (iptr & FlagsMask);
  }
  /// \brief Returns the flags value only
  nsUInt8 GetFlags() const
  {
    const std::uintptr_t& iptr = *reinterpret_cast<const std::uintptr_t*>(&m_pPtr);
    return static_cast<nsUInt8>(iptr & FlagsMask);
  }

  /// \brief Changes only the flags value. The given value must fit into the reserved bits.
  void SetFlags(nsUInt8 uiFlags)
  {
    NS_ASSERT_DEBUG(uiFlags <= FlagsMask, "The flag value {} requires more than {} bits", uiFlags, NumFlagBits);

    std::uintptr_t& iptr = *reinterpret_cast<std::uintptr_t*>(&m_pPtr);

    iptr = (iptr & PtrMask) | (uiFlags & FlagsMask);
  }

  /// \brief Returns the masked off pointer value
  operator PtrType*() { return GetPtr(); }

  /// \brief Returns the masked off pointer value
  operator const PtrType*() const { return GetPtr(); }

  /// \brief Changes the pointer value only. Flags stay unchanged.
  void operator=(PtrType* pPtr) { SetPtr(pPtr); }

  /// \brief Compares the pointer part for equality (flags are ignored)
  template <typename = typename std::enable_if<std::is_const<PtrType>::value == false>>
  bool operator==(const PtrType* pPtr) const
  {
    return GetPtr() == pPtr;
  }

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)
  /// \brief Compares the pointer part for inequality (flags are ignored)
  template <typename = typename std::enable_if<std::is_const<PtrType>::value == false>>
  bool operator!=(const PtrType* pPtr) const
  {
    return !(*this == pPtr);
  }
#endif

  bool operator==(const nsPointerWithFlags<PtrType, NumFlagBits>& rhs) const
  {
    return GetPtr() == rhs.GetPtr();
  }

  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsPointerWithFlags<PtrType, NumFlagBits>&);

  /// \brief Compares the pointer part for equality (flags are ignored)
  bool operator==(PtrType* pPtr) const { return GetPtr() == pPtr; }
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(PtrType*);

  /// \brief Compares the pointer part for equality (flags are ignored)
  bool operator==(std::nullptr_t) const { return GetPtr() == nullptr; }
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(std::nullptr_t);

  /// \brief Checks whether the pointer part is not nullptr (flags are ignored)
  explicit operator bool() const { return GetPtr() != nullptr; }

  /// \brief Dereferences the pointer
  const PtrType* operator->() const { return GetPtr(); }

  /// \brief Dereferences the pointer
  PtrType* operator->() { return GetPtr(); }

  /// \brief Dereferences the pointer
  const PtrType& operator*() const { return *GetPtr(); }

  /// \brief Dereferences the pointer
  PtrType& operator*() { return *GetPtr(); }
};
