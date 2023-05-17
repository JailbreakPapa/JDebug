#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/IO/ChunkStream.h>
#include <Foundation/Logging/Log.h>

namespace
{
  static constexpr const char* s_szOpCodeNames[] = {
    "Nop",

    "",

    "AbsF_R",
    "AbsI_R",
    "SqrtF_R",

    "ExpF_R",
    "LnF_R",
    "Log2F_R",
    "Log2I_R",
    "Log10F_R",
    "Pow2F_R",

    "SinF_R",
    "CosF_R",
    "TanF_R",

    "ASinF_R",
    "ACosF_R",
    "ATanF_R",

    "RoundF_R",
    "FloorF_R",
    "CeilF_R",
    "TruncF_R",

    "NotB_R",
    "NotI_R",

    "IToF_R",
    "FToI_R",

    "",
    "",

    "AddF_RR",
    "AddI_RR",

    "SubF_RR",
    "SubI_RR",

    "MulF_RR",
    "MulI_RR",

    "DivF_RR",
    "DivI_RR",

    "MinF_RR",
    "MinI_RR",

    "MaxF_RR",
    "MaxI_RR",

    "ShlI_RR",
    "ShrI_RR",
    "AndI_RR",
    "XorI_RR",
    "OrI_RR",

    "EqF_RR",
    "EqI_RR",
    "EqB_RR",

    "NEqF_RR",
    "NEqI_RR",
    "NEqB_RR",

    "LtF_RR",
    "LtI_RR",

    "LEqF_RR",
    "LEqI_RR",

    "GtF_RR",
    "GtI_RR",

    "GEqF_RR",
    "GEqI_RR",

    "AndB_RR",
    "OrB_RR",

    "",
    "",

    "AddF_RC",
    "AddI_RC",

    "SubF_RC",
    "SubI_RC",

    "MulF_RC",
    "MulI_RC",

    "DivF_RC",
    "DivI_RC",

    "MinF_RC",
    "MinI_RC",

    "MaxF_RC",
    "MaxI_RC",

    "ShlI_RC",
    "ShrI_RC",
    "AndI_RC",
    "XorI_RC",
    "OrI_RC",

    "EqF_RC",
    "EqI_RC",
    "EqB_RC",

    "NEqF_RC",
    "NEqI_RC",
    "NEqB_RC",

    "LtF_RC",
    "LtI_RC",

    "LEqF_RC",
    "LEqI_RC",

    "GtF_RC",
    "GtI_RC",

    "GEqF_RC",
    "GEqI_RC",

    "AndB_RC",
    "OrB_RC",

    "",
    "",

    "SelF_RRR",
    "SelI_RRR",
    "SelB_RRR",

    "",
    "",

    "MovX_R",
    "MovX_C",
    "LoadF",
    "LoadI",
    "StoreF",
    "StoreI",

    "Call",

    "",
  };

  static_assert(WD_ARRAY_SIZE(s_szOpCodeNames) == wdExpressionByteCode::OpCode::Count);
  static_assert(wdExpressionByteCode::OpCode::LastBinary - wdExpressionByteCode::OpCode::FirstBinary == wdExpressionByteCode::OpCode::LastBinaryWithConstant - wdExpressionByteCode::OpCode::FirstBinaryWithConstant);


  static constexpr wdUInt32 GetMaxOpCodeLength()
  {
    wdUInt32 uiMaxLength = 0;
    for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(s_szOpCodeNames); ++i)
    {
      uiMaxLength = wdMath::Max(uiMaxLength, wdStringUtils::GetStringElementCount(s_szOpCodeNames[i]));
    }
    return uiMaxLength;
  }

  static constexpr wdUInt32 s_uiMaxOpCodeLength = GetMaxOpCodeLength();

} // namespace

const char* wdExpressionByteCode::OpCode::GetName(Enum code)
{
  WD_ASSERT_DEBUG(code >= 0 && code < WD_ARRAY_SIZE(s_szOpCodeNames), "Out of bounds access");
  return s_szOpCodeNames[code];
}

//////////////////////////////////////////////////////////////////////////

wdExpressionByteCode::wdExpressionByteCode() = default;
wdExpressionByteCode::~wdExpressionByteCode() = default;

bool wdExpressionByteCode::operator==(const wdExpressionByteCode& other) const
{
  return m_ByteCode == other.m_ByteCode &&
         m_Inputs == other.m_Inputs &&
         m_Outputs == other.m_Outputs &&
         m_Functions == other.m_Functions;
}

void wdExpressionByteCode::Clear()
{
  m_ByteCode.Clear();
  m_Inputs.Clear();
  m_Outputs.Clear();
  m_Functions.Clear();

  m_uiNumInstructions = 0;
  m_uiNumTempRegisters = 0;
}

void wdExpressionByteCode::Disassemble(wdStringBuilder& out_sDisassembly) const
{
  out_sDisassembly.Append("// Inputs:\n");
  for (wdUInt32 i = 0; i < m_Inputs.GetCount(); ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {}({})\n", i, m_Inputs[i].m_sName, wdProcessingStream::GetDataTypeName(m_Inputs[i].m_DataType));
  }

  out_sDisassembly.Append("\n// Outputs:\n");
  for (wdUInt32 i = 0; i < m_Outputs.GetCount(); ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {}({})\n", i, m_Outputs[i].m_sName, wdProcessingStream::GetDataTypeName(m_Outputs[i].m_DataType));
  }

  out_sDisassembly.Append("\n// Functions:\n");
  for (wdUInt32 i = 0; i < m_Functions.GetCount(); ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {} {}(", i, wdExpression::RegisterType::GetName(m_Functions[i].m_OutputType), m_Functions[i].m_sName);
    const wdUInt32 uiNumArguments = m_Functions[i].m_InputTypes.GetCount();
    for (wdUInt32 j = 0; j < uiNumArguments; ++j)
    {
      out_sDisassembly.Append(wdExpression::RegisterType::GetName(m_Functions[i].m_InputTypes[j]));
      if (j < uiNumArguments - 1)
      {
        out_sDisassembly.Append(", ");
      }
    }
    out_sDisassembly.Append(")\n");
  }

  out_sDisassembly.AppendFormat("\n// Temp Registers: {}\n", m_uiNumTempRegisters);
  out_sDisassembly.AppendFormat("// Instructions: {}\n\n", m_uiNumInstructions);

  auto AppendConstant = [](wdUInt32 x, wdStringBuilder& out_sString) {
    out_sString.AppendFormat("0x{}({})", wdArgU(x, 8, true, 16), wdArgF(*reinterpret_cast<float*>(&x), 6));
  };

  const StorageType* pByteCode = GetByteCode();
  const StorageType* pByteCodeEnd = GetByteCodeEnd();

  while (pByteCode < pByteCodeEnd)
  {
    OpCode::Enum opCode = GetOpCode(pByteCode);
    {
      const char* szOpCode = OpCode::GetName(opCode);
      wdUInt32 uiOpCodeLength = wdStringUtils::GetStringElementCount(szOpCode);

      out_sDisassembly.Append(szOpCode);
      for (wdUInt32 i = uiOpCodeLength; i < s_uiMaxOpCodeLength + 1; ++i)
      {
        out_sDisassembly.Append(" ");
      }
    }

    if (opCode > OpCode::FirstUnary && opCode < OpCode::LastUnary)
    {
      wdUInt32 r = GetRegisterIndex(pByteCode);
      wdUInt32 x = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{}\n", r, x);
    }
    else if (opCode > OpCode::FirstBinary && opCode < OpCode::LastBinary)
    {
      wdUInt32 r = GetRegisterIndex(pByteCode);
      wdUInt32 a = GetRegisterIndex(pByteCode);
      wdUInt32 b = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{} r{}\n", r, a, b);
    }
    else if (opCode > OpCode::FirstBinaryWithConstant && opCode < OpCode::LastBinaryWithConstant)
    {
      wdUInt32 r = GetRegisterIndex(pByteCode);
      wdUInt32 a = GetRegisterIndex(pByteCode);
      wdUInt32 b = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{} ", r, a);
      AppendConstant(b, out_sDisassembly);
      out_sDisassembly.Append("\n");
    }
    else if (opCode > OpCode::FirstTernary && opCode < OpCode::LastTernary)
    {
      wdUInt32 r = GetRegisterIndex(pByteCode);
      wdUInt32 a = GetRegisterIndex(pByteCode);
      wdUInt32 b = GetRegisterIndex(pByteCode);
      wdUInt32 c = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{} r{} r{}\n", r, a, b, c);
    }
    else if (opCode == OpCode::MovX_C)
    {
      wdUInt32 r = GetRegisterIndex(pByteCode);
      wdUInt32 x = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} ", r);
      AppendConstant(x, out_sDisassembly);
      out_sDisassembly.Append("\n");
    }
    else if (opCode == OpCode::LoadF || opCode == OpCode::LoadI)
    {
      wdUInt32 r = GetRegisterIndex(pByteCode);
      wdUInt32 i = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} i{}({})\n", r, i, m_Inputs[i].m_sName);
    }
    else if (opCode == OpCode::StoreF || opCode == OpCode::StoreI)
    {
      wdUInt32 o = GetRegisterIndex(pByteCode);
      wdUInt32 r = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("o{}({}) r{}\n", o, m_Outputs[o].m_sName, r);
    }
    else if (opCode == OpCode::Call)
    {
      wdUInt32 uiIndex = GetFunctionIndex(pByteCode);
      const char* szName = m_Functions[uiIndex].m_sName;

      wdStringBuilder sName;
      if (wdStringUtils::IsNullOrEmpty(szName))
      {
        sName.Format("Unknown_{0}", uiIndex);
      }
      else
      {
        sName = szName;
      }

      wdUInt32 r = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("{1} r{2}", sName, r);

      wdUInt32 uiNumArgs = GetFunctionArgCount(pByteCode);
      for (wdUInt32 uiArgIndex = 0; uiArgIndex < uiNumArgs; ++uiArgIndex)
      {
        wdUInt32 x = GetRegisterIndex(pByteCode);
        out_sDisassembly.AppendFormat(" r{0}", x);
      }

      out_sDisassembly.Append("\n");
    }
    else
    {
      WD_ASSERT_NOT_IMPLEMENTED;
    }
  }
}

static constexpr wdUInt32 s_uiMetaDataVersion = 4;
static constexpr wdUInt32 s_uiCodeVersion = 3;

void wdExpressionByteCode::Save(wdStreamWriter& inout_stream) const
{
  wdChunkStreamWriter chunk(inout_stream);

  chunk.BeginStream(1);

  {
    chunk.BeginChunk("MetaData", s_uiMetaDataVersion);

    chunk << m_uiNumInstructions;
    chunk << m_uiNumTempRegisters;
    chunk.WriteArray(m_Inputs).IgnoreResult();
    chunk.WriteArray(m_Outputs).IgnoreResult();
    chunk.WriteArray(m_Functions).IgnoreResult();

    chunk.EndChunk();
  }

  {
    chunk.BeginChunk("Code", s_uiCodeVersion);

    chunk << m_ByteCode.GetCount();
    chunk.WriteBytes(m_ByteCode.GetData(), m_ByteCode.GetCount() * sizeof(StorageType)).IgnoreResult();

    chunk.EndChunk();
  }

  chunk.EndStream();
}

wdResult wdExpressionByteCode::Load(wdStreamReader& inout_stream)
{
  wdChunkStreamReader chunk(inout_stream);
  chunk.SetEndChunkFileMode(wdChunkStreamReader::EndChunkFileMode::SkipToEnd);

  chunk.BeginStream();

  while (chunk.GetCurrentChunk().m_bValid)
  {
    if (chunk.GetCurrentChunk().m_sChunkName == "MetaData")
    {
      if (chunk.GetCurrentChunk().m_uiChunkVersion >= s_uiMetaDataVersion)
      {
        chunk >> m_uiNumInstructions;
        chunk >> m_uiNumTempRegisters;
        WD_SUCCEED_OR_RETURN(chunk.ReadArray(m_Inputs));
        WD_SUCCEED_OR_RETURN(chunk.ReadArray(m_Outputs));
        WD_SUCCEED_OR_RETURN(chunk.ReadArray(m_Functions));
      }
      else
      {
        wdLog::Error("Invalid MetaData Chunk Version {}. Expected >= {}", chunk.GetCurrentChunk().m_uiChunkVersion, s_uiMetaDataVersion);

        chunk.EndStream();
        return WD_FAILURE;
      }
    }
    else if (chunk.GetCurrentChunk().m_sChunkName == "Code")
    {
      if (chunk.GetCurrentChunk().m_uiChunkVersion >= s_uiCodeVersion)
      {
        wdUInt32 uiByteCodeCount = 0;
        chunk >> uiByteCodeCount;

        m_ByteCode.SetCountUninitialized(uiByteCodeCount);
        chunk.ReadBytes(m_ByteCode.GetData(), uiByteCodeCount * sizeof(StorageType));
      }
      else
      {
        wdLog::Error("Invalid Code Chunk Version {}. Expected >= {}", chunk.GetCurrentChunk().m_uiChunkVersion, s_uiCodeVersion);

        chunk.EndStream();
        return WD_FAILURE;
      }
    }

    chunk.NextChunk();
  }

  chunk.EndStream();

  return WD_SUCCESS;
}


WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionByteCode);
