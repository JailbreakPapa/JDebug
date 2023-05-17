#pragma once

#if WD_ENABLED(WD_PLATFORM_BIG_ENDIAN)

template <typename T>
wdResult wdStreamReader::ReadWordValue(T* pWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt16));

  wdUInt16 uiTemp;

  const wdUInt32 uiRead = ReadBytes(reinterpret_cast<wdUInt8*>(&uiTemp), sizeof(T));

  *reinterpret_cast<wdUInt16*>(pWordValue) = wdEndianHelper::Switch(uiTemp);

  return (uiRead == sizeof(T)) ? WD_SUCCESS : WD_FAILURE;
}

template <typename T>
wdResult wdStreamReader::ReadDWordValue(T* pDWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt32));

  wdUInt32 uiTemp;

  const wdUInt32 uiRead = ReadBytes(reinterpret_cast<wdUInt8*>(&uiTemp), sizeof(T));

  *reinterpret_cast<wdUInt32*>(pDWordValue) = wdEndianHelper::Switch(uiTemp);

  return (uiRead == sizeof(T)) ? WD_SUCCESS : WD_FAILURE;
}

template <typename T>
wdResult wdStreamReader::ReadQWordValue(T* pQWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt64));

  wdUInt64 uiTemp;

  const wdUInt32 uiRead = ReadBytes(reinterpret_cast<wdUInt8*>(&uiTemp), sizeof(T));

  *reinterpret_cast<wdUInt64*>(pQWordValue) = wdEndianHelper::Switch(uiTemp);

  return (uiRead == sizeof(T)) ? WD_SUCCESS : WD_FAILURE;
}



template <typename T>
wdResult wdStreamWriter::WriteWordValue(const T* pWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt16));

  wdUInt16 uiTemp = *reinterpret_cast<const wdUInt16*>(pWordValue);
  uiTemp = wdEndianHelper::Switch(uiTemp);

  return WriteBytes(reinterpret_cast<wdUInt8*>(&uiTemp), sizeof(T));
}

template <typename T>
wdResult wdStreamWriter::WriteDWordValue(const T* pDWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt32));

  wdUInt32 uiTemp = *reinterpret_cast<const wdUInt16*>(pDWordValue);
  uiTemp = wdEndianHelper::Switch(uiTemp);

  return WriteBytes(reinterpret_cast<wdUInt8*>(&uiTemp), sizeof(T));
}

template <typename T>
wdResult wdStreamWriter::WriteQWordValue(const T* pQWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt64));

  wdUInt64 uiTemp = *reinterpret_cast<const wdUInt64*>(pQWordValue);
  uiTemp = wdEndianHelper::Switch(uiTemp);

  return WriteBytes(reinterpret_cast<wdUInt8*>(&uiTemp), sizeof(T));
}

#else

template <typename T>
wdResult wdStreamReader::ReadWordValue(T* pWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt16));

  if (ReadBytes(reinterpret_cast<wdUInt8*>(pWordValue), sizeof(T)) != sizeof(T))
    return WD_FAILURE;

  return WD_SUCCESS;
}

template <typename T>
wdResult wdStreamReader::ReadDWordValue(T* pDWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt32));

  if (ReadBytes(reinterpret_cast<wdUInt8*>(pDWordValue), sizeof(T)) != sizeof(T))
    return WD_FAILURE;

  return WD_SUCCESS;
}

template <typename T>
wdResult wdStreamReader::ReadQWordValue(T* pQWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt64));

  if (ReadBytes(reinterpret_cast<wdUInt8*>(pQWordValue), sizeof(T)) != sizeof(T))
    return WD_FAILURE;

  return WD_SUCCESS;
}

template <typename T>
wdResult wdStreamWriter::WriteWordValue(const T* pWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt16));

  return WriteBytes(reinterpret_cast<const wdUInt8*>(pWordValue), sizeof(T));
}

template <typename T>
wdResult wdStreamWriter::WriteDWordValue(const T* pDWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt32));

  return WriteBytes(reinterpret_cast<const wdUInt8*>(pDWordValue), sizeof(T));
}

template <typename T>
wdResult wdStreamWriter::WriteQWordValue(const T* pQWordValue)
{
  WD_CHECK_AT_COMPILETIME(sizeof(T) == sizeof(wdUInt64));

  return WriteBytes(reinterpret_cast<const wdUInt8*>(pQWordValue), sizeof(T));
}

#endif

wdTypeVersion wdStreamReader::ReadVersion(wdTypeVersion expectedMaxVersion)
{
  wdTypeVersion v = 0;
  ReadWordValue(&v).IgnoreResult();

  WD_ASSERT_ALWAYS(v <= expectedMaxVersion, "Read version ({0}) is larger than expected max version ({1}).", v, expectedMaxVersion);
  WD_ASSERT_ALWAYS(v > 0, "Invalid version.");

  return v;
}

void wdStreamWriter::WriteVersion(wdTypeVersion version)
{
  WD_ASSERT_ALWAYS(version > 0, "Version cannot be zero.");

  WriteWordValue(&version).IgnoreResult();
}


namespace wdStreamWriterUtil
{
  // single element serialization

  template <class T>
  WD_ALWAYS_INLINE auto SerializeImpl(wdStreamWriter& inout_stream, const T& obj, int) -> decltype(inout_stream << obj, wdResult(WD_SUCCESS))
  {
    inout_stream << obj;

    return WD_SUCCESS;
  }

  template <class T>
  WD_ALWAYS_INLINE auto SerializeImpl(wdStreamWriter& inout_stream, const T& obj, long) -> decltype(obj.Serialize(inout_stream).IgnoreResult(), wdResult(WD_SUCCESS))
  {
    return wdToResult(obj.Serialize(inout_stream));
  }

  template <class T>
  WD_ALWAYS_INLINE auto SerializeImpl(wdStreamWriter& inout_stream, const T& obj, float) -> decltype(obj.serialize(inout_stream).IgnoreResult(), wdResult(WD_SUCCESS))
  {
    return wdToResult(obj.serialize(inout_stream));
  }

  template <class T>
  WD_ALWAYS_INLINE auto Serialize(wdStreamWriter& inout_stream, const T& obj) -> decltype(SerializeImpl(inout_stream, obj, 0).IgnoreResult(), wdResult(WD_SUCCESS))
  {
    return SerializeImpl(inout_stream, obj, 0);
  }

  // serialization of array

#if WD_DISABLED(WD_PLATFORM_WINDOWS_UWP)
  template <class T>
  WD_ALWAYS_INLINE auto SerializeArrayImpl(wdStreamWriter& inout_stream, const T* pArray, wdUInt64 uiCount, int) -> decltype(SerializeArray(inout_stream, pArray, uiCount), wdResult(WD_SUCCESS))
  {
    return SerializeArray(inout_stream, pArray, uiCount);
  }
#endif

  template <class T>
  wdResult SerializeArrayImpl(wdStreamWriter& inout_stream, const T* pArray, wdUInt64 uiCount, long)
  {
    for (wdUInt64 i = 0; i < uiCount; ++i)
    {
      WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<T>(inout_stream, pArray[i]));
    }

    return WD_SUCCESS;
  }

  template <class T>
  WD_ALWAYS_INLINE wdResult SerializeArray(wdStreamWriter& inout_stream, const T* pArray, wdUInt64 uiCount)
  {
    return SerializeArrayImpl(inout_stream, pArray, uiCount, 0);
  }
} // namespace wdStreamWriterUtil

template <typename ArrayType, typename ValueType>
wdResult wdStreamWriter::WriteArray(const wdArrayBase<ValueType, ArrayType>& array)
{
  const wdUInt64 uiCount = array.GetCount();
  WD_SUCCEED_OR_RETURN(WriteQWordValue(&uiCount));

  return wdStreamWriterUtil::SerializeArray<ValueType>(*this, array.GetData(), array.GetCount());
}

template <typename ValueType, wdUInt16 uiSize>
wdResult wdStreamWriter::WriteArray(const wdSmallArrayBase<ValueType, uiSize>& array)
{
  const wdUInt32 uiCount = array.GetCount();
  WD_SUCCEED_OR_RETURN(WriteDWordValue(&uiCount));

  return wdStreamWriterUtil::SerializeArray<ValueType>(*this, array.GetData(), array.GetCount());
}

template <typename ValueType, wdUInt32 uiSize>
wdResult wdStreamWriter::WriteArray(const ValueType (&array)[uiSize])
{
  const wdUInt64 uiWriteSize = uiSize;
  WD_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  return wdStreamWriterUtil::SerializeArray<ValueType>(*this, array, uiSize);
}

template <typename KeyType, typename Comparer>
wdResult wdStreamWriter::WriteSet(const wdSetBase<KeyType, Comparer>& set)
{
  const wdUInt64 uiWriteSize = set.GetCount();
  WD_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  for (const auto& item : set)
  {
    WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<KeyType>(*this, item));
  }

  return WD_SUCCESS;
}

template <typename KeyType, typename ValueType, typename Comparer>
wdResult wdStreamWriter::WriteMap(const wdMapBase<KeyType, ValueType, Comparer>& map)
{
  const wdUInt64 uiWriteSize = map.GetCount();
  WD_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  for (auto It = map.GetIterator(); It.IsValid(); ++It)
  {
    WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<KeyType>(*this, It.Key()));
    WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<ValueType>(*this, It.Value()));
  }

  return WD_SUCCESS;
}

template <typename KeyType, typename ValueType, typename Hasher>
wdResult wdStreamWriter::WriteHashTable(const wdHashTableBase<KeyType, ValueType, Hasher>& hashTable)
{
  const wdUInt64 uiWriteSize = hashTable.GetCount();
  WD_SUCCEED_OR_RETURN(WriteQWordValue(&uiWriteSize));

  for (auto It = hashTable.GetIterator(); It.IsValid(); ++It)
  {
    WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<KeyType>(*this, It.Key()));
    WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<ValueType>(*this, It.Value()));
  }

  return WD_SUCCESS;
}

namespace wdStreamReaderUtil
{
  template <class T>
  WD_ALWAYS_INLINE auto DeserializeImpl(wdStreamReader& inout_stream, T& ref_obj, int) -> decltype(inout_stream >> ref_obj, wdResult(WD_SUCCESS))
  {
    inout_stream >> ref_obj;

    return WD_SUCCESS;
  }

  template <class T>
  WD_ALWAYS_INLINE auto DeserializeImpl(wdStreamReader& inout_stream, T& inout_obj, long) -> decltype(inout_obj.Deserialize(inout_stream).IgnoreResult(), wdResult(WD_SUCCESS))
  {
    return wdToResult(inout_obj.Deserialize(inout_stream));
  }

  template <class T>
  WD_ALWAYS_INLINE auto DeserializeImpl(wdStreamReader& inout_stream, T& inout_obj, float) -> decltype(inout_obj.deserialize(inout_stream).IgnoreResult(), wdResult(WD_SUCCESS))
  {
    return wdToResult(inout_obj.deserialize(inout_stream));
  }

  template <class T>
  WD_ALWAYS_INLINE auto Deserialize(wdStreamReader& inout_stream, T& inout_obj) -> decltype(DeserializeImpl(inout_stream, inout_obj, 0).IgnoreResult(), wdResult(WD_SUCCESS))
  {
    return DeserializeImpl(inout_stream, inout_obj, 0);
  }

  // serialization of array

#if WD_DISABLED(WD_PLATFORM_WINDOWS_UWP)
  template <class T>
  WD_ALWAYS_INLINE auto DeserializeArrayImpl(wdStreamReader& inout_stream, T* pArray, wdUInt64 uiCount, int) -> decltype(DeserializeArray(inout_stream, pArray, uiCount), wdResult(WD_SUCCESS))
  {
    return DeserializeArray(inout_stream, pArray, uiCount);
  }
#endif

  template <class T>
  wdResult DeserializeArrayImpl(wdStreamReader& inout_stream, T* pArray, wdUInt64 uiCount, long)
  {
    for (wdUInt64 i = 0; i < uiCount; ++i)
    {
      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize<T>(inout_stream, pArray[i]));
    }

    return WD_SUCCESS;
  }

  template <class T>
  WD_ALWAYS_INLINE wdResult DeserializeArray(wdStreamReader& inout_stream, T* pArray, wdUInt64 uiCount)
  {
    return DeserializeArrayImpl(inout_stream, pArray, uiCount, 0);
  }

} // namespace wdStreamReaderUtil

template <typename ArrayType, typename ValueType>
wdResult wdStreamReader::ReadArray(wdArrayBase<ValueType, ArrayType>& inout_array)
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < wdMath::MaxValue<wdUInt32>())
  {
    inout_array.Clear();

    if (uiCount > 0)
    {
      static_cast<ArrayType&>(inout_array).SetCount(static_cast<wdUInt32>(uiCount));

      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::DeserializeArray<ValueType>(*this, inout_array.GetData(), uiCount));
    }

    return WD_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return WD_FAILURE;
  }
}

template <typename ValueType, wdUInt16 uiSize, typename AllocatorWrapper>
wdResult wdStreamReader::ReadArray(wdSmallArray<ValueType, uiSize, AllocatorWrapper>& ref_array)
{
  wdUInt32 uiCount = 0;
  WD_SUCCEED_OR_RETURN(ReadDWordValue(&uiCount));

  if (uiCount < wdMath::MaxValue<wdUInt16>())
  {
    ref_array.Clear();

    if (uiCount > 0)
    {
      ref_array.SetCount(static_cast<wdUInt16>(uiCount));

      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::DeserializeArray<ValueType>(*this, ref_array.GetData(), uiCount));
    }

    return WD_SUCCESS;
  }
  else
  {
    // Small array uses 16 bit for counts internally. Value from file is too large.
    return WD_FAILURE;
  }
}

template <typename ValueType, wdUInt32 uiSize>
wdResult wdStreamReader::ReadArray(ValueType (&array)[uiSize])
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (static_cast<wdUInt32>(uiCount) != uiSize)
    return WD_FAILURE;

  if (uiCount < wdMath::MaxValue<wdUInt32>())
  {
    WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::DeserializeArray<ValueType>(*this, array, uiCount));

    return WD_SUCCESS;
  }

  // Containers currently use 32 bit for counts internally. Value from file is too large.
  return WD_FAILURE;
}

template <typename KeyType, typename Comparer>
wdResult wdStreamReader::ReadSet(wdSetBase<KeyType, Comparer>& inout_set)
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < wdMath::MaxValue<wdUInt32>())
  {
    inout_set.Clear();

    for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
    {
      KeyType Item;
      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize(*this, Item));

      inout_set.Insert(std::move(Item));
    }

    return WD_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return WD_FAILURE;
  }
}

template <typename KeyType, typename ValueType, typename Comparer>
wdResult wdStreamReader::ReadMap(wdMapBase<KeyType, ValueType, Comparer>& inout_map)
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < wdMath::MaxValue<wdUInt32>())
  {
    inout_map.Clear();

    for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
    {
      KeyType Key;
      ValueType Value;
      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize(*this, Key));
      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize(*this, Value));

      inout_map.Insert(std::move(Key), std::move(Value));
    }

    return WD_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return WD_FAILURE;
  }
}

template <typename KeyType, typename ValueType, typename Hasher>
wdResult wdStreamReader::ReadHashTable(wdHashTableBase<KeyType, ValueType, Hasher>& inout_hashTable)
{
  wdUInt64 uiCount = 0;
  WD_SUCCEED_OR_RETURN(ReadQWordValue(&uiCount));

  if (uiCount < wdMath::MaxValue<wdUInt32>())
  {
    inout_hashTable.Clear();
    inout_hashTable.Reserve(static_cast<wdUInt32>(uiCount));

    for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
    {
      KeyType Key;
      ValueType Value;
      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize(*this, Key));
      WD_SUCCEED_OR_RETURN(wdStreamReaderUtil::Deserialize(*this, Value));

      inout_hashTable.Insert(std::move(Key), std::move(Value));
    }

    return WD_SUCCESS;
  }
  else
  {
    // Containers currently use 32 bit for counts internally. Value from file is too large.
    return WD_FAILURE;
  }
}
