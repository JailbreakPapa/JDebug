#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Android/AndroidJni.h>
#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <android_native_app_glue.h>

thread_local JNIEnv* wdJniAttachment::s_env;
thread_local bool wdJniAttachment::s_ownsEnv;
thread_local int wdJniAttachment::s_attachCount;
thread_local wdJniErrorState wdJniAttachment::s_lastError;

wdJniAttachment::wdJniAttachment()
{
  if (s_attachCount > 0)
  {
    s_env->PushLocalFrame(16);
  }
  else
  {
    JNIEnv* env = nullptr;
    jint envStatus = wdAndroidUtils::GetAndroidJavaVM()->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool ownsEnv = (envStatus != JNI_OK);
    if (ownsEnv)
    {
      // Assign name to attachment since ART complains about it not being set.
      JavaVMAttachArgs args = {JNI_VERSION_1_6, "WD JNI", nullptr};
      wdAndroidUtils::GetAndroidJavaVM()->AttachCurrentThread(&env, &args);
    }
    else
    {
      // Assume already existing JNI environment will be alive as long as this object exists.
      WD_ASSERT_DEV(env != nullptr, "");
      env->PushLocalFrame(16);
    }

    s_env = env;
    s_ownsEnv = ownsEnv;
  }

  s_attachCount++;
}

wdJniAttachment::~wdJniAttachment()
{
  s_attachCount--;

  if (s_attachCount == 0)
  {
    ClearLastError();

    if (s_ownsEnv)
    {
      wdAndroidUtils::GetAndroidJavaVM()->DetachCurrentThread();
    }
    else
    {
      s_env->PopLocalFrame(nullptr);
    }

    s_env = nullptr;
    s_ownsEnv = false;
  }
  else
  {
    s_env->PopLocalFrame(nullptr);
  }
}

wdJniObject wdJniAttachment::GetActivity()
{
  return wdJniObject(wdAndroidUtils::GetAndroidApp()->activity->clazz, wdJniOwnerShip::BORROW);
}

JNIEnv* wdJniAttachment::GetEnv()
{
  WD_ASSERT_DEV(s_env != nullptr, "Thread not attached to the JVM - you forgot to create an instance of wdJniAttachment in the current scope.");

#  if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  void* unused;
  WD_ASSERT_DEBUG(wdAndroidUtils::GetAndroidJavaVM()->GetEnv(&unused, JNI_VERSION_1_6) == JNI_OK,
    "Current thread has lost its attachment to the JVM - some OS calls can cause this to happen. Try to reduce the attachment to a smaller scope.");
#  endif

  return s_env;
}

wdJniErrorState wdJniAttachment::GetLastError()
{
  wdJniErrorState state = s_lastError;
  return state;
}

void wdJniAttachment::ClearLastError()
{
  s_lastError = wdJniErrorState::SUCCESS;
}

void wdJniAttachment::SetLastError(wdJniErrorState state)
{
  s_lastError = state;
}

bool wdJniAttachment::HasPendingException()
{
  return GetEnv()->ExceptionCheck();
}

void wdJniAttachment::ClearPendingException()
{
  return GetEnv()->ExceptionClear();
}

wdJniObject wdJniAttachment::GetPendingException()
{
  return wdJniObject(GetEnv()->ExceptionOccurred(), wdJniOwnerShip::OWN);
}

bool wdJniAttachment::FailOnPendingErrorOrException()
{
  if (wdJniAttachment::GetLastError() != wdJniErrorState::SUCCESS)
  {
    wdLog::Error("Aborting call because the previous error state was not cleared.");
    return true;
  }

  if (wdJniAttachment::HasPendingException())
  {
    wdLog::Error("Aborting call because a Java exception is still pending.");
    wdJniAttachment::SetLastError(wdJniErrorState::PENDING_EXCEPTION);
    return true;
  }

  return false;
}

void wdJniObject::DumpTypes(const wdJniClass* inputTypes, int N, const wdJniClass* returnType)
{
  if (returnType != nullptr)
  {
    wdLog::Error("  With requested return type '{}'", returnType->ToString().GetData());
  }

  for (int paramIdx = 0; paramIdx < N; ++paramIdx)
  {
    wdLog::Error("  With passed param type #{} '{}'", paramIdx, inputTypes[paramIdx].IsNull() ? "(null)" : inputTypes[paramIdx].ToString().GetData());
  }
}

int wdJniObject::CompareMethodSpecificity(const wdJniObject& method1, const wdJniObject& method2)
{
  wdJniClass returnType1 = method1.UnsafeCall<wdJniClass>("getReturnType", "()Ljava/lang/Class;");
  wdJniClass returnType2 = method2.UnsafeCall<wdJniClass>("getReturnType", "()Ljava/lang/Class;");

  wdJniObject paramTypes1 = method1.UnsafeCall<wdJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  wdJniObject paramTypes2 = method2.UnsafeCall<wdJniObject>("getParameterTypes", "()[Ljava/lang/Class;");

  jsize N = wdJniAttachment::GetEnv()->GetArrayLength(jarray(paramTypes1.m_object));

  int decision = returnType1.IsAssignableFrom(returnType2) - returnType2.IsAssignableFrom(returnType1);

  for (jsize paramIdx = 0; paramIdx < N; ++paramIdx)
  {
    wdJniClass paramType1(
      jclass(wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes1.m_object), paramIdx)), wdJniOwnerShip::OWN);
    wdJniClass paramType2(
      jclass(wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes2.m_object), paramIdx)), wdJniOwnerShip::OWN);

    int paramDecision = paramType1.IsAssignableFrom(paramType2) - paramType2.IsAssignableFrom(paramType1);

    if (decision == 0)
    {
      // No method is more specific yet
      decision = paramDecision;
    }
    else if (paramDecision != 0 && decision != paramDecision)
    {
      // There is no clear specificity ordering - one type is more specific, but the other less so
      return 0;
    }
  }

  return decision;
}

bool wdJniObject::IsMethodViable(bool bStatic, const wdJniObject& candidateMethod, const wdJniClass& returnType, wdJniClass* inputTypes, int N)
{
  // Check if staticness matches
  if (wdJniClass("java/lang/reflect/Modifier").UnsafeCallStatic<bool>("isStatic", "(I)Z", candidateMethod.UnsafeCall<int>("getModifiers", "()I")) !=
      bStatic)
  {
    return false;
  }

  // Check if return type is assignable to the requested type
  wdJniClass candidateReturnType = candidateMethod.UnsafeCall<wdJniClass>("getReturnType", "()Ljava/lang/Class;");
  if (!returnType.IsAssignableFrom(candidateReturnType))
  {
    return false;
  }

  // Check number of parameters
  wdJniObject parameterTypes = candidateMethod.UnsafeCall<wdJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  jsize numCandidateParams = wdJniAttachment::GetEnv()->GetArrayLength(jarray(parameterTypes.m_object));
  if (numCandidateParams != N)
  {
    return false;
  }

  // Check if input parameter types are assignable to the actual parameter types
  for (jsize paramIdx = 0; paramIdx < numCandidateParams; ++paramIdx)
  {
    wdJniClass paramType(
      jclass(wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(parameterTypes.m_object), paramIdx)), wdJniOwnerShip::OWN);

    if (inputTypes[paramIdx].IsNull())
    {
      if (paramType.IsPrimitive())
      {
        return false;
      }
    }
    else
    {
      if (!paramType.IsAssignableFrom(inputTypes[paramIdx]))
      {
        return false;
      }
    }
  }

  return true;
}

wdJniObject wdJniObject::FindMethod(
  bool bStatic, const char* name, const wdJniClass& searchClass, const wdJniClass& returnType, wdJniClass* inputTypes, int N)
{
  if (searchClass.IsNull())
  {
    wdLog::Error("Attempting to find constructor for null type.");
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniObject();
  }

  wdHybridArray<wdJniObject, 32> bestCandidates;

  // In case of no parameters, fetch the method directly.
  if (N == 0)
  {
    wdJniObject candidateMethod = searchClass.UnsafeCall<wdJniObject>(
      "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;", wdJniString(name), wdJniObject());

    if (!wdJniAttachment::GetEnv()->ExceptionCheck() && IsMethodViable(bStatic, candidateMethod, returnType, inputTypes, N))
    {
      bestCandidates.PushBack(candidateMethod);
    }
    else
    {
      wdJniAttachment::GetEnv()->ExceptionClear();
    }
  }
  else
  {
    // For methods with parameters, loop over all methods to find one with the correct name and matching parameter types

    wdJniObject methodArray = searchClass.UnsafeCall<wdJniObject>("getMethods", "()[Ljava/lang/reflect/Method;");

    jsize numMethods = wdJniAttachment::GetEnv()->GetArrayLength(jarray(methodArray.m_object));
    for (jsize methodIdx = 0; methodIdx < numMethods; ++methodIdx)
    {
      wdJniObject candidateMethod(
        wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(methodArray.m_object), methodIdx), wdJniOwnerShip::OWN);

      wdJniString methodName = candidateMethod.UnsafeCall<wdJniString>("getName", "()Ljava/lang/String;");

      if (strcmp(name, methodName.GetData()) != 0)
      {
        continue;
      }

      if (!IsMethodViable(bStatic, candidateMethod, returnType, inputTypes, N))
      {
        continue;
      }

      bool isMoreSpecific = true;
      for (int candidateIdx = 0; candidateIdx < bestCandidates.GetCount(); ++candidateIdx)
      {
        int comparison = CompareMethodSpecificity(bestCandidates[candidateIdx], candidateMethod);

        if (comparison == 1)
        {
          // Remove less specific candidate and continue looping
          bestCandidates.RemoveAtAndSwap(candidateIdx);
          candidateIdx--;
        }
        else if (comparison == -1)
        {
          // We're less specific, so by transitivity there are no other methods less specific than ours that we could throw out,
          // and we can abort the loop
          isMoreSpecific = false;
          break;
        }
        else
        {
          // No relation, so do nothing
        }
      }

      if (isMoreSpecific)
      {
        bestCandidates.PushBack(candidateMethod);
      }
    }
  }

  if (bestCandidates.GetCount() == 1)
  {
    return bestCandidates[0];
  }
  else if (bestCandidates.GetCount() == 0)
  {
    wdLog::Error("Overload resolution failed: No method '{}' in class '{}' matches the requested return and parameter types.", name,
      searchClass.ToString().GetData());
    DumpTypes(inputTypes, N, &returnType);
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_METHOD);
    return wdJniObject();
  }
  else
  {
    wdLog::Error("Overload resolution failed: Call to '{}' in class '{}' is ambiguous. Cannot decide between the following candidates:", name,
      searchClass.ToString().GetData());
    for (int candidateIdx = 0; candidateIdx < bestCandidates.GetCount(); ++candidateIdx)
    {
      wdLog::Error("  Candidate #{}: '{}'", candidateIdx, bestCandidates[candidateIdx].ToString().GetData());
    }
    DumpTypes(inputTypes, N, &returnType);
    wdJniAttachment::SetLastError(wdJniErrorState::AMBIGUOUS_CALL);
    return wdJniObject();
  }
}

int wdJniObject::CompareConstructorSpecificity(const wdJniObject& method1, const wdJniObject& method2)
{
  wdJniObject paramTypes1 = method1.UnsafeCall<wdJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  wdJniObject paramTypes2 = method2.UnsafeCall<wdJniObject>("getParameterTypes", "()[Ljava/lang/Class;");

  jsize N = wdJniAttachment::GetEnv()->GetArrayLength(jarray(paramTypes1.m_object));

  int decision = 0;

  for (jsize paramIdx = 0; paramIdx < N; ++paramIdx)
  {
    wdJniClass paramType1(
      jclass(wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes1.m_object), paramIdx)), wdJniOwnerShip::OWN);
    wdJniClass paramType2(
      jclass(wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes2.m_object), paramIdx)), wdJniOwnerShip::OWN);

    int paramDecision = paramType1.IsAssignableFrom(paramType2) - paramType2.IsAssignableFrom(paramType1);

    if (decision == 0)
    {
      // No method is more specific yet
      decision = paramDecision;
    }
    else if (paramDecision != 0 && decision != paramDecision)
    {
      // There is no clear specificity ordering - one type is more specific, but the other less so
      return 0;
    }
  }

  return decision;
}

bool wdJniObject::IsConstructorViable(const wdJniObject& candidateMethod, wdJniClass* inputTypes, int N)
{
  // Check number of parameters
  wdJniObject parameterTypes = candidateMethod.UnsafeCall<wdJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  jsize numCandidateParams = wdJniAttachment::GetEnv()->GetArrayLength(jarray(parameterTypes.m_object));
  if (numCandidateParams != N)
  {
    return false;
  }

  // Check if input parameter types are assignable to the actual parameter types
  for (jsize paramIdx = 0; paramIdx < numCandidateParams; ++paramIdx)
  {
    wdJniClass paramType(
      jclass(wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(parameterTypes.m_object), paramIdx)), wdJniOwnerShip::OWN);

    if (inputTypes[paramIdx].IsNull())
    {
      if (paramType.IsPrimitive())
      {
        return false;
      }
    }
    else
    {
      if (!paramType.IsAssignableFrom(inputTypes[paramIdx]))
      {
        return false;
      }
    }
  }

  return true;
}

wdJniObject wdJniObject::FindConstructor(const wdJniClass& type, wdJniClass* inputTypes, int N)
{
  if (type.IsNull())
  {
    wdLog::Error("Attempting to find constructor for null type.");
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniObject();
  }

  wdHybridArray<wdJniObject, 32> bestCandidates;

  // In case of no parameters, fetch the method directly.
  if (N == 0)
  {
    wdJniObject candidateMethod =
      type.UnsafeCall<wdJniObject>("getConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", wdJniObject());

    if (!wdJniAttachment::GetEnv()->ExceptionCheck() && IsConstructorViable(candidateMethod, inputTypes, N))
    {
      bestCandidates.PushBack(candidateMethod);
    }
    else
    {
      wdJniAttachment::GetEnv()->ExceptionClear();
    }
  }
  else
  {
    // For methods with parameters, loop over all methods to find one with the correct name and matching parameter types

    wdJniObject methodArray = type.UnsafeCall<wdJniObject>("getConstructors", "()[Ljava/lang/reflect/Constructor;");

    jsize numMethods = wdJniAttachment::GetEnv()->GetArrayLength(jarray(methodArray.m_object));
    for (jsize methodIdx = 0; methodIdx < numMethods; ++methodIdx)
    {
      wdJniObject candidateMethod(
        wdJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(methodArray.m_object), methodIdx), wdJniOwnerShip::OWN);

      if (!IsConstructorViable(candidateMethod, inputTypes, N))
      {
        continue;
      }

      bool isMoreSpecific = true;
      for (int candidateIdx = 0; candidateIdx < bestCandidates.GetCount(); ++candidateIdx)
      {
        int comparison = CompareConstructorSpecificity(bestCandidates[candidateIdx], candidateMethod);

        if (comparison == 1)
        {
          // Remove less specific candidate and continue looping
          bestCandidates.RemoveAtAndSwap(candidateIdx);
          candidateIdx--;
        }
        else if (comparison == -1)
        {
          // We're less specific, so by transitivity there are no other methods less specific than ours that we could throw out,
          // and we can abort the loop
          isMoreSpecific = false;
          break;
        }
        else
        {
          // No relation, so do nothing
        }
      }

      if (isMoreSpecific)
      {
        bestCandidates.PushBack(candidateMethod);
      }
    }
  }

  if (bestCandidates.GetCount() == 1)
  {
    return bestCandidates[0];
  }
  else if (bestCandidates.GetCount() == 0)
  {
    wdLog::Error("Overload resolution failed: No constructor in class '{}' matches the requested parameter types.", type.ToString().GetData());
    DumpTypes(inputTypes, N, nullptr);
    wdJniAttachment::SetLastError(wdJniErrorState::NO_MATCHING_METHOD);
    return wdJniObject();
  }
  else
  {
    wdLog::Error("Overload resolution failed: Call to constructor in class '{}' is ambiguous. Cannot decide between the following candidates:",
      type.ToString().GetData());
    for (int candidateIdx = 0; candidateIdx < bestCandidates.GetCount(); ++candidateIdx)
    {
      wdLog::Error("  Candidate #{}: '{}'", candidateIdx, bestCandidates[candidateIdx].ToString().GetData());
    }
    DumpTypes(inputTypes, N, nullptr);
    wdJniAttachment::SetLastError(wdJniErrorState::AMBIGUOUS_CALL);
    return wdJniObject();
  }
}

wdJniObject::wdJniObject()
  : m_object(nullptr)
  , m_class(nullptr)
  , m_own(false)
{
}

jobject wdJniObject::GetHandle() const
{
  return m_object;
}

wdJniClass wdJniObject::GetClass() const
{
  if (!m_object)
  {
    return wdJniClass();
  }

  if (!m_class)
  {
    const_cast<wdJniObject*>(this)->m_class = wdJniAttachment::GetEnv()->GetObjectClass(m_object);
  }

  return wdJniClass(m_class, wdJniOwnerShip::BORROW);
}

wdJniString wdJniObject::ToString() const
{
  if (wdJniAttachment::FailOnPendingErrorOrException())
  {
    return wdJniString();
  }

  // Implement ToString without UnsafeCall, since UnsafeCall requires ToString for diagnostic output.
  if (IsNull())
  {
    wdLog::Error("Attempting to call method 'toString' on null object.");
    wdJniAttachment::SetLastError(wdJniErrorState::CALL_ON_NULL_OBJECT);
    return wdJniString();
  }

  jmethodID method = wdJniAttachment::GetEnv()->GetMethodID(jclass(GetClass().m_object), "toString", "()Ljava/lang/String;");
  WD_ASSERT_DEV(method, "Could not find JNI method toString()");

  return wdJniTraits<wdJniString>::CallInstanceMethod(m_object, method);
}

bool wdJniObject::IsInstanceOf(const wdJniClass& clazz) const
{
  if (IsNull())
  {
    return false;
  }

  return clazz.IsAssignableFrom(GetClass());
}

wdJniString::wdJniString()
  : wdJniObject()
  , m_utf(nullptr)
{
}

wdJniString::wdJniString(const char* str)
  : wdJniObject(wdJniAttachment::GetEnv()->NewStringUTF(str), wdJniOwnerShip::OWN)
  , m_utf(nullptr)
{
}

wdJniString::wdJniString(jstring string, wdJniOwnerShip ownerShip)
  : wdJniObject(string, ownerShip)
  , m_utf(nullptr)
{
}

wdJniString::wdJniString(const wdJniString& other)
  : wdJniObject(other)
  , m_utf(nullptr)
{
}

wdJniString::wdJniString(wdJniString&& other)
  : wdJniObject(other)
  , m_utf(nullptr)
{
  m_utf = other.m_utf;
  other.m_utf = nullptr;
}

wdJniString& wdJniString::operator=(const wdJniString& other)
{
  if (m_utf)
  {
    wdJniAttachment::GetEnv()->ReleaseStringUTFChars(jstring(GetJObject()), m_utf);
    m_utf = nullptr;
  }

  wdJniObject::operator=(other);

  return *this;
}

wdJniString& wdJniString::operator=(wdJniString&& other)
{
  if (m_utf)
  {
    wdJniAttachment::GetEnv()->ReleaseStringUTFChars(jstring(GetJObject()), m_utf);
    m_utf = nullptr;
  }

  wdJniObject::operator=(other);

  m_utf = other.m_utf;
  other.m_utf = nullptr;

  return *this;
}

wdJniString::~wdJniString()
{
  if (m_utf)
  {
    wdJniAttachment::GetEnv()->ReleaseStringUTFChars(jstring(GetJObject()), m_utf);
    m_utf = nullptr;
  }
}

const char* wdJniString::GetData() const
{
  if (IsNull())
  {
    wdLog::Error("Calling AsChar() on null Java String");
    return "<null>";
  }

  if (!m_utf)
  {
    const_cast<wdJniString*>(this)->m_utf = wdJniAttachment::GetEnv()->GetStringUTFChars(jstring(GetJObject()), nullptr);
  }

  return m_utf;
}


wdJniClass::wdJniClass()
  : wdJniObject()
{
}

wdJniClass::wdJniClass(const char* className)
  : wdJniObject(wdJniAttachment::GetEnv()->FindClass(className), wdJniOwnerShip::OWN)
{
  if (IsNull())
  {
    wdLog::Error("Class '{}' not found.", className);
    wdJniAttachment::SetLastError(wdJniErrorState::CLASS_NOT_FOUND);
  }
}

wdJniClass::wdJniClass(jclass clazz, wdJniOwnerShip ownerShip)
  : wdJniObject(clazz, ownerShip)
{
}

wdJniClass::wdJniClass(const wdJniClass& other)
  : wdJniObject(static_cast<const wdJniObject&>(other))
{
}

wdJniClass::wdJniClass(wdJniClass&& other)
  : wdJniObject(other)
{
}

wdJniClass& wdJniClass::operator=(const wdJniClass& other)
{
  wdJniObject::operator=(other);
  return *this;
}

wdJniClass& wdJniClass::operator=(wdJniClass&& other)
{
  wdJniObject::operator=(other);
  return *this;
}

jclass wdJniClass::GetHandle() const
{
  return static_cast<jclass>(GetJObject());
}

bool wdJniClass::IsAssignableFrom(const wdJniClass& other) const
{
  static bool checkedApiOrder = false;
  static bool reverseArgs = false;

  JNIEnv* env = wdJniAttachment::GetEnv();

  // Guard against JNI bug reversing order of arguments - fixed in
  // https://android.googlesource.com/platform/art/+/1268b742c8cff7318dc0b5b283cbaeabfe0725ba
  if (!checkedApiOrder)
  {
    wdJniClass objectClass("java/lang/Object");
    wdJniClass stringClass("java/lang/String");

    if (env->IsAssignableFrom(jclass(objectClass.GetJObject()), jclass(stringClass.GetJObject())))
    {
      reverseArgs = true;
    }
    checkedApiOrder = true;
  }

  if (!reverseArgs)
  {
    return env->IsAssignableFrom(jclass(other.GetJObject()), jclass(GetJObject()));
  }
  else
  {
    return env->IsAssignableFrom(jclass(GetJObject()), jclass(other.GetJObject()));
  }
}

bool wdJniClass::IsPrimitive()
{
  return UnsafeCall<bool>("isPrimitive", "()Z");
}
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Basics_Platform_Android_AndroidJni);
