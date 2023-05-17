
#include <Foundation/IO/Stream.h>

template <typename T>
WD_ALWAYS_INLINE wdResult wdDeduplicationReadContext::ReadObjectInplace(wdStreamReader& inout_stream, T& inout_obj)
{
  return ReadObject(inout_stream, inout_obj, nullptr);
}

template <typename T>
wdResult wdDeduplicationReadContext::ReadObject(wdStreamReader& inout_stream, T& obj, wdAllocatorBase* pAllocator)
{
  bool bIsRealObject;
  inout_stream >> bIsRealObject;

  WD_ASSERT_DEV(bIsRealObject, "Reading an object inplace only works for the first occurrence");

  WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize<T>(inout_stream, obj));

  m_Objects.PushBack(&obj);

  return WD_SUCCESS;
}

template <typename T>
wdResult wdDeduplicationReadContext::ReadObject(wdStreamReader& inout_stream, T*& ref_pObject, wdAllocatorBase* pAllocator)
{
  bool bIsRealObject;
  inout_stream >> bIsRealObject;

  if (bIsRealObject)
  {
    ref_pObject = WD_NEW(pAllocator, T);
    WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize<T>(inout_stream, *ref_pObject));

    m_Objects.PushBack(ref_pObject);
  }
  else
  {
    wdUInt32 uiIndex;
    inout_stream >> uiIndex;

    if (uiIndex < m_Objects.GetCount())
    {
      ref_pObject = static_cast<T*>(m_Objects[uiIndex]);
    }
    else if (uiIndex == wdInvalidIndex)
    {
      ref_pObject = nullptr;
    }
    else
    {
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}

template <typename T>
wdResult wdDeduplicationReadContext::ReadObject(wdStreamReader& inout_stream, wdSharedPtr<T>& ref_pObject, wdAllocatorBase* pAllocator)
{
  T* ptr = nullptr;
  if (ReadObject(inout_stream, ptr, pAllocator).Succeeded())
  {
    ref_pObject = wdSharedPtr<T>(ptr, pAllocator);
    return WD_SUCCESS;
  }
  return WD_FAILURE;
}

template <typename T>
wdResult wdDeduplicationReadContext::ReadObject(wdStreamReader& inout_stream, wdUniquePtr<T>& ref_pObject, wdAllocatorBase* pAllocator)
{
  T* ptr = nullptr;
  if (ReadObject(inout_stream, ptr, pAllocator).Succeeded())
  {
    ref_pObject = std::move(wdUniquePtr<T>(ptr, pAllocator));
    return WD_SUCCESS;
  }
  return WD_FAILURE;
}

template <typename ArrayType, typename ValueType>
wdResult wdDeduplicationReadContext::ReadArray(wdStreamReader& inout_stream, wdArrayBase<ValueType, ArrayType>& ref_array, wdAllocatorBase* pAllocator)
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(inout_stream.ReadQWordValue(&uiCount));

  WD_ASSERT_DEV(uiCount < std::numeric_limits<wdUInt32>::max(), "Containers currently use 32 bit for counts internally. Value from file is too large.");

  ref_array.Clear();

  if (uiCount > 0)
  {
    static_cast<ArrayType&>(ref_array).Reserve(static_cast<wdUInt32>(uiCount));

    for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
    {
      WD_SUCCEED_OR_RETURN(ReadObject(inout_stream, ref_array.ExpandAndGetRef(), pAllocator));
    }
  }

  return WD_SUCCESS;
}

template <typename KeyType, typename Comparer>
wdResult wdDeduplicationReadContext::ReadSet(wdStreamReader& inout_stream, wdSetBase<KeyType, Comparer>& ref_set, wdAllocatorBase* pAllocator)
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(inout_stream.ReadQWordValue(&uiCount));

  WD_ASSERT_DEV(uiCount < std::numeric_limits<wdUInt32>::max(), "Containers currently use 32 bit for counts internally. Value from file is too large.");

  ref_set.Clear();

  for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
  {
    KeyType key;
    WD_SUCCEED_OR_RETURN(ReadObject(inout_stream, key, pAllocator));

    ref_set.Insert(std::move(key));
  }

  return WD_SUCCESS;
}

namespace wdInternal
{
  // Internal helper to prevent the compiler from trying to find a de-serialization method for pointer types or other types which don't have
  // one.
  struct DeserializeHelper
  {
    template <typename T>
    static auto Deserialize(wdStreamReader& inout_stream, T& ref_obj, int) -> decltype(wdStreamReaderUtil::Deserialize(inout_stream, ref_obj))
    {
      return wdStreamReaderUtil::Deserialize(inout_stream, ref_obj);
    }

    template <typename T>
    static wdResult Deserialize(wdStreamReader& inout_stream, T& ref_obj, float)
    {
      WD_REPORT_FAILURE("No deserialize method available");
      return WD_FAILURE;
    }
  };
} // namespace wdInternal

template <typename KeyType, typename ValueType, typename Comparer>
wdResult wdDeduplicationReadContext::ReadMap(wdStreamReader& inout_stream, wdMapBase<KeyType, ValueType, Comparer>& ref_map, ReadMapMode mode, wdAllocatorBase* pKeyAllocator, wdAllocatorBase* pValueAllocator)
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(inout_stream.ReadQWordValue(&uiCount));

  WD_ASSERT_DEV(uiCount < std::numeric_limits<wdUInt32>::max(), "Containers currently use 32 bit for counts internally. Value from file is too large.");

  ref_map.Clear();

  if (mode == ReadMapMode::DedupKey)
  {
    for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
    {
      KeyType key;
      ValueType value;
      WD_SUCCEED_OR_RETURN(ReadObject(inout_stream, key, pKeyAllocator));
      WD_SUCCEED_OR_RETURN(wdInternal::DeserializeHelper::Deserialize<ValueType>(inout_stream, value, 0));

      ref_map.Insert(std::move(key), std::move(value));
    }
  }
  else if (mode == ReadMapMode::DedupValue)
  {
    for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
    {
      KeyType key;
      ValueType value;
      WD_SUCCEED_OR_RETURN(wdInternal::DeserializeHelper::Deserialize<KeyType>(inout_stream, key, 0));
      WD_SUCCEED_OR_RETURN(ReadObject(inout_stream, value, pValueAllocator));

      ref_map.Insert(std::move(key), std::move(value));
    }
  }
  else
  {
    for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
    {
      KeyType key;
      ValueType value;
      WD_SUCCEED_OR_RETURN(ReadObject(inout_stream, key, pKeyAllocator));
      WD_SUCCEED_OR_RETURN(ReadObject(inout_stream, value, pValueAllocator));

      ref_map.Insert(std::move(key), std::move(value));
    }
  }

  return WD_SUCCESS;
}
