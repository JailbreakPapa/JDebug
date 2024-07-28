#pragma once

#include <Core/CoreDLL.h>
#include <Core/Scripting/DuktapeHelper.h>

#ifdef BUILDSYSTEM_ENABLE_DUKTAPE_SUPPORT

class NS_CORE_DLL nsDuktapeFunction final : public nsDuktapeHelper
{
public:
  nsDuktapeFunction(duk_context* pExistingContext);
  ~nsDuktapeFunction();

  /// \name Retrieving function parameters
  ///@{

  /// Returns how many Parameters were passed to the called C-Function.
  nsUInt32 GetNumVarArgFunctionParameters() const;

  nsInt16 GetFunctionMagicValue() const;

  ///@}

  /// \name Returning values from C function
  ///@{

  nsInt32 ReturnVoid();
  nsInt32 ReturnNull();
  nsInt32 ReturnUndefined();
  nsInt32 ReturnBool(bool value);
  nsInt32 ReturnInt(nsInt32 value);
  nsInt32 ReturnUInt(nsUInt32 value);
  nsInt32 ReturnFloat(float value);
  nsInt32 ReturnNumber(double value);
  nsInt32 ReturnString(nsStringView value);
  nsInt32 ReturnCustom();

  ///@}

private:
  bool m_bDidReturnValue = false;
};

#endif
