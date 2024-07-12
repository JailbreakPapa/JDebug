#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Memory/MemoryUtils.h>

template <typename T>
NS_ALWAYS_INLINE nsHashableStruct<T>::nsHashableStruct()
{
  nsMemoryUtils::ZeroFill<T>(static_cast<T*>(this), 1);
}

template <typename T>
NS_ALWAYS_INLINE nsHashableStruct<T>::nsHashableStruct(const nsHashableStruct<T>& other)
{
  nsMemoryUtils::RawByteCopy(this, &other, sizeof(T));
}

template <typename T>
NS_ALWAYS_INLINE void nsHashableStruct<T>::operator=(const nsHashableStruct<T>& other)
{
  if (this != &other)
  {
    nsMemoryUtils::RawByteCopy(this, &other, sizeof(T));
  }
}

template <typename T>
NS_ALWAYS_INLINE nsUInt32 nsHashableStruct<T>::CalculateHash() const
{
  return nsHashingUtils::xxHash32(this, sizeof(T));
}
