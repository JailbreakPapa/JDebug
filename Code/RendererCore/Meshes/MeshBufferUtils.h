
#pragma once

#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/Resources/ResourceFormats.h>

struct nsMeshBufferResourceDescriptor;

struct nsMeshNormalPrecision
{
  using StorageType = nsUInt8;

  enum Enum
  {
    _10Bit,
    _16Bit,
    _32Bit,

    Default = _10Bit
  };

  /// \brief Convert mesh normal precision to actual resource format used for normals
  static nsGALResourceFormat::Enum ToResourceFormatNormal(Enum value);

  /// \brief Convert mesh normal precision to actual resource format used for tangents
  static nsGALResourceFormat::Enum ToResourceFormatTangent(Enum value);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsMeshNormalPrecision);

struct nsMeshTexCoordPrecision
{
  using StorageType = nsUInt8;

  enum Enum
  {
    _16Bit,
    _32Bit,

    Default = _16Bit
  };

  /// \brief Convert mesh texcoord precision to actual resource format
  static nsGALResourceFormat::Enum ToResourceFormat(Enum value);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsMeshTexCoordPrecision);

struct nsMeshBoneWeigthPrecision
{
  using StorageType = nsUInt8;

  enum Enum
  {
    _8Bit,
    _10Bit,
    _16Bit,
    _32Bit,

    Default = _8Bit
  };

  /// \brief Convert mesh texcoord precision to actual resource format
  static nsGALResourceFormat::Enum ToResourceFormat(Enum value);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsMeshBoneWeigthPrecision);

struct nsMeshVertexColorConversion
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None,
    LinearToSrgb,
    SrgbToLinear,

    Default = None
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsMeshVertexColorConversion);

struct NS_RENDERERCORE_DLL nsMeshBufferUtils
{
  static nsResult EncodeNormal(const nsVec3& vNormal, nsArrayPtr<nsUInt8> dest, nsMeshNormalPrecision::Enum normalPrecision);
  static nsResult EncodeTangent(const nsVec3& vTangent, float fTangentSign, nsArrayPtr<nsUInt8> dest, nsMeshNormalPrecision::Enum tangentPrecision);
  static nsResult EncodeTexCoord(const nsVec2& vTexCoord, nsArrayPtr<nsUInt8> dest, nsMeshTexCoordPrecision::Enum texCoordPrecision);
  static nsResult EncodeBoneWeights(const nsVec4& vWeights, nsArrayPtr<nsUInt8> dest, nsMeshBoneWeigthPrecision::Enum precision);
  static nsResult EncodeColor(const nsVec4& vColor, nsArrayPtr<nsUInt8> dest, nsMeshVertexColorConversion::Enum conversion);

  static nsResult EncodeNormal(const nsVec3& vNormal, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);
  static nsResult EncodeTangent(const nsVec3& vTangent, float fTangentSign, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);
  static nsResult EncodeTexCoord(const nsVec2& vTexCoord, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);
  static nsResult EncodeBoneWeights(const nsVec4& vWeights, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);

  static nsResult DecodeNormal(nsArrayPtr<const nsUInt8> source, nsVec3& ref_vDestNormal, nsMeshNormalPrecision::Enum normalPrecision);
  static nsResult DecodeTangent(
    nsArrayPtr<const nsUInt8> source, nsVec3& ref_vDestTangent, float& ref_fDestBiTangentSign, nsMeshNormalPrecision::Enum tangentPrecision);
  static nsResult DecodeTexCoord(nsArrayPtr<const nsUInt8> source, nsVec2& ref_vDestTexCoord, nsMeshTexCoordPrecision::Enum texCoordPrecision);

  static nsResult DecodeNormal(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec3& ref_vDestNormal);
  static nsResult DecodeTangent(
    nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec3& ref_vDestTangent, float& ref_fDestBiTangentSign);
  static nsResult DecodeTexCoord(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec2& ref_vDestTexCoord);

  // low level conversion functions
  static nsResult EncodeFromFloat(const float fSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);
  static nsResult EncodeFromVec2(const nsVec2& vSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);
  static nsResult EncodeFromVec3(const nsVec3& vSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);
  static nsResult EncodeFromVec4(const nsVec4& vSource, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat);

  static nsResult DecodeToFloat(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, float& ref_fDest);
  static nsResult DecodeToVec2(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec2& ref_vDest);
  static nsResult DecodeToVec3(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec3& ref_vDest);
  static nsResult DecodeToVec4(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec4& ref_vDest);

  /// \brief Helper function to get the position stream from the given mesh buffer descriptor
  static nsResult GetPositionStream(const nsMeshBufferResourceDescriptor& meshBufferDesc, const nsVec3*& out_pPositions, nsUInt32& out_uiElementStride);

  /// \brief Helper function to get the position and normal stream from the given mesh buffer descriptor
  static nsResult GetPositionAndNormalStream(const nsMeshBufferResourceDescriptor& meshBufferDesc, const nsVec3*& out_pPositions, const nsUInt8*& out_pNormals, nsGALResourceFormat::Enum& out_normalFormat, nsUInt32& out_uiElementStride);
};

#include <RendererCore/Meshes/Implementation/MeshBufferUtils_inl.h>
