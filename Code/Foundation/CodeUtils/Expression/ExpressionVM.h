#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/Types/UniquePtr.h>

class WD_FOUNDATION_DLL wdExpressionVM
{
public:
  wdExpressionVM();
  ~wdExpressionVM();

  void RegisterFunction(const wdExpressionFunction& func);
  void UnregisterFunction(const wdExpressionFunction& func);

  wdResult Execute(const wdExpressionByteCode& byteCode, wdArrayPtr<const wdProcessingStream> inputs, wdArrayPtr<wdProcessingStream> outputs, wdUInt32 uiNumInstances, const wdExpression::GlobalData& globalData = wdExpression::GlobalData());

private:
  void RegisterDefaultFunctions();

  wdResult ScalarizeStreams(wdArrayPtr<const wdProcessingStream> streams, wdDynamicArray<wdProcessingStream>& out_ScalarizedStreams);
  wdResult MapStreams(wdArrayPtr<const wdExpression::StreamDesc> streamDescs, wdArrayPtr<wdProcessingStream> streams, const char* szStreamType, wdUInt32 uiNumInstances, wdDynamicArray<wdProcessingStream*>& out_MappedStreams);
  wdResult MapFunctions(wdArrayPtr<const wdExpression::FunctionDesc> functionDescs, const wdExpression::GlobalData& globalData);

  wdDynamicArray<wdExpression::Register, wdAlignedAllocatorWrapper> m_Registers;

  wdDynamicArray<wdProcessingStream> m_ScalarizedInputs;
  wdDynamicArray<wdProcessingStream> m_ScalarizedOutputs;

  wdDynamicArray<wdProcessingStream*> m_MappedInputs;
  wdDynamicArray<wdProcessingStream*> m_MappedOutputs;
  wdDynamicArray<const wdExpressionFunction*> m_MappedFunctions;

  wdDynamicArray<wdExpressionFunction> m_Functions;
  wdHashTable<wdHashedString, wdUInt32> m_FunctionNamesToIndex;
};
