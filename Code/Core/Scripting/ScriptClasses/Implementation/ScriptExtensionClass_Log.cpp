#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptAttributes.h>
#include <Core/Scripting/ScriptClasses/ScriptExtensionClass_Log.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsScriptExtensionClass_Log, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(Info, In, "Text", In, "Params")->AddAttributes(new nsDynamicPinAttribute("Params")),
    NS_SCRIPT_FUNCTION_PROPERTY(Warning, In, "Text", In, "Params")->AddAttributes(new nsDynamicPinAttribute("Params")),
    NS_SCRIPT_FUNCTION_PROPERTY(Error, In, "Text", In, "Params")->AddAttributes(new nsDynamicPinAttribute("Params")),
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsScriptExtensionAttribute("Log"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

static nsStringView BuildFormattedText(nsStringView sText, const nsVariantArray& params, nsStringBuilder& ref_sStorage)
{
  nsHybridArray<nsString, 12> stringStorage;
  stringStorage.Reserve(params.GetCount());
  for (auto& param : params)
  {
    stringStorage.PushBack(param.ConvertTo<nsString>());
  }

  nsHybridArray<nsStringView, 12> stringViews;
  stringViews.Reserve(stringStorage.GetCount());
  for (auto& s : stringStorage)
  {
    stringViews.PushBack(s);
  }

  nsFormatString fs(sText);
  return fs.BuildFormattedText(ref_sStorage, stringViews.GetData(), stringViews.GetCount());
}

// static
void nsScriptExtensionClass_Log::Info(nsStringView sText, const nsVariantArray& params)
{
  nsStringBuilder sStorage;
  nsLog::Info(BuildFormattedText(sText, params, sStorage));
}

// static
void nsScriptExtensionClass_Log::Warning(nsStringView sText, const nsVariantArray& params)
{
  nsStringBuilder sStorage;
  nsLog::Warning(BuildFormattedText(sText, params, sStorage));
}

// static
void nsScriptExtensionClass_Log::Error(nsStringView sText, const nsVariantArray& params)
{
  nsStringBuilder sStorage;
  nsLog::Error(BuildFormattedText(sText, params, sStorage));
}


NS_STATICLINK_FILE(Core, Core_Scripting_ScriptClasses_Implementation_ScriptExtensionClass_Log);
