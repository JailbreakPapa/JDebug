struct nsJniModifiers
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

nsJniObject::nsJniObject(jobject object, nsJniOwnerShip ownerShip)
  : m_class(nullptr)
{
  switch (ownerShip)
  {
    case nsJniOwnerShip::OWN:
      m_object = object;
      m_own = true;
      break;

    case nsJniOwnerShip::COPY:
      m_object = nsJniAttachment::GetEnv()->NewLocalRef(object);
      m_own = true;
      break;

    case nsJniOwnerShip::BORROW:
      m_object = object;
      m_own = false;
      break;
  }
}

nsJniObject::nsJniObject(const nsJniObject& other)
  : m_class(nullptr)
{
  m_object = nsJniAttachment::GetEnv()->NewLocalRef(other.m_object);
  m_own = true;
}

nsJniObject::nsJniObject(nsJniObject&& other)
{
  m_object = other.m_object;
  m_class = other.m_class;
  m_own = other.m_own;

  other.m_object = nullptr;
  other.m_class = nullptr;
  other.m_own = false;
}

nsJniObject& nsJniObject::operator=(const nsJniObject& other)
{
  if (this == &other)
    return *this;

  Reset();
  m_object = nsJniAttachment::GetEnv()->NewLocalRef(other.m_object);
  m_own = true;
  return *this;
}

nsJniObject& nsJniObject::operator=(nsJniObject&& other)
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

nsJniObject::~nsJniObject()
{
  Reset();
}

void nsJniObject::Reset()
{
  if (m_object && m_own)
  {
    nsJniAttachment::GetEnv()->DeleteLocalRef(m_object);
    m_object = nullptr;
    m_own = false;
  }
  if (m_class)
  {
    nsJniAttachment::GetEnv()->DeleteLocalRef(m_class);
    m_class = nullptr;
  }
}

jobject nsJniObject::GetJObject() const
{
  return m_object;
}

bool nsJniObject::operator==(const nsJniObject& other) const
{
  return nsJniAttachment::GetEnv()->IsSameObject(m_object, other.m_object) == JNI_TRUE;
}

bool nsJniObject::operator!=(const nsJniObject& other) const
{
  return !operator==(other);
}

// Template specializations to dispatch to the correct JNI method for each C++ type.
template <typename T, bool unused = false>
struct nsJniTraits
{
  static_assert(unused, "The passed C++ type is not supported by the JNI wrapper. Arguments and returns types must be one of bool, signed char/jbyte, unsigned short/jchar, short/jshort, int/jint, long long/jlong, float/jfloat, double/jdouble, nsJniObject, nsJniString or nsJniClass.");

  // Places the argument inside a jvalue union.
  static jvalue ToValue(T);

  // Retrieves the Java class static type of the argument. For primitives, this is not the boxed type, but the primitive type.
  static nsJniClass GetStaticType();

  // Retrieves the Java class dynamic type of the argument. For primitives, this is not the boxed type, but the primitive type.
  static nsJniClass GetRuntimeType(T);

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
  static bool AppendSignature(const T& obj, nsStringBuilder& str);
  static const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<bool>
{
  static inline jvalue ToValue(bool value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(bool);

  static inline bool GetEmptyObject();

  template <typename... Args>
  static bool CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static bool CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, bool arg);
  static inline bool GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, bool arg);
  static inline bool GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(bool, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<jbyte>
{
  static inline jvalue ToValue(jbyte value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(jbyte);

  static inline jbyte GetEmptyObject();

  template <typename... Args>
  static jbyte CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jbyte CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jbyte arg);
  static inline jbyte GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jbyte arg);
  static inline jbyte GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jbyte, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<jchar>
{
  static inline jvalue ToValue(jchar value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(jchar);

  static inline jchar GetEmptyObject();

  template <typename... Args>
  static jchar CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jchar CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jchar arg);
  static inline jchar GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jchar arg);
  static inline jchar GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jchar, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<jshort>
{
  static inline jvalue ToValue(jshort value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(jshort);

  static inline jshort GetEmptyObject();

  template <typename... Args>
  static jshort CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jshort CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jshort arg);
  static inline jshort GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jshort arg);
  static inline jshort GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jshort, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<jint>
{
  static inline jvalue ToValue(jint value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(jint);

  static inline jint GetEmptyObject();

  template <typename... Args>
  static jint CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jint CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jint arg);
  static inline jint GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jint arg);
  static inline jint GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jint, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<jlong>
{
  static inline jvalue ToValue(jlong value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(jlong);

  static inline jlong GetEmptyObject();

  template <typename... Args>
  static jlong CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jlong CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jlong arg);
  static inline jlong GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jlong arg);
  static inline jlong GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jlong, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<jfloat>
{
  static inline jvalue ToValue(jfloat value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(jfloat);

  static inline jfloat GetEmptyObject();

  template <typename... Args>
  static jfloat CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jfloat CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jfloat arg);
  static inline jfloat GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jfloat arg);
  static inline jfloat GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jfloat, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<jdouble>
{
  static inline jvalue ToValue(jdouble value);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(jdouble);

  static inline jdouble GetEmptyObject();

  template <typename... Args>
  static jdouble CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static jdouble CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, jdouble arg);
  static inline jdouble GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, jdouble arg);
  static inline jdouble GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(jdouble, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<nsJniObject>
{
  static inline jvalue ToValue(const nsJniObject& object);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(const nsJniObject& object);

  static inline nsJniObject GetEmptyObject();

  template <typename... Args>
  static nsJniObject CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static nsJniObject CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, const nsJniObject& arg);
  static inline nsJniObject GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, const nsJniObject& arg);
  static inline nsJniObject GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(const nsJniObject& obj, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<nsJniClass>
{
  static inline jvalue ToValue(const nsJniClass& object);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(const nsJniClass& object);

  static inline nsJniClass GetEmptyObject();

  template <typename... Args>
  static nsJniClass CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static nsJniClass CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, const nsJniClass& arg);
  static inline nsJniClass GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, const nsJniClass& arg);
  static inline nsJniClass GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(const nsJniClass& obj, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<nsJniString>
{
  static inline jvalue ToValue(const nsJniString& object);

  static inline nsJniClass GetStaticType();

  static inline nsJniClass GetRuntimeType(const nsJniString& object);

  static inline nsJniString GetEmptyObject();

  template <typename... Args>
  static nsJniString CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static nsJniString CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline void SetField(jobject self, jfieldID field, const nsJniString& arg);
  static inline nsJniString GetField(jobject self, jfieldID field);

  static inline void SetStaticField(jclass clazz, jfieldID field, const nsJniString& arg);
  static inline nsJniString GetStaticField(jclass clazz, jfieldID field);

  static inline bool AppendSignature(const nsJniString& obj, nsStringBuilder& str);
  static inline const char* GetSignatureStatic();
};

template <>
struct nsJniTraits<void>
{
  static inline nsJniClass GetStaticType();

  static inline void GetEmptyObject();

  template <typename... Args>
  static void CallInstanceMethod(jobject self, jmethodID method, const Args&... args);

  template <typename... Args>
  static void CallStaticMethod(jclass clazz, jmethodID method, const Args&... args);

  static inline const char* GetSignatureStatic();
};

// Helpers to unpack variadic templates.
struct nsJniImpl
{
  static void CollectArgumentTypes(nsJniClass* target)
  {
  }

  template <typename T, typename... Tail>
  static void CollectArgumentTypes(nsJniClass* target, const T& arg, const Tail&... tail)
  {
    *target = nsJniTraits<T>::GetRuntimeType(arg);
    return nsJniImpl::CollectArgumentTypes(target + 1, tail...);
  }

  static void UnpackArgs(jvalue* target)
  {
  }

  template <typename T, typename... Tail>
  static void UnpackArgs(jvalue* target, const T& arg, const Tail&... tail)
  {
    *target = nsJniTraits<T>::ToValue(arg);
    return UnpackArgs(target + 1, tail...);
  }

  template <typename Ret, typename... Args>
  static bool BuildMethodSignature(nsStringBuilder& signature, const Args&... args)
  {
    signature.Append("(");
    if (!nsJniImpl::AppendSignature(signature, args...))
    {
      return false;
    }
    signature.Append(")");
    signature.Append(nsJniTraits<Ret>::GetSignatureStatic());
    return true;
  }

  static bool AppendSignature(nsStringBuilder& signature)
  {
    return true;
  }

  template <typename T, typename... Tail>
  static bool AppendSignature(nsStringBuilder& str, const T& arg, const Tail&... tail)
  {
    return nsJniTraits<T>::AppendSignature(arg, str) && AppendSignature(str, tail...);
  }
};

jvalue nsJniTraits<bool>::ToValue(bool value)
{
  jvalue result;
  result.z = value ? JNI_TRUE : JNI_FALSE;
  return result;
}

nsJniClass nsJniTraits<bool>::GetStaticType()
{
  return nsJniClass("java/lang/Boolean").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<bool>::GetRuntimeType(bool)
{
  return GetStaticType();
}

bool nsJniTraits<bool>::GetEmptyObject()
{
  return false;
}

template <typename... Args>
bool nsJniTraits<bool>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallBooleanMethodA(self, method, array) == JNI_TRUE;
}

template <typename... Args>
bool nsJniTraits<bool>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticBooleanMethodA(clazz, method, array) == JNI_TRUE;
}

void nsJniTraits<bool>::SetField(jobject self, jfieldID field, bool arg)
{
  return nsJniAttachment::GetEnv()->SetBooleanField(self, field, arg ? JNI_TRUE : JNI_FALSE);
}

bool nsJniTraits<bool>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetBooleanField(self, field) == JNI_TRUE;
}

void nsJniTraits<bool>::SetStaticField(jclass clazz, jfieldID field, bool arg)
{
  return nsJniAttachment::GetEnv()->SetStaticBooleanField(clazz, field, arg ? JNI_TRUE : JNI_FALSE);
}

bool nsJniTraits<bool>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticBooleanField(clazz, field) == JNI_TRUE;
}

bool nsJniTraits<bool>::AppendSignature(bool, nsStringBuilder& str)
{
  str.Append("Z");
  return true;
}

const char* nsJniTraits<bool>::GetSignatureStatic()
{
  return "Z";
}

jvalue nsJniTraits<jbyte>::ToValue(jbyte value)
{
  jvalue result;
  result.b = value;
  return result;
}

nsJniClass nsJniTraits<jbyte>::GetStaticType()
{
  return nsJniClass("java/lang/Byte").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<jbyte>::GetRuntimeType(jbyte)
{
  return GetStaticType();
}

jbyte nsJniTraits<jbyte>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jbyte nsJniTraits<jbyte>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallByteMethodA(self, method, array);
}

template <typename... Args>
jbyte nsJniTraits<jbyte>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticByteMethodA(clazz, method, array);
}

void nsJniTraits<jbyte>::SetField(jobject self, jfieldID field, jbyte arg)
{
  return nsJniAttachment::GetEnv()->SetByteField(self, field, arg);
}

jbyte nsJniTraits<jbyte>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetByteField(self, field);
}

void nsJniTraits<jbyte>::SetStaticField(jclass clazz, jfieldID field, jbyte arg)
{
  return nsJniAttachment::GetEnv()->SetStaticByteField(clazz, field, arg);
}

jbyte nsJniTraits<jbyte>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticByteField(clazz, field);
}

bool nsJniTraits<jbyte>::AppendSignature(jbyte, nsStringBuilder& str)
{
  str.Append("B");
  return true;
}

const char* nsJniTraits<jbyte>::GetSignatureStatic()
{
  return "B";
}

jvalue nsJniTraits<jchar>::ToValue(jchar value)
{
  jvalue result;
  result.c = value;
  return result;
}

nsJniClass nsJniTraits<jchar>::GetStaticType()
{
  return nsJniClass("java/lang/Character").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<jchar>::GetRuntimeType(jchar)
{
  return GetStaticType();
}

jchar nsJniTraits<jchar>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jchar nsJniTraits<jchar>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallCharMethodA(self, method, array);
}

template <typename... Args>
jchar nsJniTraits<jchar>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticCharMethodA(clazz, method, array);
}

void nsJniTraits<jchar>::SetField(jobject self, jfieldID field, jchar arg)
{
  return nsJniAttachment::GetEnv()->SetCharField(self, field, arg);
}

jchar nsJniTraits<jchar>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetCharField(self, field);
}

void nsJniTraits<jchar>::SetStaticField(jclass clazz, jfieldID field, jchar arg)
{
  return nsJniAttachment::GetEnv()->SetStaticCharField(clazz, field, arg);
}

jchar nsJniTraits<jchar>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticCharField(clazz, field);
}

bool nsJniTraits<jchar>::AppendSignature(jchar, nsStringBuilder& str)
{
  str.Append("C");
  return true;
}

const char* nsJniTraits<jchar>::GetSignatureStatic()
{
  return "C";
}

jvalue nsJniTraits<jshort>::ToValue(jshort value)
{
  jvalue result;
  result.s = value;
  return result;
}

nsJniClass nsJniTraits<jshort>::GetStaticType()
{
  return nsJniClass("java/lang/Short").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<jshort>::GetRuntimeType(jshort)
{
  return GetStaticType();
}

jshort nsJniTraits<jshort>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jshort nsJniTraits<jshort>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallShortMethodA(self, method, array);
}

template <typename... Args>
jshort nsJniTraits<jshort>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticShortMethodA(clazz, method, array);
}

void nsJniTraits<jshort>::SetField(jobject self, jfieldID field, jshort arg)
{
  return nsJniAttachment::GetEnv()->SetShortField(self, field, arg);
}

jshort nsJniTraits<jshort>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetShortField(self, field);
}

void nsJniTraits<jshort>::SetStaticField(jclass clazz, jfieldID field, jshort arg)
{
  return nsJniAttachment::GetEnv()->SetStaticShortField(clazz, field, arg);
}

jshort nsJniTraits<jshort>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticShortField(clazz, field);
}

bool nsJniTraits<jshort>::AppendSignature(jshort, nsStringBuilder& str)
{
  str.Append("S");
  return true;
}

const char* nsJniTraits<jshort>::GetSignatureStatic()
{
  return "S";
}

jvalue nsJniTraits<jint>::ToValue(jint value)
{
  jvalue result;
  result.i = value;
  return result;
}

nsJniClass nsJniTraits<jint>::GetStaticType()
{
  return nsJniClass("java/lang/Integer").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<jint>::GetRuntimeType(jint)
{
  return GetStaticType();
}

jint nsJniTraits<jint>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jint nsJniTraits<jint>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallIntMethodA(self, method, array);
}

template <typename... Args>
jint nsJniTraits<jint>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticIntMethodA(clazz, method, array);
}

void nsJniTraits<jint>::SetField(jobject self, jfieldID field, jint arg)
{
  return nsJniAttachment::GetEnv()->SetIntField(self, field, arg);
}

jint nsJniTraits<jint>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetIntField(self, field);
}

void nsJniTraits<jint>::SetStaticField(jclass clazz, jfieldID field, jint arg)
{
  return nsJniAttachment::GetEnv()->SetStaticIntField(clazz, field, arg);
}

jint nsJniTraits<jint>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticIntField(clazz, field);
}

bool nsJniTraits<jint>::AppendSignature(jint, nsStringBuilder& str)
{
  str.Append("I");
  return true;
}

const char* nsJniTraits<jint>::GetSignatureStatic()
{
  return "I";
}

jvalue nsJniTraits<jlong>::ToValue(jlong value)
{
  jvalue result;
  result.j = value;
  return result;
}

nsJniClass nsJniTraits<jlong>::GetStaticType()
{
  return nsJniClass("java/lang/Long").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<jlong>::GetRuntimeType(jlong)
{
  return GetStaticType();
}

jlong nsJniTraits<jlong>::GetEmptyObject()
{
  return 0;
}

template <typename... Args>
jlong nsJniTraits<jlong>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallLongMethodA(self, method, array);
}

template <typename... Args>
jlong nsJniTraits<jlong>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticLongMethodA(clazz, method, array);
}

void nsJniTraits<jlong>::SetField(jobject self, jfieldID field, jlong arg)
{
  return nsJniAttachment::GetEnv()->SetLongField(self, field, arg);
}

jlong nsJniTraits<jlong>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetLongField(self, field);
}

void nsJniTraits<jlong>::SetStaticField(jclass clazz, jfieldID field, jlong arg)
{
  return nsJniAttachment::GetEnv()->SetStaticLongField(clazz, field, arg);
}

jlong nsJniTraits<jlong>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticLongField(clazz, field);
}

bool nsJniTraits<jlong>::AppendSignature(jlong, nsStringBuilder& str)
{
  str.Append("J");
  return true;
}

const char* nsJniTraits<jlong>::GetSignatureStatic()
{
  return "J";
}

jvalue nsJniTraits<jfloat>::ToValue(jfloat value)
{
  jvalue result;
  result.f = value;
  return result;
}

nsJniClass nsJniTraits<jfloat>::GetStaticType()
{
  return nsJniClass("java/lang/Float").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<jfloat>::GetRuntimeType(jfloat)
{
  return GetStaticType();
}

jfloat nsJniTraits<jfloat>::GetEmptyObject()
{
  return nanf("");
}

template <typename... Args>
jfloat nsJniTraits<jfloat>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallFloatMethodA(self, method, array);
}

template <typename... Args>
jfloat nsJniTraits<jfloat>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticFloatMethodA(clazz, method, array);
}

void nsJniTraits<jfloat>::SetField(jobject self, jfieldID field, jfloat arg)
{
  return nsJniAttachment::GetEnv()->SetFloatField(self, field, arg);
}

jfloat nsJniTraits<jfloat>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetFloatField(self, field);
}

void nsJniTraits<jfloat>::SetStaticField(jclass clazz, jfieldID field, jfloat arg)
{
  return nsJniAttachment::GetEnv()->SetStaticFloatField(clazz, field, arg);
}

jfloat nsJniTraits<jfloat>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticFloatField(clazz, field);
}

bool nsJniTraits<jfloat>::AppendSignature(jfloat, nsStringBuilder& str)
{
  str.Append("F");
  return true;
}

const char* nsJniTraits<jfloat>::GetSignatureStatic()
{
  return "F";
}

jvalue nsJniTraits<jdouble>::ToValue(jdouble value)
{
  jvalue result;
  result.d = value;
  return result;
}

nsJniClass nsJniTraits<jdouble>::GetStaticType()
{
  return nsJniClass("java/lang/Double").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

nsJniClass nsJniTraits<jdouble>::GetRuntimeType(jdouble)
{
  return GetStaticType();
}

jdouble nsJniTraits<jdouble>::GetEmptyObject()
{
  return nan("");
}

template <typename... Args>
jdouble nsJniTraits<jdouble>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallDoubleMethodA(self, method, array);
}

template <typename... Args>
jdouble nsJniTraits<jdouble>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticDoubleMethodA(clazz, method, array);
}

void nsJniTraits<jdouble>::SetField(jobject self, jfieldID field, jdouble arg)
{
  return nsJniAttachment::GetEnv()->SetDoubleField(self, field, arg);
}

jdouble nsJniTraits<jdouble>::GetField(jobject self, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetDoubleField(self, field);
}

void nsJniTraits<jdouble>::SetStaticField(jclass clazz, jfieldID field, jdouble arg)
{
  return nsJniAttachment::GetEnv()->SetStaticDoubleField(clazz, field, arg);
}

jdouble nsJniTraits<jdouble>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniAttachment::GetEnv()->GetStaticDoubleField(clazz, field);
}

bool nsJniTraits<jdouble>::AppendSignature(jdouble, nsStringBuilder& str)
{
  str.Append("D");
  return true;
}

const char* nsJniTraits<jdouble>::GetSignatureStatic()
{
  return "D";
}

jvalue nsJniTraits<nsJniObject>::ToValue(const nsJniObject& value)
{
  jvalue result;
  result.l = value.GetHandle();
  return result;
}

nsJniClass nsJniTraits<nsJniObject>::GetStaticType()
{
  return nsJniClass("java/lang/Object");
}

nsJniClass nsJniTraits<nsJniObject>::GetRuntimeType(const nsJniObject& arg)
{
  return arg.GetClass();
}

nsJniObject nsJniTraits<nsJniObject>::GetEmptyObject()
{
  return nsJniObject();
}

template <typename... Args>
nsJniObject nsJniTraits<nsJniObject>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniObject(nsJniAttachment::GetEnv()->CallObjectMethodA(self, method, array), nsJniOwnerShip::OWN);
}

template <typename... Args>
nsJniObject nsJniTraits<nsJniObject>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniObject(nsJniAttachment::GetEnv()->CallStaticObjectMethodA(clazz, method, array), nsJniOwnerShip::OWN);
}

void nsJniTraits<nsJniObject>::SetField(jobject self, jfieldID field, const nsJniObject& arg)
{
  return nsJniAttachment::GetEnv()->SetObjectField(self, field, arg.GetHandle());
}

nsJniObject nsJniTraits<nsJniObject>::GetField(jobject self, jfieldID field)
{
  return nsJniObject(nsJniAttachment::GetEnv()->GetObjectField(self, field), nsJniOwnerShip::OWN);
}

void nsJniTraits<nsJniObject>::SetStaticField(jclass clazz, jfieldID field, const nsJniObject& arg)
{
  return nsJniAttachment::GetEnv()->SetStaticObjectField(clazz, field, arg.GetHandle());
}

nsJniObject nsJniTraits<nsJniObject>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniObject(nsJniAttachment::GetEnv()->GetStaticObjectField(clazz, field), nsJniOwnerShip::OWN);
}

bool nsJniTraits<nsJniObject>::AppendSignature(const nsJniObject& obj, nsStringBuilder& str)
{
  if (obj.IsNull())
  {
    // Ensure null objects never generate valid signatures in order to force using the reflection path
    return false;
  }
  else
  {
    str.Append("L");
    str.Append(obj.GetClass().UnsafeCall<nsJniString>("getName", "()Ljava/lang/String;").GetData());
    str.ReplaceAll(".", "/");
    str.Append(";");
    return true;
  }
}

const char* nsJniTraits<nsJniObject>::GetSignatureStatic()
{
  return "Ljava/lang/Object;";
}

jvalue nsJniTraits<nsJniClass>::ToValue(const nsJniClass& value)
{
  jvalue result;
  result.l = value.GetHandle();
  return result;
}

nsJniClass nsJniTraits<nsJniClass>::GetStaticType()
{
  return nsJniClass("java/lang/Class");
}

nsJniClass nsJniTraits<nsJniClass>::GetRuntimeType(const nsJniClass& arg)
{
  // Assume there are no types derived from Class
  return GetStaticType();
}

nsJniClass nsJniTraits<nsJniClass>::GetEmptyObject()
{
  return nsJniClass();
}

template <typename... Args>
nsJniClass nsJniTraits<nsJniClass>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniClass(jclass(nsJniAttachment::GetEnv()->CallObjectMethodA(self, method, array)), nsJniOwnerShip::OWN);
}

template <typename... Args>
nsJniClass nsJniTraits<nsJniClass>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniClass(jclass(nsJniAttachment::GetEnv()->CallStaticObjectMethodA(clazz, method, array)), nsJniOwnerShip::OWN);
}

void nsJniTraits<nsJniClass>::SetField(jobject self, jfieldID field, const nsJniClass& arg)
{
  return nsJniAttachment::GetEnv()->SetObjectField(self, field, arg.GetHandle());
}

nsJniClass nsJniTraits<nsJniClass>::GetField(jobject self, jfieldID field)
{
  return nsJniClass(jclass(nsJniAttachment::GetEnv()->GetObjectField(self, field)), nsJniOwnerShip::OWN);
}

void nsJniTraits<nsJniClass>::SetStaticField(jclass clazz, jfieldID field, const nsJniClass& arg)
{
  return nsJniAttachment::GetEnv()->SetStaticObjectField(clazz, field, arg.GetHandle());
}

nsJniClass nsJniTraits<nsJniClass>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniClass(jclass(nsJniAttachment::GetEnv()->GetStaticObjectField(clazz, field)), nsJniOwnerShip::OWN);
}

bool nsJniTraits<nsJniClass>::AppendSignature(const nsJniClass& obj, nsStringBuilder& str)
{
  str.Append("Ljava/lang/Class;");
  return true;
}

const char* nsJniTraits<nsJniClass>::GetSignatureStatic()
{
  return "Ljava/lang/Class;";
}

jvalue nsJniTraits<nsJniString>::ToValue(const nsJniString& value)
{
  jvalue result;
  result.l = value.GetHandle();
  return result;
}

nsJniClass nsJniTraits<nsJniString>::GetStaticType()
{
  return nsJniClass("java/lang/String");
}

nsJniClass nsJniTraits<nsJniString>::GetRuntimeType(const nsJniString& arg)
{
  // Assume there are no types derived from String
  return GetStaticType();
}

nsJniString nsJniTraits<nsJniString>::GetEmptyObject()
{
  return nsJniString();
}

template <typename... Args>
nsJniString nsJniTraits<nsJniString>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniString(jstring(nsJniAttachment::GetEnv()->CallObjectMethodA(self, method, array)), nsJniOwnerShip::OWN);
}

template <typename... Args>
nsJniString nsJniTraits<nsJniString>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniString(jstring(nsJniAttachment::GetEnv()->CallStaticObjectMethodA(clazz, method, array)), nsJniOwnerShip::OWN);
}

void nsJniTraits<nsJniString>::SetField(jobject self, jfieldID field, const nsJniString& arg)
{
  return nsJniAttachment::GetEnv()->SetObjectField(self, field, arg.GetHandle());
}

nsJniString nsJniTraits<nsJniString>::GetField(jobject self, jfieldID field)
{
  return nsJniString(jstring(nsJniAttachment::GetEnv()->GetObjectField(self, field)), nsJniOwnerShip::OWN);
}

void nsJniTraits<nsJniString>::SetStaticField(jclass clazz, jfieldID field, const nsJniString& arg)
{
  return nsJniAttachment::GetEnv()->SetStaticObjectField(clazz, field, arg.GetHandle());
}

nsJniString nsJniTraits<nsJniString>::GetStaticField(jclass clazz, jfieldID field)
{
  return nsJniString(jstring(nsJniAttachment::GetEnv()->GetStaticObjectField(clazz, field)), nsJniOwnerShip::OWN);
}

bool nsJniTraits<nsJniString>::AppendSignature(const nsJniString& obj, nsStringBuilder& str)
{
  str.Append("Ljava/lang/String;");
  return true;
}

const char* nsJniTraits<nsJniString>::GetSignatureStatic()
{
  return "Ljava/lang/String;";
}

nsJniClass nsJniTraits<void>::GetStaticType()
{
  return nsJniClass("java/lang/Void").UnsafeGetStaticField<nsJniClass>("TYPE", "Ljava/lang/Class;");
}

void nsJniTraits<void>::GetEmptyObject()
{
  return;
}

template <typename... Args>
void nsJniTraits<void>::CallInstanceMethod(jobject self, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallVoidMethodA(self, method, array);
}

template <typename... Args>
void nsJniTraits<void>::CallStaticMethod(jclass clazz, jmethodID method, const Args&... args)
{
  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniAttachment::GetEnv()->CallStaticVoidMethodA(clazz, method, array);
}

const char* nsJniTraits<void>::GetSignatureStatic()
{
  return "V";
}

template <typename... Args>
nsJniObject nsJniClass::CreateInstance(const Args&... args) const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return nsJniObject();
  }

  const size_t N = sizeof...(args);

  nsJniClass inputTypes[N];
  nsJniImpl::CollectArgumentTypes(inputTypes, args...);

  nsJniObject foundMethod = FindConstructor(*this, inputTypes, N);

  if (foundMethod.IsNull())
  {
    return nsJniObject();
  }

  jmethodID method = nsJniAttachment::GetEnv()->FromReflectedMethod(foundMethod.GetHandle());

  jvalue array[sizeof...(args)];
  nsJniImpl::UnpackArgs(array, args...);
  return nsJniObject(nsJniAttachment::GetEnv()->NewObjectA(GetHandle(), method, array), nsJniOwnerShip::OWN);
}

template <typename Ret, typename... Args>
Ret nsJniClass::CallStatic(const char* name, const Args&... args) const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  if (!GetJObject())
  {
    nsLog::Error("Attempting to call static method '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  nsStringBuilder signature;
  if (nsJniImpl::BuildMethodSignature<Ret>(signature, args...))
  {
    jmethodID method = nsJniAttachment::GetEnv()->GetStaticMethodID(GetHandle(), name, signature.GetData());

    if (method)
    {
      return nsJniTraits<Ret>::CallStaticMethod(GetHandle(), method, args...);
    }
    else
    {
      nsJniAttachment::GetEnv()->ExceptionClear();
    }
  }

  const size_t N = sizeof...(args);

  nsJniClass returnType = nsJniTraits<Ret>::GetStaticType();

  nsJniClass inputTypes[N];
  nsJniImpl::CollectArgumentTypes(inputTypes, args...);

  nsJniObject foundMethod = FindMethod(true, name, *this, returnType, inputTypes, N);

  if (foundMethod.IsNull())
  {
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = nsJniAttachment::GetEnv()->FromReflectedMethod(foundMethod.GetHandle());
  return nsJniTraits<Ret>::CallStaticMethod(GetHandle(), method, args...);
}

template <typename Ret, typename... Args>
Ret nsJniClass::UnsafeCallStatic(const char* name, const char* signature, const Args&... args) const
{
  if (!GetJObject())
  {
    nsLog::Error("Attempting to call static method '{}' on null class.", name);
    nsLog::Error("Attempting to call static method '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = nsJniAttachment::GetEnv()->GetStaticMethodID(GetHandle(), name, signature);
  if (!method)
  {
    nsLog::Error("No such static method: '{}' with signature '{}' in class '{}'.", name, signature, ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_METHOD);
    return nsJniTraits<Ret>::GetEmptyObject();
  }
  else
  {
    return nsJniTraits<Ret>::CallStaticMethod(GetHandle(), method, args...);
  }
}

template <typename Ret>
Ret nsJniClass::GetStaticField(const char* name) const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  if (!GetJObject())
  {
    nsLog::Error("Attempting to get static field '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  jfieldID fieldID = nsJniAttachment::GetEnv()->GetStaticFieldID(GetHandle(), name, nsJniTraits<Ret>::GetSignatureStatic());
  if (fieldID)
  {
    return nsJniTraits<Ret>::GetStaticField(GetHandle(), fieldID);
  }
  else
  {
    nsJniAttachment::GetEnv()->ExceptionClear();
  }

  nsJniObject field = UnsafeCall<nsJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", nsJniString(name));

  if (nsJniAttachment::GetEnv()->ExceptionOccurred())
  {
    nsJniAttachment::GetEnv()->ExceptionClear();

    nsLog::Error("No field named '{}' found in class '{}'.", name, ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);

    return nsJniTraits<Ret>::GetEmptyObject();
  }

  if ((field.UnsafeCall<jint>("getModifiers", "()I") & nsJniModifiers::STATIC) == 0)
  {
    nsLog::Error("Field named '{}' in class '{}' isn't static.", name, ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  nsJniClass fieldType = field.UnsafeCall<nsJniClass>("getType", "()Ljava/lang/Class;");

  nsJniClass returnType = nsJniTraits<Ret>::GetStaticType();

  if (!returnType.IsAssignableFrom(fieldType))
  {
    nsLog::Error("Field '{}' of type '{}' in class '{}' can't be assigned to return type '{}'.", name, fieldType.ToString().GetData(), ToString().GetData(), returnType.ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  return nsJniTraits<Ret>::GetStaticField(GetHandle(), nsJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()));
}

template <typename Ret>
Ret nsJniClass::UnsafeGetStaticField(const char* name, const char* signature) const
{
  if (!GetJObject())
  {
    nsLog::Error("Attempting to get static field '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  jfieldID field = nsJniAttachment::GetEnv()->GetStaticFieldID(GetHandle(), name, signature);
  if (!field)
  {
    nsLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return nsJniTraits<Ret>::GetEmptyObject();
  }
  else
  {
    return nsJniTraits<Ret>::GetStaticField(GetHandle(), field);
  }
}

template <typename T>
void nsJniClass::SetStaticField(const char* name, const T& arg) const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return;
  }

  if (!GetJObject())
  {
    nsLog::Error("Attempting to set static field '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  nsJniObject field = UnsafeCall<nsJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", nsJniString(name));

  if (nsJniAttachment::GetEnv()->ExceptionOccurred())
  {
    nsJniAttachment::GetEnv()->ExceptionClear();

    nsLog::Error("No field named '{}' found in class '{}'.", name, ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);

    return;
  }

  nsJniClass modifierClass("java/lang/reflect/Modifier");
  jint modifiers = field.UnsafeCall<jint>("getModifiers", "()I");

  if ((modifiers & nsJniModifiers::STATIC) == 0)
  {
    nsLog::Error("Field named '{}' in class '{}' isn't static.", name, ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  if ((modifiers & nsJniModifiers::FINAL) != 0)
  {
    nsLog::Error("Field named '{}' in class '{}' is final.", name, ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  nsJniClass fieldType = field.UnsafeCall<nsJniClass>("getType", "()Ljava/lang/Class;");

  nsJniClass argType = nsJniTraits<T>::GetRuntimeType(arg);

  if (argType.IsNull())
  {
    if (fieldType.IsPrimitive())
    {
      nsLog::Error("Field '{}' of type '{}' can't be assigned null because it is a primitive type.", name, fieldType.ToString().GetData());
      nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }
  else
  {
    if (!fieldType.IsAssignableFrom(argType))
    {
      nsLog::Error("Field '{}' of type '{}' can't be assigned from type '{}'.", name, fieldType.ToString().GetData(), argType.ToString().GetData());
      nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }

  return nsJniTraits<T>::SetStaticField(GetHandle(), nsJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()), arg);
}

template <typename T>
void nsJniClass::UnsafeSetStaticField(const char* name, const char* signature, const T& arg) const
{
  if (!GetJObject())
  {
    nsLog::Error("Attempting to set static field '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  jfieldID field = nsJniAttachment::GetEnv()->GetStaticFieldID(GetHandle(), name, signature);
  if (!field)
  {
    nsLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return;
  }
  else
  {
    return nsJniTraits<T>::SetStaticField(GetHandle(), field, arg);
  }
}

template <typename Ret, typename... Args>
Ret nsJniObject::Call(const char* name, const Args&... args) const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  if (!m_object)
  {
    nsLog::Error("Attempting to call method '{}' on null object.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  // Fast path: Lookup method via signature built from parameters.
  // This only works for exact matches, but is roughly 50 times faster.
  nsStringBuilder signature;
  if (nsJniImpl::BuildMethodSignature<Ret>(signature, args...))
  {
    jmethodID method = nsJniAttachment::GetEnv()->GetMethodID(reinterpret_cast<jclass>(GetClass().GetHandle()), name, signature.GetData());

    if (method)
    {
      return nsJniTraits<Ret>::CallInstanceMethod(m_object, method, args...);
    }
    else
    {
      nsJniAttachment::GetEnv()->ExceptionClear();
    }
  }

  // Fallback to slow path using reflection
  const size_t N = sizeof...(args);

  nsJniClass returnType = nsJniTraits<Ret>::GetStaticType();

  nsJniClass inputTypes[N];
  nsJniImpl::CollectArgumentTypes(inputTypes, args...);

  nsJniObject foundMethod = FindMethod(false, name, GetClass(), returnType, inputTypes, N);

  if (foundMethod.IsNull())
  {
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = nsJniAttachment::GetEnv()->FromReflectedMethod(foundMethod.m_object);
  return nsJniTraits<Ret>::CallInstanceMethod(m_object, method, args...);
}

template <typename Ret, typename... Args>
Ret nsJniObject::UnsafeCall(const char* name, const char* signature, const Args&... args) const
{
  if (!m_object)
  {
    nsLog::Error("Attempting to call method '{}' on null object.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  jmethodID method = nsJniAttachment::GetEnv()->GetMethodID(jclass(GetClass().m_object), name, signature);
  if (!method)
  {
    nsLog::Error("No such method: '{}' with signature '{}' in class '{}'.", name, signature, GetClass().ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_METHOD);
    return nsJniTraits<Ret>::GetEmptyObject();
  }
  else
  {
    return nsJniTraits<Ret>::CallInstanceMethod(m_object, method, args...);
  }
}

template <typename T>
void nsJniObject::SetField(const char* name, const T& arg) const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return;
  }

  if (!m_object)
  {
    nsLog::Error("Attempting to set field '{}' on null object.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  // No fast path here since we need to be able to report failures when attempting
  // to set final fields, which we can only do using reflection.

  nsJniObject field = GetClass().UnsafeCall<nsJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", nsJniString(name));

  if (nsJniAttachment::GetEnv()->ExceptionOccurred())
  {
    nsJniAttachment::GetEnv()->ExceptionClear();

    nsLog::Error("No field named '{}' found.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);

    return;
  }

  nsJniClass modifierClass("java/lang/reflect/Modifier");
  jint modifiers = field.UnsafeCall<jint>("getModifiers", "()I");

  if ((modifiers & nsJniModifiers::STATIC) != 0)
  {
    nsLog::Error("Field named '{}' in class '{}' is static.", name, GetClass().ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  if ((modifiers & nsJniModifiers::FINAL) != 0)
  {
    nsLog::Error("Field named '{}' in class '{}' is final.", name, GetClass().ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return;
  }

  nsJniClass fieldType = field.UnsafeCall<nsJniClass>("getType", "()Ljava/lang/Class;");

  nsJniClass argType = nsJniTraits<T>::GetRuntimeType(arg);

  if (argType.IsNull())
  {
    if (fieldType.IsPrimitive())
    {
      nsLog::Error("Field '{}' of type '{}'  in class '{}' can't be assigned null because it is a primitive type.", name, fieldType.ToString().GetData(), GetClass().ToString().GetData());
      nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }
  else
  {
    if (!fieldType.IsAssignableFrom(argType))
    {
      nsLog::Error("Field '{}' of type '{}' in class '{}' can't be assigned from type '{}'.", name, fieldType.ToString().GetData(), GetClass().ToString().GetData(), argType.ToString().GetData());
      nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
      return;
    }
  }

  return nsJniTraits<T>::SetField(m_object, nsJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()), arg);
}

template <typename T>
void nsJniObject::UnsafeSetField(const char* name, const char* signature, const T& arg) const
{
  if (!m_object)
  {
    nsLog::Error("Attempting to set field '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  jfieldID field = nsJniAttachment::GetEnv()->GetFieldID(jclass(GetClass().GetHandle()), name, signature);
  if (!field)
  {
    nsLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return;
  }
  else
  {
    return nsJniTraits<T>::SetField(m_object, field, arg);
  }
}

template <typename Ret>
Ret nsJniObject::GetField(const char* name) const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  if (!m_object)
  {
    nsLog::Error("Attempting to get field '{}' on null object.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  jfieldID fieldID = nsJniAttachment::GetEnv()->GetFieldID(GetClass().GetHandle(), name, nsJniTraits<Ret>::GetSignatureStatic());
  if (fieldID)
  {
    return nsJniTraits<Ret>::GetField(m_object, fieldID);
  }
  else
  {
    nsJniAttachment::GetEnv()->ExceptionClear();
  }

  nsJniObject field = GetClass().UnsafeCall<nsJniObject>("getField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", nsJniString(name));

  if (nsJniAttachment::GetEnv()->ExceptionOccurred())
  {
    nsJniAttachment::GetEnv()->ExceptionClear();

    nsLog::Error("No field named '{}' found in class '{}'.", name, GetClass().ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);

    return nsJniTraits<Ret>::GetEmptyObject();
  }

  if ((field.UnsafeCall<jint>("getModifiers", "()I") & nsJniModifiers::STATIC) != 0)
  {
    nsLog::Error("Field named '{}' in class '{}' is static.", name, GetClass().ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  nsJniClass fieldType = field.UnsafeCall<nsJniClass>("getType", "()Ljava/lang/Class;");

  nsJniClass returnType = nsJniTraits<Ret>::GetStaticType();

  if (!returnType.IsAssignableFrom(fieldType))
  {
    nsLog::Error("Field '{}' of type '{}' in class '{}' can't be assigned to return type '{}'.", name, fieldType.ToString().GetData(), GetClass().ToString().GetData(), returnType.ToString().GetData());
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return nsJniTraits<Ret>::GetEmptyObject();
  }

  return nsJniTraits<Ret>::GetField(m_object, nsJniAttachment::GetEnv()->FromReflectedField(field.GetHandle()));
}

template <typename Ret>
Ret nsJniObject::UnsafeGetField(const char* name, const char* signature) const
{
  if (!m_object)
  {
    nsLog::Error("Attempting to get field '{}' on null class.", name);
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return;
  }

  jfieldID field = nsJniAttachment::GetEnv()->GetFieldID(GetClass().GetHandle(), name, signature);
  if (!field)
  {
    nsLog::Error("No such field: '{}' with signature '{}'.", name, signature);
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_FIELD);
    return;
  }
  else
  {
    return nsJniTraits<Ret>::GetField(m_object, field);
  }
}
