#pragma once

#if NS_ENABLED(NS_PLATFORM_BIG_ENDIAN)

template <typename T>
nsResult nsStreamReader::ReadWordValue(T* pWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt16));

  nsUInt16 uiTemp;

  const nsUInt32 uiRead = ReadBytes(reinterpret_cast<nsUInt8*>(&uiTemp), sizeof(T));

  *reinterpret_cast<nsUInt16*>(pWordValue) = nsEndianHelper::Switch(uiTemp);

  return (uiRead == sizeof(T)) ? NS_SUCCESS : NS_FAILURE;
}

template <typename T>
nsResult nsStreamReader::ReadDWordValue(T* pDWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt32));

  nsUInt32 uiTemp;

  const nsUInt32 uiRead = ReadBytes(reinterpret_cast<nsUInt8*>(&uiTemp), sizeof(T));

  *reinterpret_cast<nsUInt32*>(pDWordValue) = nsEndianHelper::Switch(uiTemp);

  return (uiRead == sizeof(T)) ? NS_SUCCESS : NS_FAILURE;
}

template <typename T>
nsResult nsStreamReader::ReadQWordValue(T* pQWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt64));

  nsUInt64 uiTemp;

  const nsUInt32 uiRead = ReadBytes(reinterpret_cast<nsUInt8*>(&uiTemp), sizeof(T));

  *reinterpret_cast<nsUInt64*>(pQWordValue) = nsEndianHelper::Switch(uiTemp);

  return (uiRead == sizeof(T)) ? NS_SUCCESS : NS_FAILURE;
}



template <typename T>
nsResult nsStreamWriter::WriteWordValue(const T* pWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt16));

  nsUInt16 uiTemp = *reinterpret_cast<const nsUInt16*>(pWordValue);
  uiTemp = nsEndianHelper::Switch(uiTemp);

  return WriteBytes(reinterpret_cast<nsUInt8*>(&uiTemp), sizeof(T));
}

template <typename T>
nsResult nsStreamWriter::WriteDWordValue(const T* pDWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt32));

  nsUInt32 uiTemp = *reinterpret_cast<const nsUInt32*>(pDWordValue);
  uiTemp = nsEndianHelper::Switch(uiTemp);

  return WriteBytes(reinterpret_cast<nsUInt8*>(&uiTemp), sizeof(T));
}

template <typename T>
nsResult nsStreamWriter::WriteQWordValue(const T* pQWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt64));

  nsUInt64 uiTemp = *reinterpret_cast<const nsUInt64*>(pQWordValue);
  uiTemp = nsEndianHelper::Switch(uiTemp);

  return WriteBytes(reinterpret_cast<nsUInt8*>(&uiTemp), sizeof(T));
}

#else

template <typename T>
nsResult nsStreamReader::ReadWordValue(T* pWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt16));

  if (ReadBytes(reinterpret_cast<nsUInt8*>(pWordValue), sizeof(T)) != sizeof(T))
    return NS_FAILURE;

  return NS_SUCCESS;
}

template <typename T>
nsResult nsStreamReader::ReadDWordValue(T* pDWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt32));

  if (ReadBytes(reinterpret_cast<nsUInt8*>(pDWordValue), sizeof(T)) != sizeof(T))
    return NS_FAILURE;

  return NS_SUCCESS;
}

template <typename T>
nsResult nsStreamReader::ReadQWordValue(T* pQWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt64));

  if (ReadBytes(reinterpret_cast<nsUInt8*>(pQWordValue), sizeof(T)) != sizeof(T))
    return NS_FAILURE;

  return NS_SUCCESS;
}

template <typename T>
nsResult nsStreamWriter::WriteWordValue(const T* pWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt16));

  return WriteBytes(reinterpret_cast<const nsUInt8*>(pWordValue), sizeof(T));
}

template <typename T>
nsResult nsStreamWriter::WriteDWordValue(const T* pDWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt32));

  return WriteBytes(reinterpret_cast<const nsUInt8*>(pDWordValue), sizeof(T));
}

template <typename T>
nsResult nsStreamWriter::WriteQWordValue(const T* pQWordValue)
{
  NS_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(nsUInt64));

  return WriteBytes(reinterpret_cast<const nsUInt8*>(pQWordValue), sizeof(T));
}

#endif

nsTypeVersion nsStreamReader::ReadVersion(nsTypeVersion expectedMaxVersion)
{
  nsTypeVersion v = 0;
  ReadWordValue(&v).IgnoreResult();

  NS_ASSERT_ALWAYS(v <= expectedMaxVersion, "Read version ({0}) is larger than expected max version ({1}).", v, expectedMaxVersion);
  NS_ASSERT_ALWAYS(v > 0, "Invalid version.");

  return v;
}

void nsStreamWriter::WriteVersion(nsTypeVersion version)
{
  NS_ASSERT_ALWAYS(version > 0, "Version cannot be zero.");

  WriteWordValue(&version).IgnoreResult();
}


namespace nsStreamWriterUtil
{
  // single element serialization

  template <class T>
  NS_ALWAYS_INLINE auto SerializeImpl(nsStreamWriter& inout_stream, const T& obj, int) -> decltype(inout_stream << obj, nsResult(NS_SUCCESS))
  {
    inout_stream << obj;

    return NS_SUCCESS;
  }

  template <class T>
  NS_ALWAYS_INLINE auto SerializeImpl(nsStreamWriter& inout_stream, const T& obj, long) -> decltype(obj.Serialize(inout_stream).IgnoreResult(), nsResult(NS_SUCCESS))
  {
    return nsToResult(obj.Serialize(inout_stream));
  }

  template <class T>
  NS_ALWAYS_INLINE auto SerializeImpl(nsStreamWriter& inout_stream, const T& obj, float) -> decltype(obj.serialize(inout_stream).IgnoreResult(), nsResult(NS_SUCCESS))
  {
    return nsToResult(obj.serialize(inout_stream));
  }

  template <class T>
  NS_ALWAYS_INLINE auto Serialize(nsStreamWriter& inout_stream, const T& obj) -> decltype(SerializeImpl(inout_stream, obj, 0).IgnoreResult(), nsResult(NS_SUCCESS))
  {
    return SerializeImpl(inout_stream, obj, 0);
  }

  // serialization of array

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)
  template <class T>
  NS_ALWAYS_INLINE auto SerializeArrayImpl(nsStreamWriter& inout_stream, const T* pArray, nsUInt64 uiCount, int) -> decltype(SerializeArray(inout_stream, pArray, uiCount), nsResult(NS_SUCCESS))
  {
    return SerializeArray(inout_stream, pArray, uiCount);
  }
#endif

  template <class T>
  nsResult SerializeArrayImpl(nsStreamWriter& inout_stream, const T* pArray, nsUInt64 uiCount, long)
  {
    for (nsUInt64 i = 0; i < uiCount; ++i)
    {
      NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<T>(inout_stream, pArray[i]));
    }

    return NS_SUCCESS;
  }

  template <class T>
  NS_ALWAYS_INLINE nsResult SerializeArray(nsStreamWriter& inout_stream, const T* pArray, nsUInt64 uiCount)
  {
    return SerializeArrayImpl(inout_stream, pArray, uiCount, 0);
  }
} // namespace nsStreamWriterUtil

template <typename ArrayType, typename ValueType>
nsResult nsStreamWriter::WriteArray(const nsArrayBase<ValueType, ArrayType>& array)
{
  const nsUInt64 uiCount = array.GetCount();
  NS_SUCCEED_OR_RETURN(WriteQWordValue(&uiCount));

  return nsStreamWriterUtil::SerializeArray<ValueType>(*this, array.GetData(), array.GetCount());
}

template <typename ValueType, nsUInt16 uiSize>
nsResult nsStreamWriter::WriteArray(const nsSmallArrayBase<ValueType, uiSize>& array)
{
  const nsUInt32 uiCount = array.GetCount();
  NS_SUCCEED_OR_RETURN(WriteDWordValue(&uiCount));

  return nsStreamWriterUtil::SerializeArray<ValueType>(*this, array.GetData(), array.GetCount());
}

template <typename ValueType, nsUInt32 uiSize>
nsResult nsStreamWriter::WriteArray(const ValueType (&array)[uiSize])
{
  const nsUInt64 uiWriteSize = uiSize;
  NS_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  return nsStreamWriterUtil::SerializeArray<ValueType>(*this, array, uiSize);
}

template <typename KeyType, typename Comparer>
nsResult nsStreamWriter::WriteSet(const nsSetBase<KeyType, Comparer>& set)
{
  const nsUInt64 uiWriteSize = set.GetCount();
  NS_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  for (const auto& item : set)
  {
    NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<KeyType>(*this, item));
  }

  return NS_SUCCESS;
}

template <typename KeyType, typename ValueType, typename Comparer>
nsResult nsStreamWriter::WriteMap(const nsMapBase<KeyType, ValueType, Comparer>& map)
{
  const nsUInt64 uiWriteSize = map.GetCount();
  NS_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  for (auto It = map.GetIterator(); It.IsValid(); ++It)
  {
    NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<KeyType>(*this, It.Key()));
    NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<ValueType>(*this, It.Value()));
  }

  return NS_SUCCESS;
}

template <typename KeyType, typename ValueType, typename Hasher>
nsResult nsStreamWriter::WriteHashTable(const nsHashTableBase<KeyType, ValueType, Hasher>& hashTable)
{
  const nsUInt64 uiWriteSize = hashTable.GetCount();
  NS_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  for (auto It = hashTable.GetIterator(); It.IsValid(); ++It)
  {
    NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<KeyType>(*this, It.Key()));
    NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<ValueType>(*this, It.Value()));
  }

  return NS_SUCCESS;
}

namespace nsStreamReaderUtil
{
  template <class T>
  NS_ALWAYS_INLINE auto DeserializeImpl(nsStreamReader& inout_stream, T& ref_obj, int) -> decltype(inout_stream >> ref_obj, nsResult(NS_SUCCESS))
  {
    inout_stream >> ref_obj;

    return NS_SUCCESS;
  }

  template <class T>
  NS_ALWAYS_INLINE auto DeserializeImpl(nsStreamReader& inout_stream, T& inout_obj, long) -> decltype(inout_obj.Deserialize(inout_stream).IgnoreResult(), nsResult(NS_SUCCESS))
  {
    return nsToResult(inout_obj.Deserialize(inout_stream));
  }

  template <class T>
  NS_ALWAYS_INLINE auto DeserializeImpl(nsStreamReader& inout_stream, T& inout_obj, float) -> decltype(inout_obj.deserialize(inout_stream).IgnoreResult(), nsResult(NS_SUCCESS))
  {
    return nsToResult(inout_obj.deserialize(inout_stream));
  }

  template <class T>
  NS_ALWAYS_INLINE auto Deserialize(nsStreamReader& inout_stream, T& inout_obj) -> decltype(DeserializeImpl(inout_stream, inout_obj, 0).IgnoreResult(), nsResult(NS_SUCCESS))
  {
    return DeserializeImpl(inout_stream, inout_obj, 0);
  }

  // serialization of array

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)
  template <class T>
  NS_ALWAYS_INLINE auto DeserializeArrayImpl(nsStreamReader& inout_stream, T* pArray, nsUInt64 uiCount, int) -> decltype(DeserializeArray(inout_stream, pArray, uiCount), nsResult(NS_SUCCESS))
  {
    return DeserializeArray(inout_stream, pArray, uiCount);
  }
#endif

  template <class T>
  nsResult DeserializeArrayImpl(nsStreamReader& inout_stream, T* pArray, nsUInt64 uiCount, long)
  {
    for (nsUInt64 i = 0; i < uiCount; ++i)
    {
      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize<T>(inout_stream, pArray[i]));
    }

    return NS_SUCCESS;
  }

  template <class T>
  NS_ALWAYS_INLINE nsResult DeserializeArray(nsStreamReader& inout_stream, T* pArray, nsUInt64 uiCount)
  {
    return DeserializeArrayImpl(inout_stream, pArray, uiCount, 0);
  }

} // namespace nsStreamReaderUtil

template <typename ArrayType, typename ValueType>
nsResult nsStreamReader::ReadArray(nsArrayBase<ValueType, ArrayType>& inout_array)
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < nsMath::MaxValue<nsUInt32>())
  {
    inout_array.Clear();

    if (uiCount > 0)
    {
      static_cast<ArrayType&>(inout_array).SetCount(static_cast<nsUInt32>(uiCount));

      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::DeserializeArray<ValueType>(*this, inout_array.GetData(), uiCount));
    }

    return NS_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return NS_FAILURE;
  }
}

template <typename ValueType, nsUInt16 uiSize, typename AllocatorWrapper>
nsResult nsStreamReader::ReadArray(nsSmallArray<ValueType, uiSize, AllocatorWrapper>& ref_array)
{
  nsUInt32 uiCount = 0;
  NS_SUCCEED_OR_RETURN(ReadDWordValue(&uiCount));

  if (uiCount < nsMath::MaxValue<nsUInt16>())
  {
    ref_array.Clear();

    if (uiCount > 0)
    {
      ref_array.SetCount(static_cast<nsUInt16>(uiCount));

      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::DeserializeArray<ValueType>(*this, ref_array.GetData(), uiCount));
    }

    return NS_SUCCESS;
  }
  else
  {
    // Small array uses 16 bit for counts internally. Value from file is too large.
    return NS_FAILURE;
  }
}

template <typename ValueType, nsUInt32 uiSize>
nsResult nsStreamReader::ReadArray(ValueType (&array)[uiSize])
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (static_cast<nsUInt32>(uiCount) != uiSize)
    return NS_FAILURE;

  if (uiCount < nsMath::MaxValue<nsUInt32>())
  {
    NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::DeserializeArray<ValueType>(*this, array, uiCount));

    return NS_SUCCESS;
  }

  // Containers currently use 32 bit for counts internally. Value from file is too large.
  return NS_FAILURE;
}

template <typename KeyType, typename Comparer>
nsResult nsStreamReader::ReadSet(nsSetBase<KeyType, Comparer>& inout_set)
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < nsMath::MaxValue<nsUInt32>())
  {
    inout_set.Clear();

    for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
    {
      KeyType Item;
      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize(*this, Item));

      inout_set.Insert(std::move(Item));
    }

    return NS_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return NS_FAILURE;
  }
}

template <typename KeyType, typename ValueType, typename Comparer>
nsResult nsStreamReader::ReadMap(nsMapBase<KeyType, ValueType, Comparer>& inout_map)
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < nsMath::MaxValue<nsUInt32>())
  {
    inout_map.Clear();

    for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
    {
      KeyType Key;
      ValueType Value;
      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize(*this, Key));
      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize(*this, Value));

      inout_map.Insert(std::move(Key), std::move(Value));
    }

    return NS_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return NS_FAILURE;
  }
}

template <typename KeyType, typename ValueType, typename Hasher>
nsResult nsStreamReader::ReadHashTable(nsHashTableBase<KeyType, ValueType, Hasher>& inout_hashTable)
{
  nsUInt64 uiCount = 0;
  NS_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < nsMath::MaxValue<nsUInt32>())
  {
    inout_hashTable.Clear();
    inout_hashTable.Reserve(static_cast<nsUInt32>(uiCount));

    for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
    {
      KeyType Key;
      ValueType Value;
      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize(*this, Key));
      NS_SUCCEED_OR_RETURN(nsStreamReaderUtil::Deserialize(*this, Value));

      inout_hashTable.Insert(std::move(Key), std::move(Value));
    }

    return NS_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return NS_FAILURE;
  }
}
