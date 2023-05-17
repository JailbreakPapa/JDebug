

// for some reason MSVC does not accept the template keyword here
#if WD_ENABLED(WD_COMPILER_MSVC_PURE)
#  define CALL_FUNCTOR(functor, type) return functor.operator()<type>(std::forward<Args>(args)...)
#else
#  define CALL_FUNCTOR(functor, type) return functor.template operator()<type>(std::forward<Args>(args)...)
#endif

template <typename Functor, class... Args>
auto wdVariant::DispatchTo(Functor& ref_functor, Type::Enum type, Args&&... args)
{
  switch (type)
  {
    case Type::Bool:
      CALL_FUNCTOR(ref_functor, bool);
      break;

    case Type::Int8:
      CALL_FUNCTOR(ref_functor, wdInt8);
      break;

    case Type::UInt8:
      CALL_FUNCTOR(ref_functor, wdUInt8);
      break;

    case Type::Int16:
      CALL_FUNCTOR(ref_functor, wdInt16);
      break;

    case Type::UInt16:
      CALL_FUNCTOR(ref_functor, wdUInt16);
      break;

    case Type::Int32:
      CALL_FUNCTOR(ref_functor, wdInt32);
      break;

    case Type::UInt32:
      CALL_FUNCTOR(ref_functor, wdUInt32);
      break;

    case Type::Int64:
      CALL_FUNCTOR(ref_functor, wdInt64);
      break;

    case Type::UInt64:
      CALL_FUNCTOR(ref_functor, wdUInt64);
      break;

    case Type::Float:
      CALL_FUNCTOR(ref_functor, float);
      break;

    case Type::Double:
      CALL_FUNCTOR(ref_functor, double);
      break;

    case Type::Color:
      CALL_FUNCTOR(ref_functor, wdColor);
      break;

    case Type::ColorGamma:
      CALL_FUNCTOR(ref_functor, wdColorGammaUB);
      break;

    case Type::Vector2:
      CALL_FUNCTOR(ref_functor, wdVec2);
      break;

    case Type::Vector3:
      CALL_FUNCTOR(ref_functor, wdVec3);
      break;

    case Type::Vector4:
      CALL_FUNCTOR(ref_functor, wdVec4);
      break;

    case Type::Vector2I:
      CALL_FUNCTOR(ref_functor, wdVec2I32);
      break;

    case Type::Vector3I:
      CALL_FUNCTOR(ref_functor, wdVec3I32);
      break;

    case Type::Vector4I:
      CALL_FUNCTOR(ref_functor, wdVec4I32);
      break;

    case Type::Vector2U:
      CALL_FUNCTOR(ref_functor, wdVec2U32);
      break;

    case Type::Vector3U:
      CALL_FUNCTOR(ref_functor, wdVec3U32);
      break;

    case Type::Vector4U:
      CALL_FUNCTOR(ref_functor, wdVec4U32);
      break;

    case Type::Quaternion:
      CALL_FUNCTOR(ref_functor, wdQuat);
      break;

    case Type::Matrix3:
      CALL_FUNCTOR(ref_functor, wdMat3);
      break;

    case Type::Matrix4:
      CALL_FUNCTOR(ref_functor, wdMat4);
      break;

    case Type::Transform:
      CALL_FUNCTOR(ref_functor, wdTransform);
      break;

    case Type::String:
      CALL_FUNCTOR(ref_functor, wdString);
      break;

    case Type::StringView:
      CALL_FUNCTOR(ref_functor, wdStringView);
      break;

    case Type::DataBuffer:
      CALL_FUNCTOR(ref_functor, wdDataBuffer);
      break;

    case Type::Time:
      CALL_FUNCTOR(ref_functor, wdTime);
      break;

    case Type::Uuid:
      CALL_FUNCTOR(ref_functor, wdUuid);
      break;

    case Type::Angle:
      CALL_FUNCTOR(ref_functor, wdAngle);
      break;

    case Type::VariantArray:
      CALL_FUNCTOR(ref_functor, wdVariantArray);
      break;

    case Type::VariantDictionary:
      CALL_FUNCTOR(ref_functor, wdVariantDictionary);
      break;

    case Type::TypedObject:
      CALL_FUNCTOR(ref_functor, wdTypedObject);
      break;

    default:
      WD_REPORT_FAILURE("Could not dispatch type '{0}'", type);
      // Intended fall through to disable warning.
    case Type::TypedPointer:
      CALL_FUNCTOR(ref_functor, wdTypedPointer);
      break;
  }
}

#undef CALL_FUNCTOR

class wdVariantHelper
{
  friend class wdVariant;
  friend struct ConvertFunc;

  template <typename T>
  WD_ALWAYS_INLINE static bool CompareFloat(const wdVariant& v, const T& other, wdTraitInt<1>)
  {
    return v.ConvertNumber<double>() == static_cast<double>(other);
  }

  template <typename T>
  WD_ALWAYS_INLINE static bool CompareFloat(const wdVariant& v, const T& other, wdTraitInt<0>)
  {
    return false;
  }

  template <typename T>
  WD_ALWAYS_INLINE static bool CompareNumber(const wdVariant& v, const T& other, wdTraitInt<1>)
  {
    return v.ConvertNumber<wdInt64>() == static_cast<wdInt64>(other);
  }

  template <typename T>
  WD_ALWAYS_INLINE static bool CompareNumber(const wdVariant& v, const T& other, wdTraitInt<0>)
  {
    return false;
  }

  static void To(const wdVariant& value, bool& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= wdVariant::Type::Double)
      result = value.ConvertNumber<wdInt32>() != 0;
    else if (value.GetType() == wdVariant::Type::String)
    {
      if (wdConversionUtils::StringToBool(value.Cast<wdString>().GetData(), result) == WD_FAILURE)
      {
        result = false;
        bSuccessful = false;
      }
    }
    else
      WD_REPORT_FAILURE("Conversion to bool failed");
  }

  static void To(const wdVariant& value, wdInt8& result, bool& bSuccessful)
  {
    wdInt32 tempResult;
    To(value, tempResult, bSuccessful);
    result = (wdInt8)tempResult;
  }

  static void To(const wdVariant& value, wdUInt8& result, bool& bSuccessful)
  {
    wdUInt32 tempResult;
    To(value, tempResult, bSuccessful);
    result = (wdUInt8)tempResult;
  }

  static void To(const wdVariant& value, wdInt16& result, bool& bSuccessful)
  {
    wdInt32 tempResult;
    To(value, tempResult, bSuccessful);
    result = (wdInt16)tempResult;
  }

  static void To(const wdVariant& value, wdUInt16& result, bool& bSuccessful)
  {
    wdUInt32 tempResult;
    To(value, tempResult, bSuccessful);
    result = (wdUInt16)tempResult;
  }

  static void To(const wdVariant& value, wdInt32& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= wdVariant::Type::Double)
      result = value.ConvertNumber<wdInt32>();
    else if (value.GetType() == wdVariant::Type::String)
    {
      if (wdConversionUtils::StringToInt(value.Cast<wdString>().GetData(), result) == WD_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
    }
    else
      WD_REPORT_FAILURE("Conversion to int failed");
  }

  static void To(const wdVariant& value, wdUInt32& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= wdVariant::Type::Double)
      result = value.ConvertNumber<wdUInt32>();
    else if (value.GetType() == wdVariant::Type::String)
    {
      wdInt64 tmp = result;
      if (wdConversionUtils::StringToInt64(value.Cast<wdString>().GetData(), tmp) == WD_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
      else
        result = (wdUInt32)tmp;
    }
    else
      WD_REPORT_FAILURE("Conversion to uint failed");
  }

  static void To(const wdVariant& value, wdInt64& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= wdVariant::Type::Double)
      result = value.ConvertNumber<wdInt64>();
    else if (value.GetType() == wdVariant::Type::String)
    {
      if (wdConversionUtils::StringToInt64(value.Cast<wdString>().GetData(), result) == WD_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
    }
    else
      WD_REPORT_FAILURE("Conversion to int64 failed");
  }

  static void To(const wdVariant& value, wdUInt64& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= wdVariant::Type::Double)
      result = value.ConvertNumber<wdUInt64>();
    else if (value.GetType() == wdVariant::Type::String)
    {
      wdInt64 tmp = result;
      if (wdConversionUtils::StringToInt64(value.Cast<wdString>().GetData(), tmp) == WD_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
      else
        result = (wdUInt64)tmp;
    }
    else
      WD_REPORT_FAILURE("Conversion to uint64 failed");
  }

  static void To(const wdVariant& value, float& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= wdVariant::Type::Double)
      result = value.ConvertNumber<float>();
    else if (value.GetType() == wdVariant::Type::String)
    {
      double tmp = result;
      if (wdConversionUtils::StringToFloat(value.Cast<wdString>().GetData(), tmp) == WD_FAILURE)
      {
        result = 0.0f;
        bSuccessful = false;
      }
      else
        result = (float)tmp;
    }
    else
      WD_REPORT_FAILURE("Conversion to float failed");
  }

  static void To(const wdVariant& value, double& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= wdVariant::Type::Double)
      result = value.ConvertNumber<double>();
    else if (value.GetType() == wdVariant::Type::String)
    {
      if (wdConversionUtils::StringToFloat(value.Cast<wdString>().GetData(), result) == WD_FAILURE)
      {
        result = 0.0;
        bSuccessful = false;
      }
    }
    else
      WD_REPORT_FAILURE("Conversion to double failed");
  }

  static void To(const wdVariant& value, wdString& result, bool& bSuccessful)
  {
    bSuccessful = true;

    ToStringFunc toStringFunc;
    toStringFunc.m_pThis = &value;
    toStringFunc.m_pResult = &result;

    wdVariant::DispatchTo(toStringFunc, value.GetType());
    bSuccessful = true;
  }

  static void To(const wdVariant& value, wdTypedPointer& result, bool& bSuccessful)
  {
    bSuccessful = true;
    WD_ASSERT_DEBUG(value.GetType() == wdVariant::Type::TypedPointer, "Only ptr can be converted to void*!");
    result = value.Get<wdTypedPointer>();
  }

  static void To(const wdVariant& value, wdColor& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() == wdVariant::Type::ColorGamma)
      result = value.Get<wdColorGammaUB>();
    else
      WD_REPORT_FAILURE("Conversion to wdColor failed");
  }

  static void To(const wdVariant& value, wdColorGammaUB& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() == wdVariant::Type::Color)
      result = value.Get<wdColor>();
    else
      WD_REPORT_FAILURE("Conversion to wdColorGammaUB failed");
  }

  template <typename T, typename V1, typename V2>
  static void ToVec2X(const wdVariant& value, T& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.IsA<V1>())
    {
      const V1& v = value.Get<V1>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y));
    }
    else if (value.IsA<V2>())
    {
      const V2& v = value.Get<V2>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y));
    }
    else
    {
      WD_REPORT_FAILURE("Conversion to wdVec2X failed");
      bSuccessful = false;
    }
  }

  static void To(const wdVariant& value, wdVec2& result, bool& bSuccessful) { ToVec2X<wdVec2, wdVec2I32, wdVec2U32>(value, result, bSuccessful); }

  static void To(const wdVariant& value, wdVec2I32& result, bool& bSuccessful) { ToVec2X<wdVec2I32, wdVec2, wdVec2U32>(value, result, bSuccessful); }

  static void To(const wdVariant& value, wdVec2U32& result, bool& bSuccessful) { ToVec2X<wdVec2U32, wdVec2I32, wdVec2>(value, result, bSuccessful); }

  template <typename T, typename V1, typename V2>
  static void ToVec3X(const wdVariant& value, T& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.IsA<V1>())
    {
      const V1& v = value.Get<V1>();
      result =
        T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y), static_cast<typename T::ComponentType>(v.z));
    }
    else if (value.IsA<V2>())
    {
      const V2& v = value.Get<V2>();
      result =
        T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y), static_cast<typename T::ComponentType>(v.z));
    }
    else
    {
      WD_REPORT_FAILURE("Conversion to wdVec3X failed");
      bSuccessful = false;
    }
  }

  static void To(const wdVariant& value, wdVec3& result, bool& bSuccessful) { ToVec3X<wdVec3, wdVec3I32, wdVec3U32>(value, result, bSuccessful); }

  static void To(const wdVariant& value, wdVec3I32& result, bool& bSuccessful) { ToVec3X<wdVec3I32, wdVec3, wdVec3U32>(value, result, bSuccessful); }

  static void To(const wdVariant& value, wdVec3U32& result, bool& bSuccessful) { ToVec3X<wdVec3U32, wdVec3I32, wdVec3>(value, result, bSuccessful); }

  template <typename T, typename V1, typename V2>
  static void ToVec4X(const wdVariant& value, T& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.IsA<V1>())
    {
      const V1& v = value.Get<V1>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y),
        static_cast<typename T::ComponentType>(v.z), static_cast<typename T::ComponentType>(v.w));
    }
    else if (value.IsA<V2>())
    {
      const V2& v = value.Get<V2>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y),
        static_cast<typename T::ComponentType>(v.z), static_cast<typename T::ComponentType>(v.w));
    }
    else
    {
      WD_REPORT_FAILURE("Conversion to wdVec4X failed");
      bSuccessful = false;
    }
  }

  static void To(const wdVariant& value, wdVec4& result, bool& bSuccessful) { ToVec4X<wdVec4, wdVec4I32, wdVec4U32>(value, result, bSuccessful); }

  static void To(const wdVariant& value, wdVec4I32& result, bool& bSuccessful) { ToVec4X<wdVec4I32, wdVec4, wdVec4U32>(value, result, bSuccessful); }

  static void To(const wdVariant& value, wdVec4U32& result, bool& bSuccessful) { ToVec4X<wdVec4U32, wdVec4I32, wdVec4>(value, result, bSuccessful); }

  template <typename T>
  static void To(const wdVariant& value, T& result, bool& bSuccessful)
  {
    WD_REPORT_FAILURE("Conversion function not implemented for target type '{0}'", wdVariant::TypeDeduction<T>::value);
    bSuccessful = false;
  }

  struct ToStringFunc
  {
    template <typename T>
    WD_ALWAYS_INLINE void operator()()
    {
      wdStringBuilder tmp;
      *m_pResult = wdConversionUtils::ToString(m_pThis->Cast<T>(), tmp);
    }

    const wdVariant* m_pThis;
    wdString* m_pResult;
  };
};
