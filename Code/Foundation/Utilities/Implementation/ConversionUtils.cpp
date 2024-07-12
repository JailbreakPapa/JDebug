#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Types/Variant.h>
#include <Foundation/Utilities/ConversionUtils.h>

namespace nsConversionUtils
{

  static bool IsWhitespace(nsUInt32 c)
  {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' || c == '\a');
  }

  static void SkipWhitespace(nsStringView& ref_sText)
  {
    // we are only looking at ASCII characters here, so no need to decode Utf8 sequences

    while (!ref_sText.IsEmpty() && IsWhitespace(*ref_sText.GetStartPointer()))
    {
      ref_sText.ChopAwayFirstCharacterAscii();
    }
  }

  static nsResult FindFirstDigit(nsStringView& inout_sText, bool& out_bSignIsPositive)
  {
    out_bSignIsPositive = true;

    while (!inout_sText.IsEmpty())
    {
      // we are only looking at ASCII characters here, so no need to decode Utf8 sequences
      const char c = *inout_sText.GetStartPointer();

      // found a digit
      if (c >= '0' && c <= '9')
        break;

      // skip all whitespace
      if (IsWhitespace(c))
      {
        inout_sText.ChopAwayFirstCharacterAscii();
        continue;
      }

      // NO change sign, just ignore + signs
      if (c == '+')
      {
        inout_sText.ChopAwayFirstCharacterAscii();
        continue;
      }

      // change sign
      if (c == '-')
      {
        out_bSignIsPositive = !out_bSignIsPositive;
        inout_sText.ChopAwayFirstCharacterAscii();
        continue;
      }

      return NS_FAILURE;
    }

    // not a single digit found
    if (inout_sText.IsEmpty())
      return NS_FAILURE;

    // remove all leading zeros
    while (inout_sText.StartsWith("00"))
    {
      inout_sText.ChopAwayFirstCharacterAscii();
    }

    // if it is a leading zero before a non-zero digit, remove it (otherwise keep the zero)
    if (inout_sText.GetElementCount() >= 2 && inout_sText.StartsWith("0"))
    {
      char c = *(inout_sText.GetStartPointer() + 1);

      if (c >= '1' && c <= '9')
      {
        inout_sText.ChopAwayFirstCharacterAscii();
      }
    }

    return NS_SUCCESS;
  }

  nsResult StringToInt(nsStringView sText, nsInt32& out_iRes, const char** out_pLastParsePosition)
  {
    nsInt64 tmp = out_iRes;
    if (StringToInt64(sText, tmp, out_pLastParsePosition) == NS_SUCCESS && tmp <= (nsInt32)0x7FFFFFFF && tmp >= (nsInt32)0x80000000)
    {
      out_iRes = (nsInt32)tmp;
      return NS_SUCCESS;
    }

    return NS_FAILURE;
  }

  nsResult StringToUInt(nsStringView sText, nsUInt32& out_uiRes, const char** out_pLastParsePosition)
  {
    nsInt64 tmp = out_uiRes;
    if (StringToInt64(sText, tmp, out_pLastParsePosition) == NS_SUCCESS && tmp <= (nsUInt32)0xFFFFFFFF && tmp >= 0)
    {
      out_uiRes = (nsUInt32)tmp;
      return NS_SUCCESS;
    }

    return NS_FAILURE;
  }

  nsResult StringToInt64(nsStringView sText, nsInt64& out_iRes, const char** out_pLastParsePosition)
  {
    if (sText.IsEmpty())
      return NS_FAILURE;

    bool bSignIsPos = true;

    if (FindFirstDigit(sText, bSignIsPos) == NS_FAILURE)
      return NS_FAILURE;

    nsInt64 iCurRes = 0;
    nsInt64 iSign = bSignIsPos ? 1 : -1;
    const nsInt64 iMax = 0x7FFFFFFFFFFFFFFF;
    const nsInt64 iMin = 0x8000000000000000;

    while (!sText.IsEmpty())
    {
      const char c = *sText.GetStartPointer();

      // c++ ' seperator can appear starting with the second digit
      if (iCurRes > 0 && c == '\'')
      {
        sText.ChopAwayFirstCharacterAscii();
        continue;
      }

      // end of digits reached -> return success (allows to write something like "239*4" -> parses first part as 239)
      if (c < '0' || c > '9')
        break;

      const nsInt64 iLastDigit = c - '0';

      if ((iCurRes > iMax / 10) || (iCurRes == iMax / 10 && iLastDigit > 7)) // going to overflow
        return NS_FAILURE;

      if ((iCurRes < iMin / 10) || (iCurRes == iMin / 10 && iLastDigit > 8)) // going to underflow
        return NS_FAILURE;

      iCurRes = iCurRes * 10 + iLastDigit * iSign;                           // shift all previously read digits to the left and add the last digit

      sText.ChopAwayFirstCharacterAscii();
    }

    out_iRes = iCurRes;

    if (out_pLastParsePosition != nullptr)
      *out_pLastParsePosition = sText.GetStartPointer();

    return NS_SUCCESS;
  }

  nsResult StringToFloat(nsStringView sText, double& out_fRes, const char** out_pLastParsePosition)
  {
    if (sText.IsEmpty())
      return NS_FAILURE;

    bool bSignIsPos = true;

    if (FindFirstDigit(sText, bSignIsPos) == NS_FAILURE)
    {
      // if it is a '.' continue (this is valid)
      if (!sText.StartsWith("."))
        return NS_FAILURE;
    }

    enum NumberPart
    {
      Integer,
      Fraction,
      Exponent,
    };

    NumberPart Part = Integer;

    nsUInt64 uiIntegerPart = 0;    // with 64 Bit to represent the values a 32 Bit float value can be stored, but a 64 Bit double cannot
    nsUInt64 uiFractionalPart = 0; // lets just assume we won't have such large or precise values stored in text form
    nsUInt64 uiFractionDivisor = 1;
    nsUInt64 uiExponentPart = 0;
    bool bExponentIsPositive = true;

    while (!sText.IsEmpty())
    {
      const char c = *sText.GetStartPointer();

      // allow underscores in floats for improved readability
      if (c == '_')
      {
        sText.ChopAwayFirstCharacterAscii();
        continue;
      }

      if (Part == Integer)
      {
        if (c == '.')
        {
          Part = Fraction;
          sText.ChopAwayFirstCharacterAscii();
          continue;
        }

        // c++ ' separator can appear starting with the second digit
        if (uiIntegerPart > 0 && c == '\'')
        {
          sText.ChopAwayFirstCharacterAscii();
          continue;
        }

        if (c >= '0' && c <= '9')
        {
          uiIntegerPart *= 10;
          uiIntegerPart += c - '0';
          sText.ChopAwayFirstCharacterAscii();
          continue;
        }

        if ((c == 'e') || (c == 'E'))
        {
          Part = Exponent;
          sText.ChopAwayFirstCharacterAscii();

          if (*sText.GetStartPointer() == '-')
          {
            bExponentIsPositive = false;
            sText.ChopAwayFirstCharacterAscii();
          }
          else if (*sText.GetStartPointer() == '+')
          {
            bExponentIsPositive = true;
            sText.ChopAwayFirstCharacterAscii();
          }

          continue;
        }
      }
      else if (Part == Fraction)
      {
        if (c >= '0' && c <= '9')
        {
          uiFractionalPart *= 10;
          uiFractionalPart += c - '0';
          uiFractionDivisor *= 10;
          sText.ChopAwayFirstCharacterAscii();
          continue;
        }

        if ((c == 'e') || (c == 'E'))
        {
          Part = Exponent;
          sText.ChopAwayFirstCharacterAscii();

          if (*sText.GetStartPointer() == '-')
          {
            bExponentIsPositive = false;
            sText.ChopAwayFirstCharacterAscii();
          }
          else if (*sText.GetStartPointer() == '+')
          {
            bExponentIsPositive = true;
            sText.ChopAwayFirstCharacterAscii();
          }

          continue;
        }
      }
      else if (Part == Exponent)
      {
        if (c >= '0' && c <= '9')
        {
          uiExponentPart *= 10;
          uiExponentPart += c - '0';
          sText.ChopAwayFirstCharacterAscii();
          continue;
        }
      }

      // found something that is not part of a float value -> stop parsing here
      break;
    }

    // we might lose some precision here, but at least up to this point no precision loss was accumulated yet
    out_fRes = (double)uiIntegerPart + (double)uiFractionalPart / (double)uiFractionDivisor;

    if (!bSignIsPos)
      out_fRes = -out_fRes;

    if (out_pLastParsePosition)
      *out_pLastParsePosition = sText.GetStartPointer();

    if (Part == Exponent)
    {
      if (bExponentIsPositive)
        out_fRes *= nsMath::Pow(10.0, (double)uiExponentPart);
      else
        out_fRes /= nsMath::Pow(10.0, (double)uiExponentPart);
    }

    return NS_SUCCESS;
  }

  nsResult StringToBool(nsStringView sText, bool& out_bRes, const char** out_pLastParsePosition)
  {
    SkipWhitespace(sText);

    if (sText.IsEmpty())
      return NS_FAILURE;

    // we are only looking at ASCII characters here, so no need to decode Utf8 sequences

    if (sText.StartsWith("1"))
    {
      out_bRes = true;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 1;

      return NS_SUCCESS;
    }

    if (sText.StartsWith("0"))
    {
      out_bRes = false;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 1;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("true"))
    {
      out_bRes = true;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 4;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("false"))
    {
      out_bRes = false;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 5;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("on"))
    {
      out_bRes = true;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 2;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("off"))
    {
      out_bRes = false;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 3;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("yes"))
    {
      out_bRes = true;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 3;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("no"))
    {
      out_bRes = false;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 2;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("enable"))
    {
      out_bRes = true;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 6;

      return NS_SUCCESS;
    }

    if (sText.StartsWith_NoCase("disable"))
    {
      out_bRes = false;

      if (out_pLastParsePosition)
        *out_pLastParsePosition = sText.GetStartPointer() + 7;

      return NS_SUCCESS;
    }

    return NS_FAILURE;
  }

  nsUInt32 ExtractFloatsFromString(nsStringView sText, nsUInt32 uiNumFloats, float* out_pFloats, const char** out_pLastParsePosition)
  {
    nsUInt32 uiFloatsFound = 0;

    // just try to extract n floats from the given text
    // if n floats were extracted, or the text end is reached, stop

    while (!sText.IsEmpty() && uiFloatsFound < uiNumFloats)
    {
      double res;
      const char* szPos;

      // if successful, store the float, otherwise advance the string by one, to skip invalid characters
      if (StringToFloat(sText, res, &szPos) == NS_SUCCESS)
      {
        out_pFloats[uiFloatsFound] = (float)res;
        ++uiFloatsFound;

        sText.SetStartPosition(szPos);
      }
      else
      {
        sText.ChopAwayFirstCharacterUtf8();
      }
    }

    if (out_pLastParsePosition != nullptr)
      *out_pLastParsePosition = sText.GetStartPointer();

    return uiFloatsFound;
  }

  nsInt8 HexCharacterToIntValue(nsUInt32 uiCharacter)
  {
    if (uiCharacter >= '0' && uiCharacter <= '9')
      return static_cast<nsInt8>(uiCharacter - '0');

    if (uiCharacter >= 'a' && uiCharacter <= 'f')
      return static_cast<nsInt8>(uiCharacter - 'a' + 10);

    if (uiCharacter >= 'A' && uiCharacter <= 'F')
      return static_cast<nsInt8>(uiCharacter - 'A' + 10);

    return -1;
  }

  nsResult ConvertHexStringToUInt32(nsStringView sHex, nsUInt32& out_uiResult)
  {
    nsUInt64 uiTemp = 0;
    const nsResult res = ConvertHexStringToUInt(sHex, uiTemp, 8, nullptr);

    out_uiResult = static_cast<nsUInt32>(uiTemp);
    return res;
  }

  nsResult ConvertHexStringToUInt64(nsStringView sHex, nsUInt64& out_uiResult)
  {
    return ConvertHexStringToUInt(sHex, out_uiResult, 16, nullptr);
  }

  nsResult ConvertHexStringToUInt(nsStringView sHex, nsUInt64& out_uiResult, nsUInt32 uiMaxHexCharacters, nsUInt32* pTotalCharactersParsed)
  {
    NS_ASSERT_DEBUG(uiMaxHexCharacters <= 16, "Only HEX strings of up to 16 character can be parsed into a 64-bit integer");
    const nsUInt32 origStringElementsCount = sHex.GetElementCount();

    out_uiResult = 0;

    // skip 0x
    if (sHex.StartsWith_NoCase("0x"))
      sHex.Shrink(2, 0);

    // convert two characters to one byte, at a time
    for (nsUInt32 i = 0; i < uiMaxHexCharacters; ++i)
    {
      if (sHex.IsEmpty())
      {
        // a shorter/empty string is valid and is just interpreted as a smaller value (e.g. a 32 Bit HEX value)
        break;
      }

      const nsInt8 iValue = nsConversionUtils::HexCharacterToIntValue(sHex.GetCharacter());

      if (iValue < 0)
      {
        // invalid HEX character
        out_uiResult = 0;
        if (pTotalCharactersParsed)
        {
          *pTotalCharactersParsed = 0;
        }
        return NS_FAILURE;
      }

      out_uiResult <<= 4; // 4 Bits, ie. half a byte
      out_uiResult += iValue;

      sHex.ChopAwayFirstCharacterAscii();
    }

    if (pTotalCharactersParsed)
    {
      NS_ASSERT_DEBUG(sHex.GetElementCount() <= origStringElementsCount, "");
      *pTotalCharactersParsed = origStringElementsCount - sHex.GetElementCount();
    }

    return NS_SUCCESS;
  }

  void ConvertHexToBinary(nsStringView sHex, nsUInt8* pBinary, nsUInt32 uiBinaryBuffer)
  {
    // skip 0x
    if (sHex.StartsWith_NoCase("0x"))
      sHex.Shrink(2, 0);

    // convert two characters to one byte, at a time
    // try not to run out of buffer space
    while (sHex.GetElementCount() >= 2 && uiBinaryBuffer >= 1)
    {
      const nsUInt32 c0 = *sHex.GetStartPointer();
      const nsUInt32 c1 = *(sHex.GetStartPointer() + 1);

      nsUInt8 uiValue1 = nsConversionUtils::HexCharacterToIntValue(c0);
      nsUInt8 uiValue2 = nsConversionUtils::HexCharacterToIntValue(c1);
      nsUInt8 uiValue = 16 * uiValue1 + uiValue2;
      *pBinary = uiValue;

      pBinary += 1;
      sHex.Shrink(2, 0);

      uiBinaryBuffer -= 1;
    }
  }

  const nsStringBuilder& ToString(nsInt8 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", (nsInt32)value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(nsUInt8 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", (nsUInt32)value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(nsInt16 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", (nsInt32)value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(nsUInt16 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", (nsUInt32)value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(nsInt32 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(nsUInt32 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(nsInt64 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(nsUInt64 value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(float value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(double value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsColor& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ r={0}, g={1}, b={2}, a={3} }", value.r, value.g, value.b, value.a);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsColorGammaUB& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ r={0}, g={1}, b={2}, a={3} }", value.r, value.g, value.b, value.a);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsVec2& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ x={0}, y={1} }", value.x, value.y);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsVec3& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ x={0}, y={1}, z={2} }", value.x, value.y, value.z);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsVec4& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ x={0}, y={1}, z={2}, w={3} }", value.x, value.y, value.z, value.w);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsVec2I32& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ x={0}, y={1} }", value.x, value.y);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsVec3I32& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ x={0}, y={1}, z={2} }", value.x, value.y, value.z);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsVec4I32& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ x={0}, y={1}, z={2}, w={3} }", value.x, value.y, value.z, value.w);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsQuat& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{ x={0}, y={1}, z={2}, w={3} }", value.x, value.y, value.z, value.w);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsMat3& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetPrintf("{ c1r1=%f, c2r1=%f, c3r1=%f, "
                          "c1r2=%f, c2r2=%f, c3r2=%f, "
                          "c1r3=%f, c2r3=%f, c3r3=%f }",
      value.Element(0, 0), value.Element(1, 0), value.Element(2, 0), value.Element(0, 1), value.Element(1, 1), value.Element(2, 1),
      value.Element(0, 2), value.Element(1, 2), value.Element(2, 2));
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsMat4& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetPrintf("{ c1r1=%f, c2r1=%f, c3r1=%f, c4r1=%f, "
                          "c1r2=%f, c2r2=%f, c3r2=%f, c4r2=%f, "
                          "c1r3=%f, c2r3=%f, c3r3=%f, c4r3=%f, "
                          "c1r4=%f, c2r4=%f, c3r4=%f, c4r4=%f }",
      value.Element(0, 0), value.Element(1, 0), value.Element(2, 0), value.Element(3, 0), value.Element(0, 1), value.Element(1, 1),
      value.Element(2, 1), value.Element(3, 1), value.Element(0, 2), value.Element(1, 2), value.Element(2, 2), value.Element(3, 2),
      value.Element(0, 3), value.Element(1, 3), value.Element(2, 3), value.Element(3, 3));
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsTransform& value, nsStringBuilder& out_sResult)
  {
    nsStringBuilder tmp1, tmp2, tmp3;
    out_sResult.SetFormat("{ position={0}, rotation={1}, scale={2} }", ToString(value.m_vPosition, tmp1), ToString(value.m_qRotation, tmp2),
      ToString(value.m_vScale, tmp3));
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsAngle& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsTime& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("{0}", value);
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsHashedString& value, nsStringBuilder& out_sResult)
  {
    out_sResult = value.GetView();
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsTempHashedString& value, nsStringBuilder& out_sResult)
  {
    out_sResult.SetFormat("0x{}", nsArgU(value.GetHash(), 16, true, 16));
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsDynamicArray<nsVariant>& value, nsStringBuilder& out_sResult)
  {
    out_sResult.Append("[");
    for (const nsVariant& var : value)
    {
      out_sResult.Append(var.ConvertTo<nsString>(), ", ");
    }
    if (!value.IsEmpty())
      out_sResult.Shrink(0, 2);
    out_sResult.Append("]");
    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsHashTable<nsString, nsVariant>& value, nsStringBuilder& out_sResult)
  {
    out_sResult.Append("{");
    for (auto it : value)
    {
      out_sResult.Append(it.Key(), "=", it.Value().ConvertTo<nsString>(), ", ");
    }
    if (!value.IsEmpty())
      out_sResult.Shrink(0, 2);
    out_sResult.Append("}");
    return out_sResult;
  }

  nsUuid ConvertStringToUuid(nsStringView sText);

  const nsStringBuilder& ToString(const nsUuid& value, nsStringBuilder& out_sResult)
  {
    // Windows GUID formatting.
    struct GUID
    {
      nsUInt32 Data1;
      nsUInt16 Data2;
      nsUInt16 Data3;
      nsUInt8 Data4[8];
    };

    const GUID* pGuid = reinterpret_cast<const GUID*>(&value);

    out_sResult.SetPrintf("{ %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x }", pGuid->Data1, pGuid->Data2, pGuid->Data3, pGuid->Data4[0],
      pGuid->Data4[1], pGuid->Data4[2], pGuid->Data4[3], pGuid->Data4[4], pGuid->Data4[5], pGuid->Data4[6], pGuid->Data4[7]);

    return out_sResult;
  }

  const nsStringBuilder& ToString(const nsStringView& value, nsStringBuilder& out_sResult)
  {
    out_sResult = value;
    return out_sResult;
  }

  bool IsStringUuid(nsStringView sText)
  {
    if (sText.GetElementCount() != 40)
      return false;

    if (!sText.StartsWith("{"))
      return false;

    const char* szText = sText.GetStartPointer();

    if ((szText[1] != ' ') || (szText[10] != '-') || (szText[15] != '-') || (szText[20] != '-') || (szText[25] != '-') || (szText[38] != ' ') || (szText[39] != '}'))
    {
      return false;
    }

    return true;
  }

  nsUuid ConvertStringToUuid(nsStringView sText)
  {
    NS_ASSERT_DEBUG(IsStringUuid(sText), "The given string is not in the correct Uuid format: '{0}'", sText);

    const char* szText = sText.GetStartPointer();

    while (*szText == '{' || nsStringUtils::IsWhiteSpace(*szText))
      ++szText;

    struct GUID
    {
      nsUInt32 Data1;
      nsUInt16 Data2;
      nsUInt16 Data3;
      nsUInt8 Data4[8];
    };

    GUID guid;
    guid.Data1 = 0;
    guid.Data2 = 0;
    guid.Data3 = 0;

    for (int i = 0; i < 8; ++i)
    {
      guid.Data4[i] = 0;
      guid.Data1 = (guid.Data1 << 4) | HexCharacterToIntValue(*szText);
      ++szText;
    }

    // -
    ++szText;
    for (int i = 0; i < 4; ++i)
    {
      guid.Data2 = (guid.Data2 << 4) | HexCharacterToIntValue(*szText);
      ++szText;
    }

    // -
    ++szText;
    for (int i = 0; i < 4; ++i)
    {
      guid.Data3 = (guid.Data3 << 4) | HexCharacterToIntValue(*szText);
      ++szText;
    }

    // -
    ++szText;

    for (int i = 0; i < 2; ++i)
    {
      guid.Data4[i] = (guid.Data4[i] << 4) | HexCharacterToIntValue(*szText);
      ++szText;
      guid.Data4[i] = (guid.Data4[i] << 4) | HexCharacterToIntValue(*szText);
      ++szText;
    }

    // -
    ++szText;

    for (int i = 2; i < 8; ++i)
    {
      guid.Data4[i] = (guid.Data4[i] << 4) | HexCharacterToIntValue(*szText);
      ++szText;
      guid.Data4[i] = (guid.Data4[i] << 4) | HexCharacterToIntValue(*szText);
      ++szText;
    }

    nsUuid result;
    nsMemoryUtils::Copy<nsUuid>(&result, reinterpret_cast<nsUuid*>(&guid), 1);

    return result;
  }

#define Check(name)                                  \
  if (sColorName.IsEqual_NoCase(NS_STRINGIZE(name))) \
  return nsColor::name

  nsColor GetColorByName(nsStringView sColorName, bool* out_pValidColorName)
  {
    if (out_pValidColorName)
      *out_pValidColorName = false;

    if (sColorName.IsEmpty())
      return nsColor::Black; // considered not to be a valid color name

    const nsUInt32 uiLen = sColorName.GetElementCount();

    auto twoCharsToByte = [](const char* szColorChars, nsUInt8& out_uiByte) -> nsResult
    {
      nsInt8 firstChar = HexCharacterToIntValue(szColorChars[0]);
      nsInt8 secondChar = HexCharacterToIntValue(szColorChars[1]);
      if (firstChar < 0 || secondChar < 0)
      {
        return NS_FAILURE;
      }
      out_uiByte = (static_cast<nsUInt8>(firstChar) << 4) | static_cast<nsUInt8>(secondChar);
      return NS_SUCCESS;
    };

    if (sColorName.StartsWith("#"))
    {
      if (uiLen == 7 || uiLen == 9) // #RRGGBB or #RRGGBBAA
      {
        nsUInt8 cv[4] = {0, 0, 0, 255};

        const char* szColorName = sColorName.GetStartPointer();

        if (twoCharsToByte(szColorName + 1, cv[0]).Failed())
          return nsColor::Black;
        if (twoCharsToByte(szColorName + 3, cv[1]).Failed())
          return nsColor::Black;
        if (twoCharsToByte(szColorName + 5, cv[2]).Failed())
          return nsColor::Black;

        if (uiLen == 9)
        {
          if (twoCharsToByte(szColorName + 7, cv[3]).Failed())
            return nsColor::Black;
        }

        if (out_pValidColorName)
          *out_pValidColorName = true;

        return nsColorGammaUB(cv[0], cv[1], cv[2], cv[3]);
      }

      // else RebeccaPurple !
    }
    else
    {
      if (out_pValidColorName)
        *out_pValidColorName = true;

      Check(AliceBlue);
      Check(AntiqueWhite);
      Check(Aqua);
      Check(Aquamarine);
      Check(Azure);
      Check(Beige);
      Check(Bisque);
      Check(Black);
      Check(BlanchedAlmond);
      Check(Blue);
      Check(BlueViolet);
      Check(Brown);
      Check(BurlyWood);
      Check(CadetBlue);
      Check(Chartreuse);
      Check(Chocolate);
      Check(Coral);
      Check(CornflowerBlue); // The Original!
      Check(Cornsilk);
      Check(Crimson);
      Check(Cyan);
      Check(DarkBlue);
      Check(DarkCyan);
      Check(DarkGoldenRod);
      Check(DarkGray);
      Check(DarkGrey);
      Check(DarkGreen);
      Check(DarkKhaki);
      Check(DarkMagenta);
      Check(DarkOliveGreen);
      Check(DarkOrange);
      Check(DarkOrchid);
      Check(DarkRed);
      Check(DarkSalmon);
      Check(DarkSeaGreen);
      Check(DarkSlateBlue);
      Check(DarkSlateGray);
      Check(DarkSlateGrey);
      Check(DarkTurquoise);
      Check(DarkViolet);
      Check(DeepPink);
      Check(DeepSkyBlue);
      Check(DimGray);
      Check(DimGrey);
      Check(DodgerBlue);
      Check(FireBrick);
      Check(FloralWhite);
      Check(ForestGreen);
      Check(Fuchsia);
      Check(Gainsboro);
      Check(GhostWhite);
      Check(Gold);
      Check(GoldenRod);
      Check(Gray);
      Check(Grey);
      Check(Green);
      Check(GreenYellow);
      Check(HoneyDew);
      Check(HotPink);
      Check(IndianRed);
      Check(Indigo);
      Check(Ivory);
      Check(Khaki);
      Check(Lavender);
      Check(LavenderBlush);
      Check(LawnGreen);
      Check(LemonChiffon);
      Check(LightBlue);
      Check(LightCoral);
      Check(LightCyan);
      Check(LightGoldenRodYellow);
      Check(LightGray);
      Check(LightGrey);
      Check(LightGreen);
      Check(LightPink);
      Check(LightSalmon);
      Check(LightSeaGreen);
      Check(LightSkyBlue);
      Check(LightSlateGray);
      Check(LightSlateGrey);
      Check(LightSteelBlue);
      Check(LightYellow);
      Check(Lime);
      Check(LimeGreen);
      Check(Linen);
      Check(Magenta);
      Check(Maroon);
      Check(MediumAquaMarine);
      Check(MediumBlue);
      Check(MediumOrchid);
      Check(MediumPurple);
      Check(MediumSeaGreen);
      Check(MediumSlateBlue);
      Check(MediumSpringGreen);
      Check(MediumTurquoise);
      Check(MediumVioletRed);
      Check(MidnightBlue);
      Check(MintCream);
      Check(MistyRose);
      Check(Moccasin);
      Check(NavajoWhite);
      Check(Navy);
      Check(OldLace);
      Check(Olive);
      Check(OliveDrab);
      Check(Orange);
      Check(OrangeRed);
      Check(Orchid);
      Check(PaleGoldenRod);
      Check(PaleGreen);
      Check(PaleTurquoise);
      Check(PaleVioletRed);
      Check(PapayaWhip);
      Check(PeachPuff);
      Check(Peru);
      Check(Pink);
      Check(Plum);
      Check(PowderBlue);
      Check(Purple);
      Check(RebeccaPurple);
      Check(Red);
      Check(RosyBrown);
      Check(RoyalBlue);
      Check(SaddleBrown);
      Check(Salmon);
      Check(SandyBrown);
      Check(SeaGreen);
      Check(SeaShell);
      Check(Sienna);
      Check(Silver);
      Check(SkyBlue);
      Check(SlateBlue);
      Check(SlateGray);
      Check(SlateGrey);
      Check(Snow);
      Check(SpringGreen);
      Check(SteelBlue);
      Check(Tan);
      Check(Teal);
      Check(Thistle);
      Check(Tomato);
      Check(Turquoise);
      Check(Violet);
      Check(Wheat);
      Check(White);
      Check(WhiteSmoke);
      Check(Yellow);
      Check(YellowGreen);
    }

    if (out_pValidColorName)
      *out_pValidColorName = false;

    return nsColor::RebeccaPurple;
  }

#undef Check

#define Check(name)         \
  if (nsColor::name == col) \
  return #name

  nsString GetColorName(const nsColor& col)
  {
    Check(AliceBlue);
    Check(AntiqueWhite);
    Check(Aqua);
    Check(Aquamarine);
    Check(Azure);
    Check(Beige);
    Check(Bisque);
    Check(Black);
    Check(BlanchedAlmond);
    Check(Blue);
    Check(BlueViolet);
    Check(Brown);
    Check(BurlyWood);
    Check(CadetBlue);
    Check(Chartreuse);
    Check(Chocolate);
    Check(Coral);
    Check(CornflowerBlue); // The Original!
    Check(Cornsilk);
    Check(Crimson);
    Check(Cyan);
    Check(DarkBlue);
    Check(DarkCyan);
    Check(DarkGoldenRod);
    Check(DarkGray);
    Check(DarkGreen);
    Check(DarkKhaki);
    Check(DarkMagenta);
    Check(DarkOliveGreen);
    Check(DarkOrange);
    Check(DarkOrchid);
    Check(DarkRed);
    Check(DarkSalmon);
    Check(DarkSeaGreen);
    Check(DarkSlateBlue);
    Check(DarkSlateGray);
    Check(DarkTurquoise);
    Check(DarkViolet);
    Check(DeepPink);
    Check(DeepSkyBlue);
    Check(DimGray);
    Check(DodgerBlue);
    Check(FireBrick);
    Check(FloralWhite);
    Check(ForestGreen);
    Check(Fuchsia);
    Check(Gainsboro);
    Check(GhostWhite);
    Check(Gold);
    Check(GoldenRod);
    Check(Gray);
    Check(Green);
    Check(GreenYellow);
    Check(HoneyDew);
    Check(HotPink);
    Check(IndianRed);
    Check(Indigo);
    Check(Ivory);
    Check(Khaki);
    Check(Lavender);
    Check(LavenderBlush);
    Check(LawnGreen);
    Check(LemonChiffon);
    Check(LightBlue);
    Check(LightCoral);
    Check(LightCyan);
    Check(LightGoldenRodYellow);
    Check(LightGray);
    Check(LightGreen);
    Check(LightPink);
    Check(LightSalmon);
    Check(LightSeaGreen);
    Check(LightSkyBlue);
    Check(LightSlateGray);
    Check(LightSteelBlue);
    Check(LightYellow);
    Check(Lime);
    Check(LimeGreen);
    Check(Linen);
    Check(Magenta);
    Check(Maroon);
    Check(MediumAquaMarine);
    Check(MediumBlue);
    Check(MediumOrchid);
    Check(MediumPurple);
    Check(MediumSeaGreen);
    Check(MediumSlateBlue);
    Check(MediumSpringGreen);
    Check(MediumTurquoise);
    Check(MediumVioletRed);
    Check(MidnightBlue);
    Check(MintCream);
    Check(MistyRose);
    Check(Moccasin);
    Check(NavajoWhite);
    Check(Navy);
    Check(OldLace);
    Check(Olive);
    Check(OliveDrab);
    Check(Orange);
    Check(OrangeRed);
    Check(Orchid);
    Check(PaleGoldenRod);
    Check(PaleGreen);
    Check(PaleTurquoise);
    Check(PaleVioletRed);
    Check(PapayaWhip);
    Check(PeachPuff);
    Check(Peru);
    Check(Pink);
    Check(Plum);
    Check(PowderBlue);
    Check(Purple);
    Check(RebeccaPurple);
    Check(Red);
    Check(RosyBrown);
    Check(RoyalBlue);
    Check(SaddleBrown);
    Check(Salmon);
    Check(SandyBrown);
    Check(SeaGreen);
    Check(SeaShell);
    Check(Sienna);
    Check(Silver);
    Check(SkyBlue);
    Check(SlateBlue);
    Check(SlateGray);
    Check(Snow);
    Check(SpringGreen);
    Check(SteelBlue);
    Check(Tan);
    Check(Teal);
    Check(Thistle);
    Check(Tomato);
    Check(Turquoise);
    Check(Violet);
    Check(Wheat);
    Check(White);
    Check(WhiteSmoke);
    Check(Yellow);
    Check(YellowGreen);

    nsColorGammaUB cg = col;

    nsStringBuilder s;

    if (cg.a == 255)
    {
      s.SetFormat("#{0}{1}{2}", nsArgU(cg.r, 2, true, 16, true), nsArgU(cg.g, 2, true, 16, true), nsArgU(cg.b, 2, true, 16, true));
    }
    else
    {
      s.SetFormat("#{0}{1}{2}{3}", nsArgU(cg.r, 2, true, 16, true), nsArgU(cg.g, 2, true, 16, true), nsArgU(cg.b, 2, true, 16, true),
        nsArgU(cg.a, 2, true, 16, true));
    }

    return s;
  }

#undef Check

} // namespace nsConversionUtils
