#pragma once

#include <Foundation/Containers/Bitfield.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Enum.h>

// Standard operators for overloads of common data types

/// bool versions

inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, bool bValue)
{
  nsUInt8 uiValue = bValue ? 1 : 0;
  inout_stream.WriteBytes(&uiValue, sizeof(nsUInt8)).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, bool& out_bValue)
{
  nsUInt8 uiValue = 0;
  NS_VERIFY(inout_stream.ReadBytes(&uiValue, sizeof(nsUInt8)) == sizeof(nsUInt8), "End of stream reached.");
  out_bValue = (uiValue != 0);
  return inout_stream;
}

/// unsigned int versions

inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsUInt8 uiValue)
{
  inout_stream.WriteBytes(&uiValue, sizeof(nsUInt8)).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsUInt8& out_uiValue)
{
  NS_VERIFY(inout_stream.ReadBytes(&out_uiValue, sizeof(nsUInt8)) == sizeof(nsUInt8), "End of stream reached.");
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsUInt8* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsUInt8) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsUInt8* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsUInt8) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsUInt16 uiValue)
{
  inout_stream.WriteWordValue(&uiValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsUInt16& ref_uiValue)
{
  inout_stream.ReadWordValue(&ref_uiValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsUInt16* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsUInt16) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsUInt16* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsUInt16) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsUInt32 uiValue)
{
  inout_stream.WriteDWordValue(&uiValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsUInt32& ref_uiValue)
{
  inout_stream.ReadDWordValue(&ref_uiValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsUInt32* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsUInt32) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsUInt32* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsUInt32) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsUInt64 uiValue)
{
  inout_stream.WriteQWordValue(&uiValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsUInt64& ref_uiValue)
{
  inout_stream.ReadQWordValue(&ref_uiValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsUInt64* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsUInt64) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsUInt64* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsUInt64) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}

/// signed int versions

inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsInt8 iValue)
{
  inout_stream.WriteBytes(reinterpret_cast<const nsUInt8*>(&iValue), sizeof(nsInt8)).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsInt8& ref_iValue)
{
  NS_VERIFY(inout_stream.ReadBytes(reinterpret_cast<nsUInt8*>(&ref_iValue), sizeof(nsInt8)) == sizeof(nsInt8), "End of stream reached.");
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsInt8* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsInt8) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsInt8* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsInt8) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsInt16 iValue)
{
  inout_stream.WriteWordValue(&iValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsInt16& ref_iValue)
{
  inout_stream.ReadWordValue(&ref_iValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsInt16* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsInt16) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsInt16* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsInt16) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsInt32 iValue)
{
  inout_stream.WriteDWordValue(&iValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsInt32& ref_iValue)
{
  inout_stream.ReadDWordValue(&ref_iValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsInt32* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsInt32) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsInt32* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsInt32) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsInt64 iValue)
{
  inout_stream.WriteQWordValue(&iValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsInt64& ref_iValue)
{
  inout_stream.ReadQWordValue(&ref_iValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const nsInt64* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(nsInt64) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, nsInt64* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(nsInt64) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


/// float and double versions

inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, float fValue)
{
  inout_stream.WriteDWordValue(&fValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, float& ref_fValue)
{
  inout_stream.ReadDWordValue(&ref_fValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const float* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(float) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, float* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(float) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, double fValue)
{
  inout_stream.WriteQWordValue(&fValue).AssertSuccess();
  return inout_stream;
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, double& ref_fValue)
{
  inout_stream.ReadQWordValue(&ref_fValue).AssertSuccess();
  return inout_stream;
}

inline nsResult SerializeArray(nsStreamWriter& inout_stream, const double* pArray, nsUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(double) * uiCount);
}

inline nsResult DeserializeArray(nsStreamReader& inout_stream, double* pArray, nsUInt64 uiCount)
{
  const nsUInt64 uiNumBytes = sizeof(double) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return NS_SUCCESS;

  return NS_FAILURE;
}


// C-style strings
// No read equivalent for C-style strings (but can be read as nsString & nsStringBuilder instances)

NS_FOUNDATION_DLL nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const char* szValue);
NS_FOUNDATION_DLL nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsStringView sValue);

// nsHybridString

template <nsUInt16 Size, typename AllocatorWrapper>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsHybridString<Size, AllocatorWrapper>& sValue)
{
  inout_stream.WriteString(sValue.GetView()).AssertSuccess();
  return inout_stream;
}

template <nsUInt16 Size, typename AllocatorWrapper>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsHybridString<Size, AllocatorWrapper>& out_sValue)
{
  nsStringBuilder builder;
  inout_stream.ReadString(builder).AssertSuccess();
  out_sValue = std::move(builder);

  return inout_stream;
}

// nsStringBuilder

NS_FOUNDATION_DLL nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsStringBuilder& sValue);
NS_FOUNDATION_DLL nsStreamReader& operator>>(nsStreamReader& inout_stream, nsStringBuilder& out_sValue);

// nsEnum

template <typename T>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsEnum<T>& value)
{
  inout_stream << value.GetValue();

  return inout_stream;
}

template <typename T>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsEnum<T>& value)
{
  typename T::StorageType storedValue = T::Default;
  inout_stream >> storedValue;
  value.SetValue(storedValue);

  return inout_stream;
}

// nsBitflags

template <typename T>
inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsBitflags<T>& value)
{
  inout_stream << value.GetValue();

  return inout_stream;
}

template <typename T>
inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsBitflags<T>& value)
{
  typename T::StorageType storedValue = T::Default;
  inout_stream >> storedValue;
  value.SetValue(storedValue);

  return inout_stream;
}
