#pragma once

#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Tag.h>


// Template specialization to be able to use wdTagSet properties as WD_SET_MEMBER_PROPERTY.
template <typename T>
struct wdContainerSubTypeResolver<wdTagSetTemplate<T>>
{
  typedef const char* Type;
};

// Template specialization to be able to use wdTagSet properties as WD_SET_MEMBER_PROPERTY.
template <typename Class>
class wdMemberSetProperty<Class, wdTagSet, const char*> : public wdTypedSetProperty<typename wdTypeTraits<const char*>::NonConstReferenceType>
{
public:
  typedef wdTagSet Container;
  typedef wdConstCharPtr Type;
  typedef typename wdTypeTraits<Type>::NonConstReferenceType RealType;
  typedef const Container& (*GetConstContainerFunc)(const Class* pInstance);
  typedef Container& (*GetContainerFunc)(Class* pInstance);

  wdMemberSetProperty(const char* szPropertyName, GetConstContainerFunc constGetter, GetContainerFunc getter)
    : wdTypedSetProperty<RealType>(szPropertyName)
  {
    WD_ASSERT_DEBUG(constGetter != nullptr, "The const get count function of an set property cannot be nullptr.");

    m_ConstGetter = constGetter;
    m_Getter = getter;

    if (m_Getter == nullptr)
      wdAbstractSetProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }

  virtual bool IsEmpty(const void* pInstance) const override { return m_ConstGetter(static_cast<const Class*>(pInstance)).IsEmpty(); }

  virtual void Clear(void* pInstance) override
  {
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).Clear();
  }

  virtual void Insert(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).SetByName(*static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(m_Getter != nullptr, "The property '{0}' has no non-const set accessor function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    m_Getter(static_cast<Class*>(pInstance)).RemoveByName(*static_cast<const RealType*>(pObject));
  }

  virtual bool Contains(const void* pInstance, const void* pObject) const override
  {
    return m_ConstGetter(static_cast<const Class*>(pInstance)).IsSetByName(*static_cast<const RealType*>(pObject));
  }

  virtual void GetValues(const void* pInstance, wdDynamicArray<wdVariant>& out_keys) const override
  {
    out_keys.Clear();
    for (const auto& value : m_ConstGetter(static_cast<const Class*>(pInstance)))
    {
      out_keys.PushBack(wdVariant(value.GetTagString()));
    }
  }

private:
  GetConstContainerFunc m_ConstGetter;
  GetContainerFunc m_Getter;
};

// Template specialization to be able to use wdTagSet properties as WD_SET_ACCESSOR_PROPERTY.
template <typename Class>
class wdAccessorSetProperty<Class, const char*, const wdTagSet&> : public wdTypedSetProperty<const char*>
{
public:
  typedef const wdTagSet& Container;
  typedef wdConstCharPtr Type;

  using ContainerType = typename wdTypeTraits<Container>::NonConstReferenceType;
  using RealType = typename wdTypeTraits<Type>::NonConstReferenceType;

  using InsertFunc = void (Class::*)(Type value);
  using RemoveFunc = void (Class::*)(Type value);
  using GetValuesFunc = Container (Class::*)() const;

  wdAccessorSetProperty(const char* szPropertyName, GetValuesFunc getValues, InsertFunc insert, RemoveFunc remove)
    : wdTypedSetProperty<Type>(szPropertyName)
  {
    WD_ASSERT_DEBUG(getValues != nullptr, "The get values function of an set property cannot be nullptr.");

    m_GetValues = getValues;
    m_Insert = insert;
    m_Remove = remove;

    if (m_Insert == nullptr || m_Remove == nullptr)
      wdAbstractSetProperty::m_Flags.Add(wdPropertyFlags::ReadOnly);
  }


  virtual bool IsEmpty(const void* pInstance) const override { return (static_cast<const Class*>(pInstance)->*m_GetValues)().IsEmpty(); }

  virtual void Clear(void* pInstance) override
  {
    WD_ASSERT_DEBUG(m_Insert != nullptr && m_Remove != nullptr, "The property '{0}' has no remove and insert function, thus it is read-only",
      wdAbstractProperty::GetPropertyName());

    // We must not cache the container c here as the Remove can make it invalid
    // e.g. wdArrayPtr by value.
    while (!IsEmpty(pInstance))
    {
      // this should be decltype(auto) c = ...; but MSVC 16 is too dumb for that (MSVC 15 works fine)
      decltype((static_cast<const Class*>(pInstance)->*m_GetValues)()) c = (static_cast<const Class*>(pInstance)->*m_GetValues)();
      auto it = cbegin(c);
      const wdTag& value = *it;
      Remove(pInstance, value.GetTagString());
    }
  }

  virtual void Insert(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(m_Insert != nullptr, "The property '{0}' has no insert function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Insert)(*static_cast<const RealType*>(pObject));
  }

  virtual void Remove(void* pInstance, const void* pObject) override
  {
    WD_ASSERT_DEBUG(m_Remove != nullptr, "The property '{0}' has no setter function, thus it is read-only.", wdAbstractProperty::GetPropertyName());
    (static_cast<Class*>(pInstance)->*m_Remove)(*static_cast<const RealType*>(pObject));
  }

  virtual bool Contains(const void* pInstance, const void* pObject) const override
  {
    return (static_cast<const Class*>(pInstance)->*m_GetValues)().IsSetByName(*static_cast<const RealType*>(pObject));
  }

  virtual void GetValues(const void* pInstance, wdDynamicArray<wdVariant>& out_keys) const override
  {
    out_keys.Clear();
    for (const auto& value : (static_cast<const Class*>(pInstance)->*m_GetValues)())
    {
      out_keys.PushBack(wdVariant(value.GetTagString()));
    }
  }

private:
  GetValuesFunc m_GetValues;
  InsertFunc m_Insert;
  RemoveFunc m_Remove;
};


template <typename BlockStorageAllocator>
wdTagSetTemplate<BlockStorageAllocator>::Iterator::Iterator(const wdTagSetTemplate<BlockStorageAllocator>* pSet, bool bEnd)
  : m_pTagSet(pSet)
  , m_uiIndex(0)
{
  if (!bEnd)
  {
    m_uiIndex = m_pTagSet->GetTagBlockStart() * (sizeof(wdTagSetBlockStorage) * 8);

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
bool wdTagSetTemplate<BlockStorageAllocator>::Iterator::IsBitSet() const
{
  wdTag TempTag;
  TempTag.m_uiBlockIndex = m_uiIndex / (sizeof(wdTagSetBlockStorage) * 8);
  TempTag.m_uiBitIndex = m_uiIndex - (TempTag.m_uiBlockIndex * sizeof(wdTagSetBlockStorage) * 8);

  return m_pTagSet->IsSet(TempTag);
}

template <typename BlockStorageAllocator>
void wdTagSetTemplate<BlockStorageAllocator>::Iterator::operator++()
{
  const wdUInt32 uiMax = m_pTagSet->GetTagBlockEnd() * (sizeof(wdTagSetBlockStorage) * 8);

  do
  {
    ++m_uiIndex;
  } while (m_uiIndex < uiMax && !IsBitSet());

  if (m_uiIndex >= uiMax)
    m_uiIndex = 0xFFFFFFFF;
}

template <typename BlockStorageAllocator>
const wdTag& wdTagSetTemplate<BlockStorageAllocator>::Iterator::operator*() const
{
  return *wdTagRegistry::GetGlobalRegistry().GetTagByIndex(m_uiIndex);
}

template <typename BlockStorageAllocator>
const wdTag* wdTagSetTemplate<BlockStorageAllocator>::Iterator::operator->() const
{
  return wdTagRegistry::GetGlobalRegistry().GetTagByIndex(m_uiIndex);
}

template <typename BlockStorageAllocator>
wdTagSetTemplate<BlockStorageAllocator>::wdTagSetTemplate()
{
  SetTagBlockStart(wdSmallInvalidIndex);
  SetTagCount(0);
}

template <typename BlockStorageAllocator>
bool wdTagSetTemplate<BlockStorageAllocator>::operator==(const wdTagSetTemplate& other) const
{
  return m_TagBlocks == other.m_TagBlocks && m_TagBlocks.template GetUserData<wdUInt32>() == other.m_TagBlocks.template GetUserData<wdUInt32>();
}

template <typename BlockStorageAllocator>
bool wdTagSetTemplate<BlockStorageAllocator>::operator!=(const wdTagSetTemplate& other) const
{
  return !(*this == other);
}

template <typename BlockStorageAllocator>
void wdTagSetTemplate<BlockStorageAllocator>::Set(const wdTag& tag)
{
  WD_ASSERT_DEV(tag.IsValid(), "Only valid tags can be set in a tag set!");

  if (m_TagBlocks.IsEmpty())
  {
    Reallocate(tag.m_uiBlockIndex, tag.m_uiBlockIndex);
  }
  else if (IsTagInAllocatedRange(tag) == false)
  {
    const wdUInt32 uiNewBlockStart = wdMath::Min<wdUInt32>(tag.m_uiBlockIndex, GetTagBlockStart());
    const wdUInt32 uiNewBlockEnd = wdMath::Max<wdUInt32>(tag.m_uiBlockIndex, GetTagBlockEnd());

    Reallocate(uiNewBlockStart, uiNewBlockEnd);
  }

  wdUInt64& tagBlock = m_TagBlocks[tag.m_uiBlockIndex - GetTagBlockStart()];

  const wdUInt64 bitMask = WD_BIT(tag.m_uiBitIndex);
  const bool bBitWasSet = ((tagBlock & bitMask) != 0);

  tagBlock |= bitMask;

  if (!bBitWasSet)
  {
    IncreaseTagCount();
  }
}

template <typename BlockStorageAllocator>
void wdTagSetTemplate<BlockStorageAllocator>::Remove(const wdTag& tag)
{
  WD_ASSERT_DEV(tag.IsValid(), "Only valid tags can be cleared from a tag set!");

  if (IsTagInAllocatedRange(tag))
  {
    wdUInt64& tagBlock = m_TagBlocks[tag.m_uiBlockIndex - GetTagBlockStart()];

    const wdUInt64 bitMask = WD_BIT(tag.m_uiBitIndex);
    const bool bBitWasSet = ((tagBlock & bitMask) != 0);

    tagBlock &= ~bitMask;

    if (bBitWasSet)
    {
      DecreaseTagCount();
    }
  }
}

template <typename BlockStorageAllocator>
bool wdTagSetTemplate<BlockStorageAllocator>::IsSet(const wdTag& tag) const
{
  WD_ASSERT_DEV(tag.IsValid(), "Only valid tags can be checked!");

  if (IsTagInAllocatedRange(tag))
  {
    return (m_TagBlocks[tag.m_uiBlockIndex - GetTagBlockStart()] & WD_BIT(tag.m_uiBitIndex)) != 0;
  }
  else
  {
    return false;
  }
}

template <typename BlockStorageAllocator>
bool wdTagSetTemplate<BlockStorageAllocator>::IsAnySet(const wdTagSetTemplate& otherSet) const
{
  // If any of the sets is empty nothing can match
  if (IsEmpty() || otherSet.IsEmpty())
    return false;

  // Calculate range to compare
  const wdUInt32 uiMaxBlockStart = wdMath::Max(GetTagBlockStart(), otherSet.GetTagBlockStart());
  const wdUInt32 uiMinBlockEnd = wdMath::Min(GetTagBlockEnd(), otherSet.GetTagBlockEnd());

  if (uiMaxBlockStart > uiMinBlockEnd)
    return false;

  for (wdUInt32 i = uiMaxBlockStart; i < uiMinBlockEnd; ++i)
  {
    const wdUInt32 uiThisBlockStorageIndex = i - GetTagBlockStart();
    const wdUInt32 uiOtherBlockStorageIndex = i - otherSet.GetTagBlockStart();

    if ((m_TagBlocks[uiThisBlockStorageIndex] & otherSet.m_TagBlocks[uiOtherBlockStorageIndex]) != 0)
    {
      return true;
    }
  }

  return false;
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE wdUInt32 wdTagSetTemplate<BlockStorageAllocator>::GetNumTagsSet() const
{
  return GetTagCount();
}

template <typename BlockStorageAllocator>
WD_ALWAYS_INLINE bool wdTagSetTemplate<BlockStorageAllocator>::IsEmpty() const
{
  return GetTagCount() == 0;
}

template <typename BlockStorageAllocator>
void wdTagSetTemplate<BlockStorageAllocator>::Clear()
{
  m_TagBlocks.Clear();
  SetTagBlockStart(wdSmallInvalidIndex);
  SetTagCount(0);
}

template <typename BlockStorageAllocator>
void wdTagSetTemplate<BlockStorageAllocator>::SetByName(const char* szTag)
{
  const wdTag& tag = wdTagRegistry::GetGlobalRegistry().RegisterTag(szTag);
  Set(tag);
}

template <typename BlockStorageAllocator>
void wdTagSetTemplate<BlockStorageAllocator>::RemoveByName(const char* szTag)
{
  if (const wdTag* tag = wdTagRegistry::GetGlobalRegistry().GetTagByName(wdTempHashedString(szTag)))
  {
    Remove(*tag);
  }
}

template <typename BlockStorageAllocator>
bool wdTagSetTemplate<BlockStorageAllocator>::IsSetByName(const char* szTag) const
{
  if (const wdTag* tag = wdTagRegistry::GetGlobalRegistry().GetTagByName(wdTempHashedString(szTag)))
  {
    return IsSet(*tag);
  }

  return false;
}

template <typename BlockStorageAllocator>
WD_ALWAYS_INLINE bool wdTagSetTemplate<BlockStorageAllocator>::IsTagInAllocatedRange(const wdTag& Tag) const
{
  return Tag.m_uiBlockIndex >= GetTagBlockStart() && Tag.m_uiBlockIndex < GetTagBlockEnd();
}

template <typename BlockStorageAllocator>
void wdTagSetTemplate<BlockStorageAllocator>::Reallocate(wdUInt32 uiNewTagBlockStart, wdUInt32 uiNewMaxBlockIndex)
{
  WD_ASSERT_DEV(uiNewTagBlockStart < wdSmallInvalidIndex, "Tag block start is too big");
  const wdUInt16 uiNewBlockArraySize = static_cast<wdUInt16>((uiNewMaxBlockIndex - uiNewTagBlockStart) + 1);

  // Early out for non-filled tag sets
  if (m_TagBlocks.IsEmpty())
  {
    m_TagBlocks.SetCount(uiNewBlockArraySize);
    SetTagBlockStart(static_cast<wdUInt16>(uiNewTagBlockStart));

    return;
  }

  WD_ASSERT_DEBUG(uiNewTagBlockStart <= GetTagBlockStart(), "New block start must be smaller or equal to current block start!");

  wdSmallArray<wdUInt64, 32, BlockStorageAllocator> helperArray;
  helperArray.SetCount(uiNewBlockArraySize);

  const wdUInt32 uiOldBlockStartOffset = GetTagBlockStart() - uiNewTagBlockStart;

  // Copy old data to the new array
  wdMemoryUtils::Copy(helperArray.GetData() + uiOldBlockStartOffset, m_TagBlocks.GetData(), m_TagBlocks.GetCount());

  // Use array ptr copy assignment so it doesn't modify the user data in m_TagBlocks
  m_TagBlocks = helperArray.GetArrayPtr();
  SetTagBlockStart(static_cast<wdUInt16>(uiNewTagBlockStart));
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE wdUInt16 wdTagSetTemplate<BlockStorageAllocator>::GetTagBlockStart() const
{
  return m_TagBlocks.template GetUserData<UserData>().m_uiTagBlockStart;
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE wdUInt16 wdTagSetTemplate<BlockStorageAllocator>::GetTagBlockEnd() const
{
  return static_cast<wdUInt16>(GetTagBlockStart() + m_TagBlocks.GetCount());
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdTagSetTemplate<BlockStorageAllocator>::SetTagBlockStart(wdUInt16 uiTagBlockStart)
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagBlockStart = uiTagBlockStart;
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE wdUInt16 wdTagSetTemplate<BlockStorageAllocator>::GetTagCount() const
{
  return m_TagBlocks.template GetUserData<UserData>().m_uiTagCount;
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdTagSetTemplate<BlockStorageAllocator>::SetTagCount(wdUInt16 uiTagCount)
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagCount = uiTagCount;
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdTagSetTemplate<BlockStorageAllocator>::IncreaseTagCount()
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagCount++;
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdTagSetTemplate<BlockStorageAllocator>::DecreaseTagCount()
{
  m_TagBlocks.template GetUserData<UserData>().m_uiTagCount--;
}

static wdTypeVersion s_TagSetVersion = 1;

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
void wdTagSetTemplate<BlockStorageAllocator>::Save(wdStreamWriter& inout_stream) const
{
  const wdUInt16 uiNumTags = static_cast<wdUInt16>(GetNumTagsSet());
  inout_stream << uiNumTags;

  inout_stream.WriteVersion(s_TagSetVersion);

  for (Iterator it = GetIterator(); it.IsValid(); ++it)
  {
    const wdTag& tag = *it;

    inout_stream << tag.m_sTagString;
  }
}

template <typename BlockStorageAllocator /*= wdDefaultAllocatorWrapper*/>
void wdTagSetTemplate<BlockStorageAllocator>::Load(wdStreamReader& inout_stream, wdTagRegistry& inout_registry)
{
  wdUInt16 uiNumTags = 0;
  inout_stream >> uiNumTags;

  // Manually read version value since 0 can be a valid version here
  wdTypeVersion version;
  inout_stream.ReadWordValue(&version).IgnoreResult();

  if (version == 0)
  {
    for (wdUInt32 i = 0; i < uiNumTags; ++i)
    {
      wdUInt32 uiTagMurmurHash = 0;
      inout_stream >> uiTagMurmurHash;

      if (const wdTag* pTag = inout_registry.GetTagByMurmurHash(uiTagMurmurHash))
      {
        Set(*pTag);
      }
    }
  }
  else
  {
    for (wdUInt32 i = 0; i < uiNumTags; ++i)
    {
      wdHashedString tagString;
      inout_stream >> tagString;

      const wdTag& tag = inout_registry.RegisterTag(tagString);
      Set(tag);
    }
  }
}
