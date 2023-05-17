#pragma once

#if WD_ENABLED(WD_PLATFORM_ANDROID)

#  include <Foundation/Logging/Log.h>
#  include <Foundation/Strings/StringBuilder.h>
#  include <jni.h>

class wdJniObject;
class wdJniClass;
class wdJniString;

/// \brief Describes error conditions that occur while attempting to translate checked calls into JNI.
enum class wdJniErrorState
{
  /// \brief No JNI error occurred.
  SUCCESS = 0,

  /// \brief The method could not be executed because the JVM is still holding a pending exception.
  PENDING_EXCEPTION,

  /// \brief No method matching the passed parameters or return type was found.
  NO_MATCHING_METHOD,

  /// \brief A call could not be resolved due to multiple matching overloads.
  AMBIGUOUS_CALL,

  /// \brief No field matching the return type, static-ness or writability was found.
  NO_MATCHING_FIELD,

  /// \brief A field or method was requested on a null object.
  CALL_ON_NULL_OBJECT,

  /// \brief A class was not found.
  CLASS_NOT_FOUND,
};

/// \brief Attaches the current thread to the Java virtual machine.
///
/// Instantiating this class attaches the current thread to the JVM. You can nest attachments,
/// and each attachment will push a new local reference frame on the JVM.
///
/// \example
///   \code
///     void foo()
///     {
///       wdJniAttachment attachment;
///       wdJniObject activity = attachment.GetActivity();
///
///       // Perform Java calls
///       wdJniClass myClassType = activity.Call<wdJniObject>("getClassLoader").Call<wdJniClass>("loadClass", wdJniString("com.myproject.MyClass"));
///     }
///   \endcode
class WD_FOUNDATION_DLL wdJniAttachment
{
public:
  /// \brief Constructor.
  wdJniAttachment();

  /// \brief Destructor.
  ~wdJniAttachment();

  /// \brief Returns the Activity of the native application.
  ///
  /// The returned object wraps around activity->clazz of the native application.
  static wdJniObject GetActivity();

  /// \brief Returns the environment the current thread is attached to.
  static JNIEnv* GetEnv();

  /// \brief Returns the last error that occurred while trying to perform a checked JNI call.
  ///
  /// This error state covers general failures during JNI interop, but not exceptions that occurred during Java method execution.
  /// However, attempting to perform a Java call while an exception is pending will result in an error.
  static wdJniErrorState GetLastError();

  /// \brief Clears the last error that occurred while trying to perform a checked JNI call.
  ///
  /// This error state covers general failures during JNI interop, but not exceptions that occurred during Java method execution.
  /// However, attempting to perform a Java call while an exception is pending will result in an error.
  static void ClearLastError();

  /// \brief Returns true if an exception has been thrown in the last called Java method.
  ///
  /// If an exception occurred, no other Java method may be called until ClearPendingException has been called.
  static bool HasPendingException();

  /// \brief Returns the exception that has been thrown in the last called Java method, or a null object.
  ///
  /// If an exception occurred, no other Java method may be called until ClearPendingException has been called.
  static wdJniObject GetPendingException();

  /// \brief Clears the exception that has been thrown in the last called Java method, if any.
  static void ClearPendingException();

  /// \brief Used internally. Sets the last error state.
  static void SetLastError(wdJniErrorState state);

  /// \brief Used internally. Returns true and logs a message is an error or exception is pending.
  static bool FailOnPendingErrorOrException();

private:
  static thread_local JNIEnv* s_env;
  static thread_local bool s_ownsEnv;
  static thread_local int s_attachCount;
  static thread_local wdJniErrorState s_lastError;

  wdJniAttachment(const wdJniAttachment&);
  wdJniAttachment& operator=(const wdJniAttachment&);
};

/// \brief Describes the ownership handling of the local JNI reference.
enum class wdJniOwnerShip
{
  /// The local reference belongs to the class, and will be deleted when it goes out of scope.
  OWN,

  /// The class will create its own copy of the reference.
  COPY,

  /// The class will not delete the reference. The caller must ensure that the reference remains valid while the class instance is alive.
  BORROW
};

/// \brief Class that manages a local reference to a Java object.
class WD_FOUNDATION_DLL wdJniObject
{
public:
  /// \brief Creates a null object.
  wdJniObject();

  /// \brief Constructs an object from a JNI object handle.
  ///
  /// \param object The JNI object handle.
  /// \param ownerShip How the object handle should be managed. See wdJniOwnerShip.
  inline wdJniObject(jobject object, wdJniOwnerShip ownerShip);

  /// \brief Copy constructor. Both instances will reference the same Java object.
  inline wdJniObject(const wdJniObject& other);

  /// \brief Move constructor.
  inline wdJniObject(wdJniObject&& other);

  /// \brief Assignment operator.
  inline wdJniObject& operator=(const wdJniObject& other);

  /// \brief Move assignment operator.
  inline wdJniObject& operator=(wdJniObject&& other);

  /// \brief Destructor.
  inline virtual ~wdJniObject();

  /// \brief Compares if the two objects reference the same Java object.
  /// \param other The object to compare to.
  ///
  /// This method returns true if two wdJniObjects reference the same Java object.
  ///
  /// In order to compare the objects using \c Object.equals, use the following code instead:
  ///
  /// \code
  ///   wdJniObject o1, o2;
  ///   if(o1.Call<bool>("equals", o2))
  ///   {
  ///      // ...
  ///   }
  /// \endcode
  inline bool operator==(const wdJniObject& other) const;

  /// \brief Compares if the two objects reference different Java objects.
  /// \param other The object to compare to.
  ///
  /// This method returns true if two wdJniObjects reference different Java objects.
  ///
  /// In order to compare the objects using \c Object.equals, use the following code instead:
  ///
  /// \code
  ///   wdJniObject o1, o2;
  ///   if(!o1.Call<bool>("equals", o2))
  ///   {
  ///      // ...
  ///   }
  /// \endcode
  inline bool operator!=(const wdJniObject& other) const;

  /// \brief Returns true if the object is null.
  bool IsNull() const { return m_object == nullptr; }

  /// \brief Returns the JNI handle of the object.
  jobject GetHandle() const;

  /// \brief Returns the class type of the object.
  ///
  /// This Call is equivalent to \c o.getClass() in Java.
  wdJniClass GetClass() const;

  /// \brief Returns a string representation of the object.
  ///
  /// This call is equivalent to \c o.ToString() in Java.
  wdJniString ToString() const;

  /// \brief Returns true if the object is an instance of the given type.
  bool IsInstanceOf(const wdJniClass& clazz) const;

  /// \brief Calls an instance method on the object.
  /// \param name The name of the method to call.
  /// \param args The function arguments to pass.
  ///
  /// This function searches for a public method of the given name that is compatible with the
  /// passed arguments and the return type that is given by the template argument.
  /// If there are multiple suitable methods, dynamic overload resolution is performed to select
  /// the overload that is the most specific. Parameters that null are always assumed to be of type Object.
  ///
  /// In case the method of the given name isn't found, or there exists a method of the given name that doesn't
  /// match the requested return and argument types, or overload resolution can't find a single best method to call,
  /// this function logs a detailed error message and returns a dummy object instead.
  ///
  /// Note that no conversions between primitive types are performed, nor any boxing/unboxing conversions that are usually implicit
  /// in normal Java code. See the examples below on how to handle these cases.
  ///
  /// Varargs methods are currently not supported.
  ///
  /// \example
  ///   \code
  ///     wdJniObject myClassInstance;
  ///
  ///     // --- Overload resolution
  ///
  ///     // Java declaration: two overloads
  ///     //   public Player getPlayer(int player)
  ///     //   public Player getPlayer(String playerName)
  ///
  ///     // Call the first overload
  ///     wdJniObject player = myClassInstance.Call<wdJniObject>("getPlayer", 0);
  ///
  ///     // Call the second overload
  ///     wdJniObject player = myClassInstance.Call<wdJniObject>("getPlayer", wdJniString("player1"));
  ///
  ///     // This call will fail at runtime since there is no method getPlayer that returns int.
  ///     int player = myClassInstance.Call<int>("getPlayer", 0);
  ///
  ///     // --- Manual argument conversion
  ///
  ///     // Java declaration: public Player getPlayerById(long playerId)
  ///
  ///     // This call will fail at runtime: There is no method named getPlayerById that takes a parameter of type int.
  ///     wdJniObject player = myClassInstance.Call<wdJniObject>("getPlayerById", 0);
  ///
  ///     // Instead, explicitly cast the function parameter to the expected type according to the following table:
  ///     //
  ///     //   Java type        C++ type
  ///     //    boolean     <=>  bool (do not use jboolean!)
  ///     //    byte        <=>  jbyte or signed char
  ///     //    char        <=>  jchar or unsigned short
  ///     //    short       <=>  jshort or signed short
  ///     //    int         <=>  jint or signed int
  ///     //    long        <=>  jlong or signed long long
  ///     //    float       <=>  jfloat or float
  ///     //    double      <=>  jdouble or double
  ///     //
  ///     wdJniObject player = myClassInstance.Call<wdJniObject>("getPlayerById", jlong(0));
  ///
  ///    // --- Manual boxing/unboxing
  ///
  ///     // Java declaration: public Long SomeMethodWithBoxedTypes(Integer param)
  ///
  ///     // This call will fail at runtime: SomeMethodWithBoxedTypes takes Integer, not int, and returns Long, not long
  ///     jint param = 1234;
  ///     jlong result = myClassInstance.Call<jlong>("SomeMethodWithBoxedTypes", param);
  ///
  ///     // Instead, convert parameter into boxed type...
  ///     wdJniObject boxedParam = wdJniClass("java/lang/Integer").CreateInstance(param);
  ///
  ///     // .. Call the method...
  ///     wdJniObject boxedResult = myClassInstance.Call<wdJniObject>("SomeMethodWithBoxedTypes", boxedParam);
  ///
  ///     // ...and unbox the result
  ///     jlong result = boxedResult.Call<jlong>("longValue");
  ///   \endcode
  template <typename Ret = void, typename... Args>
  Ret Call(const char* name, const Args&... args) const;

  /// \brief Returns the value of the field with the given name.
  /// \param name The name of the field.
  ///
  /// The field type must be assignable to the type specified by the template argument.
  /// \example
  ///   \code
  ///     jint intField = myClassInstance.GetField<jint>("IntField");
  ///     wdJniObject objectField = myClassInstance.GetField<wdJniObject>("ObjectField");
  ///   \endcode
  template <typename Ret>
  Ret GetField(const char* name) const;

  /// \brief Sets the value of the field with the given name.
  /// \param name The name of the field.
  /// \param arg The new value of the field.
  ///
  /// The field type must be assignable from the given argument type.
  /// \example
  ///   \code
  ///     myClassInstance.SetField("IntField", jint(1234));
  ///     myClassInstance.SetField("ObjectField", wdJniString("SomeString");
  ///   \endcode
  template <typename T>
  void SetField(const char* name, const T& arg) const;

  /// \brief Calls an instance method by supplying the JNI function signature without performing any type checks.
  template <typename Ret, typename... Args>
  Ret UnsafeCall(const char* name, const char* signature, const Args&... args) const;

  /// \brief Returns the value of the field with the given name and signature without performing any type checks.
  template <typename Ret>
  Ret UnsafeGetField(const char* name, const char* signature) const;

  /// \brief Sets the value of the field with the given name and signature without performing any type checks.
  template <typename T>
  void UnsafeSetField(const char* name, const char* signature, const T& arg) const;

protected:
  inline void Reset();
  inline jobject GetJObject() const;

  static void DumpTypes(const wdJniClass* inputTypes, int N, const wdJniClass* returnType);

  static int CompareMethodSpecificity(const wdJniObject& method1, const wdJniObject& method2);
  static bool IsMethodViable(bool bStatic, const wdJniObject& candidateMethod, const wdJniClass& returnType, wdJniClass* inputTypes, int N);
  static wdJniObject FindMethod(bool bStatic, const char* name, const wdJniClass& type, const wdJniClass& returnType, wdJniClass* inputTypes, int N);

  static int CompareConstructorSpecificity(const wdJniObject& method1, const wdJniObject& method2);
  static bool IsConstructorViable(const wdJniObject& candidateMethod, wdJniClass* inputTypes, int N);
  static wdJniObject FindConstructor(const wdJniClass& type, wdJniClass* inputTypes, int N);

private:
  jobject m_object;
  jclass m_class;
  bool m_own;
};

/// \brief Class holding a local reference to a Java object of type String.
///
/// Conversion to/from const char* uses the modified UTF-8 encoding as described by the JNI specification.
/// This encoding is identical to UTF-8, except that null characters inside the string are encoded as 0xC0, 0x80,
/// and that code points above 0xFFFF are represented by separately encoding each of the two UTF-16 surrogate characters
/// as 3 bytes each.
class WD_FOUNDATION_DLL wdJniString : public wdJniObject
{
public:
  /// \brief Constructs a null String.
  wdJniString();

  /// \brief Constructs a String from a modified UTF-8 string.
  wdJniString(const char* str);

  /// \brief Constructs a String from a JNI string  handle.
  ///
  /// \param string The JNI string handle.
  /// \param ownerShip How the object handle should be managed. See wdJniObject::OwnerShip.
  wdJniString(jstring string, wdJniOwnerShip ownerShip);

  /// \brief Copy constructor. Both instances will reference the same Java String.
  wdJniString(const wdJniString& other);

  /// \brief Move constructor.
  wdJniString(wdJniString&& other);

  /// \brief Assignment operator. Both instances will reference the same Java String.
  wdJniString& operator=(const wdJniString& other);

  /// \brief Move assignment operator.
  wdJniString& operator=(wdJniString&& other);

  /// \brief Destructor.
  virtual ~wdJniString();

  /// \brief Returns the string as a modified UTF-8 string. The pointer is only valid over the lifetime of this object.
  const char* GetData() const;

private:
  const char* m_utf;
};

/// \brief Class holding a local reference to a Java object of type Class.
class WD_FOUNDATION_DLL wdJniClass : public wdJniObject
{
public:
  /// \brief Constructs a null Class.
  wdJniClass();

  /// \brief Constructs a class by searching for the class of the given name.
  ///
  /// \param className
  ///   The class name encoded in the JNI class name format, e.g. "java/lang/Object". Note that this
  ///   is different from the format used by ClassLoader.loadClass, which uses "java.lang.Object".
  ///
  /// If the class could not be found, isNull() will return true and wdJniAttachment::getLastError will return wdJniAttachment::CLASS_NOT_FOUND.
  ///
  /// In order to load classes from the application package, you will have to use the activity's class loader instead.
  /// For example:
  /// \code
  ///   wdJniObject classLoader = attachment.GetActivity().Call<wdJniObject>("getClassLoader");
  ///   wdJniClass myClass = classLoader.Call<wdJniClass>("loadClass", wdJniString("com.myproject.MyClass"));
  /// \endcode
  wdJniClass(const char* className);

  /// \brief Constructs a Class from a JNI class handle.
  ///
  /// \param clazz The JNI class handle.
  /// \param ownerShip How the object handle should be managed. See wdJniObject::OwnerShip.
  wdJniClass(jclass clazz, wdJniOwnerShip ownerShip);

  /// \brief Copy constructor. Both instances will reference the same Java class.
  wdJniClass(const wdJniClass& other);

  /// \brief Move constructor.
  wdJniClass(wdJniClass&& other);

  /// \brief Assignment operator. Both instances will reference the same Java class.
  wdJniClass& operator=(const wdJniClass& other);

  /// \brief Move assignment operator.
  wdJniClass& operator=(wdJniClass&& other);

  /// \brief Returns the JNI handle of the object.
  jclass GetHandle() const;

  /// \brief Constructs an instance of the class type with the given parameters.
  ///
  /// \param args The constructor arguments to pass.
  ///
  ///
  /// \example
  ///   \code
  ///     // Same as Class myClassType = activity.GetClassLoader().LoadClass("com.myproject.MyClass") in Java
  ///     wdJniClass myClassType = activity.Call<wdJniObject>("getClassLoader").Call<wdJniClass>("loadClass", wdJniString("com.myproject.MyClass"));
  ///
  ///     // Same as MyClass myClassInstance = new MyClass(true, "some string", 12345) in Java
  ///     wdJniObject myClassInstance = myClassType.CreateInstance(true, wdJniString("some string"), 12345);
  ///   \endcode
  ///
  /// See wdJniObject::Call for more information on overload resolution and argument conversion.
  template <typename... Args>
  wdJniObject CreateInstance(const Args&... args) const;

  /// \brief Returns true if an instance of this class can be assigned from \c other.
  /// \param other A Java class.
  ///
  /// This call is equivalent to Class.IsAssignableFrom() in Java.
  bool IsAssignableFrom(const wdJniClass& other) const;

  /// \brief Returns true if this class is primitive, i.e., one of boolean, byte, char, short, int, long, float, or double.
  bool IsPrimitive();

  /// \brief Calls a static method of the class type.
  ///
  /// \param name The name of the method to call.
  /// \param args The function arguments to pass.
  ///
  /// See wdJniObject::Call() for details on argument handling.
  ///
  /// \example
  ///   \code
  ///     // To call a static method of a type directly:
  ///     wdJniClass myClassType;
  ///     myClassType.CallStatic("SomeStaticMethod");
  ///     wdJniObject result = myClassType.CallStatic<wdJniObject>("SomeStaticMethodReturningObject");
  ///
  ///     // To call a static method of the type of a class instance:
  ///     wdJniObject myClassInstance;
  ///     myClassInstance.getClass().CallStatic("SomeStaticMethod");
  ///     wdJniObject result = myClassInstance.GetClass().CallStatic<wdJniObject>("SomeStaticMethodReturningObject");
  ///   \endcode
  template <typename Ret = void, typename... Args>
  Ret CallStatic(const char* name, const Args&... args) const;

  /// \brief Returns the value of the static field with the given name.
  /// \param name The name of the static field.
  /// \sa wdJniObject::GetField()
  template <typename Ret>
  Ret GetStaticField(const char* name) const;

  /// \brief Sets the value of the static field with the given name.
  /// \param name The name of the static field.
  /// \param arg The new value of the static field.
  /// \sa wdJniObject::SetField()
  template <typename T>
  void SetStaticField(const char* name, const T& arg) const;

  /// \brief Calls a static method of the class type without performing any type checks.
  template <typename Ret, typename... Args>
  Ret UnsafeCallStatic(const char* name, const char* signature, const Args&... args) const;

  /// \brief Returns the value of the static field with the given name without performing any type checks.
  template <typename Ret>
  Ret UnsafeGetStaticField(const char* name, const char* signature) const;

  /// \brief Sets the value of the static field with the given name without performing any type checks.
  template <typename T>
  void UnsafeSetStaticField(const char* name, const char* signature, const T& arg) const;
};

#  include <Foundation/Basics/Platform/Android/AndroidJni.inl>

#endif
