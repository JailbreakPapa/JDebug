#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/HashedString.h>

/// \brief A single stream in a stream group holding contiguous data of a given type.
class WD_FOUNDATION_DLL wdProcessingStream
{
public:
  /// \brief The data types which can be stored in the stream.
  /// When adding new data types the GetDataTypeSize() of wdProcessingStream needs to be updated.
  enum class DataType : wdUInt8
  {
    Half,  // wdFloat16
    Half2, // 2x wdFloat16
    Half3, // 3x wdFloat16
    Half4, // 4x wdFloat16

    Float,  // float
    Float2, // 2x float, e.g. wdVec2
    Float3, // 3x float, e.g. wdVec3
    Float4, // 4x float, e.g. wdVec4

    Byte,
    Byte2,
    Byte3,
    Byte4,

    Short,
    Short2,
    Short3,
    Short4,

    Int,
    Int2,
    Int3,
    Int4,

    Count
  };

  wdProcessingStream();
  wdProcessingStream(const wdHashedString& sName, DataType type, wdUInt16 uiStride, wdUInt16 uiAlignment);
  wdProcessingStream(const wdHashedString& sName, wdArrayPtr<wdUInt8> data, DataType type, wdUInt16 uiStride);
  wdProcessingStream(const wdHashedString& sName, wdArrayPtr<wdUInt8> data, DataType type);
  ~wdProcessingStream();

  /// \brief Returns a const pointer to the data casted to the type T, note that no type check is done!
  template <typename T>
  const T* GetData() const
  {
    return static_cast<const T*>(GetData());
  }

  /// \brief Returns a const pointer to the start of the data block.
  const void* GetData() const { return m_pData; }

  /// \brief Returns a non-const pointer to the data casted to the type T, note that no type check is done!
  template <typename T>
  T* GetWritableData() const
  {
    return static_cast<T*>(GetWritableData());
  }

  /// \brief Returns a non-const pointer to the start of the data block.
  void* GetWritableData() const { return m_pData; }

  wdUInt64 GetDataSize() const { return m_uiDataSize; }

  /// \brief Returns the name of the stream
  const wdHashedString& GetName() const { return m_sName; }

  /// \brief Returns the alignment which was used to allocate the stream.
  wdUInt16 GetAlignment() const { return m_uiAlignment; }

  /// \brief Returns the data type of the stream.
  DataType GetDataType() const { return m_Type; }

  /// \brief Returns the size of one stream element in bytes.
  wdUInt16 GetElementSize() const { return m_uiTypeSize; }

  /// \brief Returns the stride between two elements of the stream in bytes.
  wdUInt16 GetElementStride() const { return m_uiStride; }

  static wdUInt16 GetDataTypeSize(DataType type);
  static wdStringView GetDataTypeName(DataType type);

protected:
  friend class wdProcessingStreamGroup;

  void SetSize(wdUInt64 uiNumElements);
  void FreeData();

  void* m_pData = nullptr;
  wdUInt64 m_uiDataSize = 0; // in bytes

  wdUInt16 m_uiAlignment = 0;
  wdUInt16 m_uiTypeSize = 0;
  wdUInt16 m_uiStride = 0;
  DataType m_Type;
  bool m_bExternalMemory = false;

  wdHashedString m_sName;
};
