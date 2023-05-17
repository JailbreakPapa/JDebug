#pragma once

#include <Foundation/Containers/Bitfield.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Enum.h>

// Standard operators for overloads of common data types

/// bool versions

inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, bool bValue)
{
  wdUInt8 uiValue = bValue ? 1 : 0;
  inout_stream.WriteBytes(&uiValue, sizeof(wdUInt8)).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, bool& out_bValue)
{
  wdUInt8 uiValue = 0;
  WD_VERIFY(inout_stream.ReadBytes(&uiValue, sizeof(wdUInt8)) == sizeof(wdUInt8), "End of stream reached.");
  out_bValue = (uiValue != 0);
  return inout_stream;
}

/// unsigned int versions

inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdUInt8 uiValue)
{
  inout_stream.WriteBytes(&uiValue, sizeof(wdUInt8)).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdUInt8& out_uiValue)
{
  WD_VERIFY(inout_stream.ReadBytes(&out_uiValue, sizeof(wdUInt8)) == sizeof(wdUInt8), "End of stream reached.");
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdUInt8* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdUInt8) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdUInt8* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdUInt8) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdUInt16 uiValue)
{
  inout_stream.WriteWordValue(&uiValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdUInt16& ref_uiValue)
{
  inout_stream.ReadWordValue(&ref_uiValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdUInt16* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdUInt16) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdUInt16* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdUInt16) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdUInt32 uiValue)
{
  inout_stream.WriteDWordValue(&uiValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdUInt32& ref_uiValue)
{
  inout_stream.ReadDWordValue(&ref_uiValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdUInt32* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdUInt32) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdUInt32* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdUInt32) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdUInt64 uiValue)
{
  inout_stream.WriteQWordValue(&uiValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdUInt64& ref_uiValue)
{
  inout_stream.ReadQWordValue(&ref_uiValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdUInt64* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdUInt64) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdUInt64* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdUInt64) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}

/// signed int versions

inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdInt8 iValue)
{
  inout_stream.WriteBytes(reinterpret_cast<const wdUInt8*>(&iValue), sizeof(wdInt8)).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdInt8& ref_iValue)
{
  WD_VERIFY(inout_stream.ReadBytes(reinterpret_cast<wdUInt8*>(&ref_iValue), sizeof(wdInt8)) == sizeof(wdInt8), "End of stream reached.");
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdInt8* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdInt8) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdInt8* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdInt8) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdInt16 iValue)
{
  inout_stream.WriteWordValue(&iValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdInt16& ref_iValue)
{
  inout_stream.ReadWordValue(&ref_iValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdInt16* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdInt16) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdInt16* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdInt16) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdInt32 iValue)
{
  inout_stream.WriteDWordValue(&iValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdInt32& ref_iValue)
{
  inout_stream.ReadDWordValue(&ref_iValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdInt32* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdInt32) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdInt32* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdInt32) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdInt64 iValue)
{
  inout_stream.WriteQWordValue(&iValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdInt64& ref_iValue)
{
  inout_stream.ReadQWordValue(&ref_iValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const wdInt64* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(wdInt64) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, wdInt64* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(wdInt64) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


/// float and double versions

inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, float fValue)
{
  inout_stream.WriteDWordValue(&fValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, float& ref_fValue)
{
  inout_stream.ReadDWordValue(&ref_fValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const float* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(float) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, float* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(float) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, double fValue)
{
  inout_stream.WriteQWordValue(&fValue).AssertSuccess();
  return inout_stream;
}

inline wdStreamReader& operator>>(wdStreamReader& inout_stream, double& ref_fValue)
{
  inout_stream.ReadQWordValue(&ref_fValue).AssertSuccess();
  return inout_stream;
}

inline wdResult SerializeArray(wdStreamWriter& inout_stream, const double* pArray, wdUInt64 uiCount)
{
  return inout_stream.WriteBytes(pArray, sizeof(double) * uiCount);
}

inline wdResult DeserializeArray(wdStreamReader& inout_stream, double* pArray, wdUInt64 uiCount)
{
  const wdUInt64 uiNumBytes = sizeof(double) * uiCount;
  if (inout_stream.ReadBytes(pArray, uiNumBytes) == uiNumBytes)
    return WD_SUCCESS;

  return WD_FAILURE;
}


// C-style strings
// No read equivalent for C-style strings (but can be read as wdString & wdStringBuilder instances)

WD_FOUNDATION_DLL wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const char* szValue);
WD_FOUNDATION_DLL wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdStringView sValue);

// wdHybridString

template <wdUInt16 Size, typename AllocatorWrapper>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdHybridString<Size, AllocatorWrapper>& sValue)
{
  inout_stream.WriteString(sValue.GetView()).AssertSuccess();
  return inout_stream;
}

template <wdUInt16 Size, typename AllocatorWrapper>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdHybridString<Size, AllocatorWrapper>& out_sValue)
{
  wdStringBuilder builder;
  inout_stream.ReadString(builder).AssertSuccess();
  out_sValue = std::move(builder);

  return inout_stream;
}

// wdStringBuilder

WD_FOUNDATION_DLL wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdStringBuilder& sValue);
WD_FOUNDATION_DLL wdStreamReader& operator>>(wdStreamReader& inout_stream, wdStringBuilder& out_sValue);

// wdEnum

template <typename T>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdEnum<T>& value)
{
  inout_stream << value.GetValue();

  return inout_stream;
}

template <typename T>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdEnum<T>& value)
{
  typename T::StorageType storedValue = T::Default;
  inout_stream >> storedValue;
  value.SetValue(storedValue);

  return inout_stream;
}

// wdBitflags

template <typename T>
inline wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdBitflags<T>& value)
{
  inout_stream << value.GetValue();

  return inout_stream;
}

template <typename T>
inline wdStreamReader& operator>>(wdStreamReader& inout_stream, wdBitflags<T>& value)
{
  typename T::StorageType storedValue = T::Default;
  inout_stream >> storedValue;
  value.SetValue(storedValue);

  return inout_stream;
}
