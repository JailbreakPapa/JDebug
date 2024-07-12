
#include <Foundation/IO/Stream.h>

template <typename T>
NS_ALWAYS_INLINE nsResult nsDeduplicationReadContext::ReadObjectInplace(nsStreamReader& inout_stream, T& inout_obj)
{
  return ReadObject(inout_stream, inout_obj, nullptr);
}

template <typename T>
nsResult nsDeduplicationReadContext::ReadObject(nsStreamReader& inout_stream, T& obj, nsAllocator* pAllocator)
{
  bool bIsRealObject;
  inout_stream >> bIsRealObject;

  NS_ASSERT_DEV(bIsRealObject, "Reading an object inplace only works for the first occurrence");

  NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize<T>(inout_stream, obj));

  m_Objects.PushBack(&obj);

  return NS_SUCCESS;
}

template <typename T>
nsResult nsDeduplicationReadContext::ReadObject(nsStreamReader& inout_stream, T*& ref_pObject, nsAllocator* pAllocator)
{
  bool bIsRealObject;
  inout_stream >> bIsRealObject;

  if (bIsRealObject)
  {
    ref_pObject = NS_NEW(pAllocator, T);
    NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize<T>(inout_stream, *ref_pObject));

    m_Objects.PushBack(ref_pObject);
  }
  else
  {
    nsUInt32 uiIndex;
    inout_stream >> uiIndex;

    if (uiIndex < m_Objects.GetCount())
    {
      ref_pObject = static_cast<T*>(m_Objects[uiIndex]);
    }
    else if (uiIndex == nsInvalidIndex)
    {
      ref_pObject = nullptr;
    }
    else
    {
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

template <typename T>
nsResult nsDeduplicationReadContext::ReadObject(nsStreamReader& inout_stream, nsSharedPtr<T>& ref_pObject, nsAllocator* pAllocator)
{
  T* ptr = nullptr;
  if (ReadObject(inout_stream, ptr, pAllocator).Succeeded())
  {
    ref_pObject = nsSharedPtr<T>(ptr, pAllocator);
    return NS_SUCCESS;
  }
  return NS_FAILURE;
}

template <typename T>
nsResult nsDeduplicationReadContext::ReadObject(nsStreamReader& inout_stream, nsUniquePtr<T>& ref_pObject, nsAllocator* pAllocator)
{
  T* ptr = nullptr;
  if (ReadObject(inout_stream, ptr, pAllocator).Succeeded())
  {
    ref_pObject = std::move(nsUniquePtr<T>(ptr, pAllocator));
    return NS_SUCCESS;
  }
  return NS_FAILURE;
}

template <typename ArrayType, typename ValueType>
nsResult nsDeduplicationReadContext::ReadArray(nsStreamReader& inout_stream, nsArrayBase<ValueType, ArrayType>& ref_array, nsAllocator* pAllocator)
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(inout_stream.ReadQWordValue(&uiCount));

  NS_ASSERT_DEV(uiCount < std::numeric_limits<nsUInt32>::max(), "Containers currently use 32 bit for counts internally. Value from file is too large.");

  ref_array.Clear();

  if (uiCount > 0)
  {
    static_cast<ArrayType&>(ref_array).Reserve(static_cast<nsUInt32>(uiCount));

    for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
    {
      NS_SUCCEED_OR_RETURN(ReadObject(inout_stream, ref_array.ExpandAndGetRef(), pAllocator));
    }
  }

  return NS_SUCCESS;
}

template <typename KeyType, typename Comparer>
nsResult nsDeduplicationReadContext::ReadSet(nsStreamReader& inout_stream, nsSetBase<KeyType, Comparer>& ref_set, nsAllocator* pAllocator)
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(inout_stream.ReadQWordValue(&uiCount));

  NS_ASSERT_DEV(uiCount < std::numeric_limits<nsUInt32>::max(), "Containers currently use 32 bit for counts internally. Value from file is too large.");

  ref_set.Clear();

  for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
  {
    KeyType key;
    NS_SUCCEED_OR_RETURN(ReadObject(inout_stream, key, pAllocator));

    ref_set.Insert(std::move(key));
  }

  return NS_SUCCESS;
}

namespace nsInternal
{
  // Internal helper to prevent the compiler from trying to find a de-serialization method for pointer types or other types which don't have
  // one.
  struct DeserializeHelper
  {
    template <typename T>
    static auto Deserialize(nsStreamReader& inout_stream, T& ref_obj, int) -> decltype(nsStreamReaderUtil::Deserialize(inout_stream, ref_obj))
    {
      return nsStreamReaderUtil::Deserialize(inout_stream, ref_obj);
    }

    template <typename T>
    static nsResult Deserialize(nsStreamReader& inout_stream, T& ref_obj, float)
    {
      NS_REPORT_FAILURE("No deserialize method available");
      return NS_FAILURE;
    }
  };
} // namespace nsInternal

template <typename KeyType, typename ValueType, typename Comparer>
nsResult nsDeduplicationReadContext::ReadMap(nsStreamReader& inout_stream, nsMapBase<KeyType, ValueType, Comparer>& ref_map, ReadMapMode mode, nsAllocator* pKeyAllocator, nsAllocator* pValueAllocator)
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(inout_stream.ReadQWordValue(&uiCount));

  NS_ASSERT_DEV(uiCount < std::numeric_limits<nsUInt32>::max(), "Containers currently use 32 bit for counts internally. Value from file is too large.");

  ref_map.Clear();

  if (mode == ReadMapMode::DedupKey)
  {
    for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
    {
      KeyType key;
      ValueType value;
      NS_SUCCEED_OR_RETURN(ReadObject(inout_stream, key, pKeyAllocator));
      NS_SUCCEED_OR_RETURN(nsInternal::DeserializeHelper::Deserialize<ValueType>(inout_stream, value, 0));

      ref_map.Insert(std::move(key), std::move(value));
    }
  }
  else if (mode == ReadMapMode::DedupValue)
  {
    for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
    {
      KeyType key;
      ValueType value;
      NS_SUCCEED_OR_RETURN(nsInternal::DeserializeHelper::Deserialize<KeyType>(inout_stream, key, 0));
      NS_SUCCEED_OR_RETURN(ReadObject(inout_stream, value, pValueAllocator));

      ref_map.Insert(std::move(key), std::move(value));
    }
  }
  else
  {
    for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
    {
      KeyType key;
      ValueType value;
      NS_SUCCEED_OR_RETURN(ReadObject(inout_stream, key, pKeyAllocator));
      NS_SUCCEED_OR_RETURN(ReadObject(inout_stream, value, pValueAllocator));

      ref_map.Insert(std::move(key), std::move(value));
    }
  }

  return NS_SUCCESS;
}
