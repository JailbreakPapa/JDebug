
#include <Foundation/IO/Stream.h>

namespace nsInternal
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
} // namespace nsInternal

template <typename T>
NS_ALWAYS_INLINE nsResult nsDeduplicationWriteContext::WriteObject(nsStreamWriter& inout_stream, const T& obj)
{
  return WriteObjectInternal(inout_stream, nsInternal::WriteObjectHelper<T>::GetAddress(obj));
}

template <typename T>
NS_ALWAYS_INLINE nsResult nsDeduplicationWriteContext::WriteObject(nsStreamWriter& inout_stream, const nsSharedPtr<T>& pObject)
{
  return WriteObjectInternal(inout_stream, pObject.Borrow());
}

template <typename T>
NS_ALWAYS_INLINE nsResult nsDeduplicationWriteContext::WriteObject(nsStreamWriter& inout_stream, const nsUniquePtr<T>& pObject)
{
  return WriteObjectInternal(inout_stream, pObject.Borrow());
}

template <typename ArrayType, typename ValueType>
nsResult nsDeduplicationWriteContext::WriteArray(nsStreamWriter& inout_stream, const nsArrayBase<ValueType, ArrayType>& array)
{
  const nsUInt64 uiCount = array.GetCount();
  NS_SUCCEED_OR_RETURN(inout_stream.WriteQWordValue(&uiCount));

  for (nsUInt32 i = 0; i < static_cast<nsUInt32>(uiCount); ++i)
  {
    NS_SUCCEED_OR_RETURN(WriteObject(inout_stream, array[i]));
  }

  return NS_SUCCESS;
}

template <typename KeyType, typename Comparer>
nsResult nsDeduplicationWriteContext::WriteSet(nsStreamWriter& inout_stream, const nsSetBase<KeyType, Comparer>& set)
{
  const nsUInt64 uiWriteSize = set.GetCount();
  NS_SUCCEED_OR_RETURN(inout_stream.WriteQWordValue(&uiWriteSize));

  for (const auto& item : set)
  {
    NS_SUCCEED_OR_RETURN(WriteObject(inout_stream, item));
  }

  return NS_SUCCESS;
}

template <typename KeyType, typename ValueType, typename Comparer>
nsResult nsDeduplicationWriteContext::WriteMap(nsStreamWriter& inout_stream, const nsMapBase<KeyType, ValueType, Comparer>& map, WriteMapMode mode)
{
  const nsUInt64 uiWriteSize = map.GetCount();
  NS_SUCCEED_OR_RETURN(inout_stream.WriteQWordValue(&uiWriteSize));

  if (mode == WriteMapMode::DedupKey)
  {
    for (auto It = map.GetIterator(); It.IsValid(); ++It)
    {
      NS_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Key()));
      NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<ValueType>(inout_stream, It.Value()));
    }
  }
  else if (mode == WriteMapMode::DedupValue)
  {
    for (auto It = map.GetIterator(); It.IsValid(); ++It)
    {
      NS_SUCCEED_OR_RETURN(nsStreamWriterUtil::Serialize<KeyType>(inout_stream, It.Key()));
      NS_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Value()));
    }
  }
  else
  {
    for (auto It = map.GetIterator(); It.IsValid(); ++It)
    {
      NS_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Key()));
      NS_SUCCEED_OR_RETURN(WriteObject(inout_stream, It.Value()));
    }
  }

  return NS_SUCCESS;
}

template <typename T>
nsResult nsDeduplicationWriteContext::WriteObjectInternal(nsStreamWriter& stream, const T* pObject)
{
  nsUInt32 uiIndex = nsInvalidIndex;

  if (pObject)
  {
    bool bIsRealObject = !m_Objects.TryGetValue(pObject, uiIndex);
    stream << bIsRealObject;

    if (bIsRealObject)
    {
      uiIndex = m_Objects.GetCount();
      m_Objects.Insert(pObject, uiIndex);

      return nsStreamWriterUtil::Serialize<T>(stream, *pObject);
    }
    else
    {
      stream << uiIndex;
    }
  }
  else
  {
    stream << false;
    stream << nsInvalidIndex;
  }

  return NS_SUCCESS;
}
