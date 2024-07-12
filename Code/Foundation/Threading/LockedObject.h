#pragma once

/// \brief Provides access to an object while managing a lock (e.g. a mutex) that ensures that during its lifetime the access to the object
/// happens under the lock.
template <typename T, typename O>
class nsLockedObject
{
public:
  NS_ALWAYS_INLINE explicit nsLockedObject(T& ref_lock, O* pObject)
    : m_pLock(&ref_lock)
    , m_pObject(pObject)
  {
    m_pLock->Lock();
  }

  nsLockedObject() = default;

  NS_ALWAYS_INLINE nsLockedObject(nsLockedObject<T, O>&& rhs) { *this = std::move(rhs); }

  nsLockedObject(const nsLockedObject<T, O>& rhs) = delete;

  void operator=(const nsLockedObject<T, O>&& rhs)
  {
    if (m_pLock)
    {
      m_pLock->Unlock();
    }

    m_pLock = rhs.m_pLock;
    rhs.m_pLock = nullptr;
    m_pObject = rhs.m_pObject;
    rhs.m_pObject = nullptr;
  }

  void operator=(const nsLockedObject<T, O>& rhs) = delete;

  NS_ALWAYS_INLINE ~nsLockedObject()
  {
    if (m_pLock)
    {
      m_pLock->Unlock();
    }
  }

  /// \brief Whether the encapsulated object exists at all or is nullptr
  NS_ALWAYS_INLINE bool isValid() const { return m_pObject != nullptr; }

  O* Borrow() { return m_pObject; }

  const O* Borrow() const { return m_pObject; }

  O* operator->() { return m_pObject; }

  const O* operator->() const { return m_pObject; }

  O& operator*() { return *m_pObject; }

  const O& operator*() const { return *m_pObject; }

  bool operator==(const O* rhs) const { return m_pObject == rhs; }

  bool operator!=(const O* rhs) const { return m_pObject != rhs; }

  bool operator!() const { return m_pObject == nullptr; }

  operator bool() const { return m_pObject != nullptr; }

private:
  mutable T* m_pLock = nullptr;
  mutable O* m_pObject = nullptr;
};
