#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>

// Ensure that we can retrieve the base data type with this simple bit operation
static_assert(((int)nsProcessingStream::DataType::Half3 & ~3) == (int)nsProcessingStream::DataType::Half);
static_assert(((int)nsProcessingStream::DataType::Float4 & ~3) == (int)nsProcessingStream::DataType::Float);
static_assert(((int)nsProcessingStream::DataType::Byte2 & ~3) == (int)nsProcessingStream::DataType::Byte);
static_assert(((int)nsProcessingStream::DataType::Short3 & ~3) == (int)nsProcessingStream::DataType::Short);
static_assert(((int)nsProcessingStream::DataType::Int4 & ~3) == (int)nsProcessingStream::DataType::Int);

#if NS_ENABLED(NS_PLATFORM_64BIT)
static_assert(sizeof(nsProcessingStream) == 32);
#endif

nsProcessingStream::nsProcessingStream() = default;

nsProcessingStream::nsProcessingStream(const nsHashedString& sName, DataType type, nsUInt16 uiStride, nsUInt16 uiAlignment)
  : m_uiAlignment(uiAlignment)
  , m_uiTypeSize(GetDataTypeSize(type))
  , m_uiStride(uiStride)
  , m_Type(type)
  , m_sName(sName)
{
}

nsProcessingStream::nsProcessingStream(const nsHashedString& sName, nsArrayPtr<nsUInt8> data, DataType type, nsUInt16 uiStride)
  : m_pData(data.GetPtr())
  , m_uiDataSize(data.GetCount())
  , m_uiTypeSize(GetDataTypeSize(type))
  , m_uiStride(uiStride)
  , m_Type(type)
  , m_bExternalMemory(true)
  , m_sName(sName)
{
}

nsProcessingStream::nsProcessingStream(const nsHashedString& sName, nsArrayPtr<nsUInt8> data, DataType type)
  : m_pData(data.GetPtr())
  , m_uiDataSize(data.GetCount())
  , m_uiTypeSize(GetDataTypeSize(type))
  , m_uiStride(m_uiTypeSize)
  , m_Type(type)
  , m_bExternalMemory(true)
  , m_sName(sName)
{
}

nsProcessingStream::~nsProcessingStream()
{
  FreeData();
}

void nsProcessingStream::SetSize(nsUInt64 uiNumElements)
{
  nsUInt64 uiNewDataSize = uiNumElements * m_uiTypeSize;
  if (m_uiDataSize == uiNewDataSize)
    return;

  FreeData();

  if (uiNewDataSize == 0)
  {
    return;
  }

  /// \todo Allow to reuse memory from a pool ?
  if (m_uiAlignment > 0)
  {
    m_pData = nsFoundation::GetAlignedAllocator()->Allocate(static_cast<size_t>(uiNewDataSize), static_cast<size_t>(m_uiAlignment));
  }
  else
  {
    m_pData = nsFoundation::GetDefaultAllocator()->Allocate(static_cast<size_t>(uiNewDataSize), 0);
  }

  NS_ASSERT_DEV(m_pData != nullptr, "Allocating {0} elements of {1} bytes each, with {2} bytes alignment, failed", uiNumElements, ((nsUInt32)GetDataTypeSize(m_Type)), m_uiAlignment);
  m_uiDataSize = uiNewDataSize;
}

void nsProcessingStream::FreeData()
{
  if (m_pData != nullptr && m_bExternalMemory == false)
  {
    if (m_uiAlignment > 0)
    {
      nsFoundation::GetAlignedAllocator()->Deallocate(m_pData);
    }
    else
    {
      nsFoundation::GetDefaultAllocator()->Deallocate(m_pData);
    }
  }

  m_pData = nullptr;
  m_uiDataSize = 0;
}

static nsUInt16 s_TypeSize[] = {
  2,  // Half,
  4,  // Half2,
  6,  // Half3,
  8,  // Half4,

  4,  // Float,
  8,  // Float2,
  12, // Float3,
  16, // Float4,

  1,  // Byte,
  2,  // Byte2,
  3,  // Byte3,
  4,  // Byte4,

  2,  // Short,
  4,  // Short2,
  6,  // Short3,
  8,  // Short4,

  4,  // Int,
  8,  // Int2,
  12, // Int3,
  16, // Int4,
};
static_assert(NS_ARRAY_SIZE(s_TypeSize) == (size_t)nsProcessingStream::DataType::Count);

// static
nsUInt16 nsProcessingStream::GetDataTypeSize(DataType type)
{
  return s_TypeSize[(nsUInt32)type];
}

static nsStringView s_TypeName[] = {
  "Half"_nssv,   // Half,
  "Half2"_nssv,  // Half2,
  "Half3"_nssv,  // Half3,
  "Half4"_nssv,  // Half4,

  "Float"_nssv,  // Float,
  "Float2"_nssv, // Float2,
  "Float3"_nssv, // Float3,
  "Float4"_nssv, // Float4,

  "Byte"_nssv,   // Byte,
  "Byte2"_nssv,  // Byte2,
  "Byte3"_nssv,  // Byte3,
  "Byte4"_nssv,  // Byte4,

  "Short"_nssv,  // Short,
  "Short2"_nssv, // Short2,
  "Short3"_nssv, // Short3,
  "Short4"_nssv, // Short4,

  "Int"_nssv,    // Int,
  "Int2"_nssv,   // Int2,
  "Int3"_nssv,   // Int3,
  "Int4"_nssv,   // Int4,
};
static_assert(NS_ARRAY_SIZE(s_TypeName) == (size_t)nsProcessingStream::DataType::Count);

// static
nsStringView nsProcessingStream::GetDataTypeName(DataType type)
{
  return s_TypeName[(nsUInt32)type];
}
