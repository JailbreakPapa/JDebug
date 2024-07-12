#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Math/Mat3.h>
#include <Foundation/Math/Mat4.h>
#include <Foundation/Math/Quat.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Uuid.h>

/// \brief This namespace contains functions to convert between different types.
///
/// Contains helper functions to convert from strings to numerical values.
/// To convert from numerical values to strings, use nsStringBuilder::Format, which provides a rich set of formatting options.
namespace nsConversionUtils
{
  /// \brief Parses szString and converts it to an integer value. Returns NS_FAILURE if the string contains no parsable integer value.
  ///
  /// \param szString
  ///   If szString is nullptr or an empty string or starts with an some non-whitespace and non-sign character, NS_FAILURE is returned.
  ///   All whitespace at the start of the string are skipped. Each minus sign flips the sign of the output value, so --3 will return 3.
  ///   Plus signs are skipped and have no effect, so -+3 will still return -3.
  /// \param out_Res
  ///   The parsed value is returned in out_Res on success. On failure out_Res is not modified at all, so you can store a default value
  ///   in it, before calling StringToInt() and always use that value, even without checking for success or failure.
  /// \param out_LastParsePosition
  ///   On success out_LastParsePosition will contain the address of the character in szString that stopped the parser. This might point
  ///   to the zero terminator of szString, or to some non-digit character, since for example "5+6" will parse as '5' and '+' will be the
  ///   position where the parser stopped (returning NS_SUCCESS).
  ///   If szString is supposed to only contain one full integer and nothing else, then out_LastParsePosition should always point to a zero
  ///   terminator (otherwise the string was malformed).
  /// \return
  ///   NS_SUCCESS if any integer value could get properly extracted from szString (including 0). This includes that only some part of the
  ///   string was parsed until a non-digit character was encountered.
  ///   NS_FAILURE if the string starts with something that can not be interpreted as an integer.
  NS_FOUNDATION_DLL nsResult StringToInt(nsStringView sText, nsInt32& out_iRes, const char** out_pLastParsePosition = nullptr); // [tested]

  /// \brief Same as StringToInt() but expects the string to be a uint32.
  ///
  /// If the parsed value is a valid int but outside the uint32 value range, the function returns NS_FAILURE.
  NS_FOUNDATION_DLL nsResult StringToUInt(nsStringView sText, nsUInt32& out_uiRes, const char** out_pLastParsePosition = nullptr); // [tested]

  /// \brief Same as StringToInt but converts to a 64bit integer value instead.
  NS_FOUNDATION_DLL nsResult StringToInt64(nsStringView sText, nsInt64& out_iRes, const char** out_pLastParsePosition = nullptr); // [tested]

  /// \brief Parses szString and converts it to a double value. Returns NS_FAILURE if the string contains no parseable floating point value.
  ///
  /// \param szString
  ///   If szString is nullptr or an empty string or starts with some non-whitespace and non-sign character, NS_FAILURE is returned.
  ///   All whitespace at the start of the string are skipped. Each minus sign flips the sign of the output value, so --3 will return 3.
  ///   Plus signs are skipped and have no effect, so -+3 will still return -3.
  ///   The value string may contain one '.' to separate integer and fractional part. It may also contain an 'e' for scientific notation
  ///   followed by an integer exponent value. No '.' may follow after an 'e' anymore.
  ///   Additionally the value may be terminated by an 'f' to indicate a floating point value. The 'f' will be skipped (as can be observed
  ///   through out_LastParsePosition, and it will terminate further parsing, but it will not affect the precision of the result.
  ///   Commas (',') are never treated as fractional part separators (as in the German locale).
  /// \param out_Res
  ///   If NS_SUCCESS is returned, out_Res will contain the result. Otherwise it stays unmodified.
  ///   The result may have rounding errors, i.e. even though a double may be able to represent the string value exactly, there is no
  ///   guarantee that out_Res will be 100% identical. If that is required, use the C lib atof() function.
  /// \param out_LastParsePosition
  ///   On success out_LastParsePosition will contain the address of the character in szString that stopped the parser. This might point
  ///   to the zero terminator of szString, or to some unexpected character, since for example "5+6" will parse as '5' and '+' will be the
  ///   position where the parser stopped (returning NS_SUCCESS).
  ///   If you want to parse strictly (e.g. you do not want to parse "5+6" as "5") you can use this result to check that only certain
  ///   characters were encountered after the float (e.g. only '\0' or ',').
  /// \return
  ///   NS_SUCCESS if any text was encountered that can be interpreted as a floating point value.
  ///   NS_FAILURE otherwise.
  ///
  /// \note
  ///   The difference to the C lib atof() function is that this function properly returns whether something could get parsed as a floating
  ///   point value, at all. atof() just returns zero in such a case. Also the way whitespace and signs at the beginning of the string are
  ///   handled is different and StringToFloat will return 'success' if it finds anything that can be parsed as a float, even if the string
  ///   continues with invalid text. So you can parse "2.54f+3.5" as "2.54f" and out_LastParsePosition will tell you where the parser
  ///   stopped. On the down-side StringToFloat() is probably not as precise as atof(), because of a very simplistic conversion algorithm.
  ///   If you require the features of StringToFloat() and the precision of atof(), you can let StringToFloat() handle the cases for
  ///   detecting the validity, the sign and where the value ends and then use atof to parse only that substring with maximum precision.
  NS_FOUNDATION_DLL nsResult StringToFloat(nsStringView sText, double& out_fRes, const char** out_pLastParsePosition = nullptr); // [tested]

  /// \brief Parses szString and checks that the first word it finds starts with a phrase that can be interpreted as a boolean value.
  ///
  /// \param szString
  ///   If szString starts with whitespace characters, they are skipped. NS_SUCCESS is returned (and out_Res is filled with true/false),
  ///   if the string then starts with any of the following phrases: "true", "false", "on", "off", "yes", "no", "1", "0", "enable",
  ///   "disable". NS_FAILURE is returned if none of those is encountered (or the string is empty). It does not matter, whether the string
  ///   continues with some other text, e.g. "nolf" is still interpreted as "no". That means you can pass strings such as "true, a = false"
  ///   into this function to just parse the next piece of a command line.
  /// \param out_Res
  ///   If NS_SUCCESS is returned, out_Res contains a valid value. Otherwise it is not modified. That means you can initialize it with a
  ///   default value that can be used even if NS_FAILURE is returned.
  /// \param out_LastParsePosition
  ///   On success out_LastParsePosition will contain the address of the character in szString that stopped the parser. This might point
  ///   to the zero terminator of szString, or to the next character after the phrase "true", "false", "on", "off", etc. which was
  ///   interpreted as a boolean value. If you want to parse strictly (e.g. you do not want to parse "Nolf" as "no") you can use this result
  ///   to check that only certain characters were encountered after the boolean phrase (such as '\0' or ',' etc.).
  /// \return
  ///   NS_SUCCESS if any phrase was encountered that can be interpreted as a boolean value.
  ///   NS_FAILURE otherwise.
  NS_FOUNDATION_DLL nsResult StringToBool(nsStringView sText, bool& out_bRes, const char** out_pLastParsePosition = nullptr); // [tested]


  /// \brief Parses \a szText and tries to find up to \a uiNumFloats float values to extract. Skips all characters that cannot be
  /// interpreted as numbers.
  ///
  /// This function can be used to convert string representations of vectors or other more complex numbers. It will parse the string from
  /// front to back and convert anything that looks like a number and add it to the given float array. For example a text like '(1, 2, 3)'
  /// will result in up to three floats. Since any invalid character is skipped, the parenthesis and commas will be ignored (though they act
  /// as delimiters, of course).
  ///
  /// \param szText
  ///   The null terminated string to parse.
  /// \param uiNumFloats
  ///   The maximum number of floats to extract.
  /// \param out_pFloats
  ///   An array of floats that can hold at least uiNumFloats. The results are written to this array.
  /// \param out_LastParsePosition
  ///   The position in szText where the function stopped parsing. It will stop either because the end of the string was reached,
  ///   or uiNumFloats values were successfully extracted.
  /// \return
  ///   The number of successfully extracted values (and thus valid values in out_pFloats).
  NS_FOUNDATION_DLL nsUInt32 ExtractFloatsFromString(nsStringView sText, nsUInt32 uiNumFloats, float* out_pFloats, const char** out_pLastParsePosition = nullptr); // [tested]

  /// \brief Converts a hex character ('0', '1', ... '9', 'A'/'a', ... 'F'/'f') to the corresponding int value 0 - 15.
  ///
  /// \note Returns -1 for invalid HEX characters.
  NS_FOUNDATION_DLL nsInt8 HexCharacterToIntValue(nsUInt32 uiCharacter); // [tested]

  /// \brief Same as ConvertHexStringToUInt() with uiMaxHexCharacters set to 8.
  NS_FOUNDATION_DLL nsResult ConvertHexStringToUInt32(nsStringView sHex, nsUInt32& out_uiResult); // [tested]

  /// \brief Same as ConvertHexStringToUInt() with uiMaxHexCharacters set to 16.
  NS_FOUNDATION_DLL nsResult ConvertHexStringToUInt64(nsStringView sHex, nsUInt64& out_uiResult); // [tested]

  /// \brief Converts a hex string (i.e. 0xAABBCCDD) into its uint64 value.
  ///
  /// "0x" at the beginning is ignored.
  /// Empty strings are interpreted as 'valid', representing the value 0 (returns NS_SUCCESS).
  /// If the nsStringView is shorter than uiMaxHexCharacters, this is interpreted as a valid HEX value with a smaller value.
  /// If the string is longer than uiMaxHexCharacters (after the '0x'), the additional characters are not parsed, at all.
  /// If the first uiMaxHexCharacters (after the '0x') contain any non-HEX characters, parsing is interrupted and NS_FAILURE is returned.
  NS_FOUNDATION_DLL nsResult ConvertHexStringToUInt(nsStringView sHex, nsUInt64& out_uiResult, nsUInt32 uiMaxHexCharacters, nsUInt32* pTotalCharactersParsed); // [tested]

  /// \brief Converts a HEX string to a binary value.
  ///
  /// "0x" or "0X" at the start is allowed and will be skipped.
  /// A maximum of \a uiBinaryBuffer bytes is written to \a pBinary.
  /// If the string contains fewer HEX values than fit into \a pBinary, the remaining bytes will not be touched, so make sure all data is
  /// properly initialized! The hex values are read 2 characters at a time to form a single byte value. If at the end a single character is
  /// left (so an odd number of characters in total) that character is ignored entirely! Values are started to be written at \a pBinary and
  /// then the pointer is increased, so the first values in szHEX represent to least significant bytes in \a pBinary.
  ///
  /// \note This function does not validate that the incoming string is actually valid HEX. If an invalid character is used, the result will
  /// be invalid and there is no error reported.
  NS_FOUNDATION_DLL void ConvertHexToBinary(nsStringView sText, nsUInt8* pBinary, nsUInt32 uiBinaryBuffer); // [tested]

  /// \brief Converts a binary stream to a HEX string.
  ///
  /// The result is returned by calling a lambda to append to an output container.
  /// The lambda signature must be:
  /// void Append(const char* twoChars)
  /// The given string will contain exactly two characters and will be zero terminated.
  template <typename APPEND_CONTAINER_LAMBDA>
  inline void ConvertBinaryToHex(const void* pBinaryData, nsUInt32 uiBytes, APPEND_CONTAINER_LAMBDA append); // [tested]

  /// \brief Converts a string that was written with nsConversionUtils::ToString(nsUuid) back to an nsUuid object.
  NS_FOUNDATION_DLL nsUuid ConvertStringToUuid(nsStringView sText); // [tested]

  /// \brief Returns true when the given string is in the exact format "{ 05af8d07-0b38-44a6-8d50-49731ae2625d }"
  /// This includes braces, whitespaces and dashes. This is the format that ToString produces.
  NS_FOUNDATION_DLL bool IsStringUuid(nsStringView sText); // [tested]

  /// \brief Converts a bool to a string
  NS_ALWAYS_INLINE const nsStringBuilder& ToString(bool value, nsStringBuilder& out_sResult) // [tested]
  {
    out_sResult = value ? "true" : "false";
    return out_sResult;
  }

  /// \brief Converts a 8bit signed integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsInt8 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a 8bit unsigned integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsUInt8 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a 16bit signed integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsInt16 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a 16bit unsigned integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsUInt16 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a 32bit signed integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsInt32 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a 32bit unsigned integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsUInt32 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a 64bit signed integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsInt64 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a 64bit unsigned integer to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(nsUInt64 value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a float to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(float value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a double to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(double value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a color to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsColor& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a color to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsColorGammaUB& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a vec2 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsVec2& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a vec3 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsVec3& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a vec4 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsVec4& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a vec2I32 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsVec2I32& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a vec3I32 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsVec3I32& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a vec4I32 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsVec4I32& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a quat to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsQuat& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a mat3 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsMat3& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a mat4 to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsMat4& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a transform to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsTransform& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a Uuid to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsUuid& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts an angle to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsAngle& value, nsStringBuilder& out_sResult); // [tested]

  /// \brief Converts a time to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsTime& value, nsStringBuilder& out_sResult);

  /// \brief Converts a nsStringView to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsStringView& value, nsStringBuilder& out_sResult);

  /// \brief Converts a hashed string to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsHashedString& value, nsStringBuilder& out_sResult);

  /// \brief Converts a temp hashed string to a string. Will print the hash value since the original string can't be restored from a temp hashed string.
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsTempHashedString& value, nsStringBuilder& out_sResult);

  /// \brief Converts a nsVariantArray to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsDynamicArray<nsVariant>& value, nsStringBuilder& out_sResult);

  /// \brief Converts a nsVariantDictionary to a string
  NS_FOUNDATION_DLL const nsStringBuilder& ToString(const nsHashTable<nsString, nsVariant>& value, nsStringBuilder& out_sResult);

  /// \brief Fallback ToString implementation for all types that don't have one
  template <typename T>
  NS_ALWAYS_INLINE const nsStringBuilder& ToString(const T& value, nsStringBuilder& out_sResult)
  {
    out_sResult = "N/A";
    return out_sResult;
  }

  /// \brief Returns the color with the given name.
  ///
  /// Allowed are all predefined color names (case-insensitive), as well as Hex-Values in the form '#RRGGBB' and '#RRGGBBAA'
  /// If out_ValidColorName is a valid pointer, it contains true if the color name was known, otherwise false
  NS_FOUNDATION_DLL nsColor GetColorByName(nsStringView sText, bool* out_pValidColorName = nullptr); // [tested]

  /// \brief The inverse of GetColorByName
  NS_FOUNDATION_DLL nsString GetColorName(const nsColor& col); // [tested]
}; // namespace nsConversionUtils

template <typename APPEND_CONTAINER_LAMBDA>
inline void nsConversionUtils::ConvertBinaryToHex(const void* pBinaryData, nsUInt32 uiBytes, APPEND_CONTAINER_LAMBDA append) // [tested]
{
  char tmp[4];

  nsUInt8* pBytes = (nsUInt8*)pBinaryData;

  for (nsUInt32 i = 0; i < uiBytes; ++i)
  {
    nsStringUtils::snprintf(tmp, 4, "%02X", (nsUInt32)*pBytes);
    ++pBytes;

    append(tmp);
  }
}
