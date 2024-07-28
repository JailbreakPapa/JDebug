#include <Core/CorePCH.h>

#include <Core/Scripting/DuktapeFunction.h>

#ifdef BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT

#  include <Duktape/duk_module_duktape.h>
#  include <Duktape/duktape.h>

nsDuktapeFunction::nsDuktapeFunction(duk_context* pExistingContext)
  : nsDuktapeHelper(pExistingContext)
{
}

nsDuktapeFunction::~nsDuktapeFunction()
{
#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (m_bVerifyStackChange && !m_bDidReturnValue)
  {
    nsLog::Error("You need to call one nsDuktapeFunction::ReturnXY() and return its result from your C function.");
  }
#  endif
}

nsUInt32 nsDuktapeFunction::GetNumVarArgFunctionParameters() const
{
  return duk_get_top(GetContext());
}

nsInt16 nsDuktapeFunction::GetFunctionMagicValue() const
{
  return static_cast<nsInt16>(duk_get_current_magic(GetContext()));
}

nsInt32 nsDuktapeFunction::ReturnVoid()
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  return 0;
}

nsInt32 nsDuktapeFunction::ReturnNull()
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_null(GetContext());
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnUndefined()
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_undefined(GetContext());
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnBool(bool value)
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_boolean(GetContext(), value);
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnInt(nsInt32 value)
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_int(GetContext(), value);
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnUInt(nsUInt32 value)
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_uint(GetContext(), value);
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnFloat(float value)
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_number(GetContext(), value);
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnNumber(double value)
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_number(GetContext(), value);
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnString(nsStringView value)
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  duk_push_lstring(GetContext(), value.GetStartPointer(), value.GetElementCount());
  return 1;
}

nsInt32 nsDuktapeFunction::ReturnCustom()
{
  NS_ASSERT_DEV(!m_bDidReturnValue, "Only one ReturnXYZ function may be called when exiting a C function");
  m_bDidReturnValue = true;
  // push nothing, the user calls this because he pushed something custom already
  return 1;
}

#endif
