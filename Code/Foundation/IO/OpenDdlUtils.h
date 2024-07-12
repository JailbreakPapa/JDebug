#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/OpenDdlParser.h>

class nsOpenDdlReader;
class nsOpenDdlWriter;
class nsOpenDdlReaderElement;

namespace nsOpenDdlUtils
{
  /// \brief Converts the data that \a pElement points to to an nsColor.
  ///
  /// \a pElement may be a primitives list of 3 or 4 floats or of 3 or 4 unsigned int8 values.
  /// It may also be a group that contains such a primitives list as the only child.
  /// floats will be interpreted as linear colors, unsigned int 8 will be interpreted as nsColorGammaUB.
  /// If only 3 values are given, alpha will be filled with 1.0f.
  /// If less than 3 or more than 4 values are given, the function returns NS_FAILURE.
  NS_FOUNDATION_DLL nsResult ConvertToColor(const nsOpenDdlReaderElement* pElement, nsColor& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsColorGammaUB.
  ///
  /// \a pElement may be a primitives list of 3 or 4 floats or of 3 or 4 unsigned int8 values.
  /// It may also be a group that contains such a primitives list as the only child.
  /// floats will be interpreted as linear colors, unsigned int 8 will be interpreted as nsColorGammaUB.
  /// If only 3 values are given, alpha will be filled with 1.0f.
  /// If less than 3 or more than 4 values are given, the function returns NS_FAILURE.
  NS_FOUNDATION_DLL nsResult ConvertToColorGamma(const nsOpenDdlReaderElement* pElement, nsColorGammaUB& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsTime.
  ///
  /// \a pElement maybe be a primitives list of exactly 1 float or double.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToTime(const nsOpenDdlReaderElement* pElement, nsTime& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec2.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec2(const nsOpenDdlReaderElement* pElement, nsVec2& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec3.
  ///
  /// \a pElement maybe be a primitives list of exactly 3 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec3(const nsOpenDdlReaderElement* pElement, nsVec3& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec4.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec4(const nsOpenDdlReaderElement* pElement, nsVec4& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec2I32.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 int32.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec2I(const nsOpenDdlReaderElement* pElement, nsVec2I32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec3I32.
  ///
  /// \a pElement maybe be a primitives list of exactly 3 int32.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec3I(const nsOpenDdlReaderElement* pElement, nsVec3I32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec4I32.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 int32.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec4I(const nsOpenDdlReaderElement* pElement, nsVec4I32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec2U32.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 uint32.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec2U(const nsOpenDdlReaderElement* pElement, nsVec2U32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec3U32.
  ///
  /// \a pElement maybe be a primitives list of exactly 3 uint32.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec3U(const nsOpenDdlReaderElement* pElement, nsVec3U32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsVec4U32.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 uint32.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToVec4U(const nsOpenDdlReaderElement* pElement, nsVec4U32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsMat3.
  ///
  /// \a pElement maybe be a primitives list of exactly 9 floats.
  /// The elements are expected to be in column-major format. See nsMatrixLayout::ColumnMajor.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToMat3(const nsOpenDdlReaderElement* pElement, nsMat3& out_mResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsMat4.
  ///
  /// \a pElement maybe be a primitives list of exactly 16 floats.
  /// The elements are expected to be in column-major format. See nsMatrixLayout::ColumnMajor.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToMat4(const nsOpenDdlReaderElement* pElement, nsMat4& out_mResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsTransform.
  ///
  /// \a pElement maybe be a primitives list of exactly 12 floats.
  /// The first 9 elements are expected to be a mat3 in column-major format. See nsMatrixLayout::ColumnMajor.
  /// The last 3 elements are the position vector.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToTransform(const nsOpenDdlReaderElement* pElement, nsTransform& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsQuat.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToQuat(const nsOpenDdlReaderElement* pElement, nsQuat& out_qResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsUuid.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 unsigned_int64.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToUuid(const nsOpenDdlReaderElement* pElement, nsUuid& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsAngle.
  ///
  /// \a pElement maybe be a primitives list of exactly 1 float.
  /// The value is assumed to be in radians.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToAngle(const nsOpenDdlReaderElement* pElement, nsAngle& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsHashedString.
  ///
  /// \a pElement maybe be a primitives list of exactly 1 string.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToHashedString(const nsOpenDdlReaderElement* pElement, nsHashedString& out_sResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an nsTempHashedString.
  ///
  /// \a pElement maybe be a primitives list of exactly 1 uint64.
  /// It may also be a group that contains such a primitives list as the only child.
  NS_FOUNDATION_DLL nsResult ConvertToTempHashedString(const nsOpenDdlReaderElement* pElement, nsTempHashedString& out_sResult); // [tested]

  /// \brief Uses the elements custom type name to infer which type the object holds and reads it into the nsVariant.
  ///
  /// Depending on the custom type name, one of the other ConvertToXY functions is called and the respective conditions to the data format apply.
  /// Supported type names are: "Color", "ColorGamma", "Time", "Vec2", "Vec3", "Vec4", "Mat3", "Mat4", "Transform", "Quat", "Uuid", "Angle", "HashedString", "TempHashedString"
  /// Type names are case sensitive.
  NS_FOUNDATION_DLL nsResult ConvertToVariant(const nsOpenDdlReaderElement* pElement, nsVariant& out_result); // [tested]

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  /// \brief Writes an nsColor to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreColor(nsOpenDdlWriter& ref_writer, const nsColor& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsColorGammaUB to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreColorGamma(nsOpenDdlWriter& ref_writer, const nsColorGammaUB& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsTime to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreTime(nsOpenDdlWriter& ref_writer, const nsTime& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec2 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec2(nsOpenDdlWriter& ref_writer, const nsVec2& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec3 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec3(nsOpenDdlWriter& ref_writer, const nsVec3& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec4 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec4(nsOpenDdlWriter& ref_writer, const nsVec4& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec2 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec2I(nsOpenDdlWriter& ref_writer, const nsVec2I32& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec3 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec3I(nsOpenDdlWriter& ref_writer, const nsVec3I32& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec4 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec4I(nsOpenDdlWriter& ref_writer, const nsVec4I32& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec2 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec2U(nsOpenDdlWriter& ref_writer, const nsVec2U32& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec3 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec3U(nsOpenDdlWriter& ref_writer, const nsVec3U32& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVec4 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVec4U(nsOpenDdlWriter& ref_writer, const nsVec4U32& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsMat3 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreMat3(nsOpenDdlWriter& ref_writer, const nsMat3& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsMat4 to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreMat4(nsOpenDdlWriter& ref_writer, const nsMat4& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsTransform to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreTransform(nsOpenDdlWriter& ref_writer, const nsTransform& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsQuat to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreQuat(nsOpenDdlWriter& ref_writer, const nsQuat& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsUuid to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreUuid(nsOpenDdlWriter& ref_writer, const nsUuid& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsAngle to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreAngle(nsOpenDdlWriter& ref_writer, const nsAngle& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsHashedString to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreHashedString(nsOpenDdlWriter& ref_writer, const nsHashedString& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsTempHashedString to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreTempHashedString(nsOpenDdlWriter& ref_writer, const nsTempHashedString& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an nsVariant to DDL such that the type can be reconstructed.
  NS_FOUNDATION_DLL void StoreVariant(nsOpenDdlWriter& ref_writer, const nsVariant& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single string and an optional name.
  NS_FOUNDATION_DLL void StoreString(nsOpenDdlWriter& ref_writer, const nsStringView& value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreBool(nsOpenDdlWriter& ref_writer, bool value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreFloat(nsOpenDdlWriter& ref_writer, float value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreDouble(nsOpenDdlWriter& ref_writer, double value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreInt8(nsOpenDdlWriter& ref_writer, nsInt8 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreInt16(nsOpenDdlWriter& ref_writer, nsInt16 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreInt32(nsOpenDdlWriter& ref_writer, nsInt32 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreInt64(nsOpenDdlWriter& ref_writer, nsInt64 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreUInt8(nsOpenDdlWriter& ref_writer, nsUInt8 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreUInt16(nsOpenDdlWriter& ref_writer, nsUInt16 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreUInt32(nsOpenDdlWriter& ref_writer, nsUInt32 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  NS_FOUNDATION_DLL void StoreUInt64(nsOpenDdlWriter& ref_writer, nsUInt64 value, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Writes an invalid variant and an optional name.
  NS_FOUNDATION_DLL void StoreInvalid(nsOpenDdlWriter& ref_writer, nsStringView sName = {}, bool bGlobalName = false);
} // namespace nsOpenDdlUtils
