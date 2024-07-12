#pragma once

#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Tag.h>


// Template specialization to be able to use nsTagSet properties as NS_SET_MEMBER_PROPERTY.
template <typename T>
struct nsContainerSubTypeResolver<nsTagSetTemplate<T>>
{
  using Type = const char*;
};

// Template specialization to be able to use nsTagSet properties as NS_SET_MEMBER_PROPERTY.
template <typename Class>
class nsMemberSetProperty<Class, nsTagSet, const char*> : public nsTypedSetProperty<typename nsTypeTraits<const char*>::NonConstReferenceType>
{
public:
  using Container = nsTagSet;
  using Type = nsConstCharPtr;
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;
  using GetConstContainerFunc = const Container& (*)(const Class*);
  using GetContainerFunc = Container& (*)(Class*);

  nsMemberSetProperty(const char* szPropertyName, GetConstContainerFunc constGetter, GetContainerFunc getter)
    : nsTypedSetProperty<RealType>(szPropertyName)
  {
    NS_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an set property cannot be nullptr.");

    m_ConstGetter = constGetter;
    m_Getter = getter;

    if (m_Getter == nullptr)
      nsAbstractSetProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }

  virtual bool IsEmpty(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).IsEmpty(); }

  virtual void Clear(void* pInstance) const override
  {
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Clear();
  }

  virtual void Insert(void* pInstance, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).SetByName(*static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).RemoveByName(*static_cast<const RealType*>(pObject));
  }

  virtual bool Contains(const void* pInstance, const void* pObject) const override
  {
    return m_ConstGetter(static_cast<const Class*>(pInstance)).IsSetByName(*static_cast<const RealType*>(pObject));
  }

  virtual void GetValues(const void* pInstance, nsDynamicArray<nsVariant>& out_keys) const override
  {
    out_keys.Clear();
    for (const auto& value : m_ConstGetter(static_cast<const Class*>(pInstance)))
    {
      out_keys.PushBack(nsVariant(value.GetTagString()));
    }
  }

private:
  GetConstContainerFunc m_ConstGetter;
  GetContainerFunc m_Getter;
};

// Template specialization to be able to use nsTagSet properties as NS_SET_ACCESSOR_PROPERTY.
template <typename Class>
class nsAccessorSetProperty<Class, const char*, const nsTagSet&> : public nsTypedSetProperty<const char*>
{
public:
  using Container = const nsTagSet&;
  using Type = nsConstCharPtr;

  using ContainerType = typename nsTypeTraits<Container>::NonConstReferenceType;
  using RealType = typename nsTypeTraits<Type>::NonConstReferenceType;

  using InsertFunc = void (Class::*)(Type value);
  using RemoveFunc = void (Class::*)(Type value);
  using GetValuesFunc = Container (Class::*)() const;

  nsAccessorSetProperty(const char* szPropertyName, GetValuesFunc getValues, InsertFunc insert, RemoveFunc remove)
    : nsTypedSetProperty<Type>(szPropertyName)
  {
    NS_ASSERT_DEBUG(getValues != nullptr, "The get values function of an set property cannot be nullptr.");

    m_GetValues = getValues;
    m_Insert = insert;
    m_Remove = remove;

    if (m_Insert == nullptr || m_Remove == nullptr)
      nsAbstractSetProperty::m_Flags.Add(nsPropertyFlags::ReadOnly);
  }


  virtual bool IsEmpty(const void* pInstance) const override { return (static_cast<const Class*>(pInstance)->*m_GetValues)().IsEmpty(); }

  virtual void Clear(void* pInstance) const override
  {
    NS_ASSERT_DEBUG(m_Insert != nullptr && m_Remove != nullptr, "The property '{0}' has no remove and insert function, thus it is read-only",
      nsAbstractProperty::GetPropertyName());

    // We must not cache the container c here as the Remove can make it invalid
    // e.g. nsArrayPtr by value.
    while (!IsEmpty(pInstance))
    {
      // this should be decltype(auto) c = ...; but MSVC 16 is too dumb for that (MSVC 15 works fine)
      decltype((static_cast<const Class*>(pInstance)->*m_GetValues)()) c = (static_cast<const Class*>(pInstance)->*m_GetValues)();
      auto it = cbegin(c);
      const nsTag& value = *it;
      Remove(pInstance, value.GetTagString().GetData());
    }
  }

  virtual void Insert(void* pInstance, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(m_Insert != nullptr, "The property '{0}' has no insert function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Insert)(*static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const void* pObject) const override
  {
    NS_ASSERT_DEBUG(m_Remove != nullptr, "The property '{0}' has no setter function, thus it is read-only.", nsAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Remove)(*static_cast<const RealType*>(pObject));
  }

  virtual bool Contains(const void* pInstance, const void* pObject) const override
  {
    return (static_cast<const Class*>(pInstance)->*m_GetValues)().IsSetByName(*static_cast<const RealType*>(pObject));
  }

  virtual void GetValues(const void* pInstance, nsDynamicArray<nsVariant>& out_keys) const override
  {
    out_keys.Clear();
    for (const auto& value : (static_cast<const Class*>(pInstance)->*m_GetValues)())
    {
      out_keys.PushBack(nsVariant(value.GetTagString()));
    }
  }

private:
  GetValuesFunc m_GetValues;
  InsertFunc m_Insert;
  RemoveFunc m_Remove;
};


template <typename BlockStorageAllocator>
nsTagSetTemplate<BlockStorageAllocator>::Iterator::Iterator(const nsTagSetTemplate<BlockStorageAllocator>* pSet, bool bEnd)
  : m_pTagSet(pSet)

{
  if (!bEnd)
  {
    m_uiIndex = m_pTagSet->GetTagBlockStart() * (sizeof(nsTagSetBlockStorage) * 8);

    if (m_pTagSet->IsEmpty())
      m_uiIndex = 0xFFFFFFFF;
    else
    {
      if (!IsBitSet())
        operator++();
    }
  }
  else
    m_uiIndex = 0xFFFFFFFF;
}

template <typename BlockStorageAllocator>
bool nsTagSetTemplate<BlockStorageAllocator>::Iterator::IsBitSet() const
{
  nsTag TempTag;
  TempTag.m_uiBlockIndex = m_uiIndex / (sizeof(nsTagSetBlockStorage) * 8);
  TempTag.m_uiBitIndex = m_uiIndex - (TempTag.m_uiBlockIndex * sizeof(nsTagSetBlockStorage) * 8);

  return m_pTagSet->IsSet(TempTag);
}

template <typename BlockStorageAllocator>
void nsTagSetTemplate<BlockStorageAllocator>::Iterator::operator++()
{
  const nsUInt32 uiMax = m_pTagSet->GetTagBlockEnd() * (sizeof(nsTagSetBlockStorage) * 8);

  do
  {
    ++m_uiIndex;
  } while (m_uiIndex < uiMax && !IsBitSet());

  if (m_uiIndex >= uiMax)
    m_uiIndex = 0xFFFFFFFF;
}

template <typename BlockStorageAllocator>
const nsTag& nsTagSetTemplate<BlockStorageAllocator>::Iterator::operator*() const
{
  return *nsTagRegistry::GetGlobalRegistry().GetTagByIndex(m_uiIndex);
}

template <typename BlockStorageAllocator>
const nsTag* nsTagSetTemplate<BlockStorageAllocator>::Iterator::operator->() const
{
  return nsTagRegistry::GetGlobalRegistry().GetTagByIndex(m_uiIndex);
}

template <typename BlockStorageAllocator>
nsTagSetTemplate<BlockStorageAllocator>::nsTagSetTemplate()
{
  SetTagBlockStart(nsSmallInvalidIndex);
  SetTagCount(0);
}

template <typename BlockStorageAllocator>
bool nsTagSetTemplate<BlockStorageAllocator>::operator==(const nsTagSetTemplate& other) const
{
  return m_TagBlocks == other.m_TagBlocks && m_TagBlocks.template GetUserData<nsUInt32>() == other.m_TagBlocks.template GetUserData<nsUInt32>();
}

template <typename BlockStorageAllocator>
bool nsTagSetTemplate<BlockStorageAllocator>::operator!=(const nsTagSetTemplate& other) const
{
  return !(*this == other);
}

template <typename BlockStorageAllocator>
void nsTagSetTemplate<BlockStorageAllocator>::Set(const nsTag& tag)
{
  NS_ASSERT_DEV(tag.IsValid(), "Only valid tags can be set in a tag set!");

  if (m_TagBlocks.IsEmpty())
  {
    Reallocate(tag.m_uiBlockIndex, tag.m_uiBlockIndex);
  }
  else if (IsTagInAllocatedRange(tag) == false)
  {
    const nsUInt32 uiNewBlockStart = nsMath::Min<nsUInt32>(tag.m_uiBlockIndex, GetTagBlockStart());
    const nsUInt32 uiNewBlockEnd = nsMath::Max<nsUInt32>(tag.m_uiBlockIndex, GetTagBlockEnd());

    Reallocate(uiNewBlockStart, uiNewBlockEnd);
  }

  nsUInt64& tagBlock = m_TagBlocks[tag.m_uiBlockIndex - GetTagBlockStart()];

  const nsUInt64 bitMask = NS_BIT(tag.m_uiBitIndex);
  const bool bBitWasSet = ((tagBlock & bitMask) != 0);

  tagBlock |= bitMask;

  if (!bBitWasSet)
  {
    IncreaseTagCount();
  }
}

template <typename BlockStorageAllocator>
void nsTagSetTemplate<BlockStorageAllocator>::Remove(const nsTag& tag)
{
  NS_ASSERT_DEV(tag.IsValid(), "Only valid tags can be cleared from a tag set!");

  if (IsTagInAllocatedRange(tag))
  {
    nsUInt64& tagBlock = m_TagBlocks[tag.m_uiBlockIndex - GetTagBlockStart()];

    const nsUInt64 bitMask = NS_BIT(tag.m_uiBitIndex);
    const bool bBitWasSet = ((tagBlock & bitMask) != 0);

    tagBlock &= ~bitMask;

    if (bBitWasSet)
    {
      DecreaseTagCount();
    }
  }
}

template <typename BlockStorageAllocator>
bool nsTagSetTemplate<BlockStorageAllocator>::IsSet(const nsTag& tag) const
{
  NS_ASSERT_DEV(tag.IsValid(), "Only valid tags can be checked!");

  if (IsTagInAllocatedRange(tag))
  {
    return (m_TagBlocks[tag.m_uiBlockIndex - GetTagBlockStart()] & NS_BIT(tag.m_uiBitIndex)) != 0;
  }
  else
  {
    return false;
  }
}

template <typename BlockStorageAllocator>
bool nsTagSetTemplate<BlockStorageAllocator>::IsAnySet(const nsTagSetTemplate& otherSet) const
{
  // If any of the sets is empty nothing can match
  if (IsEmpty() || otherSet.IsEmpty())
    return false;

  // Calculate range to compare
  const nsUInt32 uiMaxBlockStart = nsMath::Max(GetTagBlockStart(), otherSet.GetTagBlockStart());
  const nsUInt32 uiMinBlockEnd = nsMath::Min(GetTagBlockEnd(), otherSet.GetTagBlockEnd());

  if (uiMaxBlockStart > uiMinBlockEnd)
    return false;

  for (nsUInt32 i = uiMaxBlockStart; i < uiMinBlockEnd; ++i)
  {
    const nsUInt32 uiThisBlockStorageIndex = i - GetTagBlockStart();
    const nsUInt32 uiOtherBlockStorageIndex = i - otherSet.GetTagBlockStart();

    if ((m_TagBlocks[uiThisBlockStorageIndex] & otherSet.m_TagBlocks[uiOtherBlockStorageIndex]) != 0)
    {
      return true;
    }
  }

  return false;
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE nsUInt32 nsTagSetTemplate<BlockStorageAllocator>::GetNumTagsSet() const
{
  return GetTagCount();
}

template <typename BlockStorageAllocator>
NS_ALWAYS_INLINE bool nsTagSetTemplate<BlockStorageAllocator>::IsEmpty() const
{
  return GetTagCount() == 0;
}

template <typename BlockStorageAllocator>
void nsTagSetTemplate<BlockStorageAllocator>::Clear()
{
  m_TagBlocks.Clear();
  SetTagBlockStart(nsSmallInvalidIndex);
  SetTagCount(0);
}

template <typename BlockStorageAllocator>
void nsTagSetTemplate<BlockStorageAllocator>::SetByName(nsStringView sTag)
{
  const nsTag& tag = nsTagRegistry::GetGlobalRegistry().RegisterTag(sTag);
  Set(tag);
}

template <typename BlockStorageAllocator>
void nsTagSetTemplate<BlockStorageAllocator>::RemoveByName(nsStringView sTag)
{
  if (const nsTag* tag = nsTagRegistry::GetGlobalRegistry().GetTagByName(nsTempHashedString(sTag)))
  {
    Remove(*tag);
  }
}

template <typename BlockStorageAllocator>
bool nsTagSetTemplate<BlockStorageAllocator>::IsSetByName(nsStringView sTag) const
{
  if (const nsTag* tag = nsTagRegistry::GetGlobalRegistry().GetTagByName(nsTempHashedString(sTag)))
  {
    return IsSet(*tag);
  }

  return false;
}

template <typename BlockStorageAllocator>
NS_ALWAYS_INLINE bool nsTagSetTemplate<BlockStorageAllocator>::IsTagInAllocatedRange(const nsTag& Tag) const
{
  return Tag.m_uiBlockIndex >= GetTagBlockStart() && Tag.m_uiBlockIndex < GetTagBlockEnd();
}

template <typename BlockStorageAllocator>
void nsTagSetTemplate<BlockStorageAllocator>::Reallocate(nsUInt32 uiNewTagBlockStart, nsUInt32 uiNewMaxBlockIndex)
{
  NS_ASSERT_DEV(uiNewTagBlockStart < nsSmallInvalidIndex, "Tag block start is too big");
  const nsUInt16 uiNewBlockArraySize = static_cast<nsUInt16>((uiNewMaxBlockIndex - uiNewTagBlockStart) + 1);

  // Early out for non-filled tag sets
  if (m_TagBlocks.IsEmpty())
  {
    m_TagBlocks.SetCount(uiNewBlockArraySize);
    SetTagBlockStart(static_cast<nsUInt16>(uiNewTagBlockStart));

    return;
  }

  NS_ASSERT_DEBUG(uiNewTagBlockStart <= GetTagBlockStart(), "New block start must be smaller or equal to current block start!");

  nsSmallArray<nsUInt64, 32, BlockStorageAllocator> helperArray;
  helperArray.SetCount(uiNewBlockArraySize);

  const nsUInt32 uiOldBlockStartOffset = GetTagBlockStart() - uiNewTagBlockStart;

  // Copy old data to the new array
  nsMemoryUtils::Copy(helperArray.GetData() + uiOldBlockStartOffset, m_TagBlocks.GetData(), m_TagBlocks.GetCount());

  // Use array ptr copy assignment so it doesn't modify the user data in m_TagBlocks
  m_TagBlocks = helperArray.GetArrayPtr();
  SetTagBlockStart(static_cast<nsUInt16>(uiNewTagBlockStart));
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE nsUInt16 nsTagSetTemplate<BlockStorageAllocator>::GetTagBlockStart() const
{
  return m_TagBlocks.template GetUserData<UserData>().m_uiTagBlockStart;
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE nsUInt16 nsTagSetTemplate<BlockStorageAllocator>::GetTagBlockEnd() const
{
  return static_cast<nsUInt16>(GetTagBlockStart() + m_TagBlocks.GetCount());
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsTagSetTemplate<BlockStorageAllocator>::SetTagBlockStart(nsUInt16 uiTagBlockStart)
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagBlockStart = uiTagBlockStart;
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE nsUInt16 nsTagSetTemplate<BlockStorageAllocator>::GetTagCount() const
{
  return m_TagBlocks.template GetUserData<UserData>().m_uiTagCount;
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsTagSetTemplate<BlockStorageAllocator>::SetTagCount(nsUInt16 uiTagCount)
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagCount = uiTagCount;
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsTagSetTemplate<BlockStorageAllocator>::IncreaseTagCount()
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagCount++;
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsTagSetTemplate<BlockStorageAllocator>::DecreaseTagCount()
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagCount--;
}

static nsTypeVersion s_TagSetVersion = 1;

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
void nsTagSetTemplate<BlockStorageAllocator>::Save(nsStreamWriter& inout_stream) const
{
  const nsUInt16 uiNumTags = static_cast<nsUInt16>(GetNumTagsSet());
  inout_stream << uiNumTags;

  inout_stream.WriteVersion(s_TagSetVersion);

  for (Iterator it = GetIterator(); it.IsValid(); ++it)
  {
    const nsTag& tag = *it;

    inout_stream << tag.m_sTagString;
  }
}

template <typename BlockStorageAllocator /*= nsDefaultAllocatorWrapper*/>
void nsTagSetTemplate<BlockStorageAllocator>::Load(nsStreamReader& inout_stream, nsTagRegistry& inout_registry)
{
  nsUInt16 uiNumTags = 0;
  inout_stream >> uiNumTags;

  // Manually read version value since 0 can be a valid version here
  nsTypeVersion version;
  inout_stream.ReadWordValue(&version).IgnoreResult();

  if (version == 0)
  {
    for (nsUInt32 i = 0; i < uiNumTags; ++i)
    {
      nsUInt32 uiTagMurmurHash = 0;
      inout_stream >> uiTagMurmurHash;

      if (const nsTag* pTag = inout_registry.GetTagByMurmurHash(uiTagMurmurHash))
      {
        Set(*pTag);
      }
    }
  }
  else
  {
    for (nsUInt32 i = 0; i < uiNumTags; ++i)
    {
      nsHashedString tagString;
      inout_stream >> tagString;

      const nsTag& tag = inout_registry.RegisterTag(tagString);
      Set(tag);
    }
  }
}
