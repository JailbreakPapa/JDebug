
#pragma once

#include <Foundation/Containers/SmallArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/TagRegistry.h>

class nsTag;
using nsTagSetBlockStorage = nsUInt64;

/// \brief A dynamic collection of tags featuring fast lookups.
///
/// This class can be used to store a (dynamic) collection of tags. Tags are registered within
/// the global tag registry and allocated a bit index. The tag set allows comparatively fast lookups
/// to check if a given tag is in the set or not.
/// Adding a tag may have some overhead depending whether the block storage for the tag
/// bit indices needs to be expanded or not (if the storage needs to be expanded the hybrid array will be resized).
/// Typical storage requirements for a given tag set instance should be small since the block storage is a sliding
/// window. The standard class which can be used is nsTagSet, usage of nsTagSetTemplate is only necessary
/// if the allocator needs to be overridden.
template <typename BlockStorageAllocator = nsDefaultAllocatorWrapper>
class nsTagSetTemplate
{
public:
  nsTagSetTemplate();

  bool operator==(const nsTagSetTemplate& other) const;
  bool operator!=(const nsTagSetTemplate& other) const;

  /// \brief Adds the given tag to the set.
  void Set(const nsTag& tag); // [tested]

  /// \brief Removes the given tag.
  void Remove(const nsTag& tag); // [tested]

  /// \brief Returns true, if the given tag is in the set.
  bool IsSet(const nsTag& tag) const; // [tested]

  /// \brief Returns true if this tag set contains any tag set in the given other tag set.
  bool IsAnySet(const nsTagSetTemplate& otherSet) const; // [tested]

  /// \brief Returns how many tags are in this set.
  nsUInt32 GetNumTagsSet() const;

  /// \brief True if the tag set never contained any tag or was cleared.
  bool IsEmpty() const;

  /// \brief Removes all tags from the set
  void Clear();

  /// \brief Adds the tag with the given name. If the tag does not exist, it will be registered.
  void SetByName(nsStringView sTag);

  /// \brief Removes the given tag. If it doesn't exist, nothing happens.
  void RemoveByName(nsStringView sTag);

  /// \brief Checks whether the named tag is part of this set. Returns false if the tag does not exist.
  bool IsSetByName(nsStringView sTag) const;

  /// \brief Allows to iterate over all tags in this set
  class Iterator
  {
  public:
    Iterator(const nsTagSetTemplate<BlockStorageAllocator>* pSet, bool bEnd = false);

    /// \brief Returns a reference to the current tag
    const nsTag& operator*() const;

    /// \brief Returns a pointer to the current tag
    const nsTag* operator->() const;

    /// \brief Returns whether the iterator is still pointing to a valid item
    NS_ALWAYS_INLINE bool IsValid() const { return m_uiIndex != 0xFFFFFFFF; }

    NS_ALWAYS_INLINE bool operator!=(const Iterator& rhs) const { return m_pTagSet != rhs.m_pTagSet || m_uiIndex != rhs.m_uiIndex; }

    /// \brief Advances the iterator to the next item
    void operator++();

  private:
    bool IsBitSet() const;

    const nsTagSetTemplate<BlockStorageAllocator>* m_pTagSet;
    nsUInt32 m_uiIndex = 0;
  };

  /// \brief Returns an iterator to list all tags in this set
  Iterator GetIterator() const { return Iterator(this); }

  /// \brief Writes the tag set state to a stream. Tags itself are serialized as strings.
  void Save(nsStreamWriter& inout_stream) const;

  /// \brief Reads the tag set state from a stream and registers the tags with the given registry.
  void Load(nsStreamReader& inout_stream, nsTagRegistry& inout_registry);

private:
  friend class Iterator;

  bool IsTagInAllocatedRange(const nsTag& Tag) const;

  void Reallocate(nsUInt32 uiNewTagBlockStart, nsUInt32 uiNewMaxBlockIndex);

  nsSmallArray<nsTagSetBlockStorage, 1, BlockStorageAllocator> m_TagBlocks;

  struct UserData
  {
    nsUInt16 m_uiTagBlockStart;
    nsUInt16 m_uiTagCount;
  };

  nsUInt16 GetTagBlockStart() const;
  nsUInt16 GetTagBlockEnd() const;
  void SetTagBlockStart(nsUInt16 uiTagBlockStart);

  nsUInt16 GetTagCount() const;
  void SetTagCount(nsUInt16 uiTagCount);
  void IncreaseTagCount();
  void DecreaseTagCount();
};

/// Default tag set, uses nsDefaultAllocatorWrapper for allocations.
using nsTagSet = nsTagSetTemplate<>;

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsTagSet);

template <typename BlockStorageAllocator>
typename nsTagSetTemplate<BlockStorageAllocator>::Iterator cbegin(const nsTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename nsTagSetTemplate<BlockStorageAllocator>::Iterator(&cont);
}

template <typename BlockStorageAllocator>
typename nsTagSetTemplate<BlockStorageAllocator>::Iterator cend(const nsTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename nsTagSetTemplate<BlockStorageAllocator>::Iterator(&cont, true);
}

template <typename BlockStorageAllocator>
typename nsTagSetTemplate<BlockStorageAllocator>::Iterator begin(const nsTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename nsTagSetTemplate<BlockStorageAllocator>::Iterator(&cont);
}

template <typename BlockStorageAllocator>
typename nsTagSetTemplate<BlockStorageAllocator>::Iterator end(const nsTagSetTemplate<BlockStorageAllocator>& cont)
{
  return typename nsTagSetTemplate<BlockStorageAllocator>::Iterator(&cont, true);
}

#include <Foundation/Types/Implementation/TagSet_inl.h>
