#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Android/AndroidJni.h>
#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <Foundation/Threading/Thread.h>
#  include <android_native_app_glue.h>

thread_local JNIEnv* nsJniAttachment::s_env;
thread_local bool nsJniAttachment::s_ownsEnv;
thread_local int nsJniAttachment::s_attachCount;
thread_local nsJniErrorState nsJniAttachment::s_lastError;

nsJniAttachment::nsJniAttachment()
{
  if (s_attachCount > 0)
  {
    s_env->PushLocalFrame(16);
  }
  else
  {
    JNIEnv* env = nullptr;
    jint envStatus = nsAndroidUtils::GetAndroidJavaVM()->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool ownsEnv = (envStatus != JNI_OK);
    if (ownsEnv)
    {
      const char* szThreadName = "NS JNI";
      if (const nsThread* pThread = nsThread::GetCurrentThread())
      {
        szThreadName = pThread->GetThreadName();
      }
      else if (nsThreadUtils::IsMainThread())
      {
        szThreadName = "NS Main Thread";
      }
      // Assign name to attachment since ART complains about it not being set.
      JavaVMAttachArgs args = {JNI_VERSION_1_6, szThreadName, nullptr};
      nsAndroidUtils::GetAndroidJavaVM()->AttachCurrentThread(&env, &args);
    }
    else
    {
      // Assume already existing JNI environment will be alive as long as this object exists.
      NS_ASSERT_DEV(env != nullptr, "");
      env->PushLocalFrame(16);
    }

    s_env = env;
    s_ownsEnv = ownsEnv;
  }

  s_attachCount++;
}

nsJniAttachment::~nsJniAttachment()
{
  s_attachCount--;

  if (s_attachCount == 0)
  {
    ClearLastError();

    if (s_ownsEnv)
    {
      nsAndroidUtils::GetAndroidJavaVM()->DetachCurrentThread();
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

nsJniObject nsJniAttachment::GetActivity()
{
  return nsJniObject(nsAndroidUtils::GetAndroidNativeActivity(), nsJniOwnerShip::BORROW);
}

JNIEnv* nsJniAttachment::GetEnv()
{
  NS_ASSERT_DEV(s_env != nullptr, "Thread not attached to the JVM - you forgot to create an instance of nsJniAttachment in the current scope.");

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  void* unused;
  NS_ASSERT_DEBUG(nsAndroidUtils::GetAndroidJavaVM()->GetEnv(&unused, JNI_VERSION_1_6) == JNI_OK,
    "Current thread has lost its attachment to the JVM - some OS calls can cause this to happen. Try to reduce the attachment to a smaller scope.");
#  endif

  return s_env;
}

nsJniErrorState nsJniAttachment::GetLastError()
{
  nsJniErrorState state = s_lastError;
  return state;
}

void nsJniAttachment::ClearLastError()
{
  s_lastError = nsJniErrorState::SUCCESS;
}

void nsJniAttachment::SetLastError(nsJniErrorState state)
{
  s_lastError = state;
}

bool nsJniAttachment::HasPendingException()
{
  return GetEnv()->ExceptionCheck();
}

void nsJniAttachment::ClearPendingException()
{
  return GetEnv()->ExceptionClear();
}

nsJniObject nsJniAttachment::GetPendingException()
{
  return nsJniObject(GetEnv()->ExceptionOccurred(), nsJniOwnerShip::OWN);
}

bool nsJniAttachment::FailOnPendingErrorOrException()
{
  if (nsJniAttachment::GetLastError() != nsJniErrorState::SUCCESS)
  {
    nsLog::Error("Aborting call because the previous error state was not cleared.");
    return true;
  }

  if (nsJniAttachment::HasPendingException())
  {
    nsLog::Error("Aborting call because a Java exception is still pending.");
    nsJniAttachment::SetLastError(nsJniErrorState::PENDING_EXCEPTION);
    return true;
  }

  return false;
}

void nsJniObject::DumpTypes(const nsJniClass* inputTypes, int N, const nsJniClass* returnType)
{
  if (returnType != nullptr)
  {
    nsLog::Error("  With requested return type '{}'", returnType->ToString().GetData());
  }

  for (int paramIdx = 0; paramIdx < N; ++paramIdx)
  {
    nsLog::Error("  With passed param type #{} '{}'", paramIdx, inputTypes[paramIdx].IsNull() ? "(null)" : inputTypes[paramIdx].ToString().GetData());
  }
}

int nsJniObject::CompareMethodSpecificity(const nsJniObject& method1, const nsJniObject& method2)
{
  nsJniClass returnType1 = method1.UnsafeCall<nsJniClass>("getReturnType", "()Ljava/lang/Class;");
  nsJniClass returnType2 = method2.UnsafeCall<nsJniClass>("getReturnType", "()Ljava/lang/Class;");

  nsJniObject paramTypes1 = method1.UnsafeCall<nsJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  nsJniObject paramTypes2 = method2.UnsafeCall<nsJniObject>("getParameterTypes", "()[Ljava/lang/Class;");

  jsize N = nsJniAttachment::GetEnv()->GetArrayLength(jarray(paramTypes1.m_object));

  int decision = returnType1.IsAssignableFrom(returnType2) - returnType2.IsAssignableFrom(returnType1);

  for (jsize paramIdx = 0; paramIdx < N; ++paramIdx)
  {
    nsJniClass paramType1(
      jclass(nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes1.m_object), paramIdx)), nsJniOwnerShip::OWN);
    nsJniClass paramType2(
      jclass(nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes2.m_object), paramIdx)), nsJniOwnerShip::OWN);

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

bool nsJniObject::IsMethodViable(bool bStatic, const nsJniObject& candidateMethod, const nsJniClass& returnType, nsJniClass* inputTypes, int N)
{
  // Check if staticness matches
  if (nsJniClass("java/lang/reflect/Modifier").UnsafeCallStatic<bool>("isStatic", "(I)Z", candidateMethod.UnsafeCall<int>("getModifiers", "()I")) !=
      bStatic)
  {
    return false;
  }

  // Check if return type is assignable to the requested type
  nsJniClass candidateReturnType = candidateMethod.UnsafeCall<nsJniClass>("getReturnType", "()Ljava/lang/Class;");
  if (!returnType.IsAssignableFrom(candidateReturnType))
  {
    return false;
  }

  // Check number of parameters
  nsJniObject parameterTypes = candidateMethod.UnsafeCall<nsJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  jsize numCandidateParams = nsJniAttachment::GetEnv()->GetArrayLength(jarray(parameterTypes.m_object));
  if (numCandidateParams != N)
  {
    return false;
  }

  // Check if input parameter types are assignable to the actual parameter types
  for (jsize paramIdx = 0; paramIdx < numCandidateParams; ++paramIdx)
  {
    nsJniClass paramType(
      jclass(nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(parameterTypes.m_object), paramIdx)), nsJniOwnerShip::OWN);

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

nsJniObject nsJniObject::FindMethod(
  bool bStatic, const char* name, const nsJniClass& searchClass, const nsJniClass& returnType, nsJniClass* inputTypes, int N)
{
  if (searchClass.IsNull())
  {
    nsLog::Error("Attempting to find constructor for null type.");
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniObject();
  }

  nsHybridArray<nsJniObject, 32> bestCandidates;

  // In case of no parameters, fetch the method directly.
  if (N == 0)
  {
    nsJniObject candidateMethod = searchClass.UnsafeCall<nsJniObject>(
      "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;", nsJniString(name), nsJniObject());

    if (!nsJniAttachment::GetEnv()->ExceptionCheck() && IsMethodViable(bStatic, candidateMethod, returnType, inputTypes, N))
    {
      bestCandidates.PushBack(candidateMethod);
    }
    else
    {
      nsJniAttachment::GetEnv()->ExceptionClear();
    }
  }
  else
  {
    // For methods with parameters, loop over all methods to find one with the correct name and matching parameter types

    nsJniObject methodArray = searchClass.UnsafeCall<nsJniObject>("getMethods", "()[Ljava/lang/reflect/Method;");

    jsize numMethods = nsJniAttachment::GetEnv()->GetArrayLength(jarray(methodArray.m_object));
    for (jsize methodIdx = 0; methodIdx < numMethods; ++methodIdx)
    {
      nsJniObject candidateMethod(
        nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(methodArray.m_object), methodIdx), nsJniOwnerShip::OWN);

      nsJniString methodName = candidateMethod.UnsafeCall<nsJniString>("getName", "()Ljava/lang/String;");

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
    nsLog::Error("Overload resolution failed: No method '{}' in class '{}' matches the requested return and parameter types.", name,
      searchClass.ToString().GetData());
    DumpTypes(inputTypes, N, &returnType);
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_METHOD);
    return nsJniObject();
  }
  else
  {
    nsLog::Error("Overload resolution failed: Call to '{}' in class '{}' is ambiguous. Cannot decide between the following candidates:", name,
      searchClass.ToString().GetData());
    for (int candidateIdx = 0; candidateIdx < bestCandidates.GetCount(); ++candidateIdx)
    {
      nsLog::Error("  Candidate #{}: '{}'", candidateIdx, bestCandidates[candidateIdx].ToString().GetData());
    }
    DumpTypes(inputTypes, N, &returnType);
    nsJniAttachment::SetLastError(nsJniErrorState::AMBIGUOUS_CALL);
    return nsJniObject();
  }
}

int nsJniObject::CompareConstructorSpecificity(const nsJniObject& method1, const nsJniObject& method2)
{
  nsJniObject paramTypes1 = method1.UnsafeCall<nsJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  nsJniObject paramTypes2 = method2.UnsafeCall<nsJniObject>("getParameterTypes", "()[Ljava/lang/Class;");

  jsize N = nsJniAttachment::GetEnv()->GetArrayLength(jarray(paramTypes1.m_object));

  int decision = 0;

  for (jsize paramIdx = 0; paramIdx < N; ++paramIdx)
  {
    nsJniClass paramType1(
      jclass(nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes1.m_object), paramIdx)), nsJniOwnerShip::OWN);
    nsJniClass paramType2(
      jclass(nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(paramTypes2.m_object), paramIdx)), nsJniOwnerShip::OWN);

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

bool nsJniObject::IsConstructorViable(const nsJniObject& candidateMethod, nsJniClass* inputTypes, int N)
{
  // Check number of parameters
  nsJniObject parameterTypes = candidateMethod.UnsafeCall<nsJniObject>("getParameterTypes", "()[Ljava/lang/Class;");
  jsize numCandidateParams = nsJniAttachment::GetEnv()->GetArrayLength(jarray(parameterTypes.m_object));
  if (numCandidateParams != N)
  {
    return false;
  }

  // Check if input parameter types are assignable to the actual parameter types
  for (jsize paramIdx = 0; paramIdx < numCandidateParams; ++paramIdx)
  {
    nsJniClass paramType(
      jclass(nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(parameterTypes.m_object), paramIdx)), nsJniOwnerShip::OWN);

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

nsJniObject nsJniObject::FindConstructor(const nsJniClass& type, nsJniClass* inputTypes, int N)
{
  if (type.IsNull())
  {
    nsLog::Error("Attempting to find constructor for null type.");
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniObject();
  }

  nsHybridArray<nsJniObject, 32> bestCandidates;

  // In case of no parameters, fetch the method directly.
  if (N == 0)
  {
    nsJniObject candidateMethod =
      type.UnsafeCall<nsJniObject>("getConstructor", "([Ljava/lang/Class;)Ljava/lang/reflect/Constructor;", nsJniObject());

    if (!nsJniAttachment::GetEnv()->ExceptionCheck() && IsConstructorViable(candidateMethod, inputTypes, N))
    {
      bestCandidates.PushBack(candidateMethod);
    }
    else
    {
      nsJniAttachment::GetEnv()->ExceptionClear();
    }
  }
  else
  {
    // For methods with parameters, loop over all methods to find one with the correct name and matching parameter types

    nsJniObject methodArray = type.UnsafeCall<nsJniObject>("getConstructors", "()[Ljava/lang/reflect/Constructor;");

    jsize numMethods = nsJniAttachment::GetEnv()->GetArrayLength(jarray(methodArray.m_object));
    for (jsize methodIdx = 0; methodIdx < numMethods; ++methodIdx)
    {
      nsJniObject candidateMethod(
        nsJniAttachment::GetEnv()->GetObjectArrayElement(jobjectArray(methodArray.m_object), methodIdx), nsJniOwnerShip::OWN);

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
    nsLog::Error("Overload resolution failed: No constructor in class '{}' matches the requested parameter types.", type.ToString().GetData());
    DumpTypes(inputTypes, N, nullptr);
    nsJniAttachment::SetLastError(nsJniErrorState::NO_MATCHING_METHOD);
    return nsJniObject();
  }
  else
  {
    nsLog::Error("Overload resolution failed: Call to constructor in class '{}' is ambiguous. Cannot decide between the following candidates:",
      type.ToString().GetData());
    for (int candidateIdx = 0; candidateIdx < bestCandidates.GetCount(); ++candidateIdx)
    {
      nsLog::Error("  Candidate #{}: '{}'", candidateIdx, bestCandidates[candidateIdx].ToString().GetData());
    }
    DumpTypes(inputTypes, N, nullptr);
    nsJniAttachment::SetLastError(nsJniErrorState::AMBIGUOUS_CALL);
    return nsJniObject();
  }
}

nsJniObject::nsJniObject()
  : m_object(nullptr)
  , m_class(nullptr)
  , m_own(false)
{
}

jobject nsJniObject::GetHandle() const
{
  return m_object;
}

nsJniClass nsJniObject::GetClass() const
{
  if (!m_object)
  {
    return nsJniClass();
  }

  if (!m_class)
  {
    const_cast<nsJniObject*>(this)->m_class = nsJniAttachment::GetEnv()->GetObjectClass(m_object);
  }

  return nsJniClass(m_class, nsJniOwnerShip::BORROW);
}

nsJniString nsJniObject::ToString() const
{
  if (nsJniAttachment::FailOnPendingErrorOrException())
  {
    return nsJniString();
  }

  // Implement ToString without UnsafeCall, since UnsafeCall requires ToString for diagnostic output.
  if (IsNull())
  {
    nsLog::Error("Attempting to call method 'toString' on null object.");
    nsJniAttachment::SetLastError(nsJniErrorState::CALL_ON_NULL_OBJECT);
    return nsJniString();
  }

  jmethodID method = nsJniAttachment::GetEnv()->GetMethodID(jclass(GetClass().m_object), "toString", "()Ljava/lang/String;");
  NS_ASSERT_DEV(method, "Could not find JNI method toString()");

  return nsJniTraits<nsJniString>::CallInstanceMethod(m_object, method);
}

bool nsJniObject::IsInstanceOf(const nsJniClass& clazz) const
{
  if (IsNull())
  {
    return false;
  }

  return clazz.IsAssignableFrom(GetClass());
}

nsJniString::nsJniString()
  : nsJniObject()
  , m_utf(nullptr)
{
}

nsJniString::nsJniString(const char* str)
  : nsJniObject(nsJniAttachment::GetEnv()->NewStringUTF(str), nsJniOwnerShip::OWN)
  , m_utf(nullptr)
{
}

nsJniString::nsJniString(jstring string, nsJniOwnerShip ownerShip)
  : nsJniObject(string, ownerShip)
  , m_utf(nullptr)
{
}

nsJniString::nsJniString(const nsJniString& other)
  : nsJniObject(other)
  , m_utf(nullptr)
{
}

nsJniString::nsJniString(nsJniString&& other)
  : nsJniObject(other)
  , m_utf(nullptr)
{
  m_utf = other.m_utf;
  other.m_utf = nullptr;
}

nsJniString& nsJniString::operator=(const nsJniString& other)
{
  if (m_utf)
  {
    nsJniAttachment::GetEnv()->ReleaseStringUTFChars(jstring(GetJObject()), m_utf);
    m_utf = nullptr;
  }

  nsJniObject::operator=(other);

  return *this;
}

nsJniString& nsJniString::operator=(nsJniString&& other)
{
  if (m_utf)
  {
    nsJniAttachment::GetEnv()->ReleaseStringUTFChars(jstring(GetJObject()), m_utf);
    m_utf = nullptr;
  }

  nsJniObject::operator=(other);

  m_utf = other.m_utf;
  other.m_utf = nullptr;

  return *this;
}

nsJniString::~nsJniString()
{
  if (m_utf)
  {
    nsJniAttachment::GetEnv()->ReleaseStringUTFChars(jstring(GetJObject()), m_utf);
    m_utf = nullptr;
  }
}

const char* nsJniString::GetData() const
{
  if (IsNull())
  {
    nsLog::Error("Calling AsChar() on null Java String");
    return "<null>";
  }

  if (!m_utf)
  {
    const_cast<nsJniString*>(this)->m_utf = nsJniAttachment::GetEnv()->GetStringUTFChars(jstring(GetJObject()), nullptr);
  }

  return m_utf;
}


nsJniClass::nsJniClass()
  : nsJniObject()
{
}

nsJniClass::nsJniClass(const char* className)
  : nsJniObject(nsJniAttachment::GetEnv()->FindClass(className), nsJniOwnerShip::OWN)
{
  if (IsNull())
  {
    nsLog::Error("Class '{}' not found.", className);
    nsJniAttachment::SetLastError(nsJniErrorState::CLASS_NOT_FOUND);
  }
}

nsJniClass::nsJniClass(jclass clazz, nsJniOwnerShip ownerShip)
  : nsJniObject(clazz, ownerShip)
{
}

nsJniClass::nsJniClass(const nsJniClass& other)
  : nsJniObject(static_cast<const nsJniObject&>(other))
{
}

nsJniClass::nsJniClass(nsJniClass&& other)
  : nsJniObject(other)
{
}

nsJniClass& nsJniClass::operator=(const nsJniClass& other)
{
  nsJniObject::operator=(other);
  return *this;
}

nsJniClass& nsJniClass::operator=(nsJniClass&& other)
{
  nsJniObject::operator=(other);
  return *this;
}

jclass nsJniClass::GetHandle() const
{
  return static_cast<jclass>(GetJObject());
}

bool nsJniClass::IsAssignableFrom(const nsJniClass& other) const
{
  static bool checkedApiOrder = false;
  static bool reverseArgs = false;

  JNIEnv* env = nsJniAttachment::GetEnv();

  // Guard against JNI bug reversing order of arguments - fixed in
  // https://android.googlesource.com/platform/art/+/1268b742c8cff7318dc0b5b283cbaeabfe0725ba
  if (!checkedApiOrder)
  {
    nsJniClass objectClass("java/lang/Object");
    nsJniClass stringClass("java/lang/String");

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

bool nsJniClass::IsPrimitive()
{
  return UnsafeCall<bool>("isPrimitive", "()Z");
}
#endif
