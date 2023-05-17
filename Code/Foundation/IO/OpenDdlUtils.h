#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/OpenDdlParser.h>

class wdOpenDdlReader;
class wdOpenDdlWriter;
class wdOpenDdlReaderElement;

namespace wdOpenDdlUtils
{
  /// \brief Converts the data that \a pElement points to to an wdColor.
  ///
  /// \a pElement may be a primitives list of 3 or 4 floats or of 3 or 4 unsigned int8 values.
  /// It may also be a group that contains such a primitives list as the only child.
  /// floats will be interpreted as linear colors, unsigned int 8 will be interpreted as wdColorGammaUB.
  /// If only 3 values are given, alpha will be filled with 1.0f.
  /// If less than 3 or more than 4 values are given, the function returns WD_FAILURE.
  WD_FOUNDATION_DLL wdResult ConvertToColor(const wdOpenDdlReaderElement* pElement, wdColor& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdColorGammaUB.
  ///
  /// \a pElement may be a primitives list of 3 or 4 floats or of 3 or 4 unsigned int8 values.
  /// It may also be a group that contains such a primitives list as the only child.
  /// floats will be interpreted as linear colors, unsigned int 8 will be interpreted as wdColorGammaUB.
  /// If only 3 values are given, alpha will be filled with 1.0f.
  /// If less than 3 or more than 4 values are given, the function returns WD_FAILURE.
  WD_FOUNDATION_DLL wdResult ConvertToColorGamma(const wdOpenDdlReaderElement* pElement, wdColorGammaUB& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdTime.
  ///
  /// \a pElement maybe be a primitives list of exactly 1 float or double.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToTime(const wdOpenDdlReaderElement* pElement, wdTime& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec2.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec2(const wdOpenDdlReaderElement* pElement, wdVec2& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec3.
  ///
  /// \a pElement maybe be a primitives list of exactly 3 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec3(const wdOpenDdlReaderElement* pElement, wdVec3& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec4.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec4(const wdOpenDdlReaderElement* pElement, wdVec4& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec2I32.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 int32.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec2I(const wdOpenDdlReaderElement* pElement, wdVec2I32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec3I32.
  ///
  /// \a pElement maybe be a primitives list of exactly 3 int32.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec3I(const wdOpenDdlReaderElement* pElement, wdVec3I32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec4I32.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 int32.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec4I(const wdOpenDdlReaderElement* pElement, wdVec4I32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec2U32.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 uint32.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec2U(const wdOpenDdlReaderElement* pElement, wdVec2U32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec3U32.
  ///
  /// \a pElement maybe be a primitives list of exactly 3 uint32.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec3U(const wdOpenDdlReaderElement* pElement, wdVec3U32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdVec4U32.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 uint32.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToVec4U(const wdOpenDdlReaderElement* pElement, wdVec4U32& out_vResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdMat3.
  ///
  /// \a pElement maybe be a primitives list of exactly 9 floats.
  /// The elements are expected to be in column-major format. See wdMatrixLayout::ColumnMajor.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToMat3(const wdOpenDdlReaderElement* pElement, wdMat3& out_mResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdMat4.
  ///
  /// \a pElement maybe be a primitives list of exactly 16 floats.
  /// The elements are expected to be in column-major format. See wdMatrixLayout::ColumnMajor.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToMat4(const wdOpenDdlReaderElement* pElement, wdMat4& out_mResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdTransform.
  ///
  /// \a pElement maybe be a primitives list of exactly 12 floats.
  /// The first 9 elements are expected to be a mat3 in column-major format. See wdMatrixLayout::ColumnMajor.
  /// The last 3 elements are the position vector.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToTransform(const wdOpenDdlReaderElement* pElement, wdTransform& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdQuat.
  ///
  /// \a pElement maybe be a primitives list of exactly 4 floats.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToQuat(const wdOpenDdlReaderElement* pElement, wdQuat& out_qResult); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdUuid.
  ///
  /// \a pElement maybe be a primitives list of exactly 2 unsigned_int64.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToUuid(const wdOpenDdlReaderElement* pElement, wdUuid& out_result); // [tested]

  /// \brief Converts the data that \a pElement points to to an wdAngle.
  ///
  /// \a pElement maybe be a primitives list of exactly 1 float.
  /// The value is assumed to be in degree.
  /// It may also be a group that contains such a primitives list as the only child.
  WD_FOUNDATION_DLL wdResult ConvertToAngle(const wdOpenDdlReaderElement* pElement, wdAngle& out_result); // [tested]

  /// \brief Uses the elements custom type name to infer which type the object holds and reads it into the wdVariant.
  ///
  /// Depending on the custom type name, one of the other ConvertToXY functions is called and the respective conditions to the data format apply.
  /// Supported type names are: "Color", "ColorGamma", "Time", "Vec2", "Vec3", "Vec4", "Mat3", "Mat4", "Transform", "Quat", "Uuid", "Angle"
  /// Type names are case sensitive.
  WD_FOUNDATION_DLL wdResult ConvertToVariant(const wdOpenDdlReaderElement* pElement, wdVariant& out_result); // [tested]

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  /// \brief Writes an wdColor to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreColor(
    wdOpenDdlWriter& ref_writer, const wdColor& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdColorGammaUB to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreColorGamma(
    wdOpenDdlWriter& ref_writer, const wdColorGammaUB& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdTime to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreTime(wdOpenDdlWriter& ref_writer, const wdTime& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec2 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec2(wdOpenDdlWriter& ref_writer, const wdVec2& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec3 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec3(wdOpenDdlWriter& ref_writer, const wdVec3& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec4 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec4(wdOpenDdlWriter& ref_writer, const wdVec4& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec2 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec2I(
    wdOpenDdlWriter& ref_writer, const wdVec2I32& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec3 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec3I(
    wdOpenDdlWriter& ref_writer, const wdVec3I32& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec4 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec4I(
    wdOpenDdlWriter& ref_writer, const wdVec4I32& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec2 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec2U(
    wdOpenDdlWriter& ref_writer, const wdVec2U32& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec3 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec3U(
    wdOpenDdlWriter& ref_writer, const wdVec3U32& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVec4 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVec4U(
    wdOpenDdlWriter& ref_writer, const wdVec4U32& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdMat3 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreMat3(wdOpenDdlWriter& ref_writer, const wdMat3& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdMat4 to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreMat4(wdOpenDdlWriter& ref_writer, const wdMat4& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdTransform to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreTransform(
    wdOpenDdlWriter& ref_writer, const wdTransform& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdQuat to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreQuat(wdOpenDdlWriter& ref_writer, const wdQuat& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdUuid to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreUuid(wdOpenDdlWriter& ref_writer, const wdUuid& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdAngle to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreAngle(
    wdOpenDdlWriter& ref_writer, const wdAngle& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes an wdVariant to DDL such that the type can be reconstructed.
  WD_FOUNDATION_DLL void StoreVariant(
    wdOpenDdlWriter& ref_writer, const wdVariant& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single string and an optional name.
  WD_FOUNDATION_DLL void StoreString(
    wdOpenDdlWriter& ref_writer, const wdStringView& value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreBool(wdOpenDdlWriter& ref_writer, bool value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreFloat(wdOpenDdlWriter& ref_writer, float value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreDouble(wdOpenDdlWriter& ref_writer, double value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreInt8(wdOpenDdlWriter& ref_writer, wdInt8 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreInt16(wdOpenDdlWriter& ref_writer, wdInt16 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreInt32(wdOpenDdlWriter& ref_writer, wdInt32 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreInt64(wdOpenDdlWriter& ref_writer, wdInt64 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreUInt8(wdOpenDdlWriter& ref_writer, wdUInt8 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreUInt16(wdOpenDdlWriter& ref_writer, wdUInt16 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreUInt32(wdOpenDdlWriter& ref_writer, wdUInt32 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]

  /// \brief Writes a primitives list with a single value and an optional name.
  WD_FOUNDATION_DLL void StoreUInt64(wdOpenDdlWriter& ref_writer, wdUInt64 value, const char* szName = nullptr, bool bGlobalName = false); // [tested]
} // namespace wdOpenDdlUtils
