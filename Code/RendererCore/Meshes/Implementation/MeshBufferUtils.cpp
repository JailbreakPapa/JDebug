#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Math/Float16.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Meshes/MeshBufferUtils.h>

namespace
{
  template <nsUInt32 Bits>
  NS_ALWAYS_INLINE nsUInt32 ColorFloatToUNorm(float value)
  {
    // Implemented according to
    // https://docs.microsoft.com/en-us/windows/desktop/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
    if (nsMath::IsNaN(value))
    {
      return 0;
    }
    else
    {
      float fMaxValue = ((1 << Bits) - 1);
      return static_cast<nsUInt32>(nsMath::Saturate(value) * fMaxValue + 0.5f);
    }
  }

  template <nsUInt32 Bits>
  constexpr inline float ColorUNormToFloat(nsUInt32 value)
  {
    // Implemented according to
    // https://docs.microsoft.com/en-us/windows/desktop/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
    nsUInt32 uiMaxValue = ((1 << Bits) - 1);
    float fMaxValue = ((1 << Bits) - 1);
    return (value & uiMaxValue) * (1.0f / fMaxValue);
  }
} // namespace

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsMeshNormalPrecision, 1)
  NS_ENUM_CONSTANT(nsMeshNormalPrecision::_10Bit),
  NS_ENUM_CONSTANT(nsMeshNormalPrecision::_16Bit),
  NS_ENUM_CONSTANT(nsMeshNormalPrecision::_32Bit),
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsMeshTexCoordPrecision, 1)
  NS_ENUM_CONSTANT(nsMeshTexCoordPrecision::_16Bit),
  NS_ENUM_CONSTANT(nsMeshTexCoordPrecision::_32Bit),
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsMeshBoneWeigthPrecision, 1)
  NS_ENUM_CONSTANT(nsMeshBoneWeigthPrecision::_8Bit),
  NS_ENUM_CONSTANT(nsMeshBoneWeigthPrecision::_10Bit),
  NS_ENUM_CONSTANT(nsMeshBoneWeigthPrecision::_16Bit),
  NS_ENUM_CONSTANT(nsMeshBoneWeigthPrecision::_32Bit),
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsMeshVertexColorConversion, 1)
  NS_ENUM_CONSTANT(nsMeshVertexColorConversion::None),
  NS_ENUM_CONSTANT(nsMeshVertexColorConversion::LinearToSrgb),
  NS_ENUM_CONSTANT(nsMeshVertexColorConversion::SrgbToLinear),
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

// static
nsResult nsMeshBufferUtils::EncodeFromFloat(const float fSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  NS_ASSERT_DEBUG(dest.GetCount() >= nsGALResourceFormat::GetBitsPerElement(destFormat) / 8, "Destination buffer is too small");

  switch (destFormat)
  {
    case nsGALResourceFormat::RFloat:
      *reinterpret_cast<float*>(dest.GetPtr()) = fSource;
      return NS_SUCCESS;
    case nsGALResourceFormat::RHalf:
      *reinterpret_cast<nsFloat16*>(dest.GetPtr()) = fSource;
      return NS_SUCCESS;
    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::EncodeFromVec2(const nsVec2& vSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  NS_ASSERT_DEBUG(dest.GetCount() >= nsGALResourceFormat::GetBitsPerElement(destFormat) / 8, "Destination buffer is too small");

  switch (destFormat)
  {
    case nsGALResourceFormat::RGFloat:
      *reinterpret_cast<nsVec2*>(dest.GetPtr()) = vSource;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGHalf:
      *reinterpret_cast<nsFloat16Vec2*>(dest.GetPtr()) = vSource;
      return NS_SUCCESS;

    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::EncodeFromVec3(const nsVec3& vSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  NS_ASSERT_DEBUG(dest.GetCount() >= nsGALResourceFormat::GetBitsPerElement(destFormat) / 8, "Destination buffer is too small");

  switch (destFormat)
  {
    case nsGALResourceFormat::RGBFloat:
      *reinterpret_cast<nsVec3*>(dest.GetPtr()) = vSource;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUShortNormalized:
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[0] = nsMath::ColorFloatToShort(vSource.x);
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[1] = nsMath::ColorFloatToShort(vSource.y);
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[2] = nsMath::ColorFloatToShort(vSource.z);
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[3] = 0;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAShortNormalized:
      reinterpret_cast<nsInt16*>(dest.GetPtr())[0] = nsMath::ColorFloatToSignedShort(vSource.x);
      reinterpret_cast<nsInt16*>(dest.GetPtr())[1] = nsMath::ColorFloatToSignedShort(vSource.y);
      reinterpret_cast<nsInt16*>(dest.GetPtr())[2] = nsMath::ColorFloatToSignedShort(vSource.z);
      reinterpret_cast<nsInt16*>(dest.GetPtr())[3] = 0;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGB10A2UIntNormalized:
      *reinterpret_cast<nsUInt32*>(dest.GetPtr()) = ColorFloatToUNorm<10>(vSource.x);
      *reinterpret_cast<nsUInt32*>(dest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.y) << 10;
      *reinterpret_cast<nsUInt32*>(dest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.z) << 20;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUByteNormalized:
      dest.GetPtr()[0] = nsMath::ColorFloatToByte(vSource.x);
      dest.GetPtr()[1] = nsMath::ColorFloatToByte(vSource.y);
      dest.GetPtr()[2] = nsMath::ColorFloatToByte(vSource.z);
      dest.GetPtr()[3] = 0;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAByteNormalized:
      dest.GetPtr()[0] = nsMath::ColorFloatToSignedByte(vSource.x);
      dest.GetPtr()[1] = nsMath::ColorFloatToSignedByte(vSource.y);
      dest.GetPtr()[2] = nsMath::ColorFloatToSignedByte(vSource.z);
      dest.GetPtr()[3] = 0;
      return NS_SUCCESS;
    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::EncodeFromVec4(const nsVec4& vSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  NS_ASSERT_DEBUG(dest.GetCount() >= nsGALResourceFormat::GetBitsPerElement(destFormat) / 8, "Destination buffer is too small");

  switch (destFormat)
  {
    case nsGALResourceFormat::RGBAFloat:
      *reinterpret_cast<nsVec4*>(dest.GetPtr()) = vSource;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAHalf:
      *reinterpret_cast<nsFloat16Vec4*>(dest.GetPtr()) = vSource;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUShortNormalized:
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[0] = nsMath::ColorFloatToShort(vSource.x);
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[1] = nsMath::ColorFloatToShort(vSource.y);
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[2] = nsMath::ColorFloatToShort(vSource.z);
      reinterpret_cast<nsUInt16*>(dest.GetPtr())[3] = nsMath::ColorFloatToShort(vSource.w);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAShortNormalized:
      reinterpret_cast<nsInt16*>(dest.GetPtr())[0] = nsMath::ColorFloatToSignedShort(vSource.x);
      reinterpret_cast<nsInt16*>(dest.GetPtr())[1] = nsMath::ColorFloatToSignedShort(vSource.y);
      reinterpret_cast<nsInt16*>(dest.GetPtr())[2] = nsMath::ColorFloatToSignedShort(vSource.z);
      reinterpret_cast<nsInt16*>(dest.GetPtr())[3] = nsMath::ColorFloatToSignedShort(vSource.w);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGB10A2UIntNormalized:
      *reinterpret_cast<nsUInt32*>(dest.GetPtr()) = ColorFloatToUNorm<10>(vSource.x);
      *reinterpret_cast<nsUInt32*>(dest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.y) << 10;
      *reinterpret_cast<nsUInt32*>(dest.GetPtr()) |= ColorFloatToUNorm<10>(vSource.z) << 20;
      *reinterpret_cast<nsUInt32*>(dest.GetPtr()) |= ColorFloatToUNorm<2>(vSource.w) << 30;
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUByteNormalized:
      dest.GetPtr()[0] = nsMath::ColorFloatToByte(vSource.x);
      dest.GetPtr()[1] = nsMath::ColorFloatToByte(vSource.y);
      dest.GetPtr()[2] = nsMath::ColorFloatToByte(vSource.z);
      dest.GetPtr()[3] = nsMath::ColorFloatToByte(vSource.w);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAByteNormalized:
      dest.GetPtr()[0] = nsMath::ColorFloatToSignedByte(vSource.x);
      dest.GetPtr()[1] = nsMath::ColorFloatToSignedByte(vSource.y);
      dest.GetPtr()[2] = nsMath::ColorFloatToSignedByte(vSource.z);
      dest.GetPtr()[3] = nsMath::ColorFloatToSignedByte(vSource.w);
      return NS_SUCCESS;

    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::DecodeToFloat(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, float& ref_fDest)
{
  NS_ASSERT_DEBUG(source.GetCount() >= nsGALResourceFormat::GetBitsPerElement(sourceFormat) / 8, "Source buffer is too small");

  switch (sourceFormat)
  {
    case nsGALResourceFormat::RFloat:
      ref_fDest = *reinterpret_cast<const float*>(source.GetPtr());
      return NS_SUCCESS;
    case nsGALResourceFormat::RHalf:
      ref_fDest = *reinterpret_cast<const nsFloat16*>(source.GetPtr());
      return NS_SUCCESS;
    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::DecodeToVec2(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec2& ref_vDest)
{
  NS_ASSERT_DEBUG(source.GetCount() >= nsGALResourceFormat::GetBitsPerElement(sourceFormat) / 8, "Source buffer is too small");

  switch (sourceFormat)
  {
    case nsGALResourceFormat::RGFloat:
      ref_vDest = *reinterpret_cast<const nsVec2*>(source.GetPtr());
      return NS_SUCCESS;
    case nsGALResourceFormat::RGHalf:
      ref_vDest = *reinterpret_cast<const nsFloat16Vec2*>(source.GetPtr());
      return NS_SUCCESS;
    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::DecodeToVec3(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec3& ref_vDest)
{
  NS_ASSERT_DEBUG(source.GetCount() >= nsGALResourceFormat::GetBitsPerElement(sourceFormat) / 8, "Source buffer is too small");

  switch (sourceFormat)
  {
    case nsGALResourceFormat::RGBFloat:
      ref_vDest = *reinterpret_cast<const nsVec3*>(source.GetPtr());
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUShortNormalized:
      ref_vDest.x = nsMath::ColorShortToFloat(reinterpret_cast<const nsUInt16*>(source.GetPtr())[0]);
      ref_vDest.y = nsMath::ColorShortToFloat(reinterpret_cast<const nsUInt16*>(source.GetPtr())[1]);
      ref_vDest.z = nsMath::ColorShortToFloat(reinterpret_cast<const nsUInt16*>(source.GetPtr())[2]);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAShortNormalized:
      ref_vDest.x = nsMath::ColorSignedShortToFloat(reinterpret_cast<const nsInt16*>(source.GetPtr())[0]);
      ref_vDest.y = nsMath::ColorSignedShortToFloat(reinterpret_cast<const nsInt16*>(source.GetPtr())[1]);
      ref_vDest.z = nsMath::ColorSignedShortToFloat(reinterpret_cast<const nsInt16*>(source.GetPtr())[2]);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGB10A2UIntNormalized:
      ref_vDest.x = ColorUNormToFloat<10>(*reinterpret_cast<const nsUInt32*>(source.GetPtr()));
      ref_vDest.y = ColorUNormToFloat<10>(*reinterpret_cast<const nsUInt32*>(source.GetPtr()) >> 10);
      ref_vDest.z = ColorUNormToFloat<10>(*reinterpret_cast<const nsUInt32*>(source.GetPtr()) >> 20);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUByteNormalized:
      ref_vDest.x = nsMath::ColorByteToFloat(source.GetPtr()[0]);
      ref_vDest.y = nsMath::ColorByteToFloat(source.GetPtr()[1]);
      ref_vDest.z = nsMath::ColorByteToFloat(source.GetPtr()[2]);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAByteNormalized:
      ref_vDest.x = nsMath::ColorSignedByteToFloat(source.GetPtr()[0]);
      ref_vDest.y = nsMath::ColorSignedByteToFloat(source.GetPtr()[1]);
      ref_vDest.z = nsMath::ColorSignedByteToFloat(source.GetPtr()[2]);
      return NS_SUCCESS;
    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::DecodeToVec4(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec4& ref_vDest)
{
  NS_ASSERT_DEBUG(source.GetCount() >= nsGALResourceFormat::GetBitsPerElement(sourceFormat) / 8, "Source buffer is too small");

  switch (sourceFormat)
  {
    case nsGALResourceFormat::RGBAFloat:
      ref_vDest = *reinterpret_cast<const nsVec4*>(source.GetPtr());
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAHalf:
      ref_vDest = *reinterpret_cast<const nsFloat16Vec4*>(source.GetPtr());
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUShortNormalized:
      ref_vDest.x = nsMath::ColorShortToFloat(reinterpret_cast<const nsUInt16*>(source.GetPtr())[0]);
      ref_vDest.y = nsMath::ColorShortToFloat(reinterpret_cast<const nsUInt16*>(source.GetPtr())[1]);
      ref_vDest.z = nsMath::ColorShortToFloat(reinterpret_cast<const nsUInt16*>(source.GetPtr())[2]);
      ref_vDest.w = nsMath::ColorShortToFloat(reinterpret_cast<const nsUInt16*>(source.GetPtr())[3]);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAShortNormalized:
      ref_vDest.x = nsMath::ColorSignedShortToFloat(reinterpret_cast<const nsInt16*>(source.GetPtr())[0]);
      ref_vDest.y = nsMath::ColorSignedShortToFloat(reinterpret_cast<const nsInt16*>(source.GetPtr())[1]);
      ref_vDest.z = nsMath::ColorSignedShortToFloat(reinterpret_cast<const nsInt16*>(source.GetPtr())[2]);
      ref_vDest.w = nsMath::ColorSignedShortToFloat(reinterpret_cast<const nsInt16*>(source.GetPtr())[3]);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGB10A2UIntNormalized:
      ref_vDest.x = ColorUNormToFloat<10>(*reinterpret_cast<const nsUInt32*>(source.GetPtr()));
      ref_vDest.y = ColorUNormToFloat<10>(*reinterpret_cast<const nsUInt32*>(source.GetPtr()) >> 10);
      ref_vDest.z = ColorUNormToFloat<10>(*reinterpret_cast<const nsUInt32*>(source.GetPtr()) >> 20);
      ref_vDest.w = ColorUNormToFloat<2>(*reinterpret_cast<const nsUInt32*>(source.GetPtr()) >> 30);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAUByteNormalized:
      ref_vDest.x = nsMath::ColorByteToFloat(source.GetPtr()[0]);
      ref_vDest.y = nsMath::ColorByteToFloat(source.GetPtr()[1]);
      ref_vDest.z = nsMath::ColorByteToFloat(source.GetPtr()[2]);
      ref_vDest.w = nsMath::ColorByteToFloat(source.GetPtr()[3]);
      return NS_SUCCESS;

    case nsGALResourceFormat::RGBAByteNormalized:
      ref_vDest.x = nsMath::ColorSignedByteToFloat(source.GetPtr()[0]);
      ref_vDest.y = nsMath::ColorSignedByteToFloat(source.GetPtr()[1]);
      ref_vDest.z = nsMath::ColorSignedByteToFloat(source.GetPtr()[2]);
      ref_vDest.w = nsMath::ColorSignedByteToFloat(source.GetPtr()[3]);
      return NS_SUCCESS;

    default:
      return NS_FAILURE;
  }
}

// static
nsResult nsMeshBufferUtils::GetPositionStream(const nsMeshBufferResourceDescriptor& meshBufferDesc, const nsVec3*& out_pPositions, nsUInt32& out_uiElementStride)
{
  const nsVertexDeclarationInfo& vdi = meshBufferDesc.GetVertexDeclaration();
  const nsUInt8* pRawVertexData = meshBufferDesc.GetVertexBufferData().GetPtr();

  const nsVec3* pPositions = nullptr;

  for (nsUInt32 vs = 0; vs < vdi.m_VertexStreams.GetCount(); ++vs)
  {
    if (vdi.m_VertexStreams[vs].m_Semantic == nsGALVertexAttributeSemantic::Position)
    {
      if (vdi.m_VertexStreams[vs].m_Format != nsGALResourceFormat::RGBFloat)
      {
        nsLog::Error("Unsupported vertex position format {0}", (int)vdi.m_VertexStreams[vs].m_Format);
        return NS_FAILURE; // other position formats are not supported
      }

      pPositions = reinterpret_cast<const nsVec3*>(pRawVertexData + vdi.m_VertexStreams[vs].m_uiOffset);
    }
  }

  if (pPositions == nullptr)
  {
    nsLog::Error("No position stream found");
    return NS_FAILURE;
  }

  out_pPositions = pPositions;
  out_uiElementStride = meshBufferDesc.GetVertexDataSize();
  return NS_SUCCESS;
}

// static
nsResult nsMeshBufferUtils::GetPositionAndNormalStream(const nsMeshBufferResourceDescriptor& meshBufferDesc, const nsVec3*& out_pPositions, const nsUInt8*& out_pNormals, nsGALResourceFormat::Enum& out_normalFormat, nsUInt32& out_uiElementStride)
{
  const nsVertexDeclarationInfo& vdi = meshBufferDesc.GetVertexDeclaration();
  const nsUInt8* pRawVertexData = meshBufferDesc.GetVertexBufferData().GetPtr();

  const nsVec3* pPositions = nullptr;
  const nsUInt8* pNormals = nullptr;
  nsGALResourceFormat::Enum normalFormat = nsGALResourceFormat::Invalid;

  for (nsUInt32 vs = 0; vs < vdi.m_VertexStreams.GetCount(); ++vs)
  {
    if (vdi.m_VertexStreams[vs].m_Semantic == nsGALVertexAttributeSemantic::Position)
    {
      if (vdi.m_VertexStreams[vs].m_Format != nsGALResourceFormat::RGBFloat)
      {
        nsLog::Error("Unsupported vertex position format {0}", (int)vdi.m_VertexStreams[vs].m_Format);
        return NS_FAILURE; // other position formats are not supported
      }

      pPositions = reinterpret_cast<const nsVec3*>(pRawVertexData + vdi.m_VertexStreams[vs].m_uiOffset);
    }
    else if (vdi.m_VertexStreams[vs].m_Semantic == nsGALVertexAttributeSemantic::Normal)
    {
      pNormals = pRawVertexData + vdi.m_VertexStreams[vs].m_uiOffset;
      normalFormat = vdi.m_VertexStreams[vs].m_Format;
    }
  }

  if (pPositions == nullptr || pNormals == nullptr)
  {
    nsLog::Error("No position and normal stream found");
    return NS_FAILURE;
  }

  nsUInt8 dummySource[16] = {};
  nsVec3 vNormal;
  if (DecodeNormal(nsMakeArrayPtr(dummySource), normalFormat, vNormal).Failed())
  {
    nsLog::Error("Unsupported vertex normal format {0}", normalFormat);
    return NS_FAILURE;
  }

  out_pPositions = pPositions;
  out_pNormals = pNormals;
  out_normalFormat = normalFormat;
  out_uiElementStride = meshBufferDesc.GetVertexDataSize();
  return NS_SUCCESS;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_MeshBufferUtils);
