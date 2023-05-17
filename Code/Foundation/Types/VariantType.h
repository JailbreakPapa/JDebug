#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Types/Types.h>

class wdReflectedClass;
class wdVariant;
struct wdTime;
class wdUuid;
struct wdStringView;
struct wdTypedObject;
struct wdTypedPointer;

using wdDataBuffer = wdDynamicArray<wdUInt8>;
using wdVariantArray = wdDynamicArray<wdVariant>;
using wdVariantDictionary = wdHashTable<wdString, wdVariant>;

/// \brief This enum describes the type of data that is currently stored inside the variant.
struct wdVariantType
{
  using StorageType = wdUInt8;
  /// \brief This enum describes the type of data that is currently stored inside the variant.
  /// Note that changes to this enum require an increase of the reflection version and either
  /// patches to the serializer or a re-export of binary data that contains wdVariants.
  enum Enum : wdUInt8
  {
    Invalid = 0, ///< The variant stores no (valid) data at the moment.

    /// *** Types that are flagged as 'StandardTypes' (see DetermineTypeFlags) ***
    FirstStandardType = 1,
    Bool,       ///< The variant stores a bool.
    Int8,       ///< The variant stores an wdInt8.
    UInt8,      ///< The variant stores an wdUInt8.
    Int16,      ///< The variant stores an wdInt16.
    UInt16,     ///< The variant stores an wdUInt16.
    Int32,      ///< The variant stores an wdInt32.
    UInt32,     ///< The variant stores an wdUInt32.
    Int64,      ///< The variant stores an wdInt64.
    UInt64,     ///< The variant stores an wdUInt64.
    Float,      ///< The variant stores a float.
    Double,     ///< The variant stores a double.
    Color,      ///< The variant stores an wdColor.
    Vector2,    ///< The variant stores an wdVec2.
    Vector3,    ///< The variant stores an wdVec3.
    Vector4,    ///< The variant stores an wdVec4.
    Vector2I,   ///< The variant stores an wdVec2I32.
    Vector3I,   ///< The variant stores an wdVec3I32.
    Vector4I,   ///< The variant stores an wdVec4I32.
    Vector2U,   ///< The variant stores an wdVec2U32.
    Vector3U,   ///< The variant stores an wdVec3U32.
    Vector4U,   ///< The variant stores an wdVec4U32.
    Quaternion, ///< The variant stores an wdQuat.
    Matrix3,    ///< The variant stores an wdMat3. A heap allocation is required to store this data type.
    Matrix4,    ///< The variant stores an wdMat4. A heap allocation is required to store this data type.
    Transform,  ///< The variant stores an wdTransform. A heap allocation is required to store this data type.
    String,     ///< The variant stores a string. A heap allocation is required to store this data type.
    StringView, ///< The variant stores an wdStringView.
    DataBuffer, ///< The variant stores an wdDataBuffer, a typedef to DynamicArray<wdUInt8>. A heap allocation is required to store this data type.
    Time,       ///< The variant stores an wdTime value.
    Uuid,       ///< The variant stores an wdUuid value.
    Angle,      ///< The variant stores an wdAngle value.
    ColorGamma, ///< The variant stores an wdColorGammaUB value.
    LastStandardType,
    /// *** Types that are flagged as 'StandardTypes' (see DetermineTypeFlags) ***

    FirstExtendedType = 64,
    VariantArray,      ///< The variant stores an array of wdVariant's. A heap allocation is required to store this data type.
    VariantDictionary, ///< The variant stores a dictionary (hashmap) of wdVariant's. A heap allocation is required to store this type.
    TypedPointer,      ///< The variant stores an wdTypedPointer value. Reflected type and data queries will match the pointed to object.
    TypedObject,       ///< The variant stores an wdTypedObject value. Reflected type and data queries will match the object. A heap allocation is required to store this type if it is larger than 16 bytes or not POD.
    LastExtendedType,  ///< Number of values for wdVariant::Type.

    MAX_ENUM_VALUE = LastExtendedType,
    Default = Invalid ///< Default value used by wdEnum.
  };
};

WD_DEFINE_AS_POD_TYPE(wdVariantType::Enum);

struct wdVariantClass
{
  enum Enum
  {
    Invalid,
    DirectCast,     ///< A standard type
    PointerCast,    ///< Any cast to T*
    TypedObject,    ///< wdTypedObject cast. Needed because at no point does and wdVariant ever store a wdTypedObject so it can't be returned as a const reference.
    CustomTypeCast, ///< Custom object types
  };
};

/// \brief A helper struct to convert the C++ type, which is passed as the template argument, into one of the wdVariant::Type enum values.
template <typename T>
struct wdVariantTypeDeduction
{
  enum
  {
    value = wdVariantType::Invalid,
    forceSharing = false,
    hasReflectedMembers = false,
    classification = wdVariantClass::Invalid
  };

  using StorageType = T;
};

/// \brief Declares a custom variant type, allowing it to be stored by value inside an wdVariant.
///
/// Needs to be called from the same header that defines the type.
/// \sa WD_DEFINE_CUSTOM_VARIANT_TYPE
#define WD_DECLARE_CUSTOM_VARIANT_TYPE(TYPE)          \
  template <>                                         \
  struct wdVariantTypeDeduction<TYPE>                 \
  {                                                   \
    enum                                              \
    {                                                 \
      value = wdVariantType::TypedObject,             \
      forceSharing = false,                           \
      hasReflectedMembers = true,                     \
      classification = wdVariantClass::CustomTypeCast \
    };                                                \
    using StorageType = TYPE;                         \
  };

#include <Foundation/Types/Implementation/VariantTypeDeduction_inl.h>
