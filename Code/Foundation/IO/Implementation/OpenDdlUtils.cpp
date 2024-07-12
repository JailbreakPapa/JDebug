#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/RTTI.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Uuid.h>
#include <Foundation/Types/Variant.h>
#include <Foundation/Types/VariantTypeRegistry.h>

nsResult nsOpenDdlUtils::ConvertToColor(const nsOpenDdlReaderElement* pElement, nsColor& out_result)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = pValues[3];

      return NS_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = 1.0f;

      return NS_SUCCESS;
    }
  }
  else if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::UInt8)
  {
    const nsUInt8* pValues = pElement->GetPrimitivesUInt8();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result = nsColorGammaUB(pValues[0], pValues[1], pValues[2], pValues[3]);

      return NS_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result = nsColorGammaUB(pValues[0], pValues[1], pValues[2]);

      return NS_SUCCESS;
    }
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToColorGamma(const nsOpenDdlReaderElement* pElement, nsColorGammaUB& out_result)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result = nsColor(pValues[0], pValues[1], pValues[2], pValues[3]);

      return NS_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result = nsColor(pValues[0], pValues[1], pValues[2]);

      return NS_SUCCESS;
    }
  }
  else if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::UInt8)
  {
    const nsUInt8* pValues = pElement->GetPrimitivesUInt8();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = pValues[3];

      return NS_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = 255;

      return NS_SUCCESS;
    }
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToTime(const nsOpenDdlReaderElement* pElement, nsTime& out_result)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_result = nsTime::MakeFromSeconds(pValues[0]);

    return NS_SUCCESS;
  }

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Double)
  {
    const double* pValues = pElement->GetPrimitivesDouble();

    out_result = nsTime::MakeFromSeconds(pValues[0]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec2(const nsOpenDdlReaderElement* pElement, nsVec2& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec3(const nsOpenDdlReaderElement* pElement, nsVec3& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec4(const nsOpenDdlReaderElement* pElement, nsVec4& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec2I(const nsOpenDdlReaderElement* pElement, nsVec2I32& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Int32)
  {
    const nsInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec3I(const nsOpenDdlReaderElement* pElement, nsVec3I32& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Int32)
  {
    const nsInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec4I(const nsOpenDdlReaderElement* pElement, nsVec4I32& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Int32)
  {
    const nsInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}


nsResult nsOpenDdlUtils::ConvertToVec2U(const nsOpenDdlReaderElement* pElement, nsVec2U32& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::UInt32)
  {
    const nsUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec3U(const nsOpenDdlReaderElement* pElement, nsVec3U32& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::UInt32)
  {
    const nsUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVec4U(const nsOpenDdlReaderElement* pElement, nsVec4U32& out_vResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::UInt32)
  {
    const nsUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}



nsResult nsOpenDdlUtils::ConvertToMat3(const nsOpenDdlReaderElement* pElement, nsMat3& out_mResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 9)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_mResult = nsMat3::MakeFromColumnMajorArray(pValues);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToMat4(const nsOpenDdlReaderElement* pElement, nsMat4& out_mResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 16)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_mResult = nsMat4::MakeFromColumnMajorArray(pValues);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}


nsResult nsOpenDdlUtils::ConvertToTransform(const nsOpenDdlReaderElement* pElement, nsTransform& out_result)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 10)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_result.m_vPosition.x = pValues[0];
    out_result.m_vPosition.y = pValues[1];
    out_result.m_vPosition.z = pValues[2];
    out_result.m_qRotation.x = pValues[3];
    out_result.m_qRotation.y = pValues[4];
    out_result.m_qRotation.z = pValues[5];
    out_result.m_qRotation.w = pValues[6];
    out_result.m_vScale.x = pValues[7];
    out_result.m_vScale.y = pValues[8];
    out_result.m_vScale.z = pValues[9];

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToQuat(const nsOpenDdlReaderElement* pElement, nsQuat& out_qResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_qResult = nsQuat(pValues[0], pValues[1], pValues[2], pValues[3]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToUuid(const nsOpenDdlReaderElement* pElement, nsUuid& out_result)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::UInt64)
  {
    const nsUInt64* pValues = pElement->GetPrimitivesUInt64();

    out_result = nsUuid(pValues[0], pValues[1]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToAngle(const nsOpenDdlReaderElement* pElement, nsAngle& out_result)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    // have to use radians to prevent precision loss
    out_result = nsAngle::MakeFromRadian(pValues[0]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToHashedString(const nsOpenDdlReaderElement* pElement, nsHashedString& out_sResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::String)
  {
    const nsStringView* pValues = pElement->GetPrimitivesString();

    out_sResult.Assign(pValues[0]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToTempHashedString(const nsOpenDdlReaderElement* pElement, nsTempHashedString& out_sResult)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return NS_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return NS_FAILURE;

  if (pElement->GetPrimitivesType() == nsOpenDdlPrimitiveType::UInt64)
  {
    const nsUInt64* pValues = pElement->GetPrimitivesUInt64();

    out_sResult = nsTempHashedString(pValues[0]);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsOpenDdlUtils::ConvertToVariant(const nsOpenDdlReaderElement* pElement, nsVariant& out_result)
{
  if (pElement == nullptr)
    return NS_FAILURE;

  // expect a custom type
  if (pElement->IsCustomType())
  {
    if (pElement->GetCustomType() == "VarArray")
    {
      nsVariantArray value;
      nsVariant varChild;

      /// \test This is just quickly hacked
      /// \todo Store array size for reserving var array length

      for (const nsOpenDdlReaderElement* pChild = pElement->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
      {
        if (ConvertToVariant(pChild, varChild).Failed())
          return NS_FAILURE;

        value.PushBack(varChild);
      }

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "VarDict")
    {
      nsVariantDictionary value;
      nsVariant varChild;

      /// \test This is just quickly hacked
      /// \todo Store array size for reserving var array length

      for (const nsOpenDdlReaderElement* pChild = pElement->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
      {
        // no name -> invalid dictionary entry
        if (!pChild->HasName())
          continue;

        if (ConvertToVariant(pChild, varChild).Failed())
          return NS_FAILURE;

        value[pChild->GetName()] = varChild;
      }

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "VarDataBuffer")
    {
      /// \test This is just quickly hacked

      nsDataBuffer value;

      const nsOpenDdlReaderElement* pString = pElement->GetFirstChild();

      if (!pString->HasPrimitives(nsOpenDdlPrimitiveType::String))
        return NS_FAILURE;

      const nsStringView* pValues = pString->GetPrimitivesString();

      value.SetCountUninitialized(pValues[0].GetElementCount() / 2);
      nsConversionUtils::ConvertHexToBinary(pValues[0].GetStartPointer(), value.GetData(), value.GetCount());

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Color")
    {
      nsColor value;
      if (ConvertToColor(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "ColorGamma")
    {
      nsColorGammaUB value;
      if (ConvertToColorGamma(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Time")
    {
      nsTime value;
      if (ConvertToTime(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec2")
    {
      nsVec2 value;
      if (ConvertToVec2(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec3")
    {
      nsVec3 value;
      if (ConvertToVec3(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec4")
    {
      nsVec4 value;
      if (ConvertToVec4(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec2i")
    {
      nsVec2I32 value;
      if (ConvertToVec2I(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec3i")
    {
      nsVec3I32 value;
      if (ConvertToVec3I(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec4i")
    {
      nsVec4I32 value;
      if (ConvertToVec4I(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec2u")
    {
      nsVec2U32 value;
      if (ConvertToVec2U(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec3u")
    {
      nsVec3U32 value;
      if (ConvertToVec3U(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Vec4u")
    {
      nsVec4U32 value;
      if (ConvertToVec4U(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Mat3")
    {
      nsMat3 value;
      if (ConvertToMat3(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Mat4")
    {
      nsMat4 value;
      if (ConvertToMat4(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Transform")
    {
      nsTransform value;
      if (ConvertToTransform(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Quat")
    {
      nsQuat value;
      if (ConvertToQuat(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Uuid")
    {
      nsUuid value;
      if (ConvertToUuid(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Angle")
    {
      nsAngle value;
      if (ConvertToAngle(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "HashedString")
    {
      nsHashedString value;
      if (ConvertToHashedString(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "TempHashedString")
    {
      nsTempHashedString value;
      if (ConvertToTempHashedString(pElement, value).Failed())
        return NS_FAILURE;

      out_result = value;
      return NS_SUCCESS;
    }

    if (pElement->GetCustomType() == "Invalid")
    {
      out_result = nsVariant();
      return NS_SUCCESS;
    }

    if (const nsRTTI* pRTTI = nsRTTI::FindTypeByName(pElement->GetCustomType()))
    {
      if (nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pRTTI))
      {
        if (pElement == nullptr)
          return NS_FAILURE;

        void* pObject = pRTTI->GetAllocator()->Allocate<void>();

        for (const nsOpenDdlReaderElement* pChildElement = pElement->GetFirstChild(); pChildElement != nullptr; pChildElement = pChildElement->GetSibling())
        {
          if (!pChildElement->HasName())
            continue;

          if (const nsAbstractProperty* pProp = pRTTI->FindPropertyByName(pChildElement->GetName()))
          {
            // Custom types should be POD and only consist of member properties.
            if (pProp->GetCategory() == nsPropertyCategory::Member)
            {
              nsVariant subValue;
              if (ConvertToVariant(pChildElement, subValue).Succeeded())
              {
                nsReflectionUtils::SetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), pObject, subValue);
              }
            }
          }
        }
        out_result.MoveTypedObject(pObject, pRTTI);
        return NS_SUCCESS;
      }
      else
      {
        nsLog::Error("The type '{0}' was declared but not defined, add NS_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", pElement->GetCustomType());
      }
    }
    else
    {
      nsLog::Error("The type '{0}' is unknown.", pElement->GetCustomType());
    }
  }
  else
  {
    // always expect exactly one value
    if (pElement->GetNumPrimitives() != 1)
      return NS_FAILURE;

    switch (pElement->GetPrimitivesType())
    {
      case nsOpenDdlPrimitiveType::Bool:
        out_result = pElement->GetPrimitivesBool()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::Int8:
        out_result = pElement->GetPrimitivesInt8()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::Int16:
        out_result = pElement->GetPrimitivesInt16()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::Int32:
        out_result = pElement->GetPrimitivesInt32()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::Int64:
        out_result = pElement->GetPrimitivesInt64()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::UInt8:
        out_result = pElement->GetPrimitivesUInt8()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::UInt16:
        out_result = pElement->GetPrimitivesUInt16()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::UInt32:
        out_result = pElement->GetPrimitivesUInt32()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::UInt64:
        out_result = pElement->GetPrimitivesUInt64()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::Float:
        out_result = pElement->GetPrimitivesFloat()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::Double:
        out_result = pElement->GetPrimitivesDouble()[0];
        return NS_SUCCESS;

      case nsOpenDdlPrimitiveType::String:
        out_result = nsString(pElement->GetPrimitivesString()[0]); // make sure this isn't stored as a string view by copying to to an nsString first
        return NS_SUCCESS;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }

  return NS_FAILURE;
}

void nsOpenDdlUtils::StoreColor(nsOpenDdlWriter& ref_writer, const nsColor& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Color", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreColorGamma(
  nsOpenDdlWriter& ref_writer, const nsColorGammaUB& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("ColorGamma", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt8);
    ref_writer.WriteUInt8(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreTime(nsOpenDdlWriter& ref_writer, const nsTime& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Time", sName, bGlobalName, true);
  {
    const double d = value.GetSeconds();

    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Double);
    ref_writer.WriteDouble(&d, 1);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec2(nsOpenDdlWriter& ref_writer, const nsVec2& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec3(nsOpenDdlWriter& ref_writer, const nsVec3& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec4(nsOpenDdlWriter& ref_writer, const nsVec4& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec2I(nsOpenDdlWriter& ref_writer, const nsVec2I32& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2i", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec3I(nsOpenDdlWriter& ref_writer, const nsVec3I32& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3i", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec4I(nsOpenDdlWriter& ref_writer, const nsVec4I32& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4i", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec2U(nsOpenDdlWriter& ref_writer, const nsVec2U32& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2u", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec3U(nsOpenDdlWriter& ref_writer, const nsVec3U32& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3u", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVec4U(nsOpenDdlWriter& ref_writer, const nsVec4U32& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4u", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}


void nsOpenDdlUtils::StoreMat3(nsOpenDdlWriter& ref_writer, const nsMat3& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Mat3", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);

    float f[9];
    value.GetAsArray(f, nsMatrixLayout::ColumnMajor);
    ref_writer.WriteFloat(f, 9);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreMat4(nsOpenDdlWriter& ref_writer, const nsMat4& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Mat4", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);

    float f[16];
    value.GetAsArray(f, nsMatrixLayout::ColumnMajor);
    ref_writer.WriteFloat(f, 16);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreTransform(nsOpenDdlWriter& ref_writer, const nsTransform& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Transform", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);

    float f[10];

    f[0] = value.m_vPosition.x;
    f[1] = value.m_vPosition.y;
    f[2] = value.m_vPosition.z;

    f[3] = value.m_qRotation.x;
    f[4] = value.m_qRotation.y;
    f[5] = value.m_qRotation.z;
    f[6] = value.m_qRotation.w;

    f[7] = value.m_vScale.x;
    f[8] = value.m_vScale.y;
    f[9] = value.m_vScale.z;

    ref_writer.WriteFloat(f, 10);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreQuat(nsOpenDdlWriter& ref_writer, const nsQuat& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Quat", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(&value.x, 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreUuid(nsOpenDdlWriter& ref_writer, const nsUuid& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Uuid", sName, bGlobalName, true);
  {
    nsUInt64 ui[2];
    value.GetValues(ui[0], ui[1]);

    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt64);
    ref_writer.WriteUInt64(ui, 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreAngle(nsOpenDdlWriter& ref_writer, const nsAngle& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Angle", sName, bGlobalName, true);
  {
    // have to use radians to prevent precision loss
    const float f = value.GetRadian();

    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(&f, 1);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreHashedString(nsOpenDdlWriter& ref_writer, const nsHashedString& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("HashedString", sName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::String);
    ref_writer.WriteString(value.GetView());
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreTempHashedString(nsOpenDdlWriter& ref_writer, const nsTempHashedString& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("TempHashedString", sName, bGlobalName, true);
  {
    const nsUInt64 uiHash = value.GetHash();

    ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt64);
    ref_writer.WriteUInt64(&uiHash);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void nsOpenDdlUtils::StoreVariant(nsOpenDdlWriter& ref_writer, const nsVariant& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  switch (value.GetType())
  {
    case nsVariant::Type::Invalid:
      StoreInvalid(ref_writer, sName, bGlobalName);
      return;

    case nsVariant::Type::Bool:
      StoreBool(ref_writer, value.Get<bool>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Int8:
      StoreInt8(ref_writer, value.Get<nsInt8>(), sName, bGlobalName);
      return;

    case nsVariant::Type::UInt8:
      StoreUInt8(ref_writer, value.Get<nsUInt8>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Int16:
      StoreInt16(ref_writer, value.Get<nsInt16>(), sName, bGlobalName);
      return;

    case nsVariant::Type::UInt16:
      StoreUInt16(ref_writer, value.Get<nsUInt16>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Int32:
      StoreInt32(ref_writer, value.Get<nsInt32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::UInt32:
      StoreUInt32(ref_writer, value.Get<nsUInt32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Int64:
      StoreInt64(ref_writer, value.Get<nsInt64>(), sName, bGlobalName);
      return;

    case nsVariant::Type::UInt64:
      StoreUInt64(ref_writer, value.Get<nsUInt64>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Float:
      StoreFloat(ref_writer, value.Get<float>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Double:
      StoreDouble(ref_writer, value.Get<double>(), sName, bGlobalName);
      return;

    case nsVariant::Type::String:
      nsOpenDdlUtils::StoreString(ref_writer, value.Get<nsString>(), sName, bGlobalName);
      return;

    case nsVariant::Type::StringView:
      nsOpenDdlUtils::StoreString(ref_writer, value.Get<nsStringView>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Color:
      StoreColor(ref_writer, value.Get<nsColor>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector2:
      StoreVec2(ref_writer, value.Get<nsVec2>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector3:
      StoreVec3(ref_writer, value.Get<nsVec3>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector4:
      StoreVec4(ref_writer, value.Get<nsVec4>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector2I:
      StoreVec2I(ref_writer, value.Get<nsVec2I32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector3I:
      StoreVec3I(ref_writer, value.Get<nsVec3I32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector4I:
      StoreVec4I(ref_writer, value.Get<nsVec4I32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector2U:
      StoreVec2U(ref_writer, value.Get<nsVec2U32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector3U:
      StoreVec3U(ref_writer, value.Get<nsVec3U32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Vector4U:
      StoreVec4U(ref_writer, value.Get<nsVec4U32>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Quaternion:
      StoreQuat(ref_writer, value.Get<nsQuat>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Matrix3:
      StoreMat3(ref_writer, value.Get<nsMat3>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Matrix4:
      StoreMat4(ref_writer, value.Get<nsMat4>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Transform:
      StoreTransform(ref_writer, value.Get<nsTransform>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Time:
      StoreTime(ref_writer, value.Get<nsTime>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Uuid:
      StoreUuid(ref_writer, value.Get<nsUuid>(), sName, bGlobalName);
      return;

    case nsVariant::Type::Angle:
      StoreAngle(ref_writer, value.Get<nsAngle>(), sName, bGlobalName);
      return;

    case nsVariant::Type::ColorGamma:
      StoreColorGamma(ref_writer, value.Get<nsColorGammaUB>(), sName, bGlobalName);
      return;

    case nsVariant::Type::HashedString:
      StoreHashedString(ref_writer, value.Get<nsHashedString>(), sName, bGlobalName);
      return;

    case nsVariant::Type::TempHashedString:
      StoreTempHashedString(ref_writer, value.Get<nsTempHashedString>(), sName, bGlobalName);
      return;

    case nsVariant::Type::VariantArray:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarArray", sName, bGlobalName);

      const nsVariantArray& arr = value.Get<nsVariantArray>();
      for (nsUInt32 i = 0; i < arr.GetCount(); ++i)
      {
        nsOpenDdlUtils::StoreVariant(ref_writer, arr[i]);
      }

      ref_writer.EndObject();
    }
      return;

    case nsVariant::Type::VariantDictionary:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarDict", sName, bGlobalName);

      const nsVariantDictionary& dict = value.Get<nsVariantDictionary>();
      for (auto it = dict.GetIterator(); it.IsValid(); ++it)
      {
        nsOpenDdlUtils::StoreVariant(ref_writer, it.Value(), it.Key(), false);
      }

      ref_writer.EndObject();
    }
      return;

    case nsVariant::Type::DataBuffer:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarDataBuffer", sName, bGlobalName);
      ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::String);

      const nsDataBuffer& db = value.Get<nsDataBuffer>();
      ref_writer.WriteBinaryAsString(db.GetData(), db.GetCount());

      ref_writer.EndPrimitiveList();
      ref_writer.EndObject();
    }
      return;

    case nsVariant::Type::TypedObject:
    {
      nsTypedObject obj = value.Get<nsTypedObject>();
      if (nsVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(obj.m_pType))
      {
        ref_writer.BeginObject(obj.m_pType->GetTypeName(), sName, bGlobalName);
        {
          nsHybridArray<const nsAbstractProperty*, 32> properties;
          obj.m_pType->GetAllProperties(properties);
          for (const nsAbstractProperty* pProp : properties)
          {
            // Custom types should be POD and only consist of member properties.
            switch (pProp->GetCategory())
            {
              case nsPropertyCategory::Member:
              {
                nsVariant subValue = nsReflectionUtils::GetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), obj.m_pObject);
                StoreVariant(ref_writer, subValue, pProp->GetPropertyName(), false);
              }
              break;
              case nsPropertyCategory::Array:
              case nsPropertyCategory::Set:
              case nsPropertyCategory::Map:
                NS_REPORT_FAILURE("Only member properties are supported in custom variant types!");
                break;
              case nsPropertyCategory::Constant:
              case nsPropertyCategory::Function:
                break;
            }
          }
        }
        ref_writer.EndObject();
      }
      else
      {
        nsLog::Error("The type '{0}' was declared but not defined, add NS_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", obj.m_pType->GetTypeName());
      }
    }
      return;
    default:
      NS_REPORT_FAILURE("Can't write this type of Variant");
  }
}

void nsOpenDdlUtils::StoreString(nsOpenDdlWriter& ref_writer, const nsStringView& value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::String, sName, bGlobalName);
  ref_writer.WriteString(value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreBool(nsOpenDdlWriter& ref_writer, bool value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Bool, sName, bGlobalName);
  ref_writer.WriteBool(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreFloat(nsOpenDdlWriter& ref_writer, float value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float, sName, bGlobalName);
  ref_writer.WriteFloat(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreDouble(nsOpenDdlWriter& ref_writer, double value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Double, sName, bGlobalName);
  ref_writer.WriteDouble(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreInt8(nsOpenDdlWriter& ref_writer, nsInt8 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int8, sName, bGlobalName);
  ref_writer.WriteInt8(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreInt16(nsOpenDdlWriter& ref_writer, nsInt16 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int16, sName, bGlobalName);
  ref_writer.WriteInt16(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreInt32(nsOpenDdlWriter& ref_writer, nsInt32 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int32, sName, bGlobalName);
  ref_writer.WriteInt32(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreInt64(nsOpenDdlWriter& ref_writer, nsInt64 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int64, sName, bGlobalName);
  ref_writer.WriteInt64(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreUInt8(nsOpenDdlWriter& ref_writer, nsUInt8 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt8, sName, bGlobalName);
  ref_writer.WriteUInt8(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreUInt16(nsOpenDdlWriter& ref_writer, nsUInt16 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt16, sName, bGlobalName);
  ref_writer.WriteUInt16(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreUInt32(nsOpenDdlWriter& ref_writer, nsUInt32 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt32, sName, bGlobalName);
  ref_writer.WriteUInt32(&value);
  ref_writer.EndPrimitiveList();
}

void nsOpenDdlUtils::StoreUInt64(nsOpenDdlWriter& ref_writer, nsUInt64 value, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt64, sName, bGlobalName);
  ref_writer.WriteUInt64(&value);
  ref_writer.EndPrimitiveList();
}

NS_FOUNDATION_DLL void nsOpenDdlUtils::StoreInvalid(nsOpenDdlWriter& ref_writer, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Invalid", sName, bGlobalName, true);
  ref_writer.EndObject();
}
