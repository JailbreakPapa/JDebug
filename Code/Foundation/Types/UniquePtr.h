#pragma once

#include <Foundation/Basics.h>

/// \brief A Unique ptr manages an object and destroys that object when it goes out of scope. It is ensure that only one unique ptr can
/// manage the same object.
template <typename T>
class wdUniquePtr
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdUniquePtr);

public:
  WD_DECLARE_MEM_RELOCATABLE_TYPE();

  /// \brief Creates an empty unique ptr.
  wdUniquePtr();

  /// \brief Creates a unique ptr from a freshly created instance through WD_NEW or WD_DEFAULT_NEW.
  template <typename U>
  wdUniquePtr(const wdInternal::NewInstance<U>& instance);

  /// \brief Creates a unique ptr from a pointer and an allocator. The passed allocator will be used to destroy the instance when the unique
  /// ptr goes out of scope.
  template <typename U>
  wdUniquePtr(U* pInstance, wdAllocatorBase* pAllocator);

  /// \brief Move constructs a unique ptr from another. The other unique ptr will be empty afterwards to guarantee that there is only one
  /// unique ptr managing the same object.
  template <typename U>
  wdUniquePtr(wdUniquePtr<U>&& other);

  /// \brief Initialization with nullptr to be able to return nullptr in functions that return unique ptr.
  wdUniquePtr(std::nullptr_t);

  /// \brief Destroys the managed object using the stored allocator.
  ~wdUniquePtr();

  /// \brief Sets the unique ptr from a freshly created instance through WD_NEW or WD_DEFAULT_NEW.
  template <typename U>
  wdUniquePtr<T>& operator=(const wdInternal::NewInstance<U>& instance);

  /// \brief Move assigns a unique ptr from another. The other unique ptr will be empty afterwards to guarantee that there is only one
  /// unique ptr managing the same object.
  template <typename U>
  wdUniquePtr<T>& operator=(wdUniquePtr<U>&& other);

  /// \brief Same as calling 'Reset()'
  wdUniquePtr<T>& operator=(std::nullptr_t);

  /// \brief Releases the managed object without destroying it. The unique ptr will be empty afterwards.
  T* Release();

  /// \brief Releases the managed object without destroying it. The unique ptr will be empty afterwards. Also returns the allocator that
  /// should be used to destroy the object.
  T* Release(wdAllocatorBase*& out_pAllocator);

  /// \brief Borrows the managed object. The unique ptr stays unmodified.
  T* Borrow() const;

  /// \brief Destroys the managed object and resets the unique ptr.
  void Clear();

  /// \brief Provides access to the managed object.
  T& operator*() const;

  /// \brief Provides access to the managed object.
  T* operator->() const;

  /// \brief Returns true if there is managed object and false if the unique ptr is empty.
  explicit operator bool() const;

  /// \brief Compares the unique ptr against another unique ptr.
  bool operator==(const wdUniquePtr<T>& rhs) const;
  bool operator!=(const wdUniquePtr<T>& rhs) const;
  bool operator<(const wdUniquePtr<T>& rhs) const;
  bool operator<=(const wdUniquePtr<T>& rhs) const;
  bool operator>(const wdUniquePtr<T>& rhs) const;
  bool operator>=(const wdUniquePtr<T>& rhs) const;

  /// \brief Compares the unique ptr against nullptr.
  bool operator==(std::nullptr_t) const;
  bool operator!=(std::nullptr_t) const;
  bool operator<(std::nullptr_t) const;
  bool operator<=(std::nullptr_t) const;
  bool operator>(std::nullptr_t) const;
  bool operator>=(std::nullptr_t) const;

private:
  template <typename U>
  friend class wdUniquePtr;

  T* m_pInstance = nullptr;
  wdAllocatorBase* m_pAllocator = nullptr;
};

#include <Foundation/Types/Implementation/UniquePtr_inl.h>
