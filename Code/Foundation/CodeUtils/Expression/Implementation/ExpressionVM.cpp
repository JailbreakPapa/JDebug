#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <Foundation/CodeUtils/Expression/Implementation/ExpressionVMOperations.h>
#include <Foundation/Logging/Log.h>

wdExpressionVM::wdExpressionVM()
{
  RegisterDefaultFunctions();
}
wdExpressionVM::~wdExpressionVM() = default;

void wdExpressionVM::RegisterFunction(const wdExpressionFunction& func)
{
  WD_ASSERT_DEV(func.m_Desc.m_uiNumRequiredInputs <= func.m_Desc.m_InputTypes.GetCount(), "Not enough input types defined. {} inputs are required but only {} types given.", func.m_Desc.m_uiNumRequiredInputs, func.m_Desc.m_InputTypes.GetCount());

  wdUInt32 uiFunctionIndex = m_Functions.GetCount();
  m_FunctionNamesToIndex.Insert(func.m_Desc.GetMangledName(), uiFunctionIndex);

  m_Functions.PushBack(func);
}

void wdExpressionVM::UnregisterFunction(const wdExpressionFunction& func)
{
  wdUInt32 uiFunctionIndex = 0;
  if (m_FunctionNamesToIndex.Remove(func.m_Desc.GetMangledName(), &uiFunctionIndex))
  {
    m_Functions.RemoveAtAndSwap(uiFunctionIndex);
    if (uiFunctionIndex != m_Functions.GetCount())
    {
      m_FunctionNamesToIndex[m_Functions[uiFunctionIndex].m_Desc.GetMangledName()] = uiFunctionIndex;
    }
  }
}

wdResult wdExpressionVM::Execute(const wdExpressionByteCode& byteCode, wdArrayPtr<const wdProcessingStream> inputs,
  wdArrayPtr<wdProcessingStream> outputs, wdUInt32 uiNumInstances, const wdExpression::GlobalData& globalData)
{
  WD_SUCCEED_OR_RETURN(ScalarizeStreams(inputs, m_ScalarizedInputs));
  WD_SUCCEED_OR_RETURN(ScalarizeStreams(outputs, m_ScalarizedOutputs));

  WD_SUCCEED_OR_RETURN(MapStreams(byteCode.GetInputs(), m_ScalarizedInputs, "Input", uiNumInstances, m_MappedInputs));
  WD_SUCCEED_OR_RETURN(MapStreams(byteCode.GetOutputs(), m_ScalarizedOutputs, "Output", uiNumInstances, m_MappedOutputs));
  WD_SUCCEED_OR_RETURN(MapFunctions(byteCode.GetFunctions(), globalData));

  const wdUInt32 uiTotalNumRegisters = byteCode.GetNumTempRegisters() * ((uiNumInstances + 3) / 4);
  m_Registers.SetCountUninitialized(uiTotalNumRegisters);

  // Execute bytecode
  const wdExpressionByteCode::StorageType* pByteCode = byteCode.GetByteCode();
  const wdExpressionByteCode::StorageType* pByteCodeEnd = byteCode.GetByteCodeEnd();

  ExecutionContext context;
  context.m_pRegisters = m_Registers.GetData();
  context.m_uiNumInstances = uiNumInstances;
  context.m_uiNumSimd4Instances = (uiNumInstances + 3) / 4;
  context.m_Inputs = m_MappedInputs;
  context.m_Outputs = m_MappedOutputs;
  context.m_Functions = m_MappedFunctions;
  context.m_pGlobalData = &globalData;

  while (pByteCode < pByteCodeEnd)
  {
    wdExpressionByteCode::OpCode::Enum opCode = wdExpressionByteCode::GetOpCode(pByteCode);

    OpFunc func = s_Simd4Funcs[opCode];
    if (func != nullptr)
    {
      func(pByteCode, context);
    }
    else
    {
      WD_ASSERT_NOT_IMPLEMENTED;
      wdLog::Error("Unknown OpCode '{}'. Execution aborted.", opCode);
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}

void wdExpressionVM::RegisterDefaultFunctions()
{
  RegisterFunction(wdDefaultExpressionFunctions::s_RandomFunc);
  RegisterFunction(wdDefaultExpressionFunctions::s_PerlinNoiseFunc);
}

wdResult wdExpressionVM::ScalarizeStreams(wdArrayPtr<const wdProcessingStream> streams, wdDynamicArray<wdProcessingStream>& out_ScalarizedStreams)
{
  out_ScalarizedStreams.Clear();

  for (auto& stream : streams)
  {
    const wdUInt32 uiNumElements = wdExpressionAST::DataType::GetElementCount(wdExpressionAST::DataType::FromStreamType(stream.GetDataType()));
    if (uiNumElements == 1)
    {
      out_ScalarizedStreams.PushBack(stream);
    }
    else
    {
      wdStringBuilder sNewName;
      wdHashedString sNewNameHashed;
      auto data = wdMakeArrayPtr((wdUInt8*)(stream.GetData()), static_cast<wdUInt32>(stream.GetDataSize()));
      auto elementDataType = static_cast<wdProcessingStream::DataType>((wdUInt32)stream.GetDataType() & ~3u);

      for (wdUInt32 i = 0; i < uiNumElements; ++i)
      {
        sNewName.Set(stream.GetName(), ".", wdExpressionAST::VectorComponent::GetName(static_cast<wdExpressionAST::VectorComponent::Enum>(i)));
        sNewNameHashed.Assign(sNewName);

        auto newData = data.GetSubArray(i * wdProcessingStream::GetDataTypeSize(elementDataType));

        out_ScalarizedStreams.PushBack(wdProcessingStream(sNewNameHashed, newData, elementDataType, stream.GetElementStride()));
      }
    }
  }

  return WD_SUCCESS;
}

wdResult wdExpressionVM::MapStreams(wdArrayPtr<const wdExpression::StreamDesc> streamDescs, wdArrayPtr<wdProcessingStream> streams, const char* szStreamType, wdUInt32 uiNumInstances, wdDynamicArray<wdProcessingStream*>& out_MappedStreams)
{
  out_MappedStreams.Clear();
  out_MappedStreams.Reserve(streamDescs.GetCount());

  for (auto& streamDesc : streamDescs)
  {
    bool bFound = false;

    for (wdUInt32 i = 0; i < streams.GetCount(); ++i)
    {
      auto& stream = streams[i];
      if (stream.GetName() == streamDesc.m_sName)
      {
        // verify stream data type
        if (stream.GetDataType() != streamDesc.m_DataType)
        {
          wdLog::Error("{} stream '{}' expects data of type '{}' or a compatible type. Given type '{}' is not compatible.", szStreamType, streamDesc.m_sName, wdProcessingStream::GetDataTypeName(streamDesc.m_DataType), wdProcessingStream::GetDataTypeName(stream.GetDataType()));
          return WD_FAILURE;
        }

        // verify stream size
        wdUInt32 uiElementSize = stream.GetElementSize();
        wdUInt32 uiExpectedSize = stream.GetElementStride() * (uiNumInstances - 1) + uiElementSize;

        if (stream.GetDataSize() < uiExpectedSize)
        {
          wdLog::Error("{} stream '{}' data size must be {} bytes or more. Only {} bytes given", szStreamType, streamDesc.m_sName, uiExpectedSize, stream.GetDataSize());
          return WD_FAILURE;
        }

        out_MappedStreams.PushBack(&stream);
        bFound = true;
        break;
      }
    }

    if (!bFound)
    {
      wdLog::Error("Bytecode expects an {} stream '{}'", szStreamType, streamDesc.m_sName);
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}

wdResult wdExpressionVM::MapFunctions(wdArrayPtr<const wdExpression::FunctionDesc> functionDescs, const wdExpression::GlobalData& globalData)
{
  m_MappedFunctions.Clear();
  m_MappedFunctions.Reserve(functionDescs.GetCount());

  for (auto& functionDesc : functionDescs)
  {
    wdUInt32 uiFunctionIndex = 0;
    if (!m_FunctionNamesToIndex.TryGetValue(functionDesc.m_sName, uiFunctionIndex))
    {
      wdLog::Error("Bytecode expects a function called '{0}' but it was not registered for this VM", functionDesc.m_sName);
      return WD_FAILURE;
    }

    auto& registeredFunction = m_Functions[uiFunctionIndex];

    // verify signature
    if (functionDesc.m_InputTypes != registeredFunction.m_Desc.m_InputTypes || functionDesc.m_OutputType != registeredFunction.m_Desc.m_OutputType)
    {
      wdLog::Error("Signature for registered function '{}' does not match the expected signature from bytecode", functionDesc.m_sName);
      return WD_FAILURE;
    }

    if (registeredFunction.m_ValidateGlobalDataFunc != nullptr)
    {
      if (registeredFunction.m_ValidateGlobalDataFunc(globalData).Failed())
      {
        wdLog::Error("Global data validation for function '{0}' failed.", functionDesc.m_sName);
        return WD_FAILURE;
      }
    }

    m_MappedFunctions.PushBack(&registeredFunction);
  }

  return WD_SUCCESS;
}


WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionVM);
