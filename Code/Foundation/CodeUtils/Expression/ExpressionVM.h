#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/Types/UniquePtr.h>

class NS_FOUNDATION_DLL nsExpressionVM
{
public:
  nsExpressionVM();
  ~nsExpressionVM();

  void RegisterFunction(const nsExpressionFunction& func);
  void UnregisterFunction(const nsExpressionFunction& func);

  struct Flags
  {
    using StorageType = nsUInt32;

    enum Enum
    {
      MapStreamsByName = NS_BIT(0),
      ScalarizeStreams = NS_BIT(1),

      UserFriendly = MapStreamsByName | ScalarizeStreams,
      BestPerformance = 0,

      Default = UserFriendly
    };

    struct Bits
    {
      StorageType MapStreamsByName : 1;
      StorageType ScalarizeStreams : 1;
    };
  };

  nsResult Execute(const nsExpressionByteCode& byteCode, nsArrayPtr<const nsProcessingStream> inputs, nsArrayPtr<nsProcessingStream> outputs, nsUInt32 uiNumInstances, const nsExpression::GlobalData& globalData = nsExpression::GlobalData(), nsBitflags<Flags> flags = Flags::Default);

private:
  void RegisterDefaultFunctions();

  static nsResult ScalarizeStreams(nsArrayPtr<const nsProcessingStream> streams, nsDynamicArray<nsProcessingStream>& out_ScalarizedStreams);
  static nsResult AreStreamsScalarized(nsArrayPtr<const nsProcessingStream> streams);
  static nsResult ValidateStream(const nsProcessingStream& stream, const nsExpression::StreamDesc& streamDesc, nsStringView sStreamType, nsUInt32 uiNumInstances);

  template <typename T>
  static nsResult MapStreams(nsArrayPtr<const nsExpression::StreamDesc> streamDescs, nsArrayPtr<T> streams, nsStringView sStreamType, nsUInt32 uiNumInstances, nsBitflags<Flags> flags, nsDynamicArray<T*>& out_MappedStreams);
  nsResult MapFunctions(nsArrayPtr<const nsExpression::FunctionDesc> functionDescs, const nsExpression::GlobalData& globalData);

  nsDynamicArray<nsExpression::Register, nsAlignedAllocatorWrapper> m_Registers;

  nsDynamicArray<nsProcessingStream> m_ScalarizedInputs;
  nsDynamicArray<nsProcessingStream> m_ScalarizedOutputs;

  nsDynamicArray<const nsProcessingStream*> m_MappedInputs;
  nsDynamicArray<nsProcessingStream*> m_MappedOutputs;
  nsDynamicArray<const nsExpressionFunction*> m_MappedFunctions;

  nsDynamicArray<nsExpressionFunction> m_Functions;
  nsHashTable<nsHashedString, nsUInt32> m_FunctionNamesToIndex;
};
