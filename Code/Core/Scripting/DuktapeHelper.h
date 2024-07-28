#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/String.h>

#ifdef BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT

struct duk_hthread;
using duk_context = duk_hthread;
using duk_c_function = int (*)(duk_context*);

struct nsDuktapeTypeMask
{
  using StorageType = nsUInt32;

  enum Enum
  {
    None = NS_BIT(0),      ///< no value, e.g. invalid index
    Undefined = NS_BIT(1), ///< ECMAScript undefined
    Null = NS_BIT(2),      ///< ECMAScript null
    Bool = NS_BIT(3),      ///< boolean, true or false
    Number = NS_BIT(4),    ///< any number, stored as a double
    String = NS_BIT(5),    ///< ECMAScript string: CESU-8 / extended UTF-8 encoded
    Object = NS_BIT(6),    ///< ECMAScript object: includes objects, arrays, functions, threads
    Buffer = NS_BIT(7),    ///< fixed or dynamic, garbage collected byte buffer
    Pointer = NS_BIT(8)    ///< raw void pointer

  };

  struct Bits
  {
    StorageType None : 1;
    StorageType Undefined : 1;
    StorageType Null : 1;
    StorageType Bool : 1;
    StorageType Number : 1;
    StorageType String : 1;
    StorageType Object : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsDuktapeTypeMask);

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)

#    define NS_DUK_VERIFY_STACK(duk, ExpectedStackChange) \
      duk.EnableStackChangeVerification();                \
      duk.VerifyExpectedStackChange(ExpectedStackChange, NS_SOURCE_FILE, NS_SOURCE_LINE, NS_SOURCE_FUNCTION);

#    define NS_DUK_RETURN_AND_VERIFY_STACK(duk, ReturnCode, ExpectedStackChange) \
      {                                                                          \
        auto ret = ReturnCode;                                                   \
        NS_DUK_VERIFY_STACK(duk, ExpectedStackChange);                           \
        return ret;                                                              \
      }

#    define NS_DUK_RETURN_VOID_AND_VERIFY_STACK(duk, ExpectedStackChange) \
      NS_DUK_VERIFY_STACK(duk, ExpectedStackChange);                      \
      return;


#  else

#    define NS_DUK_VERIFY_STACK(duk, ExpectedStackChange)

#    define NS_DUK_RETURN_AND_VERIFY_STACK(duk, ReturnCode, ExpectedStackChange) return ReturnCode;

#    define NS_DUK_RETURN_VOID_AND_VERIFY_STACK(duk, ExpectedStackChange) return;

#  endif

class NS_CORE_DLL nsDuktapeHelper
{
public:
  nsDuktapeHelper(duk_context* pContext);
  nsDuktapeHelper(const nsDuktapeHelper& rhs);
  ~nsDuktapeHelper();
  void operator=(const nsDuktapeHelper& rhs);

  /// \name Basics
  ///@{

  /// \brief Returns the raw Duktape context for custom operations.
  NS_ALWAYS_INLINE duk_context* GetContext() const { return m_pContext; }

  /// \brief Implicit conversion to duk_context*
  NS_ALWAYS_INLINE operator duk_context*() const { return m_pContext; }

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  void VerifyExpectedStackChange(nsInt32 iExpectedStackChange, const char* szFile, nsUInt32 uiLine, const char* szFunction) const;
#  endif

  ///@}
  /// \name Error Handling
  ///@{

  void Error(const nsFormatString& text);

  void LogStackTrace(nsInt32 iErrorObjIdx);


  ///@}
  /// \name Objects / Stash
  ///@{

  void PopStack(nsUInt32 n = 1);

  void PushGlobalObject();

  void PushGlobalStash();

  nsResult PushLocalObject(const char* szName, nsInt32 iParentObjectIndex = -1);

  ///@}
  /// \name Object Properties
  ///@{

  bool HasProperty(const char* szPropertyName, nsInt32 iParentObjectIndex = -1) const;

  bool GetBoolProperty(const char* szPropertyName, bool bFallback, nsInt32 iParentObjectIndex = -1) const;
  nsInt32 GetIntProperty(const char* szPropertyName, nsInt32 iFallback, nsInt32 iParentObjectIndex = -1) const;
  nsUInt32 GetUIntProperty(const char* szPropertyName, nsUInt32 uiFallback, nsInt32 iParentObjectIndex = -1) const;
  float GetFloatProperty(const char* szPropertyName, float fFallback, nsInt32 iParentObjectIndex = -1) const;
  double GetNumberProperty(const char* szPropertyName, double fFallback, nsInt32 iParentObjectIndex = -1) const;
  const char* GetStringProperty(const char* szPropertyName, const char* szFallback, nsInt32 iParentObjectIndex = -1) const;

  void SetBoolProperty(const char* szPropertyName, bool value, nsInt32 iParentObjectIndex = -1) const;
  void SetNumberProperty(const char* szPropertyName, double value, nsInt32 iParentObjectIndex = -1) const;
  void SetStringProperty(const char* szPropertyName, const char* value, nsInt32 iParentObjectIndex = -1) const;

  /// \note If a negative parent index is given, the parent object taken is actually ParentIdx - 1 (obj at idx -1 is the custom object to use)
  void SetCustomProperty(const char* szPropertyName, nsInt32 iParentObjectIndex = -1) const;


  ///@}
  /// \name Global State
  ///@{

  void StorePointerInStash(const char* szKey, void* pPointer);
  void* RetrievePointerFromStash(const char* szKey) const;

  void StoreStringInStash(const char* szKey, const char* value);
  const char* RetrieveStringFromStash(const char* szKey, const char* szFallback = nullptr) const;

  ///@}
  /// \name Type Checks
  ///@{

  bool IsOfType(nsBitflags<nsDuktapeTypeMask> mask, nsInt32 iStackElement = -1) const;
  bool IsBool(nsInt32 iStackElement = -1) const;
  bool IsNumber(nsInt32 iStackElement = -1) const;
  bool IsString(nsInt32 iStackElement = -1) const;
  bool IsNull(nsInt32 iStackElement = -1) const;
  bool IsUndefined(nsInt32 iStackElement = -1) const;
  bool IsObject(nsInt32 iStackElement = -1) const;
  bool IsBuffer(nsInt32 iStackElement = -1) const;
  bool IsPointer(nsInt32 iStackElement = -1) const;
  bool IsNullOrUndefined(nsInt32 iStackElement = -1) const;

  ///@}
  /// \name C Functions
  ///@{

  void RegisterGlobalFunction(const char* szFunctionName, duk_c_function function, nsUInt8 uiNumArguments, nsInt16 iMagicValue = 0);
  void RegisterGlobalFunctionWithVarArgs(const char* szFunctionName, duk_c_function function, nsInt16 iMagicValue = 0);

  void RegisterObjectFunction(
    const char* szFunctionName, duk_c_function function, nsUInt8 uiNumArguments, nsInt32 iParentObjectIndex = -1, nsInt16 iMagicValue = 0);

  nsResult PrepareGlobalFunctionCall(const char* szFunctionName);
  nsResult PrepareObjectFunctionCall(const char* szFunctionName, nsInt32 iParentObjectIndex = -1);
  nsResult CallPreparedFunction();

  nsResult PrepareMethodCall(const char* szMethodName, nsInt32 iParentObjectIndex = -1);
  nsResult CallPreparedMethod();


  ///@}
  /// \name Values / Parameters
  ///@{

  void PushInt(nsInt32 iParam);
  void PushUInt(nsUInt32 uiParam);
  void PushBool(bool bParam);
  void PushNumber(double fParam);
  void PushString(const nsStringView& sParam);
  void PushNull();
  void PushUndefined();
  void PushCustom(nsUInt32 uiNum = 1);

  bool GetBoolValue(nsInt32 iStackElement, bool bFallback = false) const;
  nsInt32 GetIntValue(nsInt32 iStackElement, nsInt32 iFallback = 0) const;
  nsUInt32 GetUIntValue(nsInt32 iStackElement, nsUInt32 uiFallback = 0) const;
  float GetFloatValue(nsInt32 iStackElement, float fFallback = 0) const;
  double GetNumberValue(nsInt32 iStackElement, double fFallback = 0) const;
  const char* GetStringValue(nsInt32 iStackElement, const char* szFallback = "") const;

  ///@}
  /// \name Executing Scripts
  ///@{

  nsResult ExecuteString(const char* szString, const char* szDebugName = "eval");

  nsResult ExecuteStream(nsStreamReader& inout_stream, const char* szDebugName);

  nsResult ExecuteFile(const char* szFile);

  ///@}

public:
#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  void EnableStackChangeVerification() const;
#  endif


protected:
  duk_context* m_pContext = nullptr;
  nsInt32 m_iPushedValues = 0;

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  nsInt32 m_iStackTopAtStart = -1000;
  mutable bool m_bVerifyStackChange = false;

#  endif
};

#endif
