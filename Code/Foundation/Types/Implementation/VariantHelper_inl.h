

// for some reason MSVC does not accept the template keyword here
#if NS_ENABLED(NS_COMPILER_MSVC_PURE)
#  define CALL_FUNCTOR(functor, type) return functor.operator()<type>(std::forward<Args>(args)...)
#else
#  define CALL_FUNCTOR(functor, type) return functor.template operator()<type>(std::forward<Args>(args)...)
#endif

template <typename Functor, class... Args>
auto nsVariant::DispatchTo(Functor& ref_functor, Type::Enum type, Args&&... args)
{
  switch (type)
  {
    case Type::Bool:
      CALL_FUNCTOR(ref_functor, bool);
      break;

    case Type::Int8:
      CALL_FUNCTOR(ref_functor, nsInt8);
      break;

    case Type::UInt8:
      CALL_FUNCTOR(ref_functor, nsUInt8);
      break;

    case Type::Int16:
      CALL_FUNCTOR(ref_functor, nsInt16);
      break;

    case Type::UInt16:
      CALL_FUNCTOR(ref_functor, nsUInt16);
      break;

    case Type::Int32:
      CALL_FUNCTOR(ref_functor, nsInt32);
      break;

    case Type::UInt32:
      CALL_FUNCTOR(ref_functor, nsUInt32);
      break;

    case Type::Int64:
      CALL_FUNCTOR(ref_functor, nsInt64);
      break;

    case Type::UInt64:
      CALL_FUNCTOR(ref_functor, nsUInt64);
      break;

    case Type::Float:
      CALL_FUNCTOR(ref_functor, float);
      break;

    case Type::Double:
      CALL_FUNCTOR(ref_functor, double);
      break;

    case Type::Color:
      CALL_FUNCTOR(ref_functor, nsColor);
      break;

    case Type::ColorGamma:
      CALL_FUNCTOR(ref_functor, nsColorGammaUB);
      break;

    case Type::Vector2:
      CALL_FUNCTOR(ref_functor, nsVec2);
      break;

    case Type::Vector3:
      CALL_FUNCTOR(ref_functor, nsVec3);
      break;

    case Type::Vector4:
      CALL_FUNCTOR(ref_functor, nsVec4);
      break;

    case Type::Vector2I:
      CALL_FUNCTOR(ref_functor, nsVec2I32);
      break;

    case Type::Vector3I:
      CALL_FUNCTOR(ref_functor, nsVec3I32);
      break;

    case Type::Vector4I:
      CALL_FUNCTOR(ref_functor, nsVec4I32);
      break;

    case Type::Vector2U:
      CALL_FUNCTOR(ref_functor, nsVec2U32);
      break;

    case Type::Vector3U:
      CALL_FUNCTOR(ref_functor, nsVec3U32);
      break;

    case Type::Vector4U:
      CALL_FUNCTOR(ref_functor, nsVec4U32);
      break;

    case Type::Quaternion:
      CALL_FUNCTOR(ref_functor, nsQuat);
      break;

    case Type::Matrix3:
      CALL_FUNCTOR(ref_functor, nsMat3);
      break;

    case Type::Matrix4:
      CALL_FUNCTOR(ref_functor, nsMat4);
      break;

    case Type::Transform:
      CALL_FUNCTOR(ref_functor, nsTransform);
      break;

    case Type::String:
      CALL_FUNCTOR(ref_functor, nsString);
      break;

    case Type::StringView:
      CALL_FUNCTOR(ref_functor, nsStringView);
      break;

    case Type::DataBuffer:
      CALL_FUNCTOR(ref_functor, nsDataBuffer);
      break;

    case Type::Time:
      CALL_FUNCTOR(ref_functor, nsTime);
      break;

    case Type::Uuid:
      CALL_FUNCTOR(ref_functor, nsUuid);
      break;

    case Type::Angle:
      CALL_FUNCTOR(ref_functor, nsAngle);
      break;

    case Type::HashedString:
      CALL_FUNCTOR(ref_functor, nsHashedString);
      break;

    case Type::TempHashedString:
      CALL_FUNCTOR(ref_functor, nsTempHashedString);
      break;

    case Type::VariantArray:
      CALL_FUNCTOR(ref_functor, nsVariantArray);
      break;

    case Type::VariantDictionary:
      CALL_FUNCTOR(ref_functor, nsVariantDictionary);
      break;

    case Type::TypedObject:
      CALL_FUNCTOR(ref_functor, nsTypedObject);
      break;

    default:
      NS_REPORT_FAILURE("Could not dispatch type '{0}'", type);
      // Intended fall through to disable warning.
    case Type::TypedPointer:
      CALL_FUNCTOR(ref_functor, nsTypedPointer);
      break;
  }
}

#undef CALL_FUNCTOR

class nsVariantHelper
{
  friend class nsVariant;
  friend struct ConvertFunc;

  static void To(const nsVariant& value, bool& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= nsVariant::Type::Double)
      result = value.ConvertNumber<nsInt32>() != 0;
    else if (value.GetType() == nsVariant::Type::String || value.GetType() == nsVariant::Type::HashedString)
    {
      nsStringView s = value.IsA<nsString>() ? value.Cast<nsString>().GetView() : value.Cast<nsHashedString>().GetView();
      if (nsConversionUtils::StringToBool(s, result) == NS_FAILURE)
      {
        result = false;
        bSuccessful = false;
      }
    }
    else
      NS_REPORT_FAILURE("Conversion to bool failed");
  }

  static void To(const nsVariant& value, nsInt8& result, bool& bSuccessful)
  {
    nsInt32 tempResult = 0;
    To(value, tempResult, bSuccessful);
    result = (nsInt8)tempResult;
  }

  static void To(const nsVariant& value, nsUInt8& result, bool& bSuccessful)
  {
    nsUInt32 tempResult = 0;
    To(value, tempResult, bSuccessful);
    result = (nsUInt8)tempResult;
  }

  static void To(const nsVariant& value, nsInt16& result, bool& bSuccessful)
  {
    nsInt32 tempResult = 0;
    To(value, tempResult, bSuccessful);
    result = (nsInt16)tempResult;
  }

  static void To(const nsVariant& value, nsUInt16& result, bool& bSuccessful)
  {
    nsUInt32 tempResult = 0;
    To(value, tempResult, bSuccessful);
    result = (nsUInt16)tempResult;
  }

  static void To(const nsVariant& value, nsInt32& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= nsVariant::Type::Double)
      result = value.ConvertNumber<nsInt32>();
    else if (value.GetType() == nsVariant::Type::String || value.GetType() == nsVariant::Type::HashedString)
    {
      nsStringView s = value.IsA<nsString>() ? value.Cast<nsString>().GetView() : value.Cast<nsHashedString>().GetView();
      if (nsConversionUtils::StringToInt(s, result) == NS_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
    }
    else
      NS_REPORT_FAILURE("Conversion to int failed");
  }

  static void To(const nsVariant& value, nsUInt32& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= nsVariant::Type::Double)
      result = value.ConvertNumber<nsUInt32>();
    else if (value.GetType() == nsVariant::Type::String || value.GetType() == nsVariant::Type::HashedString)
    {
      nsStringView s = value.IsA<nsString>() ? value.Cast<nsString>().GetView() : value.Cast<nsHashedString>().GetView();
      nsInt64 tmp = result;
      if (nsConversionUtils::StringToInt64(s, tmp) == NS_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
      else
        result = (nsUInt32)tmp;
    }
    else
      NS_REPORT_FAILURE("Conversion to uint failed");
  }

  static void To(const nsVariant& value, nsInt64& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= nsVariant::Type::Double)
      result = value.ConvertNumber<nsInt64>();
    else if (value.GetType() == nsVariant::Type::String || value.GetType() == nsVariant::Type::HashedString)
    {
      nsStringView s = value.IsA<nsString>() ? value.Cast<nsString>().GetView() : value.Cast<nsHashedString>().GetView();
      if (nsConversionUtils::StringToInt64(s, result) == NS_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
    }
    else
      NS_REPORT_FAILURE("Conversion to int64 failed");
  }

  static void To(const nsVariant& value, nsUInt64& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= nsVariant::Type::Double)
      result = value.ConvertNumber<nsUInt64>();
    else if (value.GetType() == nsVariant::Type::String || value.GetType() == nsVariant::Type::HashedString)
    {
      nsStringView s = value.IsA<nsString>() ? value.Cast<nsString>().GetView() : value.Cast<nsHashedString>().GetView();
      nsInt64 tmp = result;
      if (nsConversionUtils::StringToInt64(s, tmp) == NS_FAILURE)
      {
        result = 0;
        bSuccessful = false;
      }
      else
        result = (nsUInt64)tmp;
    }
    else
      NS_REPORT_FAILURE("Conversion to uint64 failed");
  }

  static void To(const nsVariant& value, float& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= nsVariant::Type::Double)
      result = value.ConvertNumber<float>();
    else if (value.GetType() == nsVariant::Type::String || value.GetType() == nsVariant::Type::HashedString)
    {
      nsStringView s = value.IsA<nsString>() ? value.Cast<nsString>().GetView() : value.Cast<nsHashedString>().GetView();
      double tmp = result;
      if (nsConversionUtils::StringToFloat(s, tmp) == NS_FAILURE)
      {
        result = 0.0f;
        bSuccessful = false;
      }
      else
        result = (float)tmp;
    }
    else
      NS_REPORT_FAILURE("Conversion to float failed");
  }

  static void To(const nsVariant& value, double& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() <= nsVariant::Type::Double)
      result = value.ConvertNumber<double>();
    else if (value.GetType() == nsVariant::Type::String || value.GetType() == nsVariant::Type::HashedString)
    {
      nsStringView s = value.IsA<nsString>() ? value.Cast<nsString>().GetView() : value.Cast<nsHashedString>().GetView();
      if (nsConversionUtils::StringToFloat(s, result) == NS_FAILURE)
      {
        result = 0.0;
        bSuccessful = false;
      }
    }
    else
      NS_REPORT_FAILURE("Conversion to double failed");
  }

  static void To(const nsVariant& value, nsString& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.IsValid() == false)
    {
      result = "<Invalid>";
      return;
    }

    ToStringFunc toStringFunc;
    toStringFunc.m_pThis = &value;
    toStringFunc.m_pResult = &result;

    nsVariant::DispatchTo(toStringFunc, value.GetType());
  }

  static void To(const nsVariant& value, nsStringView& result, bool& bSuccessful)
  {
    bSuccessful = true;

    result = value.IsA<nsString>() ? value.Get<nsString>().GetView() : value.Get<nsHashedString>().GetView();
  }

  static void To(const nsVariant& value, nsTypedPointer& result, bool& bSuccessful)
  {
    bSuccessful = true;
    NS_ASSERT_DEBUG(value.GetType() == nsVariant::Type::TypedPointer, "Only ptr can be converted to void*!");
    result = value.Cast<nsTypedPointer>();
  }

  static void To(const nsVariant& value, nsColor& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() == nsVariant::Type::ColorGamma)
      result = value.Cast<nsColorGammaUB>();
    else
      NS_REPORT_FAILURE("Conversion to nsColor failed");
  }

  static void To(const nsVariant& value, nsColorGammaUB& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() == nsVariant::Type::Color)
      result = value.Cast<nsColor>();
    else
      NS_REPORT_FAILURE("Conversion to nsColorGammaUB failed");
  }

  template <typename T, typename V1, typename V2>
  static void ToVec2X(const nsVariant& value, T& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.IsA<V1>())
    {
      const V1& v = value.Cast<V1>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y));
    }
    else if (value.IsA<V2>())
    {
      const V2& v = value.Cast<V2>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y));
    }
    else
    {
      NS_REPORT_FAILURE("Conversion to nsVec2X failed");
      bSuccessful = false;
    }
  }

  static void To(const nsVariant& value, nsVec2& result, bool& bSuccessful) { ToVec2X<nsVec2, nsVec2I32, nsVec2U32>(value, result, bSuccessful); }

  static void To(const nsVariant& value, nsVec2I32& result, bool& bSuccessful) { ToVec2X<nsVec2I32, nsVec2, nsVec2U32>(value, result, bSuccessful); }

  static void To(const nsVariant& value, nsVec2U32& result, bool& bSuccessful) { ToVec2X<nsVec2U32, nsVec2I32, nsVec2>(value, result, bSuccessful); }

  template <typename T, typename V1, typename V2>
  static void ToVec3X(const nsVariant& value, T& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.IsA<V1>())
    {
      const V1& v = value.Cast<V1>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y), static_cast<typename T::ComponentType>(v.z));
    }
    else if (value.IsA<V2>())
    {
      const V2& v = value.Cast<V2>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y), static_cast<typename T::ComponentType>(v.z));
    }
    else
    {
      NS_REPORT_FAILURE("Conversion to nsVec3X failed");
      bSuccessful = false;
    }
  }

  static void To(const nsVariant& value, nsVec3& result, bool& bSuccessful) { ToVec3X<nsVec3, nsVec3I32, nsVec3U32>(value, result, bSuccessful); }

  static void To(const nsVariant& value, nsVec3I32& result, bool& bSuccessful) { ToVec3X<nsVec3I32, nsVec3, nsVec3U32>(value, result, bSuccessful); }

  static void To(const nsVariant& value, nsVec3U32& result, bool& bSuccessful) { ToVec3X<nsVec3U32, nsVec3I32, nsVec3>(value, result, bSuccessful); }

  template <typename T, typename V1, typename V2>
  static void ToVec4X(const nsVariant& value, T& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.IsA<V1>())
    {
      const V1& v = value.Cast<V1>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y), static_cast<typename T::ComponentType>(v.z), static_cast<typename T::ComponentType>(v.w));
    }
    else if (value.IsA<V2>())
    {
      const V2& v = value.Cast<V2>();
      result = T(static_cast<typename T::ComponentType>(v.x), static_cast<typename T::ComponentType>(v.y), static_cast<typename T::ComponentType>(v.z), static_cast<typename T::ComponentType>(v.w));
    }
    else
    {
      NS_REPORT_FAILURE("Conversion to nsVec4X failed");
      bSuccessful = false;
    }
  }

  static void To(const nsVariant& value, nsVec4& result, bool& bSuccessful) { ToVec4X<nsVec4, nsVec4I32, nsVec4U32>(value, result, bSuccessful); }

  static void To(const nsVariant& value, nsVec4I32& result, bool& bSuccessful) { ToVec4X<nsVec4I32, nsVec4, nsVec4U32>(value, result, bSuccessful); }

  static void To(const nsVariant& value, nsVec4U32& result, bool& bSuccessful) { ToVec4X<nsVec4U32, nsVec4I32, nsVec4>(value, result, bSuccessful); }

  static void To(const nsVariant& value, nsHashedString& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() == nsVariantType::String)
      result.Assign(value.Cast<nsString>());
    else if (value.GetType() == nsVariantType::StringView)
      result.Assign(value.Cast<nsStringView>());
    else
    {
      nsString s;
      To(value, s, bSuccessful);
      result.Assign(s.GetView());
    }
  }

  static void To(const nsVariant& value, nsTempHashedString& result, bool& bSuccessful)
  {
    bSuccessful = true;

    if (value.GetType() == nsVariantType::String)
      result = value.Cast<nsString>();
    else if (value.GetType() == nsVariantType::StringView)
      result = value.Cast<nsStringView>();
    else if (value.GetType() == nsVariant::Type::HashedString)
      result = value.Cast<nsHashedString>();
    else
    {
      nsString s;
      To(value, s, bSuccessful);
      result = s.GetView();
    }
  }

  template <typename T>
  static void To(const nsVariant& value, T& result, bool& bSuccessful)
  {
    NS_REPORT_FAILURE("Conversion function not implemented for target type '{0}'", nsVariant::TypeDeduction<T>::value);
    bSuccessful = false;
  }

  struct ToStringFunc
  {
    template <typename T>
    NS_ALWAYS_INLINE void operator()()
    {
      nsStringBuilder tmp;
      *m_pResult = nsConversionUtils::ToString(m_pThis->Cast<T>(), tmp);
    }

    const nsVariant* m_pThis;
    nsString* m_pResult;
  };
};
