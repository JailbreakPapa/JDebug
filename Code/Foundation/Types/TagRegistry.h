
#pragma once

class wdHashedString;
class wdTempHashedString;
class wdTag;
class wdStreamWriter;
class wdStreamReader;

#include <Foundation/Containers/Map.h>
#include <Foundation/Threading/Mutex.h>

/// \brief The tag registry for tags in tag sets.
///
/// Normal usage of the tag registry is to get the global tag registry instance via wdTagRegistry::GetGlobalRegistry()
/// and to use this instance to register and get tags.
/// Certain special cases (e.g. tests) may actually need their own instance of the tag registry.
/// Note however that tags which were registered with one registry shouldn't be used with tag sets filled
/// with tags from another registry since there may be conflicting tag assignments.
/// The tag registry registration and tag retrieval functions are thread safe due to a mutex.
class WD_FOUNDATION_DLL wdTagRegistry
{
public:
  wdTagRegistry();

  static wdTagRegistry& GetGlobalRegistry();

  /// \brief Ensures the tag with the given name exists and returns a pointer to it.
  const wdTag& RegisterTag(wdStringView sTagString); // [tested]

  /// \brief Ensures the tag with the given name exists and returns a pointer to it.
  const wdTag& RegisterTag(const wdHashedString& sTagString); // [tested]

  /// \brief Searches for a tag with the given name and returns a pointer to it
  const wdTag* GetTagByName(const wdTempHashedString& sTagString) const; // [tested]

  /// \brief Searches for a tag with the given murmur hash. This function is only for backwards compatibility.
  const wdTag* GetTagByMurmurHash(wdUInt32 uiMurmurHash) const;

  /// \brief Returns the tag with the given index.
  const wdTag* GetTagByIndex(wdUInt32 uiIndex) const;

  /// \brief Returns the number of registered tags.
  wdUInt32 GetNumTags() const;

  /// \brief Loads the saved state and integrates it into this registry. Does not discard previously registered tag information. This function is only
  /// for backwards compatibility.
  wdResult Load(wdStreamReader& inout_stream);

protected:
  mutable wdMutex m_TagRegistryMutex;

  wdMap<wdTempHashedString, wdTag> m_RegisteredTags;
  wdDeque<wdTag*> m_TagsByIndex;
};
