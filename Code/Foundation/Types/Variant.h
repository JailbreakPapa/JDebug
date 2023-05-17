#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Math/Declarations.h>
#include <Foundation/Threading/AtomicInteger.h>
#include <Foundation/Types/TypedPointer.h>
#include <Foundation/Types/Types.h>
#include <Foundation/Types/VariantType.h>

#include <Foundation/Reflection/Implementation/DynamicRTTI.h>
#include <Foundation/Utilities/ConversionUtils.h>

class wdRTTI;

/// \brief Defines a reference to an immutable object owned by an wdVariant.
///
/// Used to store custom types inside an wdVariant. As lifetime is governed by the wdVariant, it is generally not safe to store an wdTypedObject.
/// This class is needed to be able to differentiate between wdVariantType::TypedPointer and wdVariantType::TypedObject e.g. in wdVariant::DispatchTo.
/// \sa wdVariant, WD_DECLARE_CUSTOM_VARIANT_TYPE
struct wdTypedObject
{
  WD_DECLARE_POD_TYPE();
  const void* m_pObject = nullptr;
  const wdRTTI* m_pType = nullptr;

  bool operator==(const wdTypedObject& rhs) const
  {
    return m_pObject == rhs.m_pObject;
  }
  bool operator!=(const wdTypedObject& rhs) const
  {
    return m_pObject != rhs.m_pObject;
  }
};

/// \brief wdVariant is a class that can store different types of variables, which is useful in situations where it is not clear up front,
/// which type of data will be passed around.
///
/// The variant supports a fixed list of types that it can store (\see wdVariant::Type). All types of 16 bytes or less in size can be stored
/// without requiring a heap allocation. For larger types memory is allocated on the heap. In general variants should be used for code that
/// needs to be flexible. Although wdVariant is implemented very efficiently, it should be avoided to use wdVariant in code that needs to be
/// fast.
class WD_FOUNDATION_DLL wdVariant
{
public:
  using Type = wdVariantType;
  template <typename T>
  using TypeDeduction = wdVariantTypeDeduction<T>;

  /// \brief helper struct to wrap a string pointer
  struct StringWrapper
  {
    WD_ALWAYS_INLINE StringWrapper(const char* szStr)
      : m_str(szStr)
    {
    }
    const char* m_str;
  };

  /// \brief Initializes the variant to be 'Invalid'
  wdVariant(); // [tested]

  /// \brief Copies the data from the other variant.
  ///
  /// \note If the data of the variant needed to be allocated on the heap, it will be shared among variants.
  /// Thus, once you have stored such a type inside a variant, you can copy it to other variants, without introducing
  /// additional memory allocations.
  wdVariant(const wdVariant& other); // [tested]

  /// \brief Moves the data from the other variant.
  wdVariant(wdVariant&& other) noexcept; // [tested]

  wdVariant(const bool& value);
  wdVariant(const wdInt8& value);
  wdVariant(const wdUInt8& value);
  wdVariant(const wdInt16& value);
  wdVariant(const wdUInt16& value);
  wdVariant(const wdInt32& value);
  wdVariant(const wdUInt32& value);
  wdVariant(const wdInt64& value);
  wdVariant(const wdUInt64& value);
  wdVariant(const float& value);
  wdVariant(const double& value);
  wdVariant(const wdColor& value);
  wdVariant(const wdVec2& value);
  wdVariant(const wdVec3& value);
  wdVariant(const wdVec4& value);
  wdVariant(const wdVec2I32& value);
  wdVariant(const wdVec3I32& value);
  wdVariant(const wdVec4I32& value);
  wdVariant(const wdVec2U32& value);
  wdVariant(const wdVec3U32& value);
  wdVariant(const wdVec4U32& value);
  wdVariant(const wdQuat& value);
  wdVariant(const wdMat3& value);
  wdVariant(const wdMat4& value);
  wdVariant(const wdTransform& value);
  wdVariant(const char* value);
  wdVariant(const wdString& value);
  wdVariant(const wdUntrackedString& value);
  wdVariant(const wdStringView& value);
  wdVariant(const wdDataBuffer& value);
  wdVariant(const wdTime& value);
  wdVariant(const wdUuid& value);
  wdVariant(const wdAngle& value);
  wdVariant(const wdColorGammaUB& value);

  wdVariant(const wdVariantArray& value);
  wdVariant(const wdVariantDictionary& value);

  wdVariant(const wdTypedPointer& value);
  wdVariant(const wdTypedObject& value);

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int> = 0>
  wdVariant(const T& value);

  template <typename T>
  wdVariant(const T* value);

  /// \brief Initializes to a TypedPointer of the given object and type.
  wdVariant(void* value, const wdRTTI* pType);

  /// \brief Initializes to a TypedObject by cloning the given object and type.
  void CopyTypedObject(const void* value, const wdRTTI* pType); // [tested]

  /// \brief Initializes to a TypedObject by taking ownership of the given object and type.
  void MoveTypedObject(void* value, const wdRTTI* pType); // [tested]

  /// \brief If necessary, this will deallocate any heap memory that is not in use any more.
  ~wdVariant();

  /// \brief Copies the data from the \a other variant into this one.
  void operator=(const wdVariant& other); // [tested]

  /// \brief Moves the data from the \a other variant into this one.
  void operator=(wdVariant&& other) noexcept; // [tested]

  /// \brief Deduces the type of \a T and stores \a value.
  ///
  /// If the type to be stored in the variant is not supported, a compile time error will occur.
  template <typename T>
  void operator=(const T& value); // [tested]

  /// \brief Will compare the value of this variant to that of \a other.
  ///
  /// If both variants store 'numbers' (float, double, int types) the comparison will work, even if the types are not identical.
  ///
  /// \note If the two types are not numbers and not equal, an assert will occur. So be careful to only compare variants
  /// that can either both be converted to double (\see CanConvertTo()) or whose types are equal.
  bool operator==(const wdVariant& other) const; // [tested]

  /// \brief Same as operator== (with a twist!)
  bool operator!=(const wdVariant& other) const; // [tested]

  /// \brief See non-templated operator==
  template <typename T>
  bool operator==(const T& other) const; // [tested]

  /// \brief See non-templated operator!=
  template <typename T>
  bool operator!=(const T& other) const; // [tested]

  /// \brief Returns whether this variant stores any other type than 'Invalid'.
  bool IsValid() const; // [tested]

  /// \brief Returns whether the stored type is numerical type either integer or floating point.
  ///
  /// Bool counts as number.
  bool IsNumber() const; // [tested]

  /// \brief Returns whether the stored type is floating point (float or double).
  bool IsFloatingPoint() const; // [tested]

  /// \brief Returns whether the stored type is a string (wdString or wdStringView).
  bool IsString() const; // [tested]

  /// \brief Returns whether the stored type is exactly the given type.
  ///
  /// \note This explicitly also differentiates between the different integer types.
  /// So when the variant stores an Int32, IsA<Int64>() will return false, even though the types could be converted.
  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int> = 0>
  bool IsA() const; // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int> = 0>
  bool IsA() const; // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::TypedObject, int> = 0>
  bool IsA() const; // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int> = 0>
  bool IsA() const; // [tested]

  /// \brief Returns the exact wdVariant::Type value.
  Type::Enum GetType() const; // [tested]

  /// \brief Returns the variants value as the provided type.
  ///
  /// \note This function does not do ANY type of conversion from the stored type to the given type. Not even integer conversions!
  /// If the types don't match, this function will assert!
  /// So be careful to use this function only when you know exactly that the stored type matches the expected type.
  ///
  /// Prefer to use ConvertTo() when you can instead.
  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int> = 0>
  const T& Get() const; // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int> = 0>
  T Get() const; // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::TypedObject, int> = 0>
  const T Get() const; // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int> = 0>
  const T& Get() const; // [tested]

  /// \brief Returns an writable wdTypedPointer to the internal data.
  /// If the data is currently shared a clone will be made to ensure we hold the only reference.
  wdTypedPointer GetWriteAccess(); // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int> = 0>
  T& GetWritable(); // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int> = 0>
  T GetWritable(); // [tested]

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int> = 0>
  T& GetWritable(); // [tested]


  /// \brief Returns a const void* to the internal data.
  /// For TypedPointer and TypedObject this will return a pointer to the target object.
  const void* GetData() const; // [tested]

  /// \brief Returns the wdRTTI type of the held value.
  /// For TypedPointer and TypedObject this will return the type of the target object.
  const wdRTTI* GetReflectedType() const; // [tested]

  /// \brief Returns the sub value at iIndex. This could be an element in an array or a member property inside a reflected type.
  ///
  /// Out of bounds access is handled gracefully and will return an invalid variant.
  const wdVariant operator[](wdUInt32 uiIndex) const; // [tested]

  /// \brief Returns the sub value with szKey. This could be a value in a dictionary or a member property inside a reflected type.
  ///
  /// This function will return an invalid variant if no corresponding sub value is found.
  const wdVariant operator[](StringWrapper key) const; // [tested]

  /// \brief Returns whether the stored type can generally be converted to the desired type.
  ///
  /// This function will return true for all number conversions, as float / double / int / etc. can generally be converted into each
  /// other. It will also return true for all conversion from string to number types, and from all 'simple' types (not array or dictionary)
  /// to string.
  ///
  /// \note This function only returns whether a conversion between the stored TYPE and the desired TYPE is generally possible. It does NOT
  /// return whether the stored VALUE is indeed convertible to the desired type. For example, a string is generally convertible to float, if
  /// it stores a string representation of a float value. If, however, it stores anything else, the conversion can still fail.
  ///
  /// The only way to figure out whether the stored data can be converted to some type, is to actually convert it, using ConvertTo(), and
  /// then to check the conversion status.
  template <typename T>
  bool CanConvertTo() const; // [tested]

  /// \brief Same as the templated CanConvertTo function.
  bool CanConvertTo(Type::Enum type) const; // [tested]

  /// \brief Tries to convert the stored value to the given type. The optional status parameter can be used to check whether the conversion
  /// succeeded.
  ///
  /// When CanConvertTo() returns false, ConvertTo() will also always fail. However, when CanConvertTo() returns true, this is no guarantee
  /// that ConvertTo() will succeed. Conversion between numbers and to strings will generally succeed. However, converting from a string to
  /// another type can fail or succeed, depending on the exact string value.
  template <typename T>
  T ConvertTo(wdResult* out_pConversionStatus = nullptr) const; // [tested]

  /// \brief Same as the templated function.
  wdVariant ConvertTo(Type::Enum type, wdResult* out_pConversionStatus = nullptr) const; // [tested]

  /// \brief This will call the overloaded operator() (function call operator) of the provided functor.
  ///
  /// This allows to implement a functor that overloads operator() for different types and then call the proper version of that operator,
  /// depending on the provided runtime type. Note that the proper overload of operator() is selected by providing a dummy type, but it will
  /// contain no useful value. Instead, store the other necessary data inside the functor object, before calling this function. For example,
  /// store a pointer to a variant inside the functor object and then call DispatchTo to execute the function that will handle the given
  /// type of the variant.
  template <typename Functor, class... Args>
  static auto DispatchTo(Functor& ref_functor, Type::Enum type, Args&&... args); // [tested]

  /// \brief Computes the hash value of the stored data. Returns uiSeed (unchanged) for an invalid Variant.
  wdUInt64 ComputeHash(wdUInt64 uiSeed = 0) const;

private:
  friend class wdVariantHelper;
  friend struct CompareFunc;
  friend struct GetTypeFromVariantFunc;

  struct SharedData
  {
    void* m_Ptr;
    const wdRTTI* m_pType;
    wdAtomicInteger32 m_uiRef = 1;
    WD_ALWAYS_INLINE SharedData(void* pPtr, const wdRTTI* pType)
      : m_Ptr(pPtr)
      , m_pType(pType)
    {
    }
    virtual ~SharedData() = default;
    virtual SharedData* Clone() const = 0;
  };

  template <typename T>
  class TypedSharedData : public SharedData
  {
  private:
    T m_t;

  public:
    WD_ALWAYS_INLINE TypedSharedData(const T& value, const wdRTTI* pType = nullptr)
      : SharedData(&m_t, pType)
      , m_t(value)
    {
    }

    virtual SharedData* Clone() const override
    {
      return WD_DEFAULT_NEW(TypedSharedData<T>, m_t, m_pType);
    }
  };

  class RTTISharedData : public SharedData
  {
  public:
    RTTISharedData(void* pData, const wdRTTI* pType);

    ~RTTISharedData();

    virtual SharedData* Clone() const override;
  };

  struct InlinedStruct
  {
    constexpr static int DataSize = 4 * sizeof(float) - sizeof(void*);
    wdUInt8 m_Data[DataSize];
    const wdRTTI* m_pType;
  };

  union Data
  {
    float f[4];
    SharedData* shared;
    InlinedStruct inlined;
  } m_Data;

  wdUInt32 m_uiType : 31;
  wdUInt32 m_bIsShared : 1; // NOLINT(wd*)

  template <typename T>
  void InitInplace(const T& value);

  template <typename T>
  void InitShared(const T& value);

  template <typename T>
  void InitTypedObject(const T& value, wdTraitInt<0>);
  template <typename T>
  void InitTypedObject(const T& value, wdTraitInt<1>);

  void InitTypedPointer(void* value, const wdRTTI* pType);

  void Release();
  void CopyFrom(const wdVariant& other);
  void MoveFrom(wdVariant&& other);

  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::DirectCast, int> = 0>
  const T& Cast() const;
  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::PointerCast, int> = 0>
  T Cast() const;
  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::TypedObject, int> = 0>
  const T Cast() const;
  template <typename T, typename std::enable_if_t<wdVariantTypeDeduction<T>::classification == wdVariantClass::CustomTypeCast, int> = 0>
  const T& Cast() const;

  static bool IsNumberStatic(wdUInt32 type);
  static bool IsFloatingPointStatic(wdUInt32 type);
  static bool IsStringStatic(wdUInt32 type);
  static bool IsVector2Static(wdUInt32 type);
  static bool IsVector3Static(wdUInt32 type);
  static bool IsVector4Static(wdUInt32 type);

  // Needed to prevent including wdRTTI in wdVariant.h
  static bool IsDerivedFrom(const wdRTTI* pType1, const wdRTTI* pType2);
  static const char* GetTypeName(const wdRTTI* pType);

  template <typename T>
  T ConvertNumber() const;
};

/// \brief An overload of wdDynamicCast for dynamic casting a variant to a pointer type.
///
/// If the wdVariant stores an wdTypedPointer pointer, this pointer will be dynamically cast to T*.
/// If the wdVariant stores any other type (or nothing), nullptr is returned.
template <typename T>
WD_ALWAYS_INLINE T wdDynamicCast(const wdVariant& variant)
{
  if (variant.IsA<T>())
  {
    return variant.Get<T>();
  }

  return nullptr;
}

#include <Foundation/Types/Implementation/VariantHelper_inl.h>

#include <Foundation/Types/Implementation/Variant_inl.h>
