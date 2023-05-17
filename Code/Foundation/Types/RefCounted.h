
#pragma once

#include <Foundation/Threading/AtomicUtils.h>

class WD_FOUNDATION_DLL wdRefCountingImpl
{
public:
  /// \brief Constructor
  wdRefCountingImpl() = default; // [tested]

  wdRefCountingImpl(const wdRefCountingImpl& rhs) // [tested]
  {
    // do not copy the ref count
  }

  void operator=(const wdRefCountingImpl& rhs) // [tested]
  {
    // do not copy the ref count
  }

  /// \brief Increments the reference counter. Returns the new reference count.
  inline wdInt32 AddRef() const // [tested]
  {
    return wdAtomicUtils::Increment(m_iRefCount);
  }

  /// \brief Decrements the reference counter. Returns the new reference count.
  inline wdInt32 ReleaseRef() const // [tested]
  {
    return wdAtomicUtils::Decrement(m_iRefCount);
  }

  /// \brief Returns true if the reference count is greater than 0, false otherwise
  inline bool IsReferenced() const // [tested]
  {
    return m_iRefCount > 0;
  }

  /// \brief Returns the current reference count
  inline wdInt32 GetRefCount() const // [tested]
  {
    return m_iRefCount;
  }

private:
  mutable wdInt32 m_iRefCount = 0; ///< Stores the current reference count
};

/// \brief Base class for reference counted objects.
class WD_FOUNDATION_DLL wdRefCounted : public wdRefCountingImpl
{
public:
  /// \brief Adds a virtual destructor.
  virtual ~wdRefCounted() = default;
};

/// \brief Stores a pointer to a reference counted object and automatically increases / decreases the reference count.
///
/// Note that no automatic deletion etc. happens, this is just to have shared base functionality for reference
/// counted objects. The actual action which, should happen once an object is no longer referenced, obliges
/// to the system that is using the objects.
template <typename T>
class wdScopedRefPointer
{
public:
  /// \brief Constructor.
  wdScopedRefPointer()
    : m_pReferencedObject(nullptr)
  {
  }

  /// \brief Constructor, increases the ref count of the given object.
  wdScopedRefPointer(T* pReferencedObject)
    : m_pReferencedObject(pReferencedObject)
  {
    AddReferenceIfValid();
  }

  wdScopedRefPointer(const wdScopedRefPointer<T>& other)
  {
    m_pReferencedObject = other.m_pReferencedObject;

    AddReferenceIfValid();
  }

  /// \brief Destructor - releases the reference on the ref-counted object (if there is one).
  ~wdScopedRefPointer() { ReleaseReferenceIfValid(); }

  /// \brief Assignment operator, decreases the ref count of the currently referenced object and increases the ref count of the newly
  /// assigned object.
  void operator=(T* pNewReference)
  {
    if (pNewReference == m_pReferencedObject)
      return;

    ReleaseReferenceIfValid();

    m_pReferencedObject = pNewReference;

    AddReferenceIfValid();
  }

  /// \brief Assignment operator, decreases the ref count of the currently referenced object and increases the ref count of the newly
  /// assigned object.
  void operator=(const wdScopedRefPointer<T>& other)
  {
    if (other.m_pReferencedObject == m_pReferencedObject)
      return;

    ReleaseReferenceIfValid();

    m_pReferencedObject = other.m_pReferencedObject;

    AddReferenceIfValid();
  }

  /// \brief Returns the referenced object (may be nullptr).
  operator const T*() const { return m_pReferencedObject; }

  /// \brief Returns the referenced object (may be nullptr).
  operator T*() { return m_pReferencedObject; }

  /// \brief Returns the referenced object (may be nullptr).
  const T* operator->() const
  {
    WD_ASSERT_DEV(m_pReferencedObject != nullptr, "Pointer is nullptr.");
    return m_pReferencedObject;
  }

  /// \brief Returns the referenced object (may be nullptr)
  T* operator->()
  {
    WD_ASSERT_DEV(m_pReferencedObject != nullptr, "Pointer is nullptr.");
    return m_pReferencedObject;
  }

private:
  /// \brief Internal helper function to add a reference on the current object (if != nullptr)
  inline void AddReferenceIfValid()
  {
    if (m_pReferencedObject != nullptr)
    {
      m_pReferencedObject->AddRef();
    }
  }

  /// \brief Internal helper function to release a reference on the current object (if != nullptr)
  inline void ReleaseReferenceIfValid()
  {
    if (m_pReferencedObject != nullptr)
    {
      m_pReferencedObject->ReleaseRef();
    }
  }

  T* m_pReferencedObject; ///< Stores a pointer to the referenced object
};


template <typename TYPE>
class wdRefCountedContainer : public wdRefCounted
{
public:
  TYPE m_Content;
};
