#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/HashedString.h>

/// \brief A single stream in a stream group holding contiguous data of a given type.
class NS_FOUNDATION_DLL nsProcessingStream
{
public:
  /// \brief The data types which can be stored in the stream.
  /// When adding new data types the GetDataTypeSize() of nsProcessingStream needs to be updated.
  enum class DataType : nsUInt8
  {
    Half,   // nsFloat16
    Half2,  // 2x nsFloat16
    Half3,  // 3x nsFloat16
    Half4,  // 4x nsFloat16

    Float,  // float
    Float2, // 2x float, e.g. nsVec2
    Float3, // 3x float, e.g. nsVec3
    Float4, // 4x float, e.g. nsVec4

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

  nsProcessingStream();
  nsProcessingStream(const nsHashedString& sName, DataType type, nsUInt16 uiStride, nsUInt16 uiAlignment);
  nsProcessingStream(const nsHashedString& sName, nsArrayPtr<nsUInt8> data, DataType type, nsUInt16 uiStride);
  nsProcessingStream(const nsHashedString& sName, nsArrayPtr<nsUInt8> data, DataType type);
  ~nsProcessingStream();

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

  nsUInt64 GetDataSize() const { return m_uiDataSize; }

  /// \brief Returns the name of the stream
  const nsHashedString& GetName() const { return m_sName; }

  /// \brief Returns the alignment which was used to allocate the stream.
  nsUInt16 GetAlignment() const { return m_uiAlignment; }

  /// \brief Returns the data type of the stream.
  DataType GetDataType() const { return m_Type; }

  /// \brief Returns the size of one stream element in bytes.
  nsUInt16 GetElementSize() const { return m_uiTypeSize; }

  /// \brief Returns the stride between two elements of the stream in bytes.
  nsUInt16 GetElementStride() const { return m_uiStride; }

  static nsUInt16 GetDataTypeSize(DataType type);
  static nsStringView GetDataTypeName(DataType type);

protected:
  friend class nsProcessingStreamGroup;

  void SetSize(nsUInt64 uiNumElements);
  void FreeData();

  void* m_pData = nullptr;
  nsUInt64 m_uiDataSize = 0; // in bytes

  nsUInt16 m_uiAlignment = 0;
  nsUInt16 m_uiTypeSize = 0;
  nsUInt16 m_uiStride = 0;
  DataType m_Type;
  bool m_bExternalMemory = false;

  nsHashedString m_sName;
};
