#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>

// Ensure that we can retrieve the base data type with this simple bit operation
static_assert(((int)wdProcessingStream::DataType::Half3 & ~3) == (int)wdProcessingStream::DataType::Half);
static_assert(((int)wdProcessingStream::DataType::Float4 & ~3) == (int)wdProcessingStream::DataType::Float);
static_assert(((int)wdProcessingStream::DataType::Byte2 & ~3) == (int)wdProcessingStream::DataType::Byte);
static_assert(((int)wdProcessingStream::DataType::Short3 & ~3) == (int)wdProcessingStream::DataType::Short);
static_assert(((int)wdProcessingStream::DataType::Int4 & ~3) == (int)wdProcessingStream::DataType::Int);

#if WD_ENABLED(WD_PLATFORM_64BIT)
static_assert(sizeof(wdProcessingStream) == 32);
#endif

wdProcessingStream::wdProcessingStream() = default;

wdProcessingStream::wdProcessingStream(const wdHashedString& sName, DataType type, wdUInt16 uiStride, wdUInt16 uiAlignment)
  : m_uiAlignment(uiAlignment)
  , m_uiTypeSize(GetDataTypeSize(type))
  , m_uiStride(uiStride)
  , m_Type(type)
  , m_sName(sName)
{
}

wdProcessingStream::wdProcessingStream(const wdHashedString& sName, wdArrayPtr<wdUInt8> data, DataType type, wdUInt16 uiStride)
  : m_pData(data.GetPtr())
  , m_uiDataSize(data.GetCount())
  , m_uiTypeSize(GetDataTypeSize(type))
  , m_uiStride(uiStride)
  , m_Type(type)
  , m_bExternalMemory(true)
  , m_sName(sName)
{
}

wdProcessingStream::wdProcessingStream(const wdHashedString& sName, wdArrayPtr<wdUInt8> data, DataType type)
  : m_pData(data.GetPtr())
  , m_uiDataSize(data.GetCount())
  , m_uiTypeSize(GetDataTypeSize(type))
  , m_uiStride(m_uiTypeSize)
  , m_Type(type)
  , m_bExternalMemory(true)
  , m_sName(sName)
{
}

wdProcessingStream::~wdProcessingStream()
{
  FreeData();
}

void wdProcessingStream::SetSize(wdUInt64 uiNumElements)
{
  wdUInt64 uiNewDataSize = uiNumElements * m_uiTypeSize;
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
    m_pData = wdFoundation::GetAlignedAllocator()->Allocate(static_cast<size_t>(uiNewDataSize), static_cast<size_t>(m_uiAlignment));
  }
  else
  {
    m_pData = wdFoundation::GetDefaultAllocator()->Allocate(static_cast<size_t>(uiNewDataSize), 0);
  }

  WD_ASSERT_DEV(m_pData != nullptr, "Allocating {0} elements of {1} bytes each, with {2} bytes alignment, failed", uiNumElements, ((wdUInt32)GetDataTypeSize(m_Type)), m_uiAlignment);
  m_uiDataSize = uiNewDataSize;
}

void wdProcessingStream::FreeData()
{
  if (m_pData != nullptr && m_bExternalMemory == false)
  {
    if (m_uiAlignment > 0)
    {
      wdFoundation::GetAlignedAllocator()->Deallocate(m_pData);
    }
    else
    {
      wdFoundation::GetDefaultAllocator()->Deallocate(m_pData);
    }
  }

  m_pData = nullptr;
  m_uiDataSize = 0;
}

static wdUInt16 s_TypeSize[] = {
  2, // Half,
  4, // Half2,
  6, // Half3,
  8, // Half4,

  4,  // Float,
  8,  // Float2,
  12, // Float3,
  16, // Float4,

  1, // Byte,
  2, // Byte2,
  3, // Byte3,
  4, // Byte4,

  2, // Short,
  4, // Short2,
  6, // Short3,
  8, // Short4,

  4,  // Int,
  8,  // Int2,
  12, // Int3,
  16, // Int4,
};
static_assert(WD_ARRAY_SIZE(s_TypeSize) == (size_t)wdProcessingStream::DataType::Count);

// static
wdUInt16 wdProcessingStream::GetDataTypeSize(DataType type)
{
  return s_TypeSize[(wdUInt32)type];
}

static wdStringView s_TypeName[] = {
  "Half"_wdsv,  // Half,
  "Half2"_wdsv, // Half2,
  "Half3"_wdsv, // Half3,
  "Half4"_wdsv, // Half4,

  "Float"_wdsv,  // Float,
  "Float2"_wdsv, // Float2,
  "Float3"_wdsv, // Float3,
  "Float4"_wdsv, // Float4,

  "Byte"_wdsv,  // Byte,
  "Byte2"_wdsv, // Byte2,
  "Byte3"_wdsv, // Byte3,
  "Byte4"_wdsv, // Byte4,

  "Short"_wdsv,  // Short,
  "Short2"_wdsv, // Short2,
  "Short3"_wdsv, // Short3,
  "Short4"_wdsv, // Short4,

  "Int"_wdsv,  // Int,
  "Int2"_wdsv, // Int2,
  "Int3"_wdsv, // Int3,
  "Int4"_wdsv, // Int4,
};
static_assert(WD_ARRAY_SIZE(s_TypeName) == (size_t)wdProcessingStream::DataType::Count);

// static
wdStringView wdProcessingStream::GetDataTypeName(DataType type)
{
  return s_TypeName[(wdUInt32)type];
}

WD_STATICLINK_FILE(Foundation, Foundation_DataProcessing_Stream_Implementation_ProcessingStream);
