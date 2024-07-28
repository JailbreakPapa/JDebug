#include <RendererCore/RendererCorePCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Types/Variant.h>
#include <Foundation/Utilities/ConversionUtils.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererCore/ShaderCompiler/ShaderParser.h>

using namespace nsTokenParseUtils;

namespace
{
  static nsHashTable<nsStringView, const nsRTTI*> s_NameToTypeTable;
  static nsHashTable<nsStringView, nsEnum<nsGALShaderResourceType>> s_NameToDescriptorTable;
  static nsHashTable<nsStringView, nsEnum<nsGALShaderTextureType>> s_NameToTextureTable;

  void InitializeTables()
  {
    if (!s_NameToTypeTable.IsEmpty())
      return;

    s_NameToTypeTable.Insert("float", nsGetStaticRTTI<float>());
    s_NameToTypeTable.Insert("float2", nsGetStaticRTTI<nsVec2>());
    s_NameToTypeTable.Insert("float3", nsGetStaticRTTI<nsVec3>());
    s_NameToTypeTable.Insert("float4", nsGetStaticRTTI<nsVec4>());
    s_NameToTypeTable.Insert("int", nsGetStaticRTTI<int>());
    s_NameToTypeTable.Insert("int2", nsGetStaticRTTI<nsVec2I32>());
    s_NameToTypeTable.Insert("int3", nsGetStaticRTTI<nsVec3I32>());
    s_NameToTypeTable.Insert("int4", nsGetStaticRTTI<nsVec4I32>());
    s_NameToTypeTable.Insert("uint", nsGetStaticRTTI<nsUInt32>());
    s_NameToTypeTable.Insert("uint2", nsGetStaticRTTI<nsVec2U32>());
    s_NameToTypeTable.Insert("uint3", nsGetStaticRTTI<nsVec3U32>());
    s_NameToTypeTable.Insert("uint4", nsGetStaticRTTI<nsVec4U32>());
    s_NameToTypeTable.Insert("bool", nsGetStaticRTTI<bool>());
    s_NameToTypeTable.Insert("Color", nsGetStaticRTTI<nsColor>());
    /// \todo Are we going to support linear UB colors ?
    s_NameToTypeTable.Insert("Texture2D", nsGetStaticRTTI<nsString>());
    s_NameToTypeTable.Insert("Texture3D", nsGetStaticRTTI<nsString>());
    s_NameToTypeTable.Insert("TextureCube", nsGetStaticRTTI<nsString>());

    s_NameToDescriptorTable.Insert("cbuffer"_nssv, nsGALShaderResourceType::ConstantBuffer);
    s_NameToDescriptorTable.Insert("ConstantBuffer"_nssv, nsGALShaderResourceType::ConstantBuffer);
    s_NameToDescriptorTable.Insert("SamplerState"_nssv, nsGALShaderResourceType::Sampler);
    s_NameToDescriptorTable.Insert("SamplerComparisonState"_nssv, nsGALShaderResourceType::Sampler);
    s_NameToDescriptorTable.Insert("Texture1D"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("Texture1DArray"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("Texture2D"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("Texture2DArray"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("Texture2DMS"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("Texture2DMSArray"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("Texture3D"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("TextureCube"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("TextureCubeArray"_nssv, nsGALShaderResourceType::Texture);
    s_NameToDescriptorTable.Insert("Buffer"_nssv, nsGALShaderResourceType::TexelBuffer);
    s_NameToDescriptorTable.Insert("StructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBuffer);
    s_NameToDescriptorTable.Insert("ByteAddressBuffer"_nssv, nsGALShaderResourceType::StructuredBuffer);
    s_NameToDescriptorTable.Insert("RWTexture1D"_nssv, nsGALShaderResourceType::TextureRW);
    s_NameToDescriptorTable.Insert("RWTexture1DArray"_nssv, nsGALShaderResourceType::TextureRW);
    s_NameToDescriptorTable.Insert("RWTexture2D"_nssv, nsGALShaderResourceType::TextureRW);
    s_NameToDescriptorTable.Insert("RWTexture2DArray"_nssv, nsGALShaderResourceType::TextureRW);
    s_NameToDescriptorTable.Insert("RWTexture3D"_nssv, nsGALShaderResourceType::TextureRW);
    s_NameToDescriptorTable.Insert("RWBuffer"_nssv, nsGALShaderResourceType::TexelBufferRW);
    s_NameToDescriptorTable.Insert("RWStructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);
    s_NameToDescriptorTable.Insert("RWByteAddressBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);
    s_NameToDescriptorTable.Insert("AppendStructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);
    s_NameToDescriptorTable.Insert("ConsumeStructuredBuffer"_nssv, nsGALShaderResourceType::StructuredBufferRW);

    s_NameToTextureTable.Insert("Texture1D"_nssv, nsGALShaderTextureType::Texture1D);
    s_NameToTextureTable.Insert("Texture1DArray"_nssv, nsGALShaderTextureType::Texture1DArray);
    s_NameToTextureTable.Insert("Texture2D"_nssv, nsGALShaderTextureType::Texture2D);
    s_NameToTextureTable.Insert("Texture2DArray"_nssv, nsGALShaderTextureType::Texture2DArray);
    s_NameToTextureTable.Insert("Texture2DMS"_nssv, nsGALShaderTextureType::Texture2DMS);
    s_NameToTextureTable.Insert("Texture2DMSArray"_nssv, nsGALShaderTextureType::Texture2DMSArray);
    s_NameToTextureTable.Insert("Texture3D"_nssv, nsGALShaderTextureType::Texture3D);
    s_NameToTextureTable.Insert("TextureCube"_nssv, nsGALShaderTextureType::TextureCube);
    s_NameToTextureTable.Insert("TextureCubeArray"_nssv, nsGALShaderTextureType::TextureCubeArray);
    s_NameToTextureTable.Insert("RWTexture1D"_nssv, nsGALShaderTextureType::Texture1D);
    s_NameToTextureTable.Insert("RWTexture1DArray"_nssv, nsGALShaderTextureType::Texture1DArray);
    s_NameToTextureTable.Insert("RWTexture2D"_nssv, nsGALShaderTextureType::Texture2D);
    s_NameToTextureTable.Insert("RWTexture2DArray"_nssv, nsGALShaderTextureType::Texture2DArray);
    s_NameToTextureTable.Insert("RWTexture3D"_nssv, nsGALShaderTextureType::Texture3D);
  }

  const nsRTTI* GetType(const char* szType)
  {
    InitializeTables();

    const nsRTTI* pType = nullptr;
    s_NameToTypeTable.TryGetValue(szType, pType);
    return pType;
  }

  nsVariant ParseValue(const TokenStream& tokens, nsUInt32& ref_uiCurToken)
  {
    nsUInt32 uiValueToken = ref_uiCurToken;

    if (Accept(tokens, ref_uiCurToken, nsTokenType::String1, &uiValueToken) || Accept(tokens, ref_uiCurToken, nsTokenType::String2, &uiValueToken))
    {
      nsStringBuilder sValue = tokens[uiValueToken]->m_DataView;
      sValue.Trim("\"'");

      return nsVariant(sValue.GetData());
    }

    if (Accept(tokens, ref_uiCurToken, nsTokenType::Integer, &uiValueToken))
    {
      nsString sValue = tokens[uiValueToken]->m_DataView;

      nsInt64 iValue = 0;
      if (sValue.StartsWith_NoCase("0x"))
      {
        nsUInt32 uiValue32 = 0;
        nsConversionUtils::ConvertHexStringToUInt32(sValue, uiValue32).IgnoreResult();

        iValue = uiValue32;
      }
      else
      {
        nsConversionUtils::StringToInt64(sValue, iValue).IgnoreResult();
      }

      return nsVariant(iValue);
    }

    if (Accept(tokens, ref_uiCurToken, nsTokenType::Float, &uiValueToken))
    {
      nsString sValue = tokens[uiValueToken]->m_DataView;

      double fValue = 0;
      nsConversionUtils::StringToFloat(sValue, fValue).IgnoreResult();

      return nsVariant(fValue);
    }

    if (Accept(tokens, ref_uiCurToken, "true", &uiValueToken) || Accept(tokens, ref_uiCurToken, "false", &uiValueToken))
    {
      bool bValue = tokens[uiValueToken]->m_DataView == "true";
      return nsVariant(bValue);
    }

    auto& dataView = tokens[ref_uiCurToken]->m_DataView;
    if (tokens[ref_uiCurToken]->m_iType == nsTokenType::Identifier && nsStringUtils::IsValidIdentifierName(dataView.GetStartPointer(), dataView.GetEndPointer()))
    {
      // complex type constructor
      const nsRTTI* pType = nullptr;
      if (!s_NameToTypeTable.TryGetValue(dataView, pType))
      {
        nsLog::Error("Invalid type name '{}'", dataView);
        return nsVariant();
      }

      ++ref_uiCurToken;
      Accept(tokens, ref_uiCurToken, "(");

      nsHybridArray<nsVariant, 8> constructorArgs;

      while (!Accept(tokens, ref_uiCurToken, ")"))
      {
        nsVariant value = ParseValue(tokens, ref_uiCurToken);
        if (value.IsValid())
        {
          constructorArgs.PushBack(value);
        }
        else
        {
          nsLog::Error("Invalid arguments for constructor '{}'", pType->GetTypeName());
          return NS_FAILURE;
        }

        Accept(tokens, ref_uiCurToken, ",");
      }

      // find matching constructor
      auto functions = pType->GetFunctions();
      for (auto pFunc : functions)
      {
        if (pFunc->GetFunctionType() == nsFunctionType::Constructor && pFunc->GetArgumentCount() == constructorArgs.GetCount())
        {
          nsHybridArray<nsVariant, 8> convertedArgs;
          bool bAllArgsValid = true;

          for (nsUInt32 uiArg = 0; uiArg < pFunc->GetArgumentCount(); ++uiArg)
          {
            const nsRTTI* pArgType = pFunc->GetArgumentType(uiArg);
            nsResult conversionResult = NS_FAILURE;
            convertedArgs.PushBack(constructorArgs[uiArg].ConvertTo(pArgType->GetVariantType(), &conversionResult));
            if (conversionResult.Failed())
            {
              bAllArgsValid = false;
              break;
            }
          }

          if (bAllArgsValid)
          {
            nsVariant result;
            pFunc->Execute(nullptr, convertedArgs, result);

            if (result.IsValid())
            {
              return result;
            }
          }
        }
      }
    }

    return nsVariant();
  }

  nsResult ParseAttribute(const TokenStream& tokens, nsUInt32& ref_uiCurToken, nsShaderParser::ParameterDefinition& out_parameterDefinition)
  {
    if (!Accept(tokens, ref_uiCurToken, "@"))
    {
      return NS_FAILURE;
    }

    nsUInt32 uiTypeToken = ref_uiCurToken;
    if (!Accept(tokens, ref_uiCurToken, nsTokenType::Identifier, &uiTypeToken))
    {
      return NS_FAILURE;
    }

    nsShaderParser::AttributeDefinition& attributeDef = out_parameterDefinition.m_Attributes.ExpandAndGetRef();
    attributeDef.m_sType = tokens[uiTypeToken]->m_DataView;

    Accept(tokens, ref_uiCurToken, "(");

    while (!Accept(tokens, ref_uiCurToken, ")"))
    {
      nsVariant value = ParseValue(tokens, ref_uiCurToken);
      if (value.IsValid())
      {
        attributeDef.m_Values.PushBack(value);
      }
      else
      {
        nsLog::Error("Invalid arguments for attribute '{}'", attributeDef.m_sType);
        return NS_FAILURE;
      }

      Accept(tokens, ref_uiCurToken, ",");
    }

    return NS_SUCCESS;
  }

  nsResult ParseParameter(const TokenStream& tokens, nsUInt32& ref_uiCurToken, nsShaderParser::ParameterDefinition& out_parameterDefinition)
  {
    nsUInt32 uiTypeToken = ref_uiCurToken;
    if (!Accept(tokens, ref_uiCurToken, nsTokenType::Identifier, &uiTypeToken))
    {
      return NS_FAILURE;
    }

    out_parameterDefinition.m_sType = tokens[uiTypeToken]->m_DataView;
    out_parameterDefinition.m_pType = GetType(out_parameterDefinition.m_sType);

    nsUInt32 uiNameToken = ref_uiCurToken;
    if (!Accept(tokens, ref_uiCurToken, nsTokenType::Identifier, &uiNameToken))
    {
      return NS_FAILURE;
    }

    out_parameterDefinition.m_sName = tokens[uiNameToken]->m_DataView;

    while (!Accept(tokens, ref_uiCurToken, ";"))
    {
      if (ParseAttribute(tokens, ref_uiCurToken, out_parameterDefinition).Failed())
      {
        return NS_FAILURE;
      }
    }

    return NS_SUCCESS;
  }

  nsResult ParseEnum(const TokenStream& tokens, nsUInt32& ref_uiCurToken, nsShaderParser::EnumDefinition& out_enumDefinition, bool bCheckPrefix)
  {
    if (!Accept(tokens, ref_uiCurToken, "enum"))
    {
      return NS_FAILURE;
    }

    nsUInt32 uiNameToken = ref_uiCurToken;
    if (!Accept(tokens, ref_uiCurToken, nsTokenType::Identifier, &uiNameToken))
    {
      return NS_FAILURE;
    }

    out_enumDefinition.m_sName = tokens[uiNameToken]->m_DataView;
    nsStringBuilder sEnumPrefix(out_enumDefinition.m_sName, "_");

    if (!Accept(tokens, ref_uiCurToken, "{"))
    {
      nsLog::Error("Opening bracket expected for enum definition.");
      return NS_FAILURE;
    }

    nsUInt32 uiDefaultValue = 0;
    nsUInt32 uiCurrentValue = 0;

    while (true)
    {
      nsUInt32 uiValueNameToken = ref_uiCurToken;
      if (!Accept(tokens, ref_uiCurToken, nsTokenType::Identifier, &uiValueNameToken))
      {
        return NS_FAILURE;
      }

      nsStringView sValueName = tokens[uiValueNameToken]->m_DataView;

      if (Accept(tokens, ref_uiCurToken, "="))
      {
        nsUInt32 uiValueToken = ref_uiCurToken;
        Accept(tokens, ref_uiCurToken, nsTokenType::Integer, &uiValueToken);

        nsInt32 iValue = 0;
        if (nsConversionUtils::StringToInt(tokens[uiValueToken]->m_DataView.GetStartPointer(), iValue).Succeeded() && iValue >= 0)
        {
          uiCurrentValue = iValue;
        }
        else
        {
          nsLog::Error("Invalid enum value '{0}'. Only positive numbers are allowed.", tokens[uiValueToken]->m_DataView);
        }
      }

      if (sValueName.IsEqual_NoCase("default"))
      {
        uiDefaultValue = uiCurrentValue;
      }
      else
      {
        if (bCheckPrefix && !sValueName.StartsWith(sEnumPrefix))
        {
          nsLog::Error("Enum value does not start with the expected enum name as prefix: '{0}'", sEnumPrefix);
        }

        auto& ev = out_enumDefinition.m_Values.ExpandAndGetRef();

        const nsStringBuilder sFinalName = sValueName;
        ev.m_sValueName.Assign(sFinalName.GetData());
        ev.m_iValueValue = static_cast<nsInt32>(uiCurrentValue);
      }

      if (Accept(tokens, ref_uiCurToken, ","))
      {
        ++uiCurrentValue;
      }
      else
      {
        break;
      }

      if (Accept(tokens, ref_uiCurToken, "}"))
        goto after_braces;
    }

    if (!Accept(tokens, ref_uiCurToken, "}"))
    {
      nsLog::Error("Closing bracket expected for enum definition.");
      return NS_FAILURE;
    }

  after_braces:

    out_enumDefinition.m_uiDefaultValue = uiDefaultValue;

    Accept(tokens, ref_uiCurToken, ";");

    return NS_SUCCESS;
  }

  void SkipWhitespace(nsStringView& s)
  {
    while (s.IsValid() && nsStringUtils::IsWhiteSpace(s.GetCharacter()))
    {
      ++s;
    }
  }
} // namespace

nsResult nsShaderParser::PreprocessSection(nsStreamReader& inout_stream, nsShaderHelper::nsShaderSections::Enum section, nsArrayPtr<nsString> customDefines, nsStringBuilder& out_sResult)
{
  nsString sContent;
  sContent.ReadAll(inout_stream);

  nsShaderHelper::nsTextSectionizer sections;
  nsShaderHelper::GetShaderSections(sContent, sections);

  nsUInt32 uiFirstLine = 0;
  nsStringView sSectionContent = sections.GetSectionContent(section, uiFirstLine);

  nsPreprocessor pp;
  pp.SetPassThroughPragma(false);
  pp.SetPassThroughLine(false);

  // setup defines
  {
    NS_SUCCEED_OR_RETURN(pp.AddCustomDefine("TRUE 1"));
    NS_SUCCEED_OR_RETURN(pp.AddCustomDefine("FALSE 0"));
    NS_SUCCEED_OR_RETURN(pp.AddCustomDefine("PLATFORM_SHADER ="));

    for (auto& sDefine : customDefines)
    {
      NS_SUCCEED_OR_RETURN(pp.AddCustomDefine(sDefine));
    }
  }

  pp.SetFileOpenFunction([&](nsStringView sAbsoluteFile, nsDynamicArray<nsUInt8>& out_fileContent, nsTimestamp& out_fileModification)
    {
        if (sAbsoluteFile == "SectionContent")
        {
          out_fileContent.PushBackRange(nsMakeArrayPtr((const nsUInt8*)sSectionContent.GetStartPointer(), sSectionContent.GetElementCount()));
          return NS_SUCCESS;
        }

        nsFileReader r;
        if (r.Open(sAbsoluteFile).Failed())
        {
          nsLog::Error("Could not find include file '{0}'", sAbsoluteFile);
          return NS_FAILURE;
        }

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
        nsFileStats stats;
        if (nsFileSystem::GetFileStats(sAbsoluteFile, stats).Succeeded())
        {
          out_fileModification = stats.m_LastModificationTime;
        }
#endif

        nsUInt8 Temp[4096];
        while (nsUInt64 uiRead = r.ReadBytes(Temp, 4096))
        {
          out_fileContent.PushBackRange(nsArrayPtr<nsUInt8>(Temp, (nsUInt32)uiRead));
        }

        return NS_SUCCESS; });

  bool bFoundUndefinedVars = false;
  pp.m_ProcessingEvents.AddEventHandler([&bFoundUndefinedVars](const nsPreprocessor::ProcessingEvent& e)
    {
        if (e.m_Type == nsPreprocessor::ProcessingEvent::EvaluateUnknown)
        {
          bFoundUndefinedVars = true;

          nsLog::Error("Undefined variable is evaluated: '{0}' (File: '{1}', Line: {2}. Only material permutation variables are allowed in material config sections.", e.m_pToken->m_DataView, e.m_pToken->m_File, e.m_pToken->m_uiLine);
        } });

  if (pp.Process("SectionContent", out_sResult, false).Failed() || bFoundUndefinedVars)
  {
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

// static
void nsShaderParser::ParseMaterialParameterSection(nsStreamReader& inout_stream, nsDynamicArray<ParameterDefinition>& out_parameter, nsDynamicArray<EnumDefinition>& out_enumDefinitions)
{
  nsStringBuilder sContent;
  if (PreprocessSection(inout_stream, nsShaderHelper::nsShaderSections::MATERIALPARAMETER, nsArrayPtr<nsString>(), sContent).Failed())
  {
    nsLog::Error("Failed to preprocess material parameter section");
    return;
  }

  nsTokenizer tokenizer;
  tokenizer.Tokenize(nsMakeArrayPtr((const nsUInt8*)sContent.GetData(), sContent.GetElementCount()), nsLog::GetThreadLocalLogSystem());

  TokenStream tokens;
  tokenizer.GetAllLines(tokens);

  nsUInt32 uiCurToken = 0;

  while (!Accept(tokens, uiCurToken, nsTokenType::EndOfFile))
  {
    EnumDefinition enumDef;
    if (ParseEnum(tokens, uiCurToken, enumDef, false).Succeeded())
    {
      NS_ASSERT_DEV(!enumDef.m_sName.IsEmpty(), "");

      out_enumDefinitions.PushBack(std::move(enumDef));
      continue;
    }

    ParameterDefinition paramDef;
    if (ParseParameter(tokens, uiCurToken, paramDef).Succeeded())
    {
      out_parameter.PushBack(std::move(paramDef));
      continue;
    }

    nsLog::Error("Invalid token in material parameter section '{}'", tokens[uiCurToken]->m_DataView);
    break;
  }
}

// static
void nsShaderParser::ParsePermutationSection(nsStreamReader& inout_stream, nsDynamicArray<nsHashedString>& out_permVars, nsDynamicArray<nsPermutationVar>& out_fixedPermVars)
{
  nsString sContent;
  sContent.ReadAll(inout_stream);

  nsShaderHelper::nsTextSectionizer Sections;
  nsShaderHelper::GetShaderSections(sContent.GetData(), Sections);

  nsUInt32 uiFirstLine = 0;
  nsStringView sPermutations = Sections.GetSectionContent(nsShaderHelper::nsShaderSections::PERMUTATIONS, uiFirstLine);
  ParsePermutationSection(sPermutations, out_permVars, out_fixedPermVars);
}

// static
void nsShaderParser::ParsePermutationSection(nsStringView s, nsDynamicArray<nsHashedString>& out_permVars, nsDynamicArray<nsPermutationVar>& out_fixedPermVars)
{
  out_permVars.Clear();
  out_fixedPermVars.Clear();

  nsTokenizer tokenizer;
  tokenizer.Tokenize(nsArrayPtr<const nsUInt8>((const nsUInt8*)s.GetStartPointer(), s.GetElementCount()), nsLog::GetThreadLocalLogSystem());

  enum class State
  {
    Idle,
    HasName,
    HasEqual,
    HasValue
  };

  State state = State::Idle;
  nsStringBuilder sToken, sVarName;

  for (const auto& token : tokenizer.GetTokens())
  {
    if (token.m_iType == nsTokenType::Whitespace || token.m_iType == nsTokenType::BlockComment || token.m_iType == nsTokenType::LineComment)
      continue;

    if (token.m_iType == nsTokenType::String1 || token.m_iType == nsTokenType::String2 || token.m_iType == nsTokenType::RawString1)
    {
      sToken = token.m_DataView;
      nsLog::Error("Strings are not allowed in the permutation section: '{0}'", sToken);
      return;
    }

    if (token.m_iType == nsTokenType::Newline || token.m_iType == nsTokenType::EndOfFile)
    {
      if (state == State::HasEqual)
      {
        nsLog::Error("Missing assignment value in permutation section");
        return;
      }

      if (state == State::HasName)
      {
        out_permVars.ExpandAndGetRef().Assign(sVarName.GetData());
      }

      state = State::Idle;
      continue;
    }

    sToken = token.m_DataView;

    if (token.m_iType == nsTokenType::NonIdentifier)
    {
      if (sToken == "=" && state == State::HasName)
      {
        state = State::HasEqual;
        continue;
      }
    }
    else if (token.m_iType == nsTokenType::Identifier)
    {
      if (state == State::Idle)
      {
        sVarName = sToken;
        state = State::HasName;
        continue;
      }

      if (state == State::HasEqual)
      {
        auto& res = out_fixedPermVars.ExpandAndGetRef();
        res.m_sName.Assign(sVarName.GetData());
        res.m_sValue.Assign(sToken.GetData());
        state = State::HasValue;
        continue;
      }
    }

    nsLog::Error("Invalid permutation section at token '{0}'", sToken);
  }
}

// static
void nsShaderParser::ParsePermutationVarConfig(nsStringView s, nsVariant& out_defaultValue, EnumDefinition& out_enumDefinition)
{
  SkipWhitespace(s);

  nsStringBuilder name;

  if (s.StartsWith("bool"))
  {
    bool bDefaultValue = false;

    const char* szDefaultValue = s.FindSubString("=");
    if (szDefaultValue != nullptr)
    {
      name.SetSubString_FromTo(s.GetStartPointer() + 4, szDefaultValue);

      ++szDefaultValue;
      nsConversionUtils::StringToBool(szDefaultValue, bDefaultValue).IgnoreResult();
    }
    else
    {
      name.SetSubString_FromTo(s.GetStartPointer() + 4, s.GetEndPointer());
    }

    name.Trim(" \t\r\n");
    out_enumDefinition.m_sName = name;
    out_defaultValue = bDefaultValue;
  }
  else if (s.StartsWith("enum"))
  {
    nsTokenizer tokenizer;
    tokenizer.Tokenize(nsArrayPtr<const nsUInt8>((const nsUInt8*)s.GetStartPointer(), s.GetElementCount()), nsLog::GetThreadLocalLogSystem());

    TokenStream tokens;
    tokenizer.GetAllLines(tokens);

    nsUInt32 uiCurToken = 0;
    if (ParseEnum(tokens, uiCurToken, out_enumDefinition, true).Failed())
    {
      nsLog::Error("Invalid enum PermutationVar definition.");
    }
    else
    {
      NS_ASSERT_DEV(!out_enumDefinition.m_sName.IsEmpty(), "");

      out_defaultValue = out_enumDefinition.m_uiDefaultValue;
    }
  }
  else
  {
    nsLog::Error("Unknown permutation var type");
  }
}

nsResult ParseResource(const TokenStream& tokens, nsUInt32& ref_uiCurToken, nsShaderResourceDefinition& out_resourceDefinition)
{
  // Match type
  nsUInt32 uiTypeToken = ref_uiCurToken;
  if (!Accept(tokens, ref_uiCurToken, nsTokenType::Identifier, &uiTypeToken))
  {
    return NS_FAILURE;
  }
  if (!s_NameToDescriptorTable.TryGetValue(tokens[uiTypeToken]->m_DataView, out_resourceDefinition.m_Binding.m_ResourceType))
    return NS_FAILURE;
  s_NameToTextureTable.TryGetValue(tokens[uiTypeToken]->m_DataView, out_resourceDefinition.m_Binding.m_TextureType);

  // Skip optional template
  TokenMatch templatePattern[] = {"<"_nssv, nsTokenType::Identifier, ">"_nssv};
  nsHybridArray<nsUInt32, 8> acceptedTokens;
  Accept(tokens, ref_uiCurToken, templatePattern, &acceptedTokens);

  // Match name
  nsUInt32 uiNameToken = ref_uiCurToken;
  if (!Accept(tokens, ref_uiCurToken, nsTokenType::Identifier, &uiNameToken))
  {
    return NS_FAILURE;
  }
  out_resourceDefinition.m_Binding.m_sName.Assign(tokens[uiNameToken]->m_DataView);
  nsUInt32 uiEndToken = uiNameToken;

  // Match optional array
  TokenMatch arrayPattern[] = {"["_nssv, nsTokenType::Integer, "]"_nssv};
  TokenMatch bindlessPattern[] = {"["_nssv, "]"_nssv};
  if (Accept(tokens, ref_uiCurToken, arrayPattern, &acceptedTokens))
  {
    nsConversionUtils::StringToUInt(tokens[acceptedTokens[1]]->m_DataView, out_resourceDefinition.m_Binding.m_uiArraySize).AssertSuccess("Tokenizer error");
    uiEndToken = acceptedTokens.PeekBack();
  }
  else if (Accept(tokens, ref_uiCurToken, bindlessPattern, &acceptedTokens))
  {
    out_resourceDefinition.m_Binding.m_uiArraySize = 0;
    uiEndToken = acceptedTokens.PeekBack();
  }
  out_resourceDefinition.m_sDeclaration = nsStringView(tokens[uiTypeToken]->m_DataView.GetStartPointer(), tokens[uiEndToken]->m_DataView.GetEndPointer());

  // Match optional register
  TokenMatch slotPattern[] = {":"_nssv, "register"_nssv, "("_nssv, nsTokenType::Identifier, ")"_nssv};
  TokenMatch slotAndSetPattern[] = {":"_nssv, "register"_nssv, "("_nssv, nsTokenType::Identifier, ","_nssv, nsTokenType::Identifier, ")"_nssv};
  if (Accept(tokens, ref_uiCurToken, slotPattern, &acceptedTokens))
  {
    nsStringView sSlot = tokens[acceptedTokens[3]]->m_DataView;
    sSlot.Trim("tsubx");
    if (sSlot.IsEqual_NoCase("AUTO")) // See shader macros in StandardMacros.h
    {
      out_resourceDefinition.m_Binding.m_iSlot = -1;
    }
    else
    {
      nsInt32 iSlot;
      nsConversionUtils::StringToInt(sSlot, iSlot).AssertSuccess("Failed to parse slot index of shader resource");
      out_resourceDefinition.m_Binding.m_iSlot = static_cast<nsInt16>(iSlot);
    }
    uiEndToken = acceptedTokens.PeekBack();
  }
  else if (Accept(tokens, ref_uiCurToken, slotAndSetPattern, &acceptedTokens))
  {
    nsStringView sSlot = tokens[acceptedTokens[3]]->m_DataView;
    sSlot.Trim("tsubx");
    if (sSlot.IsEqual_NoCase("AUTO")) // See shader macros in StandardMacros.h
    {
      out_resourceDefinition.m_Binding.m_iSlot = -1;
    }
    else
    {
      nsInt32 iSlot;
      nsConversionUtils::StringToInt(sSlot, iSlot).AssertSuccess("Failed to parse slot index of shader resource");
      out_resourceDefinition.m_Binding.m_iSlot = static_cast<nsInt16>(iSlot);
    }
    nsStringView sSet = tokens[acceptedTokens[5]]->m_DataView;
    sSet.TrimWordStart("space"_nssv);
    nsInt32 iSet;
    nsConversionUtils::StringToInt(sSet, iSet).AssertSuccess("Failed to parse set index of shader resource");
    out_resourceDefinition.m_Binding.m_iSet = static_cast<nsInt16>(iSet);
    uiEndToken = acceptedTokens.PeekBack();
  }

  out_resourceDefinition.m_sDeclarationAndRegister = nsStringView(tokens[uiTypeToken]->m_DataView.GetStartPointer(), tokens[uiEndToken]->m_DataView.GetEndPointer());
  // Match ; (resource declaration done) or { (constant buffer member declaration starts)
  if (!Accept(tokens, ref_uiCurToken, ";"_nssv) && !Accept(tokens, ref_uiCurToken, "{"_nssv))
    return NS_FAILURE;

  return NS_SUCCESS;
}

void nsShaderParser::ParseShaderResources(nsStringView sShaderStageSource, nsDynamicArray<nsShaderResourceDefinition>& out_resources)
{
  if (sShaderStageSource.IsEmpty())
  {
    out_resources.Clear();
    return;
  }

  InitializeTables();

  nsTokenizer tokenizer;
  tokenizer.SetTreatHashSignAsLineComment(true);
  tokenizer.Tokenize(nsArrayPtr<const nsUInt8>((const nsUInt8*)sShaderStageSource.GetStartPointer(), sShaderStageSource.GetElementCount()), nsLog::GetThreadLocalLogSystem(), false);

  TokenStream tokens;
  tokenizer.GetAllLines(tokens);

  nsUInt32 uiCurToken = 0;

  while (!Accept(tokens, uiCurToken, nsTokenType::EndOfFile))
  {
    nsShaderResourceDefinition resourceDef;
    if (ParseResource(tokens, uiCurToken, resourceDef).Succeeded())
    {
      out_resources.PushBack(std::move(resourceDef));
      continue;
    }
    ++uiCurToken;
  }
}

nsResult nsShaderParser::MergeShaderResourceBindings(const nsShaderProgramData& spd, nsHashTable<nsHashedString, nsShaderResourceBinding>& out_bindings, nsLogInterface* pLog)
{
  nsUInt32 uiSize = 0;
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    uiSize += spd.m_Resources[stage].GetCount();
  }

  out_bindings.Clear();
  out_bindings.Reserve(uiSize);

  nsMap<nsHashedString, const nsShaderResourceDefinition*> resourceFirstOccurence;

  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    for (const nsShaderResourceDefinition& res : spd.m_Resources[stage])
    {
      nsHashedString sName = res.m_Binding.m_sName;
      auto it = out_bindings.Find(sName);
      if (it.IsValid())
      {
        nsShaderResourceBinding& current = it.Value();
        if (current.m_ResourceType != res.m_Binding.m_ResourceType || current.m_TextureType != res.m_Binding.m_TextureType || current.m_uiArraySize != res.m_Binding.m_uiArraySize)
        {
          nsLog::Error(pLog, "A shared shader resource '{}' has a mismatching signatures between stages: '{}' vs '{}'", sName, resourceFirstOccurence.Find(sName).Value()->m_sDeclarationAndRegister, res.m_sDeclarationAndRegister);
          return NS_FAILURE;
        }

        current.m_Stages |= nsGALShaderStageFlags::MakeFromShaderStage((nsGALShaderStage::Enum)stage);
      }
      else
      {
        out_bindings.Insert(sName, res.m_Binding);
        resourceFirstOccurence.Insert(sName, &res);
        out_bindings.Find(sName).Value().m_Stages |= nsGALShaderStageFlags::MakeFromShaderStage((nsGALShaderStage::Enum)stage);
      }
    }
  }
  return NS_SUCCESS;
}

nsResult nsShaderParser::SanityCheckShaderResourceBindings(const nsHashTable<nsHashedString, nsShaderResourceBinding>& bindings, nsLogInterface* pLog)
{
  for (auto it : bindings)
  {
    if (it.Value().m_iSet < 0)
    {
      nsLog::Error(pLog, "Shader resource '{}' does not have a set defined.", it.Key());
      return NS_FAILURE;
    }
    if (it.Value().m_iSlot < 0)
    {
      nsLog::Error(pLog, "Shader resource '{}' does not have a slot defined.", it.Key());
      return NS_FAILURE;
    }
  }
  return NS_SUCCESS;
}

void nsShaderParser::ApplyShaderResourceBindings(nsStringView sPlatform, nsStringView sShaderStageSource, const nsDynamicArray<nsShaderResourceDefinition>& resources, const nsHashTable<nsHashedString, nsShaderResourceBinding>& bindings, const CreateResourceDeclaration& createDeclaration, nsStringBuilder& out_sShaderStageSource)
{
  nsDeque<nsString> partStorage;
  nsHybridArray<nsStringView, 16> parts;

  nsStringBuilder sDeclaration;
  const char* szStart = sShaderStageSource.GetStartPointer();
  for (int i = 0; i < resources.GetCount(); ++i)
  {
    parts.PushBack(nsStringView(szStart, resources[i].m_sDeclarationAndRegister.GetStartPointer()));

    nsShaderResourceBinding* pBinding = nullptr;
    bindings.TryGetValue(resources[i].m_Binding.m_sName, pBinding);

    NS_ASSERT_DEV(pBinding != nullptr, "Every resource should be present in the map.");
    createDeclaration(sPlatform, resources[i].m_sDeclaration, *pBinding, sDeclaration);
    nsString& sStorage = partStorage.ExpandAndGetRef();
    sStorage = sDeclaration;
    parts.PushBack(sStorage);
    szStart = resources[i].m_sDeclarationAndRegister.GetEndPointer();
  }
  parts.PushBack(nsStringView(szStart, sShaderStageSource.GetEndPointer()));

  nsUInt32 uiSize = 0;
  for (const nsStringView& sPart : parts)
    uiSize += sPart.GetElementCount();

  out_sShaderStageSource.Clear();
  out_sShaderStageSource.Reserve(uiSize);

  for (const nsStringView& sPart : parts)
  {
    out_sShaderStageSource.Append(sPart);
  }
}
