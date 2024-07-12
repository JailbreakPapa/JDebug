#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/IO/ChunkStream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>

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

  static_assert(NS_ARRAY_SIZE(s_szOpCodeNames) == nsExpressionByteCode::OpCode::Count);
  static_assert(nsExpressionByteCode::OpCode::LastBinary - nsExpressionByteCode::OpCode::FirstBinary == nsExpressionByteCode::OpCode::LastBinaryWithConstant - nsExpressionByteCode::OpCode::FirstBinaryWithConstant);


  static constexpr nsUInt32 GetMaxOpCodeLength()
  {
    nsUInt32 uiMaxLength = 0;
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(s_szOpCodeNames); ++i)
    {
      uiMaxLength = nsMath::Max(uiMaxLength, nsStringUtils::GetStringElementCount(s_szOpCodeNames[i]));
    }
    return uiMaxLength;
  }

  static constexpr nsUInt32 s_uiMaxOpCodeLength = GetMaxOpCodeLength();

} // namespace

const char* nsExpressionByteCode::OpCode::GetName(Enum code)
{
  NS_ASSERT_DEBUG(code >= 0 && code < NS_ARRAY_SIZE(s_szOpCodeNames), "Out of bounds access");
  return s_szOpCodeNames[code];
}

//////////////////////////////////////////////////////////////////////////

//clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsExpressionByteCode, nsNoBase, 1, nsRTTINoAllocator)
NS_END_STATIC_REFLECTED_TYPE;
//clang-format on

nsExpressionByteCode::nsExpressionByteCode() = default;

nsExpressionByteCode::nsExpressionByteCode(const nsExpressionByteCode& other)
{
  *this = other;
}

nsExpressionByteCode::~nsExpressionByteCode()
{
  Clear();
}

void nsExpressionByteCode::operator=(const nsExpressionByteCode& other)
{
  Clear();
  Init(other.GetByteCode(), other.GetInputs(), other.GetOutputs(), other.GetFunctions(), other.GetNumTempRegisters(), other.GetNumInstructions());
}

bool nsExpressionByteCode::operator==(const nsExpressionByteCode& other) const
{
  return GetByteCode() == other.GetByteCode() &&
         GetInputs() == other.GetInputs() &&
         GetOutputs() == other.GetOutputs() &&
         GetFunctions() == other.GetFunctions();
}

void nsExpressionByteCode::Clear()
{
  nsMemoryUtils::Destruct(m_pInputs, m_uiNumInputs);
  nsMemoryUtils::Destruct(m_pOutputs, m_uiNumOutputs);
  nsMemoryUtils::Destruct(m_pFunctions, m_uiNumFunctions);

  m_pInputs = nullptr;
  m_pOutputs = nullptr;
  m_pFunctions = nullptr;
  m_pByteCode = nullptr;

  m_uiByteCodeCount = 0;
  m_uiNumInputs = 0;
  m_uiNumOutputs = 0;
  m_uiNumFunctions = 0;

  m_uiNumTempRegisters = 0;
  m_uiNumInstructions = 0;

  m_Data.Clear();
}

void nsExpressionByteCode::Disassemble(nsStringBuilder& out_sDisassembly) const
{
  out_sDisassembly.Append("// Inputs:\n");
  for (nsUInt32 i = 0; i < m_uiNumInputs; ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {}({})\n", i, m_pInputs[i].m_sName, nsProcessingStream::GetDataTypeName(m_pInputs[i].m_DataType));
  }

  out_sDisassembly.Append("\n// Outputs:\n");
  for (nsUInt32 i = 0; i < m_uiNumOutputs; ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {}({})\n", i, m_pOutputs[i].m_sName, nsProcessingStream::GetDataTypeName(m_pOutputs[i].m_DataType));
  }

  out_sDisassembly.Append("\n// Functions:\n");
  for (nsUInt32 i = 0; i < m_uiNumFunctions; ++i)
  {
    out_sDisassembly.AppendFormat("//  {}: {} {}(", i, nsExpression::RegisterType::GetName(m_pFunctions[i].m_OutputType), m_pFunctions[i].m_sName);
    const nsUInt32 uiNumArguments = m_pFunctions[i].m_InputTypes.GetCount();
    for (nsUInt32 j = 0; j < uiNumArguments; ++j)
    {
      out_sDisassembly.Append(nsExpression::RegisterType::GetName(m_pFunctions[i].m_InputTypes[j]));
      if (j < uiNumArguments - 1)
      {
        out_sDisassembly.Append(", ");
      }
    }
    out_sDisassembly.Append(")\n");
  }

  out_sDisassembly.AppendFormat("\n// Temp Registers: {}\n", GetNumTempRegisters());
  out_sDisassembly.AppendFormat("// Instructions: {}\n\n", GetNumInstructions());

  auto AppendConstant = [](nsUInt32 x, nsStringBuilder& out_sString)
  {
    out_sString.AppendFormat("0x{}({})", nsArgU(x, 8, true, 16), nsArgF(*reinterpret_cast<float*>(&x), 6));
  };

  const StorageType* pByteCode = GetByteCodeStart();
  const StorageType* pByteCodeEnd = GetByteCodeEnd();

  while (pByteCode < pByteCodeEnd)
  {
    OpCode::Enum opCode = GetOpCode(pByteCode);
    {
      const char* szOpCode = OpCode::GetName(opCode);
      nsUInt32 uiOpCodeLength = nsStringUtils::GetStringElementCount(szOpCode);

      out_sDisassembly.Append(szOpCode);
      for (nsUInt32 i = uiOpCodeLength; i < s_uiMaxOpCodeLength + 1; ++i)
      {
        out_sDisassembly.Append(" ");
      }
    }

    if (opCode > OpCode::FirstUnary && opCode < OpCode::LastUnary)
    {
      nsUInt32 r = GetRegisterIndex(pByteCode);
      nsUInt32 x = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{}\n", r, x);
    }
    else if (opCode > OpCode::FirstBinary && opCode < OpCode::LastBinary)
    {
      nsUInt32 r = GetRegisterIndex(pByteCode);
      nsUInt32 a = GetRegisterIndex(pByteCode);
      nsUInt32 b = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{} r{}\n", r, a, b);
    }
    else if (opCode > OpCode::FirstBinaryWithConstant && opCode < OpCode::LastBinaryWithConstant)
    {
      nsUInt32 r = GetRegisterIndex(pByteCode);
      nsUInt32 a = GetRegisterIndex(pByteCode);
      nsUInt32 b = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{} ", r, a);
      AppendConstant(b, out_sDisassembly);
      out_sDisassembly.Append("\n");
    }
    else if (opCode > OpCode::FirstTernary && opCode < OpCode::LastTernary)
    {
      nsUInt32 r = GetRegisterIndex(pByteCode);
      nsUInt32 a = GetRegisterIndex(pByteCode);
      nsUInt32 b = GetRegisterIndex(pByteCode);
      nsUInt32 c = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} r{} r{} r{}\n", r, a, b, c);
    }
    else if (opCode == OpCode::MovX_C)
    {
      nsUInt32 r = GetRegisterIndex(pByteCode);
      nsUInt32 x = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} ", r);
      AppendConstant(x, out_sDisassembly);
      out_sDisassembly.Append("\n");
    }
    else if (opCode == OpCode::LoadF || opCode == OpCode::LoadI)
    {
      nsUInt32 r = GetRegisterIndex(pByteCode);
      nsUInt32 i = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("r{} i{}({})\n", r, i, m_pInputs[i].m_sName);
    }
    else if (opCode == OpCode::StoreF || opCode == OpCode::StoreI)
    {
      nsUInt32 o = GetRegisterIndex(pByteCode);
      nsUInt32 r = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("o{}({}) r{}\n", o, m_pOutputs[o].m_sName, r);
    }
    else if (opCode == OpCode::Call)
    {
      nsUInt32 uiIndex = GetFunctionIndex(pByteCode);
      const char* szName = m_pFunctions[uiIndex].m_sName;

      nsStringBuilder sName;
      if (nsStringUtils::IsNullOrEmpty(szName))
      {
        sName.SetFormat("Unknown_{0}", uiIndex);
      }
      else
      {
        sName = szName;
      }

      nsUInt32 r = GetRegisterIndex(pByteCode);

      out_sDisassembly.AppendFormat("{1} r{2}", sName, r);

      nsUInt32 uiNumArgs = GetFunctionArgCount(pByteCode);
      for (nsUInt32 uiArgIndex = 0; uiArgIndex < uiNumArgs; ++uiArgIndex)
      {
        nsUInt32 x = GetRegisterIndex(pByteCode);
        out_sDisassembly.AppendFormat(" r{0}", x);
      }

      out_sDisassembly.Append("\n");
    }
    else
    {
      NS_ASSERT_NOT_IMPLEMENTED;
    }
  }
}

static constexpr nsTypeVersion s_uiByteCodeVersion = 6;

nsResult nsExpressionByteCode::Save(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_uiByteCodeVersion);

  nsUInt32 uiDataSize = static_cast<nsUInt32>(m_Data.GetByteBlobPtr().GetCount());

  inout_stream << uiDataSize;

  inout_stream << m_uiNumInputs;
  for (auto& input : GetInputs())
  {
    NS_SUCCEED_OR_RETURN(input.Serialize(inout_stream));
  }

  inout_stream << m_uiNumOutputs;
  for (auto& output : GetOutputs())
  {
    NS_SUCCEED_OR_RETURN(output.Serialize(inout_stream));
  }

  inout_stream << m_uiNumFunctions;
  for (auto& function : GetFunctions())
  {
    NS_SUCCEED_OR_RETURN(function.Serialize(inout_stream));
  }

  inout_stream << m_uiByteCodeCount;
  NS_SUCCEED_OR_RETURN(inout_stream.WriteBytes(m_pByteCode, m_uiByteCodeCount * sizeof(StorageType)));

  inout_stream << m_uiNumTempRegisters;
  inout_stream << m_uiNumInstructions;

  return NS_SUCCESS;
}

nsResult nsExpressionByteCode::Load(nsStreamReader& inout_stream, nsByteArrayPtr externalMemory /*= nsByteArrayPtr()*/)
{
  nsTypeVersion version = inout_stream.ReadVersion(s_uiByteCodeVersion);
  if (version != s_uiByteCodeVersion)
  {
    nsLog::Error("Invalid expression byte code version {}. Expected {}", version, s_uiByteCodeVersion);
    return NS_FAILURE;
  }

  nsUInt32 uiDataSize = 0;
  inout_stream >> uiDataSize;

  void* pData = nullptr;
  if (externalMemory.IsEmpty())
  {
    m_Data.SetCountUninitialized(uiDataSize);
    m_Data.ZeroFill();
    pData = m_Data.GetByteBlobPtr().GetPtr();
  }
  else
  {
    if (externalMemory.GetCount() < uiDataSize)
    {
      nsLog::Error("External memory is too small. Expected at least {} bytes but got {} bytes.", uiDataSize, externalMemory.GetCount());
      return NS_FAILURE;
    }

    if (nsMemoryUtils::IsAligned(externalMemory.GetPtr(), NS_ALIGNMENT_OF(nsExpression::StreamDesc)) == false)
    {
      nsLog::Error("External memory is not properly aligned. Expected an alignment of at least {} bytes.", NS_ALIGNMENT_OF(nsExpression::StreamDesc));
      return NS_FAILURE;
    }

    pData = externalMemory.GetPtr();
  }

  // Inputs
  {
    inout_stream >> m_uiNumInputs;
    m_pInputs = static_cast<nsExpression::StreamDesc*>(pData);
    for (nsUInt32 i = 0; i < m_uiNumInputs; ++i)
    {
      NS_SUCCEED_OR_RETURN(m_pInputs[i].Deserialize(inout_stream));
    }

    pData = nsMemoryUtils::AddByteOffset(pData, GetInputs().ToByteArray().GetCount());
  }

  // Outputs
  {
    inout_stream >> m_uiNumOutputs;
    m_pOutputs = static_cast<nsExpression::StreamDesc*>(pData);
    for (nsUInt32 i = 0; i < m_uiNumOutputs; ++i)
    {
      NS_SUCCEED_OR_RETURN(m_pOutputs[i].Deserialize(inout_stream));
    }

    pData = nsMemoryUtils::AddByteOffset(pData, GetOutputs().ToByteArray().GetCount());
  }

  // Functions
  {
    pData = nsMemoryUtils::AlignForwards(pData, NS_ALIGNMENT_OF(nsExpression::FunctionDesc));

    inout_stream >> m_uiNumFunctions;
    m_pFunctions = static_cast<nsExpression::FunctionDesc*>(pData);
    for (nsUInt32 i = 0; i < m_uiNumFunctions; ++i)
    {
      NS_SUCCEED_OR_RETURN(m_pFunctions[i].Deserialize(inout_stream));
    }

    pData = nsMemoryUtils::AddByteOffset(pData, GetFunctions().ToByteArray().GetCount());
  }

  // ByteCode
  {
    pData = nsMemoryUtils::AlignForwards(pData, NS_ALIGNMENT_OF(StorageType));

    inout_stream >> m_uiByteCodeCount;
    m_pByteCode = static_cast<StorageType*>(pData);
    inout_stream.ReadBytes(m_pByteCode, m_uiByteCodeCount * sizeof(StorageType));
  }

  inout_stream >> m_uiNumTempRegisters;
  inout_stream >> m_uiNumInstructions;

  return NS_SUCCESS;
}

void nsExpressionByteCode::Init(nsArrayPtr<const StorageType> byteCode, nsArrayPtr<const nsExpression::StreamDesc> inputs, nsArrayPtr<const nsExpression::StreamDesc> outputs, nsArrayPtr<const nsExpression::FunctionDesc> functions, nsUInt32 uiNumTempRegisters, nsUInt32 uiNumInstructions)
{
  nsUInt32 uiOutputsOffset = 0;
  nsUInt32 uiFunctionsOffset = 0;
  nsUInt32 uiByteCodeOffset = 0;

  nsUInt32 uiDataSize = 0;
  uiDataSize += inputs.ToByteArray().GetCount();
  uiOutputsOffset = uiDataSize;
  uiDataSize += outputs.ToByteArray().GetCount();

  uiDataSize = nsMemoryUtils::AlignSize<nsUInt32>(uiDataSize, NS_ALIGNMENT_OF(nsExpression::FunctionDesc));
  uiFunctionsOffset = uiDataSize;
  uiDataSize += functions.ToByteArray().GetCount();

  uiDataSize = nsMemoryUtils::AlignSize<nsUInt32>(uiDataSize, NS_ALIGNMENT_OF(StorageType));
  uiByteCodeOffset = uiDataSize;
  uiDataSize += byteCode.ToByteArray().GetCount();

  m_Data.SetCountUninitialized(uiDataSize);
  m_Data.ZeroFill();

  void* pData = m_Data.GetByteBlobPtr().GetPtr();

  NS_ASSERT_DEV(inputs.GetCount() < nsSmallInvalidIndex, "Too many inputs");
  m_pInputs = static_cast<nsExpression::StreamDesc*>(pData);
  m_uiNumInputs = static_cast<nsUInt16>(inputs.GetCount());
  nsMemoryUtils::Copy(m_pInputs, inputs.GetPtr(), m_uiNumInputs);

  NS_ASSERT_DEV(outputs.GetCount() < nsSmallInvalidIndex, "Too many outputs");
  m_pOutputs = static_cast<nsExpression::StreamDesc*>(nsMemoryUtils::AddByteOffset(pData, uiOutputsOffset));
  m_uiNumOutputs = static_cast<nsUInt16>(outputs.GetCount());
  nsMemoryUtils::Copy(m_pOutputs, outputs.GetPtr(), m_uiNumOutputs);

  NS_ASSERT_DEV(functions.GetCount() < nsSmallInvalidIndex, "Too many functions");
  m_pFunctions = static_cast<nsExpression::FunctionDesc*>(nsMemoryUtils::AddByteOffset(pData, uiFunctionsOffset));
  m_uiNumFunctions = static_cast<nsUInt16>(functions.GetCount());
  nsMemoryUtils::Copy(m_pFunctions, functions.GetPtr(), m_uiNumFunctions);

  m_pByteCode = static_cast<StorageType*>(nsMemoryUtils::AddByteOffset(pData, uiByteCodeOffset));
  m_uiByteCodeCount = byteCode.GetCount();
  nsMemoryUtils::Copy(m_pByteCode, byteCode.GetPtr(), m_uiByteCodeCount);

  NS_ASSERT_DEV(uiNumTempRegisters < nsSmallInvalidIndex, "Too many temp registers");
  m_uiNumTempRegisters = static_cast<nsUInt16>(uiNumTempRegisters);
  m_uiNumInstructions = uiNumInstructions;
}


NS_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionByteCode);
