#pragma once

#include <Foundation/Algorithm/HashingUtils.h>

inline nsHashedString::nsHashedString(const nsHashedString& rhs)
{
  m_Data = rhs.m_Data;

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  // the string has a refcount of at least one (rhs holds a reference), thus it will definitely not get deleted on some other thread
  // therefore we can simply increase the refcount without locking
  m_Data.Value().m_iRefCount.Increment();
#endif
}

NS_FORCE_INLINE nsHashedString::nsHashedString(nsHashedString&& rhs)
{
  m_Data = rhs.m_Data;
  rhs.m_Data = HashedType(); // This leaves the string in an invalid state, all operations will fail except the destructor
}

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
inline nsHashedString::~nsHashedString()
{
  // Explicit check if data is still valid. It can be invalid if this string has been moved.
  if (m_Data.IsValid())
  {
    // just decrease the refcount of the object that we are set to, it might reach refcount zero, but we don't care about that here
    m_Data.Value().m_iRefCount.Decrement();
  }
}
#endif

inline void nsHashedString::operator=(const nsHashedString& rhs)
{
  // first increase the other refcount, then decrease ours
  HashedType tmp = rhs.m_Data;

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  tmp.Value().m_iRefCount.Increment();

  m_Data.Value().m_iRefCount.Decrement();
#endif

  m_Data = tmp;
}

NS_FORCE_INLINE void nsHashedString::operator=(nsHashedString&& rhs)
{
#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  m_Data.Value().m_iRefCount.Decrement();
#endif

  m_Data = rhs.m_Data;
  rhs.m_Data = HashedType();
}

template <size_t N>
NS_FORCE_INLINE void nsHashedString::Assign(const char (&string)[N])
{
#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  HashedType tmp = m_Data;
#endif
  // this function will already increase the refcount as needed
  m_Data = AddHashedString(string, nsHashingUtils::StringHash(string));

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  tmp.Value().m_iRefCount.Decrement();
#endif
}

NS_FORCE_INLINE void nsHashedString::Assign(nsStringView sString)
{
#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  HashedType tmp = m_Data;
#endif
  // this function will already increase the refcount as needed
  m_Data = AddHashedString(sString, nsHashingUtils::StringHash(sString));

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  tmp.Value().m_iRefCount.Decrement();
#endif
}

inline bool nsHashedString::operator==(const nsHashedString& rhs) const
{
  return m_Data == rhs.m_Data;
}

inline bool nsHashedString::operator==(const nsTempHashedString& rhs) const
{
  return m_Data.Key() == rhs.m_uiHash;
}

inline bool nsHashedString::operator<(const nsHashedString& rhs) const
{
  return m_Data.Key() < rhs.m_Data.Key();
}

inline bool nsHashedString::operator<(const nsTempHashedString& rhs) const
{
  return m_Data.Key() < rhs.m_uiHash;
}

NS_ALWAYS_INLINE const nsString& nsHashedString::GetString() const
{
  return m_Data.Value().m_sString;
}

NS_ALWAYS_INLINE const char* nsHashedString::GetData() const
{
  return m_Data.Value().m_sString.GetData();
}

NS_ALWAYS_INLINE nsUInt64 nsHashedString::GetHash() const
{
  return m_Data.Key();
}

template <size_t N>
NS_FORCE_INLINE nsHashedString nsMakeHashedString(const char (&string)[N])
{
  nsHashedString sResult;
  sResult.Assign(string);
  return sResult;
}

//////////////////////////////////////////////////////////////////////////

NS_ALWAYS_INLINE nsTempHashedString::nsTempHashedString()
{
  constexpr nsUInt64 uiEmptyHash = nsHashingUtils::StringHash("");
  m_uiHash = uiEmptyHash;
}

template <size_t N>
NS_ALWAYS_INLINE nsTempHashedString::nsTempHashedString(const char (&string)[N])
{
  m_uiHash = nsHashingUtils::StringHash<N>(string);
}

NS_ALWAYS_INLINE nsTempHashedString::nsTempHashedString(nsStringView sString)
{
  m_uiHash = nsHashingUtils::StringHash(sString);
}

NS_ALWAYS_INLINE nsTempHashedString::nsTempHashedString(const nsTempHashedString& rhs)
{
  m_uiHash = rhs.m_uiHash;
}

NS_ALWAYS_INLINE nsTempHashedString::nsTempHashedString(const nsHashedString& rhs)
{
  m_uiHash = rhs.GetHash();
}

NS_ALWAYS_INLINE nsTempHashedString::nsTempHashedString(nsUInt64 uiHash)
{
  m_uiHash = uiHash;
}

template <size_t N>
NS_ALWAYS_INLINE void nsTempHashedString::operator=(const char (&string)[N])
{
  m_uiHash = nsHashingUtils::StringHash<N>(string);
}

NS_ALWAYS_INLINE void nsTempHashedString::operator=(nsStringView sString)
{
  m_uiHash = nsHashingUtils::StringHash(sString);
}

NS_ALWAYS_INLINE void nsTempHashedString::operator=(const nsTempHashedString& rhs)
{
  m_uiHash = rhs.m_uiHash;
}

NS_ALWAYS_INLINE void nsTempHashedString::operator=(const nsHashedString& rhs)
{
  m_uiHash = rhs.GetHash();
}

NS_ALWAYS_INLINE bool nsTempHashedString::operator==(const nsTempHashedString& rhs) const
{
  return m_uiHash == rhs.m_uiHash;
}

NS_ALWAYS_INLINE bool nsTempHashedString::operator<(const nsTempHashedString& rhs) const
{
  return m_uiHash < rhs.m_uiHash;
}

NS_ALWAYS_INLINE bool nsTempHashedString::IsEmpty() const
{
  constexpr nsUInt64 uiEmptyHash = nsHashingUtils::StringHash("");
  return m_uiHash == uiEmptyHash;
}

NS_ALWAYS_INLINE void nsTempHashedString::Clear()
{
  *this = nsTempHashedString();
}

NS_ALWAYS_INLINE nsUInt64 nsTempHashedString::GetHash() const
{
  return m_uiHash;
}

//////////////////////////////////////////////////////////////////////////

template <>
struct nsHashHelper<nsHashedString>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsHashedString& value)
  {
    return nsHashingUtils::StringHashTo32(value.GetHash());
  }

  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsTempHashedString& value)
  {
    return nsHashingUtils::StringHashTo32(value.GetHash());
  }

  NS_ALWAYS_INLINE static bool Equal(const nsHashedString& a, const nsHashedString& b) { return a == b; }

  NS_ALWAYS_INLINE static bool Equal(const nsHashedString& a, const nsTempHashedString& b) { return a == b; }
};

template <>
struct nsHashHelper<nsTempHashedString>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsTempHashedString& value)
  {
    return nsHashingUtils::StringHashTo32(value.GetHash());
  }

  NS_ALWAYS_INLINE static bool Equal(const nsTempHashedString& a, const nsTempHashedString& b) { return a == b; }
};
