
#pragma once

class nsHashedString;
class nsTempHashedString;
class nsTag;
class nsStreamWriter;
class nsStreamReader;

#include <Foundation/Containers/Map.h>
#include <Foundation/Threading/Mutex.h>

/// \brief The tag registry for tags in tag sets.
///
/// Normal usage of the tag registry is to get the global tag registry instance via nsTagRegistry::GetGlobalRegistry()
/// and to use this instance to register and get tags.
/// Certain special cases (e.g. tests) may actually need their own instance of the tag registry.
/// Note however that tags which were registered with one registry shouldn't be used with tag sets filled
/// with tags from another registry since there may be conflicting tag assignments.
/// The tag registry registration and tag retrieval functions are thread safe due to a mutex.
class NS_FOUNDATION_DLL nsTagRegistry
{
public:
  nsTagRegistry();

  static nsTagRegistry& GetGlobalRegistry();

  /// \brief Ensures the tag with the given name exists and returns a pointer to it.
  const nsTag& RegisterTag(nsStringView sTagString); // [tested]

  /// \brief Ensures the tag with the given name exists and returns a pointer to it.
  const nsTag& RegisterTag(const nsHashedString& sTagString); // [tested]

  /// \brief Searches for a tag with the given name and returns a pointer to it
  const nsTag* GetTagByName(const nsTempHashedString& sTagString) const; // [tested]

  /// \brief Searches for a tag with the given murmur hash. This function is only for backwards compatibility.
  const nsTag* GetTagByMurmurHash(nsUInt32 uiMurmurHash) const;

  /// \brief Returns the tag with the given index.
  const nsTag* GetTagByIndex(nsUInt32 uiIndex) const;

  /// \brief Returns the number of registered tags.
  nsUInt32 GetNumTags() const;

  /// \brief Loads the saved state and integrates it into this registry. Does not discard previously registered tag information. This function is only
  /// for backwards compatibility.
  nsResult Load(nsStreamReader& inout_stream);

protected:
  mutable nsMutex m_TagRegistryMutex;

  nsMap<nsTempHashedString, nsTag> m_RegisteredTags;
  nsDeque<nsTag*> m_TagsByIndex;
};
