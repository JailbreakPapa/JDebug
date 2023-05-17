
#include <Foundation/IO/Stream.h>

namespace wdInternal
{
  // This internal helper is needed to differentiate between reference and pointer which is not possible with regular function overloading
  // in this case.
  template <typename T>
  struct WriteObjectHelper
  {
    static const T* GetAddress(const T& obj) { return &obj; }
  };

  template <typename T>
  struct WriteObjectHelper<T*>
  {
    static const T* GetAddress(const T* pObj) { return pObj; }
  };
} // namespace wdInternal

template <typename T>
WD_ALWAYS_INLINE wdResult wdDeduplicationWriteContext::WriteObject(wdStreamWriter& inout_stream, const T& obj)
{
  return WriteObjectInternal(inout_stream, wdInternal::WriteObjectHelper<T>::GetAddress(obj));
}

template <typename T>
WD_ALWAYS_INLINE wdResult wdDeduplicationWriteContext::WriteObject(wdStreamWriter& inout_stream, const wdSharedPtr<T>& pObject)
{
  return WriteObjectInternal(inout_stream, pObject.Borrow());
}

template <typename T>
WD_ALWAYS_INLINE wdResult wdDeduplicationWriteContext::WriteObject(wdStreamWriter& inout_stream, const wdUniquePtr<T>& pObject)
{
  return WriteObjectInternal(inout_stream, pObject.Borrow());
}

template <typename ArrayType, typename ValueType>
wdResult wdDeduplicationWriteContext::WriteArray(wdStreamWriter& inout_stream, const wdArrayBase<ValueType, ArrayType>& array)
{
  const wdUInt64 uiCount = array.GetCount();
  WD_SUCCEED_OR_RETURN(inout_stream.WriteQWordValue(&uiCount));

  for (wdUInt32 i = 0; i < static_cast<wdUInt32>(uiCount); ++i)
  {
    WD_SUCCEED_OR_RETURN(WriteObject(inout_stream, array[i]));
  }

  return WD_SUCCESS;
}

template <typename KeyType, typename Comparer>
wdResult wdDeduplicationWriteContext::WriteSet(wdStreamWriter& inout_stream, const wdSetBase<KeyType, Comparer>& set)
{
  const wdUInt64 uiWriteSize = set.GetCount();
  WD_SUCCEED_OR_RETURN(inout_stream.WriteQWordValue(&uiWriteSize));

  for (const auto& item : set)
  {
    WD_SUCCEED_OR_RETURN(WriteObject(inout_stream, item));
  }

  return WD_SUCCESS;
}

template <typename KeyType, typename ValueType, typename Comparer>
wdResult wdDeduplicationWriteContext::WriteMap(wdStreamWriter& inout_stream, const wdMapBase<KeyType, ValueType, Comparer>& map, WriteMapMode mode)
{
  const wdUInt64 uiWriteSize = map.GetCount();
  WD_SUCCEED_OR_RETURN(inout_stream.WriteQWordValue(&uiWriteSize));

  if (mode == WriteMapMode::DedupKey)
  {
    for (auto It = map.GetIterator(); It.IsValid(); ++It)
    {
      WD_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Key()));
      WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<ValueType>(inout_stream, It.Value()));
    }
  }
  else if (mode == WriteMapMode::DedupValue)
  {
    for (auto It = map.GetIterator(); It.IsValid(); ++It)
    {
      WD_SUCCEED_OR_RETURN(wdStreamWriterUtil::Serialize<KeyType>(inout_stream, It.Key()));
      WD_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Value()));
    }
  }
  else
  {
    for (auto It = map.GetIterator(); It.IsValid(); ++It)
    {
      WD_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Key()));
      WD_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Value()));
    }
  }

  return WD_SUCCESS;
}

template <typename T>
wdResult wdDeduplicationWriteContext::WriteObjectInternal(wdStreamWriter& stream, const T* pObject)
{
  wdUInt32 uiIndex = wdInvalidIndex;

  if (pObject)
  {
    bool bIsRealObject = !m_Objects.TryGetValue(pObject, uiIndex);
    stream << bIsRealObject;

    if (bIsRealObject)
    {
      uiIndex = m_Objects.GetCount();
      m_Objects.Insert(pObject, uiIndex);

      return wdStreamWriterUtil::Serialize<T>(stream, *pObject);
    }
    else
    {
      stream << uiIndex;
    }
  }
  else
  {
    stream << false;
    stream << wdInvalidIndex;
  }

  return WD_SUCCESS;
}
