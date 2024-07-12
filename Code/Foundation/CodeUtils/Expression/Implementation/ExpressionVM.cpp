#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <Foundation/CodeUtils/Expression/Implementation/ExpressionVMOperations.h>
#include <Foundation/Logging/Log.h>

nsExpressionVM::nsExpressionVM()
{
  RegisterDefaultFunctions();
}
nsExpressionVM::~nsExpressionVM() = default;

void nsExpressionVM::RegisterFunction(const nsExpressionFunction& func)
{
  NS_ASSERT_DEV(func.m_Desc.m_uiNumRequiredInputs <= func.m_Desc.m_InputTypes.GetCount(), "Not enough input types defined. {} inputs are required but only {} types given.", func.m_Desc.m_uiNumRequiredInputs, func.m_Desc.m_InputTypes.GetCount());

  nsUInt32 uiFunctionIndex = m_Functions.GetCount();
  m_FunctionNamesToIndex.Insert(func.m_Desc.GetMangledName(), uiFunctionIndex);

  m_Functions.PushBack(func);
}

void nsExpressionVM::UnregisterFunction(const nsExpressionFunction& func)
{
  nsUInt32 uiFunctionIndex = 0;
  if (m_FunctionNamesToIndex.Remove(func.m_Desc.GetMangledName(), &uiFunctionIndex))
  {
    m_Functions.RemoveAtAndSwap(uiFunctionIndex);
    if (uiFunctionIndex != m_Functions.GetCount())
    {
      m_FunctionNamesToIndex[m_Functions[uiFunctionIndex].m_Desc.GetMangledName()] = uiFunctionIndex;
    }
  }
}

nsResult nsExpressionVM::Execute(const nsExpressionByteCode& byteCode, nsArrayPtr<const nsProcessingStream> inputs,
  nsArrayPtr<nsProcessingStream> outputs, nsUInt32 uiNumInstances, const nsExpression::GlobalData& globalData, nsBitflags<Flags> flags)
{
  if (flags.IsSet(Flags::ScalarizeStreams))
  {
    NS_SUCCEED_OR_RETURN(ScalarizeStreams(inputs, m_ScalarizedInputs));
    NS_SUCCEED_OR_RETURN(ScalarizeStreams(outputs, m_ScalarizedOutputs));

    inputs = m_ScalarizedInputs;
    outputs = m_ScalarizedOutputs;
  }
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  else
  {
    AreStreamsScalarized(inputs).AssertSuccess("Input streams are not scalarized");
    AreStreamsScalarized(outputs).AssertSuccess("Output streams are not scalarized");
  }
#endif

  NS_SUCCEED_OR_RETURN(MapStreams(byteCode.GetInputs(), inputs, "Input", uiNumInstances, flags, m_MappedInputs));
  NS_SUCCEED_OR_RETURN(MapStreams(byteCode.GetOutputs(), outputs, "Output", uiNumInstances, flags, m_MappedOutputs));

  NS_SUCCEED_OR_RETURN(MapFunctions(byteCode.GetFunctions(), globalData));

  const nsUInt32 uiTotalNumRegisters = byteCode.GetNumTempRegisters() * ((uiNumInstances + 3) / 4);
  m_Registers.SetCountUninitialized(uiTotalNumRegisters);

  // Execute bytecode
  const nsExpressionByteCode::StorageType* pByteCode = byteCode.GetByteCodeStart();
  const nsExpressionByteCode::StorageType* pByteCodeEnd = byteCode.GetByteCodeEnd();

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
    nsExpressionByteCode::OpCode::Enum opCode = nsExpressionByteCode::GetOpCode(pByteCode);

    OpFunc func = s_Simd4Funcs[opCode];
    if (func != nullptr)
    {
      func(pByteCode, context);
    }
    else
    {
      NS_ASSERT_NOT_IMPLEMENTED;
      nsLog::Error("Unknown OpCode '{}'. Execution aborted.", opCode);
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

void nsExpressionVM::RegisterDefaultFunctions()
{
  RegisterFunction(nsDefaultExpressionFunctions::s_RandomFunc);
  RegisterFunction(nsDefaultExpressionFunctions::s_PerlinNoiseFunc);
}

nsResult nsExpressionVM::ScalarizeStreams(nsArrayPtr<const nsProcessingStream> streams, nsDynamicArray<nsProcessingStream>& out_ScalarizedStreams)
{
  out_ScalarizedStreams.Clear();

  for (auto& stream : streams)
  {
    const nsUInt32 uiNumElements = nsExpressionAST::DataType::GetElementCount(nsExpressionAST::DataType::FromStreamType(stream.GetDataType()));
    if (uiNumElements == 1)
    {
      out_ScalarizedStreams.PushBack(stream);
    }
    else
    {
      nsStringBuilder sNewName;
      nsHashedString sNewNameHashed;
      auto data = nsMakeArrayPtr((nsUInt8*)(stream.GetData()), static_cast<nsUInt32>(stream.GetDataSize()));
      auto elementDataType = static_cast<nsProcessingStream::DataType>((nsUInt32)stream.GetDataType() & ~3u);

      for (nsUInt32 i = 0; i < uiNumElements; ++i)
      {
        sNewName.Set(stream.GetName(), ".", nsExpressionAST::VectorComponent::GetName(static_cast<nsExpressionAST::VectorComponent::Enum>(i)));
        sNewNameHashed.Assign(sNewName);

        auto newData = data.GetSubArray(i * nsProcessingStream::GetDataTypeSize(elementDataType));

        out_ScalarizedStreams.PushBack(nsProcessingStream(sNewNameHashed, newData, elementDataType, stream.GetElementStride()));
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsExpressionVM::AreStreamsScalarized(nsArrayPtr<const nsProcessingStream> streams)
{
  for (auto& stream : streams)
  {
    const nsUInt32 uiNumElements = nsExpressionAST::DataType::GetElementCount(nsExpressionAST::DataType::FromStreamType(stream.GetDataType()));
    if (uiNumElements > 1)
    {
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}


nsResult nsExpressionVM::ValidateStream(const nsProcessingStream& stream, const nsExpression::StreamDesc& streamDesc, nsStringView sStreamType, nsUInt32 uiNumInstances)
{
  // verify stream data type
  if (stream.GetDataType() != streamDesc.m_DataType)
  {
    nsLog::Error("{} stream '{}' expects data of type '{}' or a compatible type. Given type '{}' is not compatible.", sStreamType, streamDesc.m_sName, nsProcessingStream::GetDataTypeName(streamDesc.m_DataType), nsProcessingStream::GetDataTypeName(stream.GetDataType()));
    return NS_FAILURE;
  }

  // verify stream size
  nsUInt32 uiElementSize = stream.GetElementSize();
  nsUInt32 uiExpectedSize = stream.GetElementStride() * (uiNumInstances - 1) + uiElementSize;

  if (stream.GetDataSize() < uiExpectedSize)
  {
    nsLog::Error("{} stream '{}' data size must be {} bytes or more. Only {} bytes given", sStreamType, streamDesc.m_sName, uiExpectedSize, stream.GetDataSize());
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

template <typename T>
nsResult nsExpressionVM::MapStreams(nsArrayPtr<const nsExpression::StreamDesc> streamDescs, nsArrayPtr<T> streams, nsStringView sStreamType, nsUInt32 uiNumInstances, nsBitflags<Flags> flags, nsDynamicArray<T*>& out_MappedStreams)
{
  out_MappedStreams.Clear();
  out_MappedStreams.Reserve(streamDescs.GetCount());

  if (flags.IsSet(Flags::MapStreamsByName))
  {
    for (auto& streamDesc : streamDescs)
    {
      bool bFound = false;

      for (nsUInt32 i = 0; i < streams.GetCount(); ++i)
      {
        auto& stream = streams[i];
        if (stream.GetName() == streamDesc.m_sName)
        {
          NS_SUCCEED_OR_RETURN(ValidateStream(stream, streamDesc, sStreamType, uiNumInstances));

          out_MappedStreams.PushBack(&stream);
          bFound = true;
          break;
        }
      }

      if (!bFound)
      {
        nsLog::Error("Bytecode expects an {} stream '{}'", sStreamType, streamDesc.m_sName);
        return NS_FAILURE;
      }
    }
  }
  else
  {
    if (streams.GetCount() != streamDescs.GetCount())
      return NS_FAILURE;

    for (nsUInt32 i = 0; i < streams.GetCount(); ++i)
    {
      auto& stream = streams.GetPtr()[i];

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
      auto& streamDesc = streamDescs.GetPtr()[i];
      NS_SUCCEED_OR_RETURN(ValidateStream(stream, streamDesc, sStreamType, uiNumInstances));
#endif

      out_MappedStreams.PushBack(&stream);
    }
  }

  return NS_SUCCESS;
}

nsResult nsExpressionVM::MapFunctions(nsArrayPtr<const nsExpression::FunctionDesc> functionDescs, const nsExpression::GlobalData& globalData)
{
  m_MappedFunctions.Clear();
  m_MappedFunctions.Reserve(functionDescs.GetCount());

  for (auto& functionDesc : functionDescs)
  {
    nsUInt32 uiFunctionIndex = 0;
    if (!m_FunctionNamesToIndex.TryGetValue(functionDesc.m_sName, uiFunctionIndex))
    {
      nsLog::Error("Bytecode expects a function called '{0}' but it was not registered for this VM", functionDesc.m_sName);
      return NS_FAILURE;
    }

    auto& registeredFunction = m_Functions[uiFunctionIndex];

    // verify signature
    if (functionDesc.m_InputTypes != registeredFunction.m_Desc.m_InputTypes || functionDesc.m_OutputType != registeredFunction.m_Desc.m_OutputType)
    {
      nsLog::Error("Signature for registered function '{}' does not match the expected signature from bytecode", functionDesc.m_sName);
      return NS_FAILURE;
    }

    if (registeredFunction.m_ValidateGlobalDataFunc != nullptr)
    {
      if (registeredFunction.m_ValidateGlobalDataFunc(globalData).Failed())
      {
        nsLog::Error("Global data validation for function '{0}' failed.", functionDesc.m_sName);
        return NS_FAILURE;
      }
    }

    m_MappedFunctions.PushBack(&registeredFunction);
  }

  return NS_SUCCESS;
}
