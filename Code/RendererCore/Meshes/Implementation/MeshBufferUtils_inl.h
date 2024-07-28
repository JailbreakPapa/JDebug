
// static
NS_ALWAYS_INLINE nsGALResourceFormat::Enum nsMeshNormalPrecision::ToResourceFormatNormal(Enum value)
{
  return value == _10Bit ? nsGALResourceFormat::RGB10A2UIntNormalized
                         : (value == _16Bit ? nsGALResourceFormat::RGBAUShortNormalized : nsGALResourceFormat::XYZFloat);
}

// static
NS_ALWAYS_INLINE nsGALResourceFormat::Enum nsMeshNormalPrecision::ToResourceFormatTangent(Enum value)
{
  return value == _10Bit ? nsGALResourceFormat::RGB10A2UIntNormalized
                         : (value == _16Bit ? nsGALResourceFormat::RGBAUShortNormalized : nsGALResourceFormat::XYZWFloat);
}

//////////////////////////////////////////////////////////////////////////

// static
NS_ALWAYS_INLINE nsGALResourceFormat::Enum nsMeshTexCoordPrecision::ToResourceFormat(Enum value)
{
  return value == _16Bit ? nsGALResourceFormat::UVHalf : nsGALResourceFormat::UVFloat;
}

//////////////////////////////////////////////////////////////////////////

// static
NS_ALWAYS_INLINE nsGALResourceFormat::Enum nsMeshBoneWeigthPrecision::ToResourceFormat(Enum value)
{
  switch (value)
  {
    case _8Bit:
      return nsGALResourceFormat::RGBAUByteNormalized;
    case _10Bit:
      return nsGALResourceFormat::RGB10A2UIntNormalized;
    case _16Bit:
      return nsGALResourceFormat::RGBAUShortNormalized;
    case _32Bit:
      return nsGALResourceFormat::RGBAFloat;
      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return nsGALResourceFormat::RGBAUByteNormalized;
}

//////////////////////////////////////////////////////////////////////////

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeNormal(const nsVec3& vNormal, nsArrayPtr<nsUInt8> dest, nsMeshNormalPrecision::Enum normalPrecision)
{
  return EncodeNormal(vNormal, dest, nsMeshNormalPrecision::ToResourceFormatNormal(normalPrecision));
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeTangent(const nsVec3& vTangent, float fTangentSign, nsArrayPtr<nsUInt8> dest, nsMeshNormalPrecision::Enum tangentPrecision)
{
  return EncodeTangent(vTangent, fTangentSign, dest, nsMeshNormalPrecision::ToResourceFormatTangent(tangentPrecision));
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeTexCoord(const nsVec2& vTexCoord, nsArrayPtr<nsUInt8> dest, nsMeshTexCoordPrecision::Enum texCoordPrecision)
{
  return EncodeTexCoord(vTexCoord, dest, nsMeshTexCoordPrecision::ToResourceFormat(texCoordPrecision));
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeBoneWeights(const nsVec4& vWeights, nsArrayPtr<nsUInt8> dest, nsMeshBoneWeigthPrecision::Enum precision)
{
  return EncodeBoneWeights(vWeights, dest, nsMeshBoneWeigthPrecision::ToResourceFormat(precision));
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeColor(const nsVec4& vColor, nsArrayPtr<nsUInt8> dest, nsMeshVertexColorConversion::Enum conversion)
{
  nsVec4 finalColor;
  if (conversion == nsMeshVertexColorConversion::LinearToSrgb)
  {
    finalColor = nsColor::LinearToGamma(vColor.GetAsVec3()).GetAsVec4(vColor.w);
  }
  else if (conversion == nsMeshVertexColorConversion::SrgbToLinear)
  {
    finalColor = nsColor::GammaToLinear(vColor.GetAsVec3()).GetAsVec4(vColor.w);
  }
  else
  {
    finalColor = vColor;
  }

  return EncodeFromVec4(finalColor, dest, nsGALResourceFormat::RGBAUByteNormalized);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeNormal(const nsVec3& vNormal, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  // we store normals in unsigned formats thus we need to map from -1..1 to 0..1 here
  return EncodeFromVec3(vNormal * 0.5f + nsVec3(0.5f), dest, destFormat);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeTangent(const nsVec3& vTangent, float fTangentSign, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  // make sure biTangentSign is either -1 or 1
  fTangentSign = (fTangentSign < 0.0f) ? -1.0f : 1.0f;

  // we store tangents in unsigned formats thus we need to map from -1..1 to 0..1 here
  return EncodeFromVec4(vTangent.GetAsVec4(fTangentSign) * 0.5f + nsVec4(0.5f), dest, destFormat);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeTexCoord(const nsVec2& vTexCoord, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  return EncodeFromVec2(vTexCoord, dest, destFormat);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::EncodeBoneWeights(const nsVec4& vWeights, nsArrayPtr<nsUInt8> dest, nsGALResourceFormat::Enum destFormat)
{
  return EncodeFromVec4(vWeights, dest, destFormat);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::DecodeNormal(nsArrayPtr<const nsUInt8> source, nsVec3& ref_vDestNormal, nsMeshNormalPrecision::Enum normalPrecision)
{
  return DecodeNormal(source, nsMeshNormalPrecision::ToResourceFormatNormal(normalPrecision), ref_vDestNormal);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::DecodeTangent(nsArrayPtr<const nsUInt8> source, nsVec3& ref_vDestTangent, float& ref_fDestBiTangentSign, nsMeshNormalPrecision::Enum tangentPrecision)
{
  return DecodeTangent(source, nsMeshNormalPrecision::ToResourceFormatTangent(tangentPrecision), ref_vDestTangent, ref_fDestBiTangentSign);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::DecodeTexCoord(nsArrayPtr<const nsUInt8> source, nsVec2& ref_vDestTexCoord, nsMeshTexCoordPrecision::Enum texCoordPrecision)
{
  return DecodeTexCoord(source, nsMeshTexCoordPrecision::ToResourceFormat(texCoordPrecision), ref_vDestTexCoord);
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::DecodeNormal(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec3& ref_vDestNormal)
{
  nsVec3 tempNormal;
  NS_SUCCEED_OR_RETURN(DecodeToVec3(source, sourceFormat, tempNormal));
  ref_vDestNormal = tempNormal * 2.0f - nsVec3(1.0f);
  return NS_SUCCESS;
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::DecodeTangent(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec3& ref_vDestTangent, float& ref_fDestBiTangentSign)
{
  nsVec4 tempTangent;
  NS_SUCCEED_OR_RETURN(DecodeToVec4(source, sourceFormat, tempTangent));
  ref_vDestTangent = tempTangent.GetAsVec3() * 2.0f - nsVec3(1.0f);
  ref_fDestBiTangentSign = tempTangent.w * 2.0f - 1.0f;
  return NS_SUCCESS;
}

// static
NS_ALWAYS_INLINE nsResult nsMeshBufferUtils::DecodeTexCoord(nsArrayPtr<const nsUInt8> source, nsGALResourceFormat::Enum sourceFormat, nsVec2& ref_vDestTexCoord)
{
  return DecodeToVec2(source, sourceFormat, ref_vDestTexCoord);
}
