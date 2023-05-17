
#pragma once

#include <Foundation/Containers/SmallArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/TagRegistry.h>

class wdTag;
typedef wdUInt64 wdTagSetBlockStorage;

/// \brief A dynamic collection of tags featuring fast lookups.
///
/// This class can be used to store a (dynamic) collection of tags. Tags are registered within
/// the global tag registry and allocated a bit index. The tag set allows comparatively fast lookups
/// to check if a given tag is in the set or not.
/// Adding a tag may have some overhead depending whether the block storage for the tag
/// bit indices needs to be expanded or not (if the storage needs to be expanded the hybrid array will be resized).
/// Typical storage requirements for a given tag set instance should be small since the block storage is a sliding
/// window. The standard class which can be used is wdTagSet, usage of wdTagSetTemplate is only necessary
/// if the allocator needs to be overridden.
template <typename BlockStorageAllocator = wdDefaultAllocatorWrapper>
class wdTagSetTemplate
{
public:
  wdTagSetTemplate();

  bool operator==(const wdTagSetTemplate& other) const;
  bool operator!=(const wdTagSetTemplate& other) const;

  /// \brief Adds the given tag to the set.
  void Set(const wdTag& tag); // [tested]

  /// \brief Removes the given tag.
  void Remove(const wdTag& tag); // [tested]

  /// \brief Returns true, if the given tag is in the set.
  bool IsSet(const wdTag& tag) const; // [tested]

  /// \brief Returns true if this tag set contains any tag set in the given other tag set.
  bool IsAnySet(const wdTagSetTemplate& otherSet) const; // [tested]

  /// \brief Returns how many tags are in this set.
  wdUInt32 GetNumTagsSet() const;

  /// \brief True if the tag set never contained any tag or was cleared.
  bool IsEmpty() const;

  /// \brief Removes all tags from the set
  void Clear();

  /// \brief Adds the tag with the given name. If the tag does not exist, it will be registered.
  void SetByName(const char* szTag);

  /// \brief Removes the given tag. If it doesn't exist, nothing happens.
  void RemoveByName(const char* szTag);

  /// \brief Checks whether the named tag is part of this set. Returns false if the tag does not exist.
  bool IsSetByName(const char* szTag) const;

  /// \brief Allows to iterate over all tags in this set
  class Iterator
  {
  public:
    Iterator(const wdTagSetTemplate<BlockStorageAllocator>* pSet, bool bEnd = false);

    /// \brief Returns a reference to the current tag
    const wdTag& operator*() const;

    /// \brief Returns a pointer to the current tag
    const wdTag* operator->() const;

    /// \brief Returns whether the iterator is still pointing to a valid item
    WD_ALWAYS_INLINE bool IsValid() const { return m_uiIndex != 0xFFFFFFFF; }

    WD_ALWAYS_INLINE bool operator!=(const Iterator& rhs) const { return m_pTagSet != rhs.m_pTagSet || m_uiIndex != rhs.m_uiIndex; }

    /// \brief Advances the iterator to the next item
    void operator++();

  private:
    bool IsBitSet() const;

    const wdTagSetTemplate<BlockStorageAllocator>* m_pTagSet;
    wdUInt32 m_uiIndex;
  };

  /// \brief Returns an iterator to list all tags in this set
  Iterator GetIterator() const { return Iterator(this); }

  /// \brief Writes the tag set state to a stream. Tags itself are serialized as strings.
  void Save(wdStreamWriter& inout_stream) const;

  /// \brief Reads the tag set state from a stream and registers the tags with the given registry.
  void Load(wdStreamReader& inout_stream, wdTagRegistry& inout_registry);

private:
  friend class Iterator;

  bool IsTagInAllocatedRange(const wdTag& Tag) const;

  void Reallocate(wdUInt32 uiNewTagBlockStart, wdUInt32 uiNewMaxBlockIndex);

  wdSmallArray<wdTagSetBlockStorage, 1, BlockStorageAllocator> m_TagBlocks;

  struct UserData
  {
    wdUInt16 m_uiTagBlockStart;
    wdUInt16 m_uiTagCount;
  };

  wdUInt16 GetTagBlockStart() const;
  wdUInt16 GetTagBlockEnd() const;
  void SetTagBlockStart(wdUInt16 uiTagBlockStart);

  wdUInt16 GetTagCount() const;
  void SetTagCount(wdUInt16 uiTagCount);
  void IncreaseTagCount();
  void DecreaseTagCount();
};

/// Default tag set, uses wdDefaultAllocatorWrapper for allocations.
typedef wdTagSetTemplate<> wdTagSet;

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdTagSet);

template <typename BlockStorageAllocator>
typename wdTagSetTemplate<BlockStorageAllocator>::Iterator cbegin(const wdTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename wdTagSetTemplate<BlockStorageAllocator>::Iterator(&cont);
}

template <typename BlockStorageAllocator>
typename wdTagSetTemplate<BlockStorageAllocator>::Iterator cend(const wdTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename wdTagSetTemplate<BlockStorageAllocator>::Iterator(&cont, true);
}

template <typename BlockStorageAllocator>
typename wdTagSetTemplate<BlockStorageAllocator>::Iterator begin(const wdTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename wdTagSetTemplate<BlockStorageAllocator>::Iterator(&cont);
}

template <typename BlockStorageAllocator>
typename wdTagSetTemplate<BlockStorageAllocator>::Iterator end(const wdTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename wdTagSetTemplate<BlockStorageAllocator>::Iterator(&cont, true);
}

#include <Foundation/Types/Implementation/TagSet_inl.h>
