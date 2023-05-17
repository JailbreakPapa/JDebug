
#pragma once

#include <Foundation/Strings/HashedString.h>

typedef wdUInt64 wdTagSetBlockStorage;

/// \brief The tag class stores the necessary lookup information for a single tag which can be used in conjunction with the tag set.
///
/// A tag is the storage for a small amount of lookup information for a single tag. Instances
/// of wdTag can be used in checks with the tag set. Note that fetching information for the tag needs to access
/// the global tag registry which involves a mutex lock. It is thus
/// recommended to fetch tag instances early and reuse them for the actual tests and to avoid querying the tag registry
/// all the time (e.g. due to tag instances being kept on the stack).
class WD_FOUNDATION_DLL wdTag
{
public:
  WD_ALWAYS_INLINE wdTag();

  WD_ALWAYS_INLINE bool operator==(const wdTag& rhs) const; // [tested]

  WD_ALWAYS_INLINE bool operator!=(const wdTag& rhs) const; // [tested]

  WD_ALWAYS_INLINE bool operator<(const wdTag& rhs) const;

  WD_ALWAYS_INLINE const wdString& GetTagString() const; // [tested]

  WD_ALWAYS_INLINE bool IsValid() const; // [tested]

private:
  template <typename BlockStorageAllocator>
  friend class wdTagSetTemplate;
  friend class wdTagRegistry;

  wdHashedString m_sTagString;

  wdUInt32 m_uiBitIndex;
  wdUInt32 m_uiBlockIndex;
};

#include <Foundation/Types/TagSet.h>

#include <Foundation/Types/Implementation/Tag_inl.h>
