#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Memory/MemoryUtils.h>

template <typename T>
WD_ALWAYS_INLINE wdHashableStruct<T>::wdHashableStruct()
{
  wdMemoryUtils::ZeroFill<T>(static_cast<T*>(this), 1);
}

template <typename T>
WD_ALWAYS_INLINE wdHashableStruct<T>::wdHashableStruct(const wdHashableStruct<T>& other)
{
  wdMemoryUtils::RawByteCopy(this, &other, sizeof(T));
}

template <typename T>
WD_ALWAYS_INLINE void wdHashableStruct<T>::operator=(const wdHashableStruct<T>& other)
{
  if (this != &other)
  {
    wdMemoryUtils::RawByteCopy(this, &other, sizeof(T));
  }
}

template <typename T>
WD_ALWAYS_INLINE wdUInt32 wdHashableStruct<T>::CalculateHash() const
{
  return wdHashingUtils::xxHash32(this, sizeof(T));
}
