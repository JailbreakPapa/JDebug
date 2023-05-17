#pragma once

#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/UniquePtr.h>

/// \brief A Shared ptr manages a shared object and destroys that object when no one references it anymore. The managed object must derive
/// from wdRefCounted.
template <typename T>
class wdSharedPtr
{
public:
  WD_DECLARE_MEM_RELOCATABLE_TYPE();

  /// \brief Creates an empty shared ptr.
  wdSharedPtr();

  /// \brief Creates a shared ptr from a freshly created instance through WD_NEW or WD_DEFAULT_NEW.
  template <typename U>
  wdSharedPtr(const wdInternal::NewInstance<U>& instance);

  /// \brief Creates a shared ptr from a pointer and an allocator. The passed allocator will be used to destroy the instance when the shared
  /// ptr goes out of scope.
  template <typename U>
  wdSharedPtr(U* pInstance, wdAllocatorBase* pAllocator);

  /// \brief Copy constructs a shared ptr from another. Both will hold a reference to the managed object afterwards.
  wdSharedPtr(const wdSharedPtr<T>& other);

  /// \brief Copy constructs a shared ptr from another. Both will hold a reference to the managed object afterwards.
  template <typename U>
  wdSharedPtr(const wdSharedPtr<U>& other);

  /// \brief Move constructs a shared ptr from another. The other shared ptr will be empty afterwards.
  template <typename U>
  wdSharedPtr(wdSharedPtr<U>&& other);

  /// \brief Move constructs a shared ptr from a unique ptr. The unique ptr will be empty afterwards.
  template <typename U>
  wdSharedPtr(wdUniquePtr<U>&& other);

  /// \brief Initialization with nullptr to be able to return nullptr in functions that return shared ptr.
  wdSharedPtr(std::nullptr_t);

  /// \brief Destroys the managed object using the stored allocator if no one else references it anymore.
  ~wdSharedPtr();

  /// \brief Sets the shared ptr from a freshly created instance through WD_NEW or WD_DEFAULT_NEW.
  template <typename U>
  wdSharedPtr<T>& operator=(const wdInternal::NewInstance<U>& instance);

  /// \brief Sets the shared ptr from another. Both will hold a reference to the managed object afterwards.
  wdSharedPtr<T>& operator=(const wdSharedPtr<T>& other);

  /// \brief Sets the shared ptr from another. Both will hold a reference to the managed object afterwards.
  template <typename U>
  wdSharedPtr<T>& operator=(const wdSharedPtr<U>& other);

  /// \brief Move assigns a shared ptr from another. The other shared ptr will be empty afterwards.
  template <typename U>
  wdSharedPtr<T>& operator=(wdSharedPtr<U>&& other);

  /// \brief Move assigns a shared ptr from a unique ptr. The unique ptr will be empty afterwards.
  template <typename U>
  wdSharedPtr<T>& operator=(wdUniquePtr<U>&& other);

  /// \brief Assigns a nullptr to the shared ptr. Same as Reset.
  wdSharedPtr<T>& operator=(std::nullptr_t);

  /// \brief Borrows the managed object. The shared ptr stays unmodified.
  T* Borrow() const;

  /// \brief Destroys the managed object if no one else references it anymore and resets the shared ptr.
  void Clear();

  /// \brief Provides access to the managed object.
  T& operator*() const;

  /// \brief Provides access to the managed object.
  T* operator->() const;

  /// \brief Provides access to the managed object.
  operator const T*() const;

  /// \brief Provides access to the managed object.
  operator T*();

  /// \brief Returns true if there is managed object and false if the shared ptr is empty.
  explicit operator bool() const;

  /// \brief Compares the shared ptr against another shared ptr.
  bool operator==(const wdSharedPtr<T>& rhs) const;
  bool operator!=(const wdSharedPtr<T>& rhs) const;
  bool operator<(const wdSharedPtr<T>& rhs) const;
  bool operator<=(const wdSharedPtr<T>& rhs) const;
  bool operator>(const wdSharedPtr<T>& rhs) const;
  bool operator>=(const wdSharedPtr<T>& rhs) const;

  /// \brief Compares the shared ptr against nullptr.
  bool operator==(std::nullptr_t) const;
  bool operator!=(std::nullptr_t) const;
  bool operator<(std::nullptr_t) const;
  bool operator<=(std::nullptr_t) const;
  bool operator>(std::nullptr_t) const;
  bool operator>=(std::nullptr_t) const;

  /// \brief Returns a copy of this, as an wdSharedPtr<DERIVED>. Downcasts the stored pointer (using static_cast).
  ///
  /// Does not check whether the cast would be valid, that is all your responsibility.
  template <typename DERIVED>
  wdSharedPtr<DERIVED> Downcast() const
  {
    return wdSharedPtr<DERIVED>(static_cast<DERIVED*>(m_pInstance), m_pAllocator);
  }

private:
  template <typename U>
  friend class wdSharedPtr;

  void AddReferenceIfValid();
  void ReleaseReferenceIfValid();

  T* m_pInstance;
  wdAllocatorBase* m_pAllocator;
};

#include <Foundation/Types/Implementation/SharedPtr_inl.h>
