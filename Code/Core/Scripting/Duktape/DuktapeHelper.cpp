#include <Core/CorePCH.h>

#include <Core/Scripting/DuktapeHelper.h>

#ifdef BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT

#  include <Duktape/duktape.h>
#  include <Foundation/IO/FileSystem/FileReader.h>

NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::None == DUK_TYPE_MASK_NONE);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::Undefined == DUK_TYPE_MASK_UNDEFINED);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::Null == DUK_TYPE_MASK_NULL);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::Bool == DUK_TYPE_MASK_BOOLEAN);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::Number == DUK_TYPE_MASK_NUMBER);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::String == DUK_TYPE_MASK_STRING);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::Object == DUK_TYPE_MASK_OBJECT);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::Buffer == DUK_TYPE_MASK_BUFFER);
NS_CHECK_AT_COMPILETIME(nsDuktapeTypeMask::Pointer == DUK_TYPE_MASK_POINTER);

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)


void nsDuktapeHelper::EnableStackChangeVerification() const
{
  m_bVerifyStackChange = true;
}

#  endif

nsDuktapeHelper::nsDuktapeHelper(duk_context* pContext)
  : m_pContext(pContext)
{
#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (m_pContext)
  {
    m_bVerifyStackChange = false;
    m_iStackTopAtStart = duk_get_top(m_pContext);
  }
#  endif
}

nsDuktapeHelper::nsDuktapeHelper(const nsDuktapeHelper& rhs)
  : nsDuktapeHelper(rhs.GetContext())
{
}

nsDuktapeHelper::~nsDuktapeHelper() = default;

void nsDuktapeHelper::operator=(const nsDuktapeHelper& rhs)
{
  if (this == &rhs)
    return;

  *this = nsDuktapeHelper(rhs.GetContext());
}

#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
void nsDuktapeHelper::VerifyExpectedStackChange(nsInt32 iExpectedStackChange, const char* szFile, nsUInt32 uiLine, const char* szFunction) const
{
  if (m_bVerifyStackChange && m_pContext)
  {
    const nsInt32 iCurTop = duk_get_top(m_pContext);
    const nsInt32 iStackChange = iCurTop - m_iStackTopAtStart;

    if (iStackChange != iExpectedStackChange)
    {
      nsLog::Error("{}:{} ({}): Stack change {} != {}", szFile, uiLine, szFunction, iStackChange, iExpectedStackChange);
    }
  }
}
#  endif

void nsDuktapeHelper::Error(const nsFormatString& text)
{
  nsStringBuilder tmp;
  duk_error(m_pContext, DUK_ERR_ERROR, text.GetTextCStr(tmp));
}

void nsDuktapeHelper::LogStackTrace(nsInt32 iErrorObjIdx)
{
  if (duk_is_error(m_pContext, iErrorObjIdx))
  {
    NS_LOG_BLOCK("Stack Trace");

    duk_get_prop_string(m_pContext, iErrorObjIdx, "stack");

    const nsStringBuilder stack = duk_safe_to_string(m_pContext, iErrorObjIdx);
    nsHybridArray<nsStringView, 32> lines;
    stack.Split(false, lines, "\n", "\r");

    for (nsStringView line : lines)
    {
      nsLog::Dev("{}", line);
    }

    duk_pop(m_pContext);
  }
}

void nsDuktapeHelper::PopStack(nsUInt32 n /*= 1*/)
{
  duk_pop_n(m_pContext, n);
}

void nsDuktapeHelper::PushGlobalObject()
{
  duk_push_global_object(m_pContext); // [ global ]
}

void nsDuktapeHelper::PushGlobalStash()
{
  duk_push_global_stash(m_pContext); // [ stash ]
}

nsResult nsDuktapeHelper::PushLocalObject(const char* szName, nsInt32 iParentObjectIndex /* = -1*/)
{
  duk_require_top_index(m_pContext);

  if (duk_get_prop_string(m_pContext, iParentObjectIndex, szName) == false) // [ obj/undef ]
  {
    duk_pop(m_pContext);                                                    // [ ]
    return NS_FAILURE;
  }

  // [ object ]
  return NS_SUCCESS;
}

bool nsDuktapeHelper::HasProperty(const char* szPropertyName, nsInt32 iParentObjectIndex /*= -1*/) const
{
  return duk_is_object(m_pContext, iParentObjectIndex) && duk_has_prop_string(m_pContext, iParentObjectIndex, szPropertyName);
}

bool nsDuktapeHelper::GetBoolProperty(const char* szPropertyName, bool bFallback, nsInt32 iParentObjectIndex /*= -1*/) const
{
  bool result = bFallback;

  if (duk_is_object(m_pContext, iParentObjectIndex))
  {
    if (duk_get_prop_string(m_pContext, iParentObjectIndex, szPropertyName)) // [ value/undef ]
    {
      result = duk_get_boolean_default(m_pContext, -1, bFallback);           // [ value ]
    }

    duk_pop(m_pContext);                                                     // [ ]
  }

  return result;
}

nsInt32 nsDuktapeHelper::GetIntProperty(const char* szPropertyName, nsInt32 iFallback, nsInt32 iParentObjectIndex /*= -1*/) const
{
  nsInt32 result = iFallback;

  if (duk_is_object(m_pContext, iParentObjectIndex))
  {
    if (duk_get_prop_string(m_pContext, iParentObjectIndex, szPropertyName)) // [ value/undef ]
    {
      result = duk_get_int_default(m_pContext, -1, iFallback);               // [ value ]
    }

    duk_pop(m_pContext);                                                     // [ ]
  }

  return result;
}

nsUInt32 nsDuktapeHelper::GetUIntProperty(const char* szPropertyName, nsUInt32 uiFallback, nsInt32 iParentObjectIndex /*= -1*/) const
{
  nsUInt32 result = uiFallback;

  if (duk_is_object(m_pContext, iParentObjectIndex))
  {
    if (duk_get_prop_string(m_pContext, iParentObjectIndex, szPropertyName)) // [ value/undef ]
    {
      result = duk_get_uint_default(m_pContext, -1, uiFallback);             // [ value ]
    }

    duk_pop(m_pContext);                                                     // [ ]
  }

  return result;
}

float nsDuktapeHelper::GetFloatProperty(const char* szPropertyName, float fFallback, nsInt32 iParentObjectIndex /*= -1*/) const
{
  return static_cast<float>(GetNumberProperty(szPropertyName, fFallback, iParentObjectIndex));
}

double nsDuktapeHelper::GetNumberProperty(const char* szPropertyName, double fFallback, nsInt32 iParentObjectIndex /*= -1*/) const
{
  double result = fFallback;

  if (duk_is_object(m_pContext, iParentObjectIndex))
  {
    if (duk_get_prop_string(m_pContext, iParentObjectIndex, szPropertyName)) // [ value/undef ]
    {
      result = duk_get_number_default(m_pContext, -1, fFallback);            // [ value ]
    }

    duk_pop(m_pContext);                                                     // [ ]
  }

  return result;
}

const char* nsDuktapeHelper::GetStringProperty(const char* szPropertyName, const char* szFallback, nsInt32 iParentObjectIndex /*= -1*/) const
{
  const char* result = szFallback;

  if (duk_is_object(m_pContext, iParentObjectIndex))
  {
    if (duk_get_prop_string(m_pContext, iParentObjectIndex, szPropertyName)) // [ value/undef ]
    {
      result = duk_get_string_default(m_pContext, -1, szFallback);           // [ value ]
    }

    duk_pop(m_pContext);                                                     // [ ]
  }

  return result;
}

void nsDuktapeHelper::SetBoolProperty(const char* szPropertyName, bool value, nsInt32 iParentObjectIndex /*= -1*/) const
{
  nsDuktapeHelper duk(m_pContext);

  duk_push_boolean(m_pContext, value);                                       // [ value ]

  if (iParentObjectIndex >= 0)
    duk_put_prop_string(m_pContext, iParentObjectIndex, szPropertyName);     // [ ]
  else
    duk_put_prop_string(m_pContext, iParentObjectIndex - 1, szPropertyName); // [ ]

  NS_DUK_RETURN_VOID_AND_VERIFY_STACK(duk, 0);
}

void nsDuktapeHelper::SetNumberProperty(const char* szPropertyName, double value, nsInt32 iParentObjectIndex /*= -1*/) const
{
  nsDuktapeHelper duk(m_pContext);

  duk_push_number(m_pContext, value);                                        // [ value ]

  if (iParentObjectIndex >= 0)
    duk_put_prop_string(m_pContext, iParentObjectIndex, szPropertyName);     // [ ]
  else
    duk_put_prop_string(m_pContext, iParentObjectIndex - 1, szPropertyName); // [ ]

  NS_DUK_RETURN_VOID_AND_VERIFY_STACK(duk, 0);
}

void nsDuktapeHelper::SetStringProperty(const char* szPropertyName, const char* value, nsInt32 iParentObjectIndex /*= -1*/) const
{
  nsDuktapeHelper duk(m_pContext);

  duk_push_string(m_pContext, value);                                        // [ value ]

  if (iParentObjectIndex >= 0)
    duk_put_prop_string(m_pContext, iParentObjectIndex, szPropertyName);     // [ ]
  else
    duk_put_prop_string(m_pContext, iParentObjectIndex - 1, szPropertyName); // [ ]

  NS_DUK_RETURN_VOID_AND_VERIFY_STACK(duk, 0);
}

void nsDuktapeHelper::SetCustomProperty(const char* szPropertyName, nsInt32 iParentObjectIndex /*= -1*/) const
{
  nsDuktapeHelper duk(m_pContext);                                           // [ value ]

  if (iParentObjectIndex >= 0)
    duk_put_prop_string(m_pContext, iParentObjectIndex, szPropertyName);     // [ ]
  else
    duk_put_prop_string(m_pContext, iParentObjectIndex - 1, szPropertyName); // [ ]

  NS_DUK_RETURN_VOID_AND_VERIFY_STACK(duk, -1);
}

void nsDuktapeHelper::StorePointerInStash(const char* szKey, void* pPointer)
{
  duk_push_global_stash(m_pContext);                                                      // [ stash ]
  *reinterpret_cast<void**>(duk_push_fixed_buffer(m_pContext, sizeof(void*))) = pPointer; // [ stash buffer ]
  duk_put_prop_string(m_pContext, -2, szKey);                                             // [ stash ]
  duk_pop(m_pContext);                                                                    // [ ]
}

void* nsDuktapeHelper::RetrievePointerFromStash(const char* szKey) const
{
  void* pPointer = nullptr;

  duk_push_global_stash(m_pContext);              // [ stash ]

  if (duk_get_prop_string(m_pContext, -1, szKey)) // [ stash obj/undef ]
  {
    NS_ASSERT_DEBUG(duk_is_buffer(m_pContext, -1), "Object '{}' in stash is not a buffer", szKey);

    pPointer = *reinterpret_cast<void**>(duk_get_buffer(m_pContext, -1, nullptr)); // [ stash obj/undef ]
  }

  duk_pop_2(m_pContext);                                                           // [ ]

  return pPointer;
}

void nsDuktapeHelper::StoreStringInStash(const char* szKey, const char* value)
{
  duk_push_global_stash(m_pContext);          // [ stash ]
  duk_push_string(m_pContext, value);         // [ stash value ]
  duk_put_prop_string(m_pContext, -2, szKey); // [ stash ]
  duk_pop(m_pContext);                        // [ ]
}

const char* nsDuktapeHelper::RetrieveStringFromStash(const char* szKey, const char* szFallback /*= nullptr*/) const
{
  duk_push_global_stash(m_pContext);               // [ stash ]

  if (!duk_get_prop_string(m_pContext, -1, szKey)) // [ stash string/undef ]
  {
    duk_pop_2(m_pContext);                         // [ ]
    return szFallback;
  }

  szFallback = duk_get_string_default(m_pContext, -1, szFallback); // [ stash string ]
  duk_pop_2(m_pContext);                                           // [ ]

  return szFallback;
}

bool nsDuktapeHelper::IsOfType(nsBitflags<nsDuktapeTypeMask> mask, nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, mask.GetValue());
}

bool nsDuktapeHelper::IsBool(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_BOOLEAN);
}

bool nsDuktapeHelper::IsNumber(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_NUMBER);
}

bool nsDuktapeHelper::IsString(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_STRING);
}

bool nsDuktapeHelper::IsNull(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_NULL);
}

bool nsDuktapeHelper::IsUndefined(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_UNDEFINED);
}

bool nsDuktapeHelper::IsObject(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_OBJECT);
}

bool nsDuktapeHelper::IsBuffer(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_BUFFER);
}

bool nsDuktapeHelper::IsPointer(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_POINTER);
}

bool nsDuktapeHelper::IsNullOrUndefined(nsInt32 iStackElement /*= -1*/) const
{
  return duk_check_type_mask(m_pContext, iStackElement, DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_UNDEFINED);
}

void nsDuktapeHelper::RegisterGlobalFunction(
  const char* szFunctionName, duk_c_function function, nsUInt8 uiNumArguments, nsInt16 iMagicValue /*= 0*/)
{
  // TODO: could store iFuncIdx for faster function calls

  duk_push_global_object(m_pContext);                                                // [ global ]
  /*const int iFuncIdx =*/duk_push_c_function(m_pContext, function, uiNumArguments); // [ global func ]
  duk_set_magic(m_pContext, -1, iMagicValue);                                        // [ global func ]
  duk_put_prop_string(m_pContext, -2, szFunctionName);                               // [ global ]
  duk_pop(m_pContext);                                                               // [ ]
}

void nsDuktapeHelper::RegisterGlobalFunctionWithVarArgs(const char* szFunctionName, duk_c_function function, nsInt16 iMagicValue /*= 0*/)
{
  // TODO: could store iFuncIdx for faster function calls

  duk_push_global_object(m_pContext);                                             // [ global ]
  /*const int iFuncIdx =*/duk_push_c_function(m_pContext, function, DUK_VARARGS); // [ global func ]
  duk_set_magic(m_pContext, -1, iMagicValue);                                     // [ global func ]
  duk_put_prop_string(m_pContext, -2, szFunctionName);                            // [ global ]
  duk_pop(m_pContext);                                                            // [ ]
}

void nsDuktapeHelper::RegisterObjectFunction(
  const char* szFunctionName, duk_c_function function, nsUInt8 uiNumArguments, nsInt32 iParentObjectIndex /*= -1*/, nsInt16 iMagicValue /*= 0*/)
{
  /*const int iFuncIdx =*/duk_push_c_function(m_pContext, function, uiNumArguments); // [ func ]
  duk_set_magic(m_pContext, -1, iMagicValue);                                        // [ func ]

  if (iParentObjectIndex < 0)
  {
    duk_put_prop_string(m_pContext, iParentObjectIndex - 1, szFunctionName); // [ ]
  }
  else
  {
    duk_put_prop_string(m_pContext, iParentObjectIndex, szFunctionName); // [ ]
  }
}

nsResult nsDuktapeHelper::PrepareGlobalFunctionCall(const char* szFunctionName)
{
  if (!duk_get_global_string(m_pContext, szFunctionName)) // [ func/undef ]
    goto failure;

  if (!duk_is_function(m_pContext, -1))                   // [ func ]
    goto failure;

  m_iPushedValues = 0;
  return NS_SUCCESS;   // [ func ]

failure:
  duk_pop(m_pContext); // [ ]
  return NS_FAILURE;
}

nsResult nsDuktapeHelper::PrepareObjectFunctionCall(const char* szFunctionName, nsInt32 iParentObjectIndex /*= -1*/)
{
  duk_require_top_index(m_pContext);

  if (!duk_get_prop_string(m_pContext, iParentObjectIndex, szFunctionName)) // [ func/undef ]
    goto failure;

  if (!duk_is_function(m_pContext, -1))                                     // [ func ]
    goto failure;

  m_iPushedValues = 0;
  return NS_SUCCESS;   // [ func ]

failure:
  duk_pop(m_pContext); // [ ]
  return NS_FAILURE;
}

nsResult nsDuktapeHelper::CallPreparedFunction()
{
  if (duk_pcall(m_pContext, m_iPushedValues) == DUK_EXEC_SUCCESS) // [ func n-args ] -> [ result/error ]
  {
    return NS_SUCCESS;                                            // [ result ]
  }
  else
  {
    nsLog::Error("[duktape]{}", duk_safe_to_string(m_pContext, -1));

    LogStackTrace(-1);

    return NS_FAILURE; // [ error ]
  }
}

nsResult nsDuktapeHelper::PrepareMethodCall(const char* szMethodName, nsInt32 iParentObjectIndex /*= -1*/)
{
  if (!duk_get_prop_string(m_pContext, iParentObjectIndex, szMethodName)) // [ func/undef ]
    goto failure;

  if (!duk_is_function(m_pContext, -1))                                   // [ func ]
    goto failure;

  if (iParentObjectIndex < 0)
  {
    duk_dup(m_pContext, iParentObjectIndex - 1); // [ func this ]
  }
  else
  {
    duk_dup(m_pContext, iParentObjectIndex); // [ func this ]
  }

  m_iPushedValues = 0;
  return NS_SUCCESS;   // [ func this ]

failure:
  duk_pop(m_pContext); // [ ]
  return NS_FAILURE;
}

nsResult nsDuktapeHelper::CallPreparedMethod()
{
  if (duk_pcall_method(m_pContext, m_iPushedValues) == DUK_EXEC_SUCCESS) // [ func this n-args ] -> [ result/error ]
  {
    return NS_SUCCESS;                                                   // [ result ]
  }
  else
  {
    nsLog::Error("[duktape]{}", duk_safe_to_string(m_pContext, -1));

    LogStackTrace(-1);

    return NS_FAILURE; // [ error ]
  }
}

void nsDuktapeHelper::PushInt(nsInt32 iParam)
{
  duk_push_int(m_pContext, iParam); // [ value ]
  ++m_iPushedValues;
}

void nsDuktapeHelper::PushUInt(nsUInt32 uiParam)
{
  duk_push_uint(m_pContext, uiParam); // [ value ]
  ++m_iPushedValues;
}

void nsDuktapeHelper::PushBool(bool bParam)
{
  duk_push_boolean(m_pContext, bParam); // [ value ]
  ++m_iPushedValues;
}

void nsDuktapeHelper::PushNumber(double fParam)
{
  duk_push_number(m_pContext, fParam); // [ value ]
  ++m_iPushedValues;
}

void nsDuktapeHelper::PushString(const nsStringView& sParam)
{
  duk_push_lstring(m_pContext, sParam.GetStartPointer(), sParam.GetElementCount()); // [ value ]
  ++m_iPushedValues;
}

void nsDuktapeHelper::PushNull()
{
  duk_push_null(m_pContext); // [ null ]
  ++m_iPushedValues;
}

void nsDuktapeHelper::PushUndefined()
{
  duk_push_undefined(m_pContext); // [ undefined ]
  ++m_iPushedValues;
}

void nsDuktapeHelper::PushCustom(nsUInt32 uiNum)
{
  m_iPushedValues += uiNum;
}

bool nsDuktapeHelper::GetBoolValue(nsInt32 iStackElement, bool bFallback /*= false*/) const
{
  return duk_get_boolean_default(m_pContext, iStackElement, bFallback);
}

nsInt32 nsDuktapeHelper::GetIntValue(nsInt32 iStackElement, nsInt32 iFallback /*= 0*/) const
{
  return duk_get_int_default(m_pContext, iStackElement, iFallback);
}

nsUInt32 nsDuktapeHelper::GetUIntValue(nsInt32 iStackElement, nsUInt32 uiFallback /*= 0*/) const
{
  return duk_get_uint_default(m_pContext, iStackElement, uiFallback);
}

float nsDuktapeHelper::GetFloatValue(nsInt32 iStackElement, float fFallback /*= 0*/) const
{
  return static_cast<float>(duk_get_number_default(m_pContext, iStackElement, fFallback));
}

double nsDuktapeHelper::GetNumberValue(nsInt32 iStackElement, double fFallback /*= 0*/) const
{
  return duk_get_number_default(m_pContext, iStackElement, fFallback);
}

const char* nsDuktapeHelper::GetStringValue(nsInt32 iStackElement, const char* szFallback /*= ""*/) const
{
  return duk_get_string_default(m_pContext, iStackElement, szFallback);
}

nsResult nsDuktapeHelper::ExecuteString(const char* szString, const char* szDebugName /*= "eval"*/)
{
  duk_push_string(m_pContext, szDebugName);                       // [ filename ]
  if (duk_pcompile_string_filename(m_pContext, 0, szString) != 0) // [ function/error ]
  {
    NS_LOG_BLOCK("DukTape::ExecuteString", "Compilation failed");

    nsLog::Error("[duktape]{}", duk_safe_to_string(m_pContext, -1)); // [ error ]

    LogStackTrace(-1);

    // TODO: print out line by line
    nsLog::Info("[duktape]Source: {0}", szString);

    duk_pop(m_pContext); // [ ]
    return NS_FAILURE;
  }

  // [ function ]

  if (duk_pcall(m_pContext, 0) != DUK_EXEC_SUCCESS) // [ result/error ]
  {
    NS_LOG_BLOCK("DukTape::ExecuteString", "Execution failed");

    nsLog::Error("[duktape]{}", duk_safe_to_string(m_pContext, -1)); // [ error ]

    LogStackTrace(-1);

    // TODO: print out line by line
    nsLog::Info("[duktape]Source: {0}", szString);

    duk_pop(m_pContext); // [ ]
    return NS_FAILURE;
  }

  duk_pop(m_pContext); // [ ]
  return NS_SUCCESS;
}

nsResult nsDuktapeHelper::ExecuteStream(nsStreamReader& inout_stream, const char* szDebugName)
{
  nsStringBuilder source;
  source.ReadAll(inout_stream);

  return ExecuteString(source, szDebugName);
}

nsResult nsDuktapeHelper::ExecuteFile(const char* szFile)
{
  nsFileReader file;
  NS_SUCCEED_OR_RETURN(file.Open(szFile));

  return ExecuteStream(file, szFile);
}

#endif
