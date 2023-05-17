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

wdResult wdOpenDdlUtils::ConvertToColor(const wdOpenDdlReaderElement* pElement, wdColor& out_result)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = pValues[3];

      return WD_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = 1.0f;

      return WD_SUCCESS;
    }
  }
  else if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::UInt8)
  {
    const wdUInt8* pValues = pElement->GetPrimitivesUInt8();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result = wdColorGammaUB(pValues[0], pValues[1], pValues[2], pValues[3]);

      return WD_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result = wdColorGammaUB(pValues[0], pValues[1], pValues[2]);

      return WD_SUCCESS;
    }
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToColorGamma(const wdOpenDdlReaderElement* pElement, wdColorGammaUB& out_result)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result = wdColor(pValues[0], pValues[1], pValues[2], pValues[3]);

      return WD_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result = wdColor(pValues[0], pValues[1], pValues[2]);

      return WD_SUCCESS;
    }
  }
  else if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::UInt8)
  {
    const wdUInt8* pValues = pElement->GetPrimitivesUInt8();

    if (pElement->GetNumPrimitives() == 4)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = pValues[3];

      return WD_SUCCESS;
    }

    if (pElement->GetNumPrimitives() == 3)
    {
      out_result.r = pValues[0];
      out_result.g = pValues[1];
      out_result.b = pValues[2];
      out_result.a = 255;

      return WD_SUCCESS;
    }
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToTime(const wdOpenDdlReaderElement* pElement, wdTime& out_result)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_result = wdTime::Seconds(pValues[0]);

    return WD_SUCCESS;
  }

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Double)
  {
    const double* pValues = pElement->GetPrimitivesDouble();

    out_result = wdTime::Seconds(pValues[0]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec2(const wdOpenDdlReaderElement* pElement, wdVec2& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec3(const wdOpenDdlReaderElement* pElement, wdVec3& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec4(const wdOpenDdlReaderElement* pElement, wdVec4& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec2I(const wdOpenDdlReaderElement* pElement, wdVec2I32& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Int32)
  {
    const wdInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec3I(const wdOpenDdlReaderElement* pElement, wdVec3I32& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Int32)
  {
    const wdInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec4I(const wdOpenDdlReaderElement* pElement, wdVec4I32& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Int32)
  {
    const wdInt32* pValues = pElement->GetPrimitivesInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}


wdResult wdOpenDdlUtils::ConvertToVec2U(const wdOpenDdlReaderElement* pElement, wdVec2U32& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::UInt32)
  {
    const wdUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec3U(const wdOpenDdlReaderElement* pElement, wdVec3U32& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 3)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::UInt32)
  {
    const wdUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVec4U(const wdOpenDdlReaderElement* pElement, wdVec4U32& out_vResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::UInt32)
  {
    const wdUInt32* pValues = pElement->GetPrimitivesUInt32();

    out_vResult.Set(pValues[0], pValues[1], pValues[2], pValues[3]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}



wdResult wdOpenDdlUtils::ConvertToMat3(const wdOpenDdlReaderElement* pElement, wdMat3& out_mResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 9)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_mResult.SetFromArray(pValues, wdMatrixLayout::ColumnMajor);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToMat4(const wdOpenDdlReaderElement* pElement, wdMat4& out_mResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 16)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_mResult.SetFromArray(pValues, wdMatrixLayout::ColumnMajor);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}


wdResult wdOpenDdlUtils::ConvertToTransform(const wdOpenDdlReaderElement* pElement, wdTransform& out_result)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 10)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_result.m_vPosition.x = pValues[0];
    out_result.m_vPosition.y = pValues[1];
    out_result.m_vPosition.z = pValues[2];
    out_result.m_qRotation.v.x = pValues[3];
    out_result.m_qRotation.v.y = pValues[4];
    out_result.m_qRotation.v.z = pValues[5];
    out_result.m_qRotation.w = pValues[6];
    out_result.m_vScale.x = pValues[7];
    out_result.m_vScale.y = pValues[8];
    out_result.m_vScale.z = pValues[9];

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToQuat(const wdOpenDdlReaderElement* pElement, wdQuat& out_qResult)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 4)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    out_qResult.SetElements(pValues[0], pValues[1], pValues[2], pValues[3]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToUuid(const wdOpenDdlReaderElement* pElement, wdUuid& out_result)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 2)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::UInt64)
  {
    const wdUInt64* pValues = pElement->GetPrimitivesUInt64();

    out_result = wdUuid(pValues[0], pValues[1]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToAngle(const wdOpenDdlReaderElement* pElement, wdAngle& out_result)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // go into the element, if we are at the group level
  if (pElement->IsCustomType())
  {
    if (pElement->GetNumChildObjects() != 1)
      return WD_FAILURE;

    pElement = pElement->GetFirstChild();
  }

  if (pElement->GetNumPrimitives() != 1)
    return WD_FAILURE;

  if (pElement->GetPrimitivesType() == wdOpenDdlPrimitiveType::Float)
  {
    const float* pValues = pElement->GetPrimitivesFloat();

    // have to use radians to prevent precision loss
    out_result = wdAngle::Radian(pValues[0]);

    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdOpenDdlUtils::ConvertToVariant(const wdOpenDdlReaderElement* pElement, wdVariant& out_result)
{
  if (pElement == nullptr)
    return WD_FAILURE;

  // expect a custom type
  if (pElement->IsCustomType())
  {
    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "VarArray"))
    {
      wdVariantArray value;
      wdVariant varChild;

      /// \test This is just quickly hacked
      /// \todo Store array size for reserving var array length

      for (const wdOpenDdlReaderElement* pChild = pElement->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
      {
        if (ConvertToVariant(pChild, varChild).Failed())
          return WD_FAILURE;

        value.PushBack(varChild);
      }

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "VarDict"))
    {
      wdVariantDictionary value;
      wdVariant varChild;

      /// \test This is just quickly hacked
      /// \todo Store array size for reserving var array length

      for (const wdOpenDdlReaderElement* pChild = pElement->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
      {
        // no name -> invalid dictionary entry
        if (!pChild->HasName())
          continue;

        if (ConvertToVariant(pChild, varChild).Failed())
          return WD_FAILURE;

        value[pChild->GetName()] = varChild;
      }

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "VarDataBuffer"))
    {
      /// \test This is just quickly hacked

      wdDataBuffer value;

      const wdOpenDdlReaderElement* pString = pElement->GetFirstChild();

      if (!pString->HasPrimitives(wdOpenDdlPrimitiveType::String))
        return WD_FAILURE;

      const wdStringView* pValues = pString->GetPrimitivesString();

      value.SetCountUninitialized(pValues[0].GetElementCount() / 2);
      wdConversionUtils::ConvertHexToBinary(pValues[0].GetStartPointer(), value.GetData(), value.GetCount());

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Color"))
    {
      wdColor value;
      if (ConvertToColor(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "ColorGamma"))
    {
      wdColorGammaUB value;
      if (ConvertToColorGamma(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Time"))
    {
      wdTime value;
      if (ConvertToTime(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec2"))
    {
      wdVec2 value;
      if (ConvertToVec2(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec3"))
    {
      wdVec3 value;
      if (ConvertToVec3(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec4"))
    {
      wdVec4 value;
      if (ConvertToVec4(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec2i"))
    {
      wdVec2I32 value;
      if (ConvertToVec2I(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec3i"))
    {
      wdVec3I32 value;
      if (ConvertToVec3I(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec4i"))
    {
      wdVec4I32 value;
      if (ConvertToVec4I(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec2u"))
    {
      wdVec2U32 value;
      if (ConvertToVec2U(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec3u"))
    {
      wdVec3U32 value;
      if (ConvertToVec3U(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Vec4u"))
    {
      wdVec4U32 value;
      if (ConvertToVec4U(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Mat3"))
    {
      wdMat3 value;
      if (ConvertToMat3(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Mat4"))
    {
      wdMat4 value;
      if (ConvertToMat4(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Transform"))
    {
      wdTransform value;
      if (ConvertToTransform(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Quat"))
    {
      wdQuat value;
      if (ConvertToQuat(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Uuid"))
    {
      wdUuid value;
      if (ConvertToUuid(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (wdStringUtils::IsEqual(pElement->GetCustomType(), "Angle"))
    {
      wdAngle value;
      if (ConvertToAngle(pElement, value).Failed())
        return WD_FAILURE;

      out_result = value;
      return WD_SUCCESS;
    }

    if (const wdRTTI* pRTTI = wdRTTI::FindTypeByName(pElement->GetCustomType()))
    {
      if (wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(pRTTI))
      {
        if (pElement == nullptr)
          return WD_FAILURE;

        void* pObject = pRTTI->GetAllocator()->Allocate<void>();

        for (const wdOpenDdlReaderElement* pChildElement = pElement->GetFirstChild(); pChildElement != nullptr; pChildElement = pChildElement->GetSibling())
        {
          if (!pChildElement->HasName())
            continue;

          if (wdAbstractProperty* pProp = pRTTI->FindPropertyByName(pChildElement->GetName()))
          {
            // Custom types should be POD and only consist of member properties.
            if (pProp->GetCategory() == wdPropertyCategory::Member)
            {
              wdVariant subValue;
              if (ConvertToVariant(pChildElement, subValue).Succeeded())
              {
                wdReflectionUtils::SetMemberPropertyValue(static_cast<wdAbstractMemberProperty*>(pProp), pObject, subValue);
              }
            }
          }
        }
        out_result.MoveTypedObject(pObject, pRTTI);
        return WD_SUCCESS;
      }
      else
      {
        wdLog::Error("The type '{0}' was declared but not defined, add WD_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", pElement->GetCustomType());
      }
    }
    else
    {
      wdLog::Error("The type '{0}' is unknown.", pElement->GetCustomType());
    }
  }
  else
  {
    // always expect exactly one value
    if (pElement->GetNumPrimitives() != 1)
      return WD_FAILURE;

    switch (pElement->GetPrimitivesType())
    {
      case wdOpenDdlPrimitiveType::Bool:
        out_result = pElement->GetPrimitivesBool()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::Int8:
        out_result = pElement->GetPrimitivesInt8()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::Int16:
        out_result = pElement->GetPrimitivesInt16()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::Int32:
        out_result = pElement->GetPrimitivesInt32()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::Int64:
        out_result = pElement->GetPrimitivesInt64()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::UInt8:
        out_result = pElement->GetPrimitivesUInt8()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::UInt16:
        out_result = pElement->GetPrimitivesUInt16()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::UInt32:
        out_result = pElement->GetPrimitivesUInt32()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::UInt64:
        out_result = pElement->GetPrimitivesUInt64()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::Float:
        out_result = pElement->GetPrimitivesFloat()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::Double:
        out_result = pElement->GetPrimitivesDouble()[0];
        return WD_SUCCESS;

      case wdOpenDdlPrimitiveType::String:
        out_result = wdString(pElement->GetPrimitivesString()[0]); // make sure this isn't stored as a string view by copying to to an wdString first
        return WD_SUCCESS;

      default:
        WD_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }

  return WD_FAILURE;
}

void wdOpenDdlUtils::StoreColor(wdOpenDdlWriter& ref_writer, const wdColor& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Color", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreColorGamma(
  wdOpenDdlWriter& ref_writer, const wdColorGammaUB& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("ColorGamma", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt8);
    ref_writer.WriteUInt8(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreTime(wdOpenDdlWriter& ref_writer, const wdTime& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Time", szName, bGlobalName, true);
  {
    const double d = value.GetSeconds();

    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Double);
    ref_writer.WriteDouble(&d, 1);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec2(wdOpenDdlWriter& ref_writer, const wdVec2& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec3(wdOpenDdlWriter& ref_writer, const wdVec3& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec4(wdOpenDdlWriter& ref_writer, const wdVec4& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec2I(wdOpenDdlWriter& ref_writer, const wdVec2I32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2i", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec3I(wdOpenDdlWriter& ref_writer, const wdVec3I32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3i", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec4I(wdOpenDdlWriter& ref_writer, const wdVec4I32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4i", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Int32);
    ref_writer.WriteInt32(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec2U(wdOpenDdlWriter& ref_writer, const wdVec2U32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec2u", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec3U(wdOpenDdlWriter& ref_writer, const wdVec3U32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec3u", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 3);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVec4U(wdOpenDdlWriter& ref_writer, const wdVec4U32& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Vec4u", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt32);
    ref_writer.WriteUInt32(value.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}


void wdOpenDdlUtils::StoreMat3(wdOpenDdlWriter& ref_writer, const wdMat3& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Mat3", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);

    float f[9];
    value.GetAsArray(f, wdMatrixLayout::ColumnMajor);
    ref_writer.WriteFloat(f, 9);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreMat4(wdOpenDdlWriter& ref_writer, const wdMat4& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Mat4", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);

    float f[16];
    value.GetAsArray(f, wdMatrixLayout::ColumnMajor);
    ref_writer.WriteFloat(f, 16);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreTransform(wdOpenDdlWriter& ref_writer, const wdTransform& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Transform", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);

    float f[10];

    f[0] = value.m_vPosition.x;
    f[1] = value.m_vPosition.y;
    f[2] = value.m_vPosition.z;

    f[3] = value.m_qRotation.v.x;
    f[4] = value.m_qRotation.v.y;
    f[5] = value.m_qRotation.v.z;
    f[6] = value.m_qRotation.w;

    f[7] = value.m_vScale.x;
    f[8] = value.m_vScale.y;
    f[9] = value.m_vScale.z;

    ref_writer.WriteFloat(f, 10);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreQuat(wdOpenDdlWriter& ref_writer, const wdQuat& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Quat", szName, bGlobalName, true);
  {
    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(value.v.GetData(), 4);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreUuid(wdOpenDdlWriter& ref_writer, const wdUuid& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Uuid", szName, bGlobalName, true);
  {
    wdUInt64 ui[2];
    value.GetValues(ui[0], ui[1]);

    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt64);
    ref_writer.WriteUInt64(ui, 2);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreAngle(wdOpenDdlWriter& ref_writer, const wdAngle& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginObject("Angle", szName, bGlobalName, true);
  {
    // have to use radians to prevent precision loss
    const float f = value.GetRadian();

    ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float);
    ref_writer.WriteFloat(&f, 1);
    ref_writer.EndPrimitiveList();
  }
  ref_writer.EndObject();
}

void wdOpenDdlUtils::StoreVariant(wdOpenDdlWriter& ref_writer, const wdVariant& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  switch (value.GetType())
  {
    case wdVariant::Type::Invalid:
      return; // store anything ?

    case wdVariant::Type::Bool:
    {
      StoreBool(ref_writer, value.Get<bool>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::Int8:
    {
      StoreInt8(ref_writer, value.Get<wdInt8>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::UInt8:
    {
      StoreUInt8(ref_writer, value.Get<wdUInt8>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::Int16:
    {
      StoreInt16(ref_writer, value.Get<wdInt16>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::UInt16:
    {
      StoreUInt16(ref_writer, value.Get<wdUInt16>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::Int32:
    {
      StoreInt32(ref_writer, value.Get<wdInt32>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::UInt32:
    {
      StoreUInt32(ref_writer, value.Get<wdUInt32>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::Int64:
    {
      StoreInt64(ref_writer, value.Get<wdInt64>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::UInt64:
    {
      StoreUInt64(ref_writer, value.Get<wdUInt64>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::Float:
    {
      StoreFloat(ref_writer, value.Get<float>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::Double:
    {
      StoreDouble(ref_writer, value.Get<double>(), szName, bGlobalName);
    }
      return;

    case wdVariant::Type::String:
    {
      const wdString& var = value.Get<wdString>();
      wdOpenDdlUtils::StoreString(ref_writer, var, szName, bGlobalName);
    }
      return;

    case wdVariant::Type::StringView:
    {
      const wdStringView& var = value.Get<wdStringView>();
      wdOpenDdlUtils::StoreString(ref_writer, var, szName, bGlobalName);
    }
      return;

    case wdVariant::Type::Color:
      StoreColor(ref_writer, value.Get<wdColor>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector2:
      StoreVec2(ref_writer, value.Get<wdVec2>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector3:
      StoreVec3(ref_writer, value.Get<wdVec3>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector4:
      StoreVec4(ref_writer, value.Get<wdVec4>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector2I:
      StoreVec2I(ref_writer, value.Get<wdVec2I32>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector3I:
      StoreVec3I(ref_writer, value.Get<wdVec3I32>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector4I:
      StoreVec4I(ref_writer, value.Get<wdVec4I32>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector2U:
      StoreVec2U(ref_writer, value.Get<wdVec2U32>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector3U:
      StoreVec3U(ref_writer, value.Get<wdVec3U32>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Vector4U:
      StoreVec4U(ref_writer, value.Get<wdVec4U32>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Quaternion:
      StoreQuat(ref_writer, value.Get<wdQuat>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Matrix3:
      StoreMat3(ref_writer, value.Get<wdMat3>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Matrix4:
      StoreMat4(ref_writer, value.Get<wdMat4>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Transform:
      StoreTransform(ref_writer, value.Get<wdTransform>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Time:
      StoreTime(ref_writer, value.Get<wdTime>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Uuid:
      StoreUuid(ref_writer, value.Get<wdUuid>(), szName, bGlobalName);
      return;

    case wdVariant::Type::Angle:
      StoreAngle(ref_writer, value.Get<wdAngle>(), szName, bGlobalName);
      return;

    case wdVariant::Type::ColorGamma:
      StoreColorGamma(ref_writer, value.Get<wdColorGammaUB>(), szName, bGlobalName);
      return;

    case wdVariant::Type::VariantArray:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarArray", szName, bGlobalName);

      const wdVariantArray& arr = value.Get<wdVariantArray>();
      for (wdUInt32 i = 0; i < arr.GetCount(); ++i)
      {
        wdOpenDdlUtils::StoreVariant(ref_writer, arr[i]);
      }

      ref_writer.EndObject();
    }
      return;

    case wdVariant::Type::VariantDictionary:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarDict", szName, bGlobalName);

      const wdVariantDictionary& dict = value.Get<wdVariantDictionary>();
      for (auto it = dict.GetIterator(); it.IsValid(); ++it)
      {
        wdOpenDdlUtils::StoreVariant(ref_writer, it.Value(), it.Key(), false);
      }

      ref_writer.EndObject();
    }
      return;

    case wdVariant::Type::DataBuffer:
    {
      /// \test This is just quickly hacked

      ref_writer.BeginObject("VarDataBuffer", szName, bGlobalName);
      ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::String);

      const wdDataBuffer& db = value.Get<wdDataBuffer>();
      ref_writer.WriteBinaryAsString(db.GetData(), db.GetCount());

      ref_writer.EndPrimitiveList();
      ref_writer.EndObject();
    }
      return;

    case wdVariant::Type::TypedObject:
    {
      wdTypedObject obj = value.Get<wdTypedObject>();
      if (wdVariantTypeRegistry::GetSingleton()->FindVariantTypeInfo(obj.m_pType))
      {
        ref_writer.BeginObject(obj.m_pType->GetTypeName(), szName, bGlobalName);
        {
          wdHybridArray<wdAbstractProperty*, 32> properties;
          obj.m_pType->GetAllProperties(properties);
          for (const wdAbstractProperty* pProp : properties)
          {
            // Custom types should be POD and only consist of member properties.
            switch (pProp->GetCategory())
            {
              case wdPropertyCategory::Member:
              {
                wdVariant subValue = wdReflectionUtils::GetMemberPropertyValue(static_cast<const wdAbstractMemberProperty*>(pProp), obj.m_pObject);
                StoreVariant(ref_writer, subValue, pProp->GetPropertyName(), false);
              }
              break;
              case wdPropertyCategory::Array:
              case wdPropertyCategory::Set:
              case wdPropertyCategory::Map:
                WD_REPORT_FAILURE("Only member properties are supported in custom variant types!");
                break;
              case wdPropertyCategory::Constant:
              case wdPropertyCategory::Function:
                break;
            }
          }
        }
        ref_writer.EndObject();
      }
      else
      {
        wdLog::Error("The type '{0}' was declared but not defined, add WD_DEFINE_CUSTOM_VARIANT_TYPE({0}); to a cpp to enable serialization of this variant type.", obj.m_pType->GetTypeName());
      }
    }
      return;
    default:
      WD_REPORT_FAILURE("Can't write this type of Variant");
  }
}

void wdOpenDdlUtils::StoreString(wdOpenDdlWriter& ref_writer, const wdStringView& value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::String, szName, bGlobalName);
  ref_writer.WriteString(value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreBool(wdOpenDdlWriter& ref_writer, bool value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Bool, szName, bGlobalName);
  ref_writer.WriteBool(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreFloat(wdOpenDdlWriter& ref_writer, float value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Float, szName, bGlobalName);
  ref_writer.WriteFloat(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreDouble(wdOpenDdlWriter& ref_writer, double value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Double, szName, bGlobalName);
  ref_writer.WriteDouble(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreInt8(wdOpenDdlWriter& ref_writer, wdInt8 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Int8, szName, bGlobalName);
  ref_writer.WriteInt8(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreInt16(wdOpenDdlWriter& ref_writer, wdInt16 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Int16, szName, bGlobalName);
  ref_writer.WriteInt16(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreInt32(wdOpenDdlWriter& ref_writer, wdInt32 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Int32, szName, bGlobalName);
  ref_writer.WriteInt32(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreInt64(wdOpenDdlWriter& ref_writer, wdInt64 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::Int64, szName, bGlobalName);
  ref_writer.WriteInt64(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreUInt8(wdOpenDdlWriter& ref_writer, wdUInt8 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt8, szName, bGlobalName);
  ref_writer.WriteUInt8(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreUInt16(wdOpenDdlWriter& ref_writer, wdUInt16 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt16, szName, bGlobalName);
  ref_writer.WriteUInt16(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreUInt32(wdOpenDdlWriter& ref_writer, wdUInt32 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt32, szName, bGlobalName);
  ref_writer.WriteUInt32(&value);
  ref_writer.EndPrimitiveList();
}

void wdOpenDdlUtils::StoreUInt64(wdOpenDdlWriter& ref_writer, wdUInt64 value, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  ref_writer.BeginPrimitiveList(wdOpenDdlPrimitiveType::UInt64, szName, bGlobalName);
  ref_writer.WriteUInt64(&value);
  ref_writer.EndPrimitiveList();
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_OpenDdlUtils);
