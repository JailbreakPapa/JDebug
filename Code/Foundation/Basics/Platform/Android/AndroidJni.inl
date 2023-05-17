struct wdJniModifiers
{
  enum Enum
  {
    PUBLIC = 1,
    PRIVATE = 2,
    PROTECTED = 4,
    STATIC = 8,
    FINAL = 16,
    SYNCHRONIZED = 32,
    VOLATILE = 64,
    TRANSIENT = 128,
    NATIVE = 256,
    INTERFACE = 512,
    ABSTRACT = 1024,
    STRICT = 2048,
  };
};

wdJniObject::wdJniObject(jobject object, wdJniOwnerShip ownerShip)
  : m_class(nullptr)
{
  switch (ownerShip)
  {
    case wdJniOwnerShip::OWN:
      m_object = object;
      m_own = true;
      break;

    case wdJniOwnerShip::COPY:
      m_object = wdJniAttachment::GetEnv()->NewLocalRef(object);
      m_own = true;
      break;

    case wdJniOwnerShip::BORROW:
      m_object = object;
      m_own = false;
      break;
  }
}

wdJniObject::wdJniObject(const wdJniObject& other)
  : m_class(nullptr)
{
  m_object = wdJniAttachment::GetEnv()->NewLocalRef(other.m_object);
  m_own = true;
}

wdJniObject::wdJniObject(wdJniObject&& other)
{
  m_object = other.m_object;
  m_class = other.m_class;
  m_own = other.m_own;

  other.m_object = nullptr;
  other.m_class = nullptr;
  other.m_own = false;
}

wdJniObject& wdJniObject::operator=(const wdJniObject& other)
{
  if (this == &other)
    return *this;

  Reset();
  m_object = wdJniAttachment::GetEnv()->NewLocalRef(other.m_object);
  m_own = true;
  return *this;
}

wdJniObject& wdJniObject::operator=(wdJniObject&& other)
{
  if (this == &other)
    return *this;

  Reset();

  m_object = other.m_object;
  m_class = other.m_class;
  m_own = other.m_own;

  other.m_object = nullptr;
  other.m_class = nullptr;
  other.m_own = false;

  return *this;
}

wdJniObject::~wdJniObject()
{
  Reset();
}

void wdJniObject::Reset()
{
  if (m_object && m_own)
  {
    wdJniAttachment::GetEnv()->DeleteLocalRef(m_object);
    m_object = nullptr;
    m_own = false;
  }
  if (m_class)
  {
    wdJniAttachment::GetEnv()->DeleteLocalRef(m_class);
    m_class = nullptr;
  }
}

jobject wdJniObject::GetJObject() const
{
  return m_object;
}

bool wdJniObject::operator==(const wdJniObject& other) const
{
  return wdJniAttachment::GetEnv()->IsSameObject(m_object, other.m_object) == JNI_TRUE;
}

bool wdJniObject::operator!=(const wdJniObject& other) const
{
  return !operator==(other);
}

// Template specializations to dispatch to the correct JNI method for each C++ type.
template <typename T, bool unused = false>
struct wdJniTraits
{
  static_assert(unused, "The passed C++ type is not supported by the JNI wrapper. Arguments and returns types must be one of bool, signed char/jbyte, unsigned short/jchar, short/jshort, int/jint, long long/jlong, float/jfloat, double/jdouble, wdJniObject, wdJniString or wdJniClass.");

  // Places the argument inside a jvalue union.
  static jvalue ToValue(T);

  // Retrieves the Java class static type of the argument. For primitives, this is not the boxed type, but the primitive type.
  static wdJniClass GetStaticType();

  // Retrieves the Java class dynamic type of the argument. For primitives, this is not the boxed type, but the primitive type.
  static wdJniClass GetRuntimeType(T);

  // Creates an invalid/null object to return in case of errors.
  static T GetEmptyObject();

  // Call an instance method with the return type.
  template <typename... Args>
  static T CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  // Call a static method with the return type.
  template <typename... Args>
  static T CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  // Sets/gets a field of the type.
  static void SetField(jobject self, jfieldID field, T);
  static T GetField(jobject self, jfieldID field);

  // Sets/gets a static field of the type.
  static void SetStaticField(jclass clazz, jfieldID field, T);
  static T GetStaticField(jclass clazz, jfieldID field);

  // Appends the JNI type signature of this type to the string buf
  static bool AppendSignature(const T& obj, wdStringBuilder& str);
  static const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<bool>
{
  static inline jvalue ToValue(bool value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(bool);

  static inline bool GetEmptyObject();

  template <typename... Args>
  static bool CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static bool CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, bool arg);
  static inline bool GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, bool arg);
  static inline bool GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(bool, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<jbyte>
{
  static inline jvalue ToValue(jbyte value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(jbyte);

  static inline jbyte GetEmptyObject();

  template <typename... Args>
  static jbyte CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jbyte CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jbyte arg);
  static inline jbyte GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jbyte arg);
  static inline jbyte GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jbyte, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<jchar>
{
  static inline jvalue ToValue(jchar value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(jchar);

  static inline jchar GetEmptyObject();

  template <typename... Args>
  static jchar CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jchar CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jchar arg);
  static inline jchar GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jchar arg);
  static inline jchar GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jchar, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<jshort>
{
  static inline jvalue ToValue(jshort value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(jshort);

  static inline jshort GetEmptyObject();

  template <typename... Args>
  static jshort CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jshort CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jshort arg);
  static inline jshort GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jshort arg);
  static inline jshort GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jshort, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<jint>
{
  static inline jvalue ToValue(jint value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(jint);

  static inline jint GetEmptyObject();

  template <typename... Args>
  static jint CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jint CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jint arg);
  static inline jint GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jint arg);
  static inline jint GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jint, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<jlong>
{
  static inline jvalue ToValue(jlong value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(jlong);

  static inline jlong GetEmptyObject();

  template <typename... Args>
  static jlong CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jlong CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jlong arg);
  static inline jlong GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jlong arg);
  static inline jlong GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jlong, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<jfloat>
{
  static inline jvalue ToValue(jfloat value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(jfloat);

  static inline jfloat GetEmptyObject();

  template <typename... Args>
  static jfloat CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jfloat CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jfloat arg);
  static inline jfloat GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jfloat arg);
  static inline jfloat GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jfloat, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<jdouble>
{
  static inline jvalue ToValue(jdouble value);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(jdouble);

  static inline jdouble GetEmptyObject();

  template <typename... Args>
  static jdouble CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jdouble CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jdouble arg);
  static inline jdouble GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jdouble arg);
  static inline jdouble GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jdouble, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<wdJniObject>
{
  static inline jvalue ToValue(const wdJniObject& object);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(const wdJniObject& object);

  static inline wdJniObject GetEmptyObject();

  template <typename... Args>
  static wdJniObject CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static wdJniObject CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, const wdJniObject& arg);
  static inline wdJniObject GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, const wdJniObject& arg);
  static inline wdJniObject GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(const wdJniObject& obj, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<wdJniClass>
{
  static inline jvalue ToValue(const wdJniClass& object);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(const wdJniClass& object);

  static inline wdJniClass GetEmptyObject();

  template <typename... Args>
  static wdJniClass CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static wdJniClass CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, const wdJniClass& arg);
  static inline wdJniClass GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, const wdJniClass& arg);
  static inline wdJniClass GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(const wdJniClass& obj, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<wdJniString>
{
  static inline jvalue ToValue(const wdJniString& object);

  static inline wdJniClass GetStaticType();

  static inline wdJniClass GetRuntimeType(const wdJniString& object);

  static inline wdJniString GetEmptyObject();

  template <typename... Args>
  static wdJniString CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static wdJniString CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, const wdJniString& arg);
  static inline wdJniString GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, const wdJniString& arg);
  static inline wdJniString GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(const wdJniString& obj, wdStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct wdJniTraits<void>
{
  static inline wdJniClass GetStaticType();

  static inline void GetEmptyObject();

  template <typename... Args>
  static void CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static void CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline const char* GetSignatureStatic();
};

// Helpers to unpack variadic templates.
struct wdJniImpl
{
  static void CollectArgumentTypes(wdJniClass* target)
  {
  }

  template <typename T, typename... Tail>
  static void CollectArgumentTypes(wdJniClass* target, const T& arg, const Tail&... tail)
  {
    *target = wdJniTraits<T>::GetRuntimeType(arg);
    return wdJniImpl::CollectArgumentTypes(target + 1, tail...);
  }

  static void UnpackArgs(jvalue* target)
  {
  }

  template <typename T, typename... Tail>
  static void UnpackArgs(jvalue* target, const T& arg, const Tail&... tail)
  {
    *target = wdJniTraits<T>::ToValue(arg);
    return UnpackArgs(target + 1, tail...);
  }

  template <typename Ret, typename... Args>
  static bool BuildMethodSignature(wdStringBuilder& signature, const Args&... args)
  {
    signature.Append("(");
    if (!wdJniImpl::AppendSignature(signature, args...))
    {
      return false;
    }
    signature.Append(")");
    signature.Append(wdJniTraits<Ret>::GetSignatureStatic());
    return true;
  }

  static bool AppendSignature(wdStringBuilder& signature)
  {
    return true;
  }

  template <typename T, typename... Tail>
  static bool AppendSignature(wdStringBuilder& str, const T& arg, const Tail&... tail)
  {
    return wdJniTraits<T>::AppendSignature(arg, str) && AppendSignature(str, tail...);
  }
};

jvalue wdJniTraits<bool>::ToValue(bool value)
{
  jvalue result;
  result.z = value ? JNI_TRUE : JNI_FALSE;
  return result;
}

wdJniClass wdJniTraits<bool>::GetStaticType()
{
  return wdJniClass("java/lang/Boolean").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<bool>::GetRuntimeType(bool)
{
  return GetStaticType();
}

bool wdJniTraits<bool>::GetEmptyObject()
{
  return false;
}

template <typename... Args>
bool wdJniTraits<bool>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallBooleanMethodA(self, method, array) == JNI_TRUE;
}

template <typename... Args>
bool wdJniTraits<bool>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticBooleanMethodA(clazz, method, array) == JNI_TRUE;
}

void wdJniTraits<bool>::SetField(jobject self, jfieldID field, bool arg)
{
  return wdJniAttachment::GetEnv()->SetBooleanField(self, field, arg ? JNI_TRUE : JNI_FALSE);
}

bool wdJniTraits<bool>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetBooleanField(self, field) == JNI_TRUE;
}

void wdJniTraits<bool>::SetStaticField(jclass clazz, jfieldID field, bool arg)
{
  return wdJniAttachment::GetEnv()->SetStaticBooleanField(clazz, field, arg ? JNI_TRUE : JNI_FALSE);
}

bool wdJniTraits<bool>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticBooleanField(clazz, field) == JNI_TRUE;
}

bool wdJniTraits<bool>::AppendSignature(bool, wdStringBuilder& str)
{
  str.Append("Z");
  return true;
}

const char* wdJniTraits<bool>::GetSignatureStatic()
{
  return "Z";
}

jvalue wdJniTraits<jbyte>::ToValue(jbyte value)
{
  jvalue result;
  result.b = value;
  return result;
}

wdJniClass wdJniTraits<jbyte>::GetStaticType()
{
  return wdJniClass("java/lang/Byte").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<jbyte>::GetRuntimeType(jbyte)
{
  return GetStaticType();
}

jbyte wdJniTraits<jbyte>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jbyte wdJniTraits<jbyte>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallByteMethodA(self, method, array);
}

template <typename... Args>
jbyte wdJniTraits<jbyte>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticByteMethodA(clazz, method, array);
}

void wdJniTraits<jbyte>::SetField(jobject self, jfieldID field, jbyte arg)
{
  return wdJniAttachment::GetEnv()->SetByteField(self, field, arg);
}

jbyte wdJniTraits<jbyte>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetByteField(self, field);
}

void wdJniTraits<jbyte>::SetStaticField(jclass clazz, jfieldID field, jbyte arg)
{
  return wdJniAttachment::GetEnv()->SetStaticByteField(clazz, field, arg);
}

jbyte wdJniTraits<jbyte>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticByteField(clazz, field);
}

bool wdJniTraits<jbyte>::AppendSignature(jbyte, wdStringBuilder& str)
{
  str.Append("B");
  return true;
}

const char* wdJniTraits<jbyte>::GetSignatureStatic()
{
  return "B";
}

jvalue wdJniTraits<jchar>::ToValue(jchar value)
{
  jvalue result;
  result.c = value;
  return result;
}

wdJniClass wdJniTraits<jchar>::GetStaticType()
{
  return wdJniClass("java/lang/Character").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<jchar>::GetRuntimeType(jchar)
{
  return GetStaticType();
}

jchar wdJniTraits<jchar>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jchar wdJniTraits<jchar>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallCharMethodA(self, method, array);
}

template <typename... Args>
jchar wdJniTraits<jchar>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticCharMethodA(clazz, method, array);
}

void wdJniTraits<jchar>::SetField(jobject self, jfieldID field, jchar arg)
{
  return wdJniAttachment::GetEnv()->SetCharField(self, field, arg);
}

jchar wdJniTraits<jchar>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetCharField(self, field);
}

void wdJniTraits<jchar>::SetStaticField(jclass clazz, jfieldID field, jchar arg)
{
  return wdJniAttachment::GetEnv()->SetStaticCharField(clazz, field, arg);
}

jchar wdJniTraits<jchar>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticCharField(clazz, field);
}

bool wdJniTraits<jchar>::AppendSignature(jchar, wdStringBuilder& str)
{
  str.Append("C");
  return true;
}

const char* wdJniTraits<jchar>::GetSignatureStatic()
{
  return "C";
}

jvalue wdJniTraits<jshort>::ToValue(jshort value)
{
  jvalue result;
  result.s = value;
  return result;
}

wdJniClass wdJniTraits<jshort>::GetStaticType()
{
  return wdJniClass("java/lang/Short").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<jshort>::GetRuntimeType(jshort)
{
  return GetStaticType();
}

jshort wdJniTraits<jshort>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jshort wdJniTraits<jshort>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallShortMethodA(self, method, array);
}

template <typename... Args>
jshort wdJniTraits<jshort>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticShortMethodA(clazz, method, array);
}

void wdJniTraits<jshort>::SetField(jobject self, jfieldID field, jshort arg)
{
  return wdJniAttachment::GetEnv()->SetShortField(self, field, arg);
}

jshort wdJniTraits<jshort>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetShortField(self, field);
}

void wdJniTraits<jshort>::SetStaticField(jclass clazz, jfieldID field, jshort arg)
{
  return wdJniAttachment::GetEnv()->SetStaticShortField(clazz, field, arg);
}

jshort wdJniTraits<jshort>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticShortField(clazz, field);
}

bool wdJniTraits<jshort>::AppendSignature(jshort, wdStringBuilder& str)
{
  str.Append("S");
  return true;
}

const char* wdJniTraits<jshort>::GetSignatureStatic()
{
  return "S";
}

jvalue wdJniTraits<jint>::ToValue(jint value)
{
  jvalue result;
  result.i = value;
  return result;
}

wdJniClass wdJniTraits<jint>::GetStaticType()
{
  return wdJniClass("java/lang/Integer").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<jint>::GetRuntimeType(jint)
{
  return GetStaticType();
}

jint wdJniTraits<jint>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jint wdJniTraits<jint>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallIntMethodA(self, method, array);
}

template <typename... Args>
jint wdJniTraits<jint>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticIntMethodA(clazz, method, array);
}

void wdJniTraits<jint>::SetField(jobject self, jfieldID field, jint arg)
{
  return wdJniAttachment::GetEnv()->SetIntField(self, field, arg);
}

jint wdJniTraits<jint>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetIntField(self, field);
}

void wdJniTraits<jint>::SetStaticField(jclass clazz, jfieldID field, jint arg)
{
  return wdJniAttachment::GetEnv()->SetStaticIntField(clazz, field, arg);
}

jint wdJniTraits<jint>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticIntField(clazz, field);
}

bool wdJniTraits<jint>::AppendSignature(jint, wdStringBuilder& str)
{
  str.Append("I");
  return true;
}

const char* wdJniTraits<jint>::GetSignatureStatic()
{
  return "I";
}

jvalue wdJniTraits<jlong>::ToValue(jlong value)
{
  jvalue result;
  result.j = value;
  return result;
}

wdJniClass wdJniTraits<jlong>::GetStaticType()
{
  return wdJniClass("java/lang/Long").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<jlong>::GetRuntimeType(jlong)
{
  return GetStaticType();
}

jlong wdJniTraits<jlong>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jlong wdJniTraits<jlong>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallLongMethodA(self, method, array);
}

template <typename... Args>
jlong wdJniTraits<jlong>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticLongMethodA(clazz, method, array);
}

void wdJniTraits<jlong>::SetField(jobject self, jfieldID field, jlong arg)
{
  return wdJniAttachment::GetEnv()->SetLongField(self, field, arg);
}

jlong wdJniTraits<jlong>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetLongField(self, field);
}

void wdJniTraits<jlong>::SetStaticField(jclass clazz, jfieldID field, jlong arg)
{
  return wdJniAttachment::GetEnv()->SetStaticLongField(clazz, field, arg);
}

jlong wdJniTraits<jlong>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticLongField(clazz, field);
}

bool wdJniTraits<jlong>::AppendSignature(jlong, wdStringBuilder& str)
{
  str.Append("J");
  return true;
}

const char* wdJniTraits<jlong>::GetSignatureStatic()
{
  return "J";
}

jvalue wdJniTraits<jfloat>::ToValue(jfloat value)
{
  jvalue result;
  result.f = value;
  return result;
}

wdJniClass wdJniTraits<jfloat>::GetStaticType()
{
  return wdJniClass("java/lang/Float").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<jfloat>::GetRuntimeType(jfloat)
{
  return GetStaticType();
}

jfloat wdJniTraits<jfloat>::GetEmptyObject()
{
  return nanf("");
}

template <typename... Args>
jfloat wdJniTraits<jfloat>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallFloatMethodA(self, method, array);
}

template <typename... Args>
jfloat wdJniTraits<jfloat>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticFloatMethodA(clazz, method, array);
}

void wdJniTraits<jfloat>::SetField(jobject self, jfieldID field, jfloat arg)
{
  return wdJniAttachment::GetEnv()->SetFloatField(self, field, arg);
}

jfloat wdJniTraits<jfloat>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetFloatField(self, field);
}

void wdJniTraits<jfloat>::SetStaticField(jclass clazz, jfieldID field, jfloat arg)
{
  return wdJniAttachment::GetEnv()->SetStaticFloatField(clazz, field, arg);
}

jfloat wdJniTraits<jfloat>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticFloatField(clazz, field);
}

bool wdJniTraits<jfloat>::AppendSignature(jfloat, wdStringBuilder& str)
{
  str.Append("F");
  return true;
}

const char* wdJniTraits<jfloat>::GetSignatureStatic()
{
  return "F";
}

jvalue wdJniTraits<jdouble>::ToValue(jdouble value)
{
  jvalue result;
  result.d = value;
  return result;
}

wdJniClass wdJniTraits<jdouble>::GetStaticType()
{
  return wdJniClass("java/lang/Double").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

wdJniClass wdJniTraits<jdouble>::GetRuntimeType(jdouble)
{
  return GetStaticType();
}

jdouble wdJniTraits<jdouble>::GetEmptyObject()
{
  return nan("");
}

template <typename... Args>
jdouble wdJniTraits<jdouble>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallDoubleMethodA(self, method, array);
}

template <typename... Args>
jdouble wdJniTraits<jdouble>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticDoubleMethodA(clazz, method, array);
}

void wdJniTraits<jdouble>::SetField(jobject self, jfieldID field, jdouble arg)
{
  return wdJniAttachment::GetEnv()->SetDoubleField(self, field, arg);
}

jdouble wdJniTraits<jdouble>::GetField(jobject self, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetDoubleField(self, field);
}

void wdJniTraits<jdouble>::SetStaticField(jclass clazz, jfieldID field, jdouble arg)
{
  return wdJniAttachment::GetEnv()->SetStaticDoubleField(clazz, field, arg);
}

jdouble wdJniTraits<jdouble>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniAttachment::GetEnv()->GetStaticDoubleField(clazz, field);
}

bool wdJniTraits<jdouble>::AppendSignature(jdouble, wdStringBuilder& str)
{
  str.Append("D");
  return true;
}

const char* wdJniTraits<jdouble>::GetSignatureStatic()
{
  return "D";
}

jvalue wdJniTraits<wdJniObject>::ToValue(const wdJniObject& value)
{
  jvalue result;
  result.l = value.GetHandle();
  return result;
}

wdJniClass wdJniTraits<wdJniObject>::GetStaticType()
{
  return wdJniClass("java/lang/Object");
}

wdJniClass wdJniTraits<wdJniObject>::GetRuntimeType(const wdJniObject& arg)
{
  return arg.GetClass();
}

wdJniObject wdJniTraits<wdJniObject>::GetEmptyObject()
{
  return wdJniObject();
}

template <typename... Args>
wdJniObject wdJniTraits<wdJniObject>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniObject(wdJniAttachment::GetEnv()->CallObjectMethodA(self, method, array), wdJniOwnerShip::OWN);
}

template <typename... Args>
wdJniObject wdJniTraits<wdJniObject>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniObject(wdJniAttachment::GetEnv()->CallStaticObjectMethodA(clazz, method, array), wdJniOwnerShip::OWN);
}

void wdJniTraits<wdJniObject>::SetField(jobject self, jfieldID field, const wdJniObject& arg)
{
  return wdJniAttachment::GetEnv()->SetObjectField(self, field, arg.GetHandle());
}

wdJniObject wdJniTraits<wdJniObject>::GetField(jobject self, jfieldID field)
{
  return wdJniObject(wdJniAttachment::GetEnv()->GetObjectField(self, field), wdJniOwnerShip::OWN);
}

void wdJniTraits<wdJniObject>::SetStaticField(jclass clazz, jfieldID field, const wdJniObject& arg)
{
  return wdJniAttachment::GetEnv()->SetStaticObjectField(clazz, field, arg.GetHandle());
}

wdJniObject wdJniTraits<wdJniObject>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniObject(wdJniAttachment::GetEnv()->GetStaticObjectField(clazz, field), wdJniOwnerShip::OWN);
}

bool wdJniTraits<wdJniObject>::AppendSignature(const wdJniObject& obj, wdStringBuilder& str)
{
  if (obj.IsNull())
  {
    // Ensure null objects never generate valid signatures in order to force using the reflection path
    return false;
  }
  else
  {
    str.Append("L");
    str.Append(obj.GetClass().UnsafeCall<wdJniString>("getName", "()Ljava/lang/String;").GetData());
    str.ReplaceAll(".", "/");
    str.Append(";");
    return true;
  }
}

const char* wdJniTraits<wdJniObject>::GetSignatureStatic()
{
  return "Ljava/lang/Object;";
}

jvalue wdJniTraits<wdJniClass>::ToValue(const wdJniClass& value)
{
  jvalue result;
  result.l = value.GetHandle();
  return result;
}

wdJniClass wdJniTraits<wdJniClass>::GetStaticType()
{
  return wdJniClass("java/lang/Class");
}

wdJniClass wdJniTraits<wdJniClass>::GetRuntimeType(const wdJniClass& arg)
{
  // Assume there are no types derived from Class
  return GetStaticType();
}

wdJniClass wdJniTraits<wdJniClass>::GetEmptyObject()
{
  return wdJniClass();
}

template <typename... Args>
wdJniClass wdJniTraits<wdJniClass>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniClass(jclass(wdJniAttachment::GetEnv()->CallObjectMethodA(self, method, array)), wdJniOwnerShip::OWN);
}

template <typename... Args>
wdJniClass wdJniTraits<wdJniClass>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniClass(jclass(wdJniAttachment::GetEnv()->CallStaticObjectMethodA(clazz, method, array)), wdJniOwnerShip::OWN);
}

void wdJniTraits<wdJniClass>::SetField(jobject self, jfieldID field, const wdJniClass& arg)
{
  return wdJniAttachment::GetEnv()->SetObjectField(self, field, arg.GetHandle());
}

wdJniClass wdJniTraits<wdJniClass>::GetField(jobject self, jfieldID field)
{
  return wdJniClass(jclass(wdJniAttachment::GetEnv()->GetObjectField(self, field)), wdJniOwnerShip::OWN);
}

void wdJniTraits<wdJniClass>::SetStaticField(jclass clazz, jfieldID field, const wdJniClass& arg)
{
  return wdJniAttachment::GetEnv()->SetStaticObjectField(clazz, field, arg.GetHandle());
}

wdJniClass wdJniTraits<wdJniClass>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniClass(jclass(wdJniAttachment::GetEnv()->GetStaticObjectField(clazz, field)), wdJniOwnerShip::OWN);
}

bool wdJniTraits<wdJniClass>::AppendSignature(const wdJniClass& obj, wdStringBuilder& str)
{
  str.Append("Ljava/lang/Class;");
  return true;
}

const char* wdJniTraits<wdJniClass>::GetSignatureStatic()
{
  return "Ljava/lang/Class;";
}

jvalue wdJniTraits<wdJniString>::ToValue(const wdJniString& value)
{
  jvalue result;
  result.l = value.GetHandle();
  return result;
}

wdJniClass wdJniTraits<wdJniString>::GetStaticType()
{
  return wdJniClass("java/lang/String");
}

wdJniClass wdJniTraits<wdJniString>::GetRuntimeType(const wdJniString& arg)
{
  // Assume there are no types derived from String
  return GetStaticType();
}

wdJniString wdJniTraits<wdJniString>::GetEmptyObject()
{
  return wdJniString();
}

template <typename... Args>
wdJniString wdJniTraits<wdJniString>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniString(jstring(wdJniAttachment::GetEnv()->CallObjectMethodA(self, method, array)), wdJniOwnerShip::OWN);
}

template <typename... Args>
wdJniString wdJniTraits<wdJniString>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniString(jstring(wdJniAttachment::GetEnv()->CallStaticObjectMethodA(clazz, method, array)), wdJniOwnerShip::OWN);
}

void wdJniTraits<wdJniString>::SetField(jobject self, jfieldID field, const wdJniString& arg)
{
  return wdJniAttachment::GetEnv()->SetObjectField(self, field, arg.GetHandle());
}

wdJniString wdJniTraits<wdJniString>::GetField(jobject self, jfieldID field)
{
  return wdJniString(jstring(wdJniAttachment::GetEnv()->GetObjectField(self, field)), wdJniOwnerShip::OWN);
}

void wdJniTraits<wdJniString>::SetStaticField(jclass clazz, jfieldID field, const wdJniString& arg)
{
  return wdJniAttachment::GetEnv()->SetStaticObjectField(clazz, field, arg.GetHandle());
}

wdJniString wdJniTraits<wdJniString>::GetStaticField(jclass clazz, jfieldID field)
{
  return wdJniString(jstring(wdJniAttachment::GetEnv()->GetStaticObjectField(clazz, field)), wdJniOwnerShip::OWN);
}

bool wdJniTraits<wdJniString>::AppendSignature(const wdJniString& obj, wdStringBuilder& str)
{
  str.Append("Ljava/lang/String;");
  return true;
}

const char* wdJniTraits<wdJniString>::GetSignatureStatic()
{
  return "Ljava/lang/String;";
}

wdJniClass wdJniTraits<void>::GetStaticType()
{
  return wdJniClass("java/lang/Void").UnsafeGetStaticField<wdJniClass>("TYPE", "Ljava/lang/Class;");
}

void wdJniTraits<void>::GetEmptyObject()
{
  return;
}

template <typename... Args>
void wdJniTraits<void>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallVoidMethodA(self, method, array);
}

template <typename... Args>
void wdJniTraits<void>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniAttachment::GetEnv()->CallStaticVoidMethodA(clazz, method, array);
}

const char* wdJniTraits<void>::GetSignatureStatic()
{
  return "V";
}

template <typename... Args>
wdJniObject wdJniClass::CreateInstance(const Args&... args) const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return wdJniObject();
  }

  const size_t N = sizeof...(args);

  wdJniClass inputTypes[N];
  wdJniImpl::CollectArgumentTypes(inputTypes, args...);

  wdJniObject foundMethod = FindConstructor(*this, inputTypes, N);

  if (foundMethod.IsNull())
  {
    return wdJniObject();
  }

  jmethodID method = wdJniAttachment::GetEnv()->FromReflectedMethod(foundMethod.GetHandle());

  jvalue array[sizeof...(args)];
  wdJniImpl::UnpackArgs(array, args...);
  return wdJniObject(wdJniAttachment::GetEnv()->NewObjectA(GetHandle(), method, array), wdJniOwnerShip::OWN);
}

template <typename Ret, typename... Args>
Ret wdJniClass::CallStatic(const char* name, const Args&... args) const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  if (!GetJObject())
  {
    wdLog::Error("Attempting to call static method '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  wdStringBuilder signature;
  if (wdJniImpl::BuildMethodSignature<Ret>(signature, args...))
  {
    jmethodID method = wdJniAttachment::GetEnv()->GetStaticMethodID(GetHandle(), name, signature.GetData());

    if (method)
    {
      return wdJniTraits<Ret>::CallStaticMethod(GetHandle(), method, args...);
    }
    else
    {
      wdJniAttachment::GetEnv()->ExceptionClear();
    }
  }

  const size_t N = sizeof...(args);

  wdJniClass returnType = wdJniTraits<Ret>::GetStaticType();

  wdJniClass inputTypes[N];
  wdJniImpl::CollectArgumentTypes(inputTypes, args...);

  wdJniObject foundMethod = FindMethod(true, name, *this, returnType, inputTypes, N);

  if (foundMethod.IsNull())
  {
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = wdJniAttachment::GetEnv()->FromReflectedMethod(foundMethod.GetHandle());
  return wdJniTraits<Ret>::CallStaticMethod(GetHandle(), method, args...);
}

template <typename Ret, typename... Args>
Ret wdJniClass::UnsafeCallStatic(const char* name, const char* signature, const Args&... args) const
{
  if (!GetJObject())
  {
    wdLog::Error("Attempting to call static method '{}' on null class.", name);
    wdLog::Error("Attempting to call static method '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = wdJniAttachment::GetEnv()->GetStaticMethodID(GetHandle(), name, signature);
  if (!method)
  {
    wdLog::Error("No such static method: '{}' with signature '{}' in class '{}'.", name, signature, ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_METHOD);
    return wdJniTraits<Ret>::GetEmptyObject();
  }
  else
  {
    return wdJniTraits<Ret>::CallStaticMethod(GetHandle(), method, args...);
  }
}

template <typename Ret>
Ret wdJniClass::GetStaticField(const char* name) const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  if (!GetJObject())
  {
    wdLog::Error("Attempting to get static field '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  jfieldID fieldID = wdJniAttachment::GetEnv()->GetStaticFieldID(GetHandle(), name, wdJniTraits<Ret>::GetSignatureStatic());
  if (fieldID)
  {
    return wdJniTraits<Ret>::GetStaticField(GetHandle(), fieldID);
  }
  else
  {
    wdJniAttachment::GetEnv()->ExceptionClear();
  }

  wdJniObject field = UnsafeCall<wdJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", wdJniString(name));

  if (wdJniAttachment::GetEnv()->ExceptionOccurred())
  {
    wdJniAttachment::GetEnv()->ExceptionClear();

    wdLog::Error("No field named '{}' found in class '{}'.", name, ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);

    return wdJniTraits<Ret>::GetEmptyObject();
  }

  if ((field.UnsafeCall<jint>("getModifiers", "()I") & wdJniModifiers::STATIC) == 0)
  {
    wdLog::Error("Field named '{}' in class '{}' isn't static.", name, ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  wdJniClass fieldType = field.UnsafeCall<wdJniClass>("getType", "()Ljava/lang/Class;");

  wdJniClass returnType = wdJniTraits<Ret>::GetStaticType();

  if (!returnType.IsAssignableFrom(fieldType))
  {
    wdLog::Error("Field '{}' of type '{}' in class '{}' can't be assigned to return type '{}'.", name, fieldType.ToString().GetData(), ToString().GetData(), returnType.ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  return wdJniTraits<Ret>::GetStaticField(GetHandle(), wdJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()));
}

template <typename Ret>
Ret wdJniClass::UnsafeGetStaticField(const char* name, const char* signature) const
{
  if (!GetJObject())
  {
    wdLog::Error("Attempting to get static field '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  jfieldID field = wdJniAttachment::GetEnv()->GetStaticFieldID(GetHandle(), name, signature);
  if (!field)
  {
    wdLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return wdJniTraits<Ret>::GetEmptyObject();
  }
  else
  {
    return wdJniTraits<Ret>::GetStaticField(GetHandle(), field);
  }
}

template <typename T>
void wdJniClass::SetStaticField(const char* name, const T& arg) const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return;
  }

  if (!GetJObject())
  {
    wdLog::Error("Attempting to set static field '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  wdJniObject field = UnsafeCall<wdJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", wdJniString(name));

  if (wdJniAttachment::GetEnv()->ExceptionOccurred())
  {
    wdJniAttachment::GetEnv()->ExceptionClear();

    wdLog::Error("No field named '{}' found in class '{}'.", name, ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);

    return;
  }

  wdJniClass modifierClass("java/lang/reflect/Modifier");
  jint modifiers = field.UnsafeCall<jint>("getModifiers", "()I");

  if ((modifiers & wdJniModifiers::STATIC) == 0)
  {
    wdLog::Error("Field named '{}' in class '{}' isn't static.", name, ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  if ((modifiers & wdJniModifiers::FINAL) != 0)
  {
    wdLog::Error("Field named '{}' in class '{}' is final.", name, ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  wdJniClass fieldType = field.UnsafeCall<wdJniClass>("getType", "()Ljava/lang/Class;");

  wdJniClass argType = wdJniTraits<T>::GetRuntimeType(arg);

  if (argType.IsNull())
  {
    if (fieldType.IsPrimitive())
    {
      wdLog::Error("Field '{}' of type '{}' can't be assigned null because it is a primitive type.", name, fieldType.ToString().GetData());
      wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }
  else
  {
    if (!fieldType.IsAssignableFrom(argType))
    {
      wdLog::Error("Field '{}' of type '{}' can't be assigned from type '{}'.", name, fieldType.ToString().GetData(), argType.ToString().GetData());
      wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }

  return wdJniTraits<T>::SetStaticField(GetHandle(), wdJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()), arg);
}

template <typename T>
void wdJniClass::UnsafeSetStaticField(const char* name, const char* signature, const T& arg) const
{
  if (!GetJObject())
  {
    wdLog::Error("Attempting to set static field '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  jfieldID field = wdJniAttachment::GetEnv()->GetStaticFieldID(GetHandle(), name, signature);
  if (!field)
  {
    wdLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return;
  }
  else
  {
    return wdJniTraits<T>::SetStaticField(GetHandle(), field, arg);
  }
}

template <typename Ret, typename... Args>
Ret wdJniObject::Call(const char* name, const Args&... args) const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  if (!m_object)
  {
    wdLog::Error("Attempting to call method '{}' on null object.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  // Fast path: Lookup method via signature built from parameters.
  // This only works for exact matches, but is roughly 50 times faster.
  wdStringBuilder signature;
  if (wdJniImpl::BuildMethodSignature<Ret>(signature, args...))
  {
    jmethodID method = wdJniAttachment::GetEnv()->GetMethodID(reinterpret_cast<jclass>(GetClass().GetHandle()), name, signature.GetData());

    if (method)
    {
      return wdJniTraits<Ret>::CallInstanceMethod(m_object, method, args...);
    }
    else
    {
      wdJniAttachment::GetEnv()->ExceptionClear();
    }
  }

  // Fallback to slow path using reflection
  const size_t N = sizeof...(args);

  wdJniClass returnType = wdJniTraits<Ret>::GetStaticType();

  wdJniClass inputTypes[N];
  wdJniImpl::CollectArgumentTypes(inputTypes, args...);

  wdJniObject foundMethod = FindMethod(false, name, GetClass(), returnType, inputTypes, N);

  if (foundMethod.IsNull())
  {
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = wdJniAttachment::GetEnv()->FromReflectedMethod(foundMethod.m_object);
  return wdJniTraits<Ret>::CallInstanceMethod(m_object, method, args...);
}

template <typename Ret, typename... Args>
Ret wdJniObject::UnsafeCall(const char* name, const char* signature, const Args&... args) const
{
  if (!m_object)
  {
    wdLog::Error("Attempting to call method '{}' on null object.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = wdJniAttachment::GetEnv()->GetMethodID(jclass(GetClass().m_object), name, signature);
  if (!method)
  {
    wdLog::Error("No such method: '{}' with signature '{}' in class '{}'.", name, signature, GetClass().ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_METHOD);
    return wdJniTraits<Ret>::GetEmptyObject();
  }
  else
  {
    return wdJniTraits<Ret>::CallInstanceMethod(m_object, method, args...);
  }
}

template <typename T>
void wdJniObject::SetField(const char* name, const T& arg) const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return;
  }

  if (!m_object)
  {
    wdLog::Error("Attempting to set field '{}' on null object.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  // No fast path here since we need to be able to report failures when attempting
  // to set final fields, which we can only do using reflection.

  wdJniObject field = GetClass().UnsafeCall<wdJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", wdJniString(name));

  if (wdJniAttachment::GetEnv()->ExceptionOccurred())
  {
    wdJniAttachment::GetEnv()->ExceptionClear();

    wdLog::Error("No field named '{}' found.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);

    return;
  }

  wdJniClass modifierClass("java/lang/reflect/Modifier");
  jint modifiers = field.UnsafeCall<jint>("getModifiers", "()I");

  if ((modifiers & wdJniModifiers::STATIC) != 0)
  {
    wdLog::Error("Field named '{}' in class '{}' is static.", name, GetClass().ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  if ((modifiers & wdJniModifiers::FINAL) != 0)
  {
    wdLog::Error("Field named '{}' in class '{}' is final.", name, GetClass().ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  wdJniClass fieldType = field.UnsafeCall<wdJniClass>("getType", "()Ljava/lang/Class;");

  wdJniClass argType = wdJniTraits<T>::GetRuntimeType(arg);

  if (argType.IsNull())
  {
    if (fieldType.IsPrimitive())
    {
      wdLog::Error("Field '{}' of type '{}'  in class '{}' can't be assigned null because it is a primitive type.", name, fieldType.ToString().GetData(), GetClass().ToString().GetData());
      wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }
  else
  {
    if (!fieldType.IsAssignableFrom(argType))
    {
      wdLog::Error("Field '{}' of type '{}' in class '{}' can't be assigned from type '{}'.", name, fieldType.ToString().GetData(), GetClass().ToString().GetData(), argType.ToString().GetData());
      wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }

  return wdJniTraits<T>::SetField(m_object, wdJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()), arg);
}

template <typename T>
void wdJniObject::UnsafeSetField(const char* name, const char* signature, const T& arg) const
{
  if (!m_object)
  {
    wdLog::Error("Attempting to set field '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  jfieldID field = wdJniAttachment::GetEnv()->GetFieldID(jclass(GetClass().GetHandle()), name, signature);
  if (!field)
  {
    wdLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return;
  }
  else
  {
    return wdJniTraits<T>::SetField(m_object, field, arg);
  }
}

template <typename Ret>
Ret wdJniObject::GetField(const char* name) const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  if (!m_object)
  {
    wdLog::Error("Attempting to get field '{}' on null object.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  jfieldID fieldID = wdJniAttachment::GetEnv()->GetFieldID(GetClass().GetHandle(), name, wdJniTraits<Ret>::GetSignatureStatic());
  if (fieldID)
  {
    return wdJniTraits<Ret>::GetField(m_object, fieldID);
  }
  else
  {
    wdJniAttachment::GetEnv()->ExceptionClear();
  }

  wdJniObject field = GetClass().UnsafeCall<wdJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", wdJniString(name));

  if (wdJniAttachment::GetEnv()->ExceptionOccurred())
  {
    wdJniAttachment::GetEnv()->ExceptionClear();

    wdLog::Error("No field named '{}' found in class '{}'.", name, GetClass().ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);

    return wdJniTraits<Ret>::GetEmptyObject();
  }

  if ((field.UnsafeCall<jint>("getModifiers", "()I") & wdJniModifiers::STATIC) != 0)
  {
    wdLog::Error("Field named '{}' in class '{}' is static.", name, GetClass().ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  wdJniClass fieldType = field.UnsafeCall<wdJniClass>("getType", "()Ljava/lang/Class;");

  wdJniClass returnType = wdJniTraits<Ret>::GetStaticType();

  if (!returnType.IsAssignableFrom(fieldType))
  {
    wdLog::Error("Field '{}' of type '{}' in class '{}' can't be assigned to return type '{}'.", name, fieldType.ToString().GetData(), GetClass().ToString().GetData(), returnType.ToString().GetData());
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return wdJniTraits<Ret>::GetEmptyObject();
  }

  return wdJniTraits<Ret>::GetField(m_object, wdJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()));
}

template <typename Ret>
Ret wdJniObject::UnsafeGetField(const char* name, const char* signature) const
{
  if (!m_object)
  {
    wdLog::Error("Attempting to get field '{}' on null class.", name);
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  jfieldID field = wdJniAttachment::GetEnv()->GetFieldID(GetClass().GetHandle(), name, signature);
  if (!field)
  {
    wdLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_FIELD);
    return;
  }
  else
  {
    return wdJniTraits<Ret>::GetField(m_object, field);
  }
}
