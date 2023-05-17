#pragma once

#include <Foundation/Algorithm/HashingUtils.h>

inline wdHashedString::wdHashedString(const wdHashedString& rhs)
{
  m_Data = rhs.m_Data;

#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  // the string has a refcount of at least one (rhs holds a reference), thus it will definitely not get deleted on some other thread
  // therefore we can simply increase the refcount without locking
  m_Data.Value().m_iRefCount.Increment();
#endif
}

WD_FORCE_INLINE wdHashedString::wdHashedString(wdHashedString&& rhs)
{
  m_Data = rhs.m_Data;
  rhs.m_Data = HashedType(); // This leaves the string in an invalid state, all operations will fail except the destructor
}

inline wdHashedString::~wdHashedString()
{
#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  // Explicit check if data is still valid. It can be invalid if this string has been moved.
  if (m_Data.IsValid())
  {
    // just decrease the refcount of the object that we are set to, it might reach refcount zero, but we don't care about that here
    m_Data.Value().m_iRefCount.Decrement();
  }
#endif
}

inline void wdHashedString::operator=(const wdHashedString& rhs)
{
  // first increase the other refcount, then decrease ours
  HashedType tmp = rhs.m_Data;

#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  tmp.Value().m_iRefCount.Increment();

  m_Data.Value().m_iRefCount.Decrement();
#endif

  m_Data = tmp;
}

WD_FORCE_INLINE void wdHashedString::operator=(wdHashedString&& rhs)
{
#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  m_Data.Value().m_iRefCount.Decrement();
#endif

  m_Data = rhs.m_Data;
  rhs.m_Data = HashedType();
}

template <size_t N>
WD_FORCE_INLINE void wdHashedString::Assign(const char (&string)[N])
{
#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  HashedType tmp = m_Data;
#endif
  // this function will already increase the refcount as needed
  m_Data = AddHashedString(string, wdHashingUtils::StringHash(string));

#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  tmp.Value().m_iRefCount.Decrement();
#endif
}

WD_FORCE_INLINE void wdHashedString::Assign(wdStringView sString)
{
#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  HashedType tmp = m_Data;
#endif
  // this function will already increase the refcount as needed
  m_Data = AddHashedString(sString, wdHashingUtils::StringHash(sString));

#if WD_ENABLED(WD_HASHED_STRING_REF_COUNTING)
  tmp.Value().m_iRefCount.Decrement();
#endif
}

inline bool wdHashedString::operator==(const wdHashedString& rhs) const
{
  return m_Data == rhs.m_Data;
}

inline bool wdHashedString::operator!=(const wdHashedString& rhs) const
{
  return !(*this == rhs);
}

inline bool wdHashedString::operator==(const wdTempHashedString& rhs) const
{
  return m_Data.Key() == rhs.m_uiHash;
}

inline bool wdHashedString::operator!=(const wdTempHashedString& rhs) const
{
  return !(*this == rhs);
}

inline bool wdHashedString::operator<(const wdHashedString& rhs) const
{
  return m_Data.Key() < rhs.m_Data.Key();
}

inline bool wdHashedString::operator<(const wdTempHashedString& rhs) const
{
  return m_Data.Key() < rhs.m_uiHash;
}

WD_ALWAYS_INLINE const wdString& wdHashedString::GetString() const
{
  return m_Data.Value().m_sString;
}

WD_ALWAYS_INLINE const char* wdHashedString::GetData() const
{
  return m_Data.Value().m_sString.GetData();
}

WD_ALWAYS_INLINE wdUInt64 wdHashedString::GetHash() const
{
  return m_Data.Key();
}

template <size_t N>
WD_FORCE_INLINE wdHashedString wdMakeHashedString(const char (&string)[N])
{
  wdHashedString sResult;
  sResult.Assign(string);
  return sResult;
}

//////////////////////////////////////////////////////////////////////////

WD_ALWAYS_INLINE wdTempHashedString::wdTempHashedString()
{
  constexpr wdUInt64 uiEmptyHash = wdHashingUtils::StringHash("");
  m_uiHash = uiEmptyHash;
}

template <size_t N>
WD_ALWAYS_INLINE wdTempHashedString::wdTempHashedString(const char (&string)[N])
{
  m_uiHash = wdHashingUtils::StringHash<N>(string);
}

WD_ALWAYS_INLINE wdTempHashedString::wdTempHashedString(wdStringView sString)
{
  m_uiHash = wdHashingUtils::StringHash(sString);
}

WD_ALWAYS_INLINE wdTempHashedString::wdTempHashedString(const wdTempHashedString& rhs)
{
  m_uiHash = rhs.m_uiHash;
}

WD_ALWAYS_INLINE wdTempHashedString::wdTempHashedString(const wdHashedString& rhs)
{
  m_uiHash = rhs.GetHash();
}

WD_ALWAYS_INLINE wdTempHashedString::wdTempHashedString(wdUInt64 uiHash)
{
  m_uiHash = uiHash;
}

template <size_t N>
WD_ALWAYS_INLINE void wdTempHashedString::operator=(const char (&string)[N])
{
  m_uiHash = wdHashingUtils::StringHash<N>(string);
}

WD_ALWAYS_INLINE void wdTempHashedString::operator=(wdStringView sString)
{
  m_uiHash = wdHashingUtils::StringHash(sString);
}

WD_ALWAYS_INLINE void wdTempHashedString::operator=(const wdTempHashedString& rhs)
{
  m_uiHash = rhs.m_uiHash;
}

WD_ALWAYS_INLINE void wdTempHashedString::operator=(const wdHashedString& rhs)
{
  m_uiHash = rhs.GetHash();
}

WD_ALWAYS_INLINE bool wdTempHashedString::operator==(const wdTempHashedString& rhs) const
{
  return m_uiHash == rhs.m_uiHash;
}

WD_ALWAYS_INLINE bool wdTempHashedString::operator!=(const wdTempHashedString& rhs) const
{
  return !(m_uiHash == rhs.m_uiHash);
}

WD_ALWAYS_INLINE bool wdTempHashedString::operator<(const wdTempHashedString& rhs) const
{
  return m_uiHash < rhs.m_uiHash;
}

WD_ALWAYS_INLINE bool wdTempHashedString::IsEmpty() const
{
  constexpr wdUInt64 uiEmptyHash = wdHashingUtils::StringHash("");
  return m_uiHash == uiEmptyHash;
}

WD_ALWAYS_INLINE void wdTempHashedString::Clear()
{
  *this = wdTempHashedString();
}

WD_ALWAYS_INLINE wdUInt64 wdTempHashedString::GetHash() const
{
  return m_uiHash;
}

//////////////////////////////////////////////////////////////////////////

template <>
struct wdHashHelper<wdHashedString>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(const wdHashedString& value)
  {
    return wdHashingUtils::StringHashTo32(value.GetHash());
  }

  WD_ALWAYS_INLINE static wdUInt32 Hash(const wdTempHashedString& value)
  {
    return wdHashingUtils::StringHashTo32(value.GetHash());
  }

  WD_ALWAYS_INLINE static bool Equal(const wdHashedString& a, const wdHashedString& b) { return a == b; }

  WD_ALWAYS_INLINE static bool Equal(const wdHashedString& a, const wdTempHashedString& b) { return a == b; }
};

template <>
struct wdHashHelper<wdTempHashedString>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(const wdTempHashedString& value)
  {
    return wdHashingUtils::StringHashTo32(value.GetHash());
  }

  WD_ALWAYS_INLINE static bool Equal(const wdTempHashedString& a, const wdTempHashedString& b) { return a == b; }
};
