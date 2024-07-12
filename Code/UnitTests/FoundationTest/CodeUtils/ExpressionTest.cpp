#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionCompiler.h>
#include <Foundation/CodeUtils/Expression/ExpressionParser.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Math/Float16.h>
#include <Foundation/Types/UniquePtr.h>

namespace
{
  static nsUInt32 s_uiNumASTDumps = 0;

  void MakeASTOutputPath(nsStringView sOutputName, nsStringBuilder& out_sOutputPath)
  {
    nsUInt32 uiCounter = s_uiNumASTDumps;
    ++s_uiNumASTDumps;

    out_sOutputPath.SetFormat(":output/Expression/{}_{}_AST.dgml", nsArgU(uiCounter, 2, true), sOutputName);
  }

  void DumpDisassembly(const nsExpressionByteCode& byteCode, nsStringView sOutputName, nsUInt32 uiCounter)
  {
    nsStringBuilder sDisassembly;
    byteCode.Disassemble(sDisassembly);

    nsStringBuilder sFileName;
    sFileName.SetFormat(":output/Expression/{}_{}_ByteCode.txt", nsArgU(uiCounter, 2, true), sOutputName);

    nsFileWriter fileWriter;
    if (fileWriter.Open(sFileName).Succeeded())
    {
      fileWriter.WriteBytes(sDisassembly.GetData(), sDisassembly.GetElementCount()).IgnoreResult();

      nsLog::Error("Disassembly was dumped to: {}", sFileName);
    }
    else
    {
      nsLog::Error("Failed to dump Disassembly to: {}", sFileName);
    }
  }

  static nsUInt32 s_uiNumByteCodeComparisons = 0;

  bool CompareByteCode(const nsExpressionByteCode& testCode, const nsExpressionByteCode& referenceCode)
  {
    nsUInt32 uiCounter = s_uiNumByteCodeComparisons;
    ++s_uiNumByteCodeComparisons;

    if (testCode != referenceCode)
    {
      DumpDisassembly(referenceCode, "Reference", uiCounter);
      DumpDisassembly(testCode, "Test", uiCounter);
      return false;
    }

    return true;
  }

  static nsHashedString s_sA = nsMakeHashedString("a");
  static nsHashedString s_sB = nsMakeHashedString("b");
  static nsHashedString s_sC = nsMakeHashedString("c");
  static nsHashedString s_sD = nsMakeHashedString("d");
  static nsHashedString s_sOutput = nsMakeHashedString("output");

  static nsUniquePtr<nsExpressionParser> s_pParser;
  static nsUniquePtr<nsExpressionCompiler> s_pCompiler;
  static nsUniquePtr<nsExpressionVM> s_pVM;

  template <typename T>
  struct StreamDataTypeDeduction
  {
  };

  template <>
  struct StreamDataTypeDeduction<nsFloat16>
  {
    static constexpr nsProcessingStream::DataType Type = nsProcessingStream::DataType::Half;
    static nsFloat16 Default() { return nsMath::MinValue<float>(); }
  };

  template <>
  struct StreamDataTypeDeduction<float>
  {
    static constexpr nsProcessingStream::DataType Type = nsProcessingStream::DataType::Float;
    static float Default() { return nsMath::MinValue<float>(); }
  };

  template <>
  struct StreamDataTypeDeduction<nsInt8>
  {
    static constexpr nsProcessingStream::DataType Type = nsProcessingStream::DataType::Byte;
    static nsInt8 Default() { return nsMath::MinValue<nsInt8>(); }
  };

  template <>
  struct StreamDataTypeDeduction<nsInt16>
  {
    static constexpr nsProcessingStream::DataType Type = nsProcessingStream::DataType::Short;
    static nsInt16 Default() { return nsMath::MinValue<nsInt16>(); }
  };

  template <>
  struct StreamDataTypeDeduction<int>
  {
    static constexpr nsProcessingStream::DataType Type = nsProcessingStream::DataType::Int;
    static int Default() { return nsMath::MinValue<int>(); }
  };

  template <>
  struct StreamDataTypeDeduction<nsVec3>
  {
    static constexpr nsProcessingStream::DataType Type = nsProcessingStream::DataType::Float3;
    static nsVec3 Default() { return nsVec3(nsMath::MinValue<float>()); }
  };

  template <>
  struct StreamDataTypeDeduction<nsVec3I32>
  {
    static constexpr nsProcessingStream::DataType Type = nsProcessingStream::DataType::Int3;
    static nsVec3I32 Default() { return nsVec3I32(nsMath::MinValue<int>()); }
  };

  template <typename T>
  void Compile(nsStringView sCode, nsExpressionByteCode& out_byteCode, nsStringView sDumpAstOutputName = nsStringView())
  {
    nsExpression::StreamDesc inputs[] = {
      {s_sA, StreamDataTypeDeduction<T>::Type},
      {s_sB, StreamDataTypeDeduction<T>::Type},
      {s_sC, StreamDataTypeDeduction<T>::Type},
      {s_sD, StreamDataTypeDeduction<T>::Type},
    };

    nsExpression::StreamDesc outputs[] = {
      {s_sOutput, StreamDataTypeDeduction<T>::Type},
    };

    nsExpressionAST ast;
    NS_TEST_BOOL(s_pParser->Parse(sCode, inputs, outputs, {}, ast).Succeeded());

    nsStringBuilder sOutputPath;
    if (sDumpAstOutputName.IsEmpty() == false)
    {
      MakeASTOutputPath(sDumpAstOutputName, sOutputPath);
    }
    NS_TEST_BOOL(s_pCompiler->Compile(ast, out_byteCode, sOutputPath).Succeeded());
  }

  template <typename T>
  T Execute(const nsExpressionByteCode& byteCode, T a = T(0), T b = T(0), T c = T(0), T d = T(0))
  {
    nsProcessingStream inputs[] = {
      nsProcessingStream(s_sA, nsMakeArrayPtr(&a, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
      nsProcessingStream(s_sB, nsMakeArrayPtr(&b, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
      nsProcessingStream(s_sC, nsMakeArrayPtr(&c, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
      nsProcessingStream(s_sD, nsMakeArrayPtr(&d, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
    };

    T output = StreamDataTypeDeduction<T>::Default();
    nsProcessingStream outputs[] = {
      nsProcessingStream(s_sOutput, nsMakeArrayPtr(&output, 1).ToByteArray(), StreamDataTypeDeduction<T>::Type),
    };

    NS_TEST_BOOL(s_pVM->Execute(byteCode, inputs, outputs, 1).Succeeded());

    return output;
  };

  template <typename T>
  T TestInstruction(nsStringView sCode, T a = T(0), T b = T(0), T c = T(0), T d = T(0), bool bDumpASTs = false)
  {
    nsExpressionByteCode byteCode;
    Compile<T>(sCode, byteCode, bDumpASTs ? "TestInstruction" : "");
    return Execute<T>(byteCode, a, b, c, d);
  }

  template <typename T>
  T TestConstant(nsStringView sCode, bool bDumpASTs = false)
  {
    nsExpressionByteCode byteCode;
    Compile<T>(sCode, byteCode, bDumpASTs ? "TestConstant" : "");
    NS_TEST_INT(byteCode.GetNumInstructions(), 2); // MovX_C, StoreX
    NS_TEST_INT(byteCode.GetNumTempRegisters(), 1);
    return Execute<T>(byteCode);
  }

  enum TestBinaryFlags
  {
    LeftConstantOptimization = NS_BIT(0),
    NoInstructionsCountCheck = NS_BIT(2),
  };

  template <typename R, typename T, nsUInt32 flags>
  void TestBinaryInstruction(nsStringView sOp, T a, T b, T expectedResult, bool bDumpASTs = false)
  {
    constexpr bool boolInputs = std::is_same<T, bool>::value;
    using U = typename std::conditional<boolInputs, int, T>::type;

    U aAsU;
    U bAsU;
    U expectedResultAsU;
    if constexpr (boolInputs)
    {
      aAsU = a ? 1 : 0;
      bAsU = b ? 1 : 0;
      expectedResultAsU = expectedResult ? 1 : 0;
    }
    else
    {
      aAsU = a;
      bAsU = b;
      expectedResultAsU = expectedResult;
    }

    auto TestRes = [](U res, U expectedRes, const char* szCode, const char* szAValue, const char* szBValue)
    {
      if constexpr (std::is_same<T, float>::value)
      {
        NS_TEST_FLOAT_MSG(res, expectedRes, nsMath::DefaultEpsilon<float>(), "%s (a=%s, b=%s)", szCode, szAValue, szBValue);
      }
      else if constexpr (std::is_same<T, int>::value)
      {
        NS_TEST_INT_MSG(res, expectedRes, "%s (a=%s, b=%s)", szCode, szAValue, szBValue);
      }
      else if constexpr (std::is_same<T, bool>::value)
      {
        const char* szRes = (res != 0) ? "true" : "false";
        const char* szExpectedRes = (expectedRes != 0) ? "true" : "false";
        NS_TEST_STRING_MSG(szRes, szExpectedRes, "%s (a=%s, b=%s)", szCode, szAValue, szBValue);
      }
      else if constexpr (std::is_same<T, nsVec3>::value)
      {
        NS_TEST_VEC3_MSG(res, expectedRes, nsMath::DefaultEpsilon<float>(), "%s (a=%s, b=%s)", szCode, szAValue, szBValue);
      }
      else if constexpr (std::is_same<T, nsVec3I32>::value)
      {
        NS_TEST_INT_MSG(res.x, expectedRes.x, "%s (a=%s, b=%s)", szCode, szAValue, szBValue);
        NS_TEST_INT_MSG(res.y, expectedRes.y, "%s (a=%s, b=%s)", szCode, szAValue, szBValue);
        NS_TEST_INT_MSG(res.z, expectedRes.z, "%s (a=%s, b=%s)", szCode, szAValue, szBValue);
      }
      else
      {
        NS_ASSERT_NOT_IMPLEMENTED;
      }
    };

    const bool functionStyleSyntax = sOp.FindSubString("(");
    const char* formatString = functionStyleSyntax ? "output = {0}{1}, {2})" : "output = {1} {0} {2}";
    const char* aInput = boolInputs ? "(a != 0)" : "a";
    const char* bInput = boolInputs ? "(b != 0)" : "b";

    nsStringBuilder aValue;
    nsStringBuilder bValue;
    if constexpr (std::is_same<T, nsVec3>::value || std::is_same<T, nsVec3I32>::value)
    {
      aValue.SetFormat("vec3({}, {}, {})", a.x, a.y, a.z);
      bValue.SetFormat("vec3({}, {}, {})", b.x, b.y, b.z);
    }
    else
    {
      aValue.SetFormat("{}", a);
      bValue.SetFormat("{}", b);
    }

    int oneConstantInstructions = 3; // LoadX, OpX_RC, StoreX
    int oneConstantRegisters = 1;
    if constexpr (std::is_same<R, bool>::value)
    {
      oneConstantInstructions += 3; // + MovX_C, MovX_C, SelI_RRR
      oneConstantRegisters += 2;    // Two more registers needed for constants above
    }
    if constexpr (boolInputs)
    {
      oneConstantInstructions += 1; // + NotEqI_RC
    }

    int numOutputElements = 1;
    bool hasDifferentOutputElements = false;
    if constexpr (std::is_same<T, nsVec3>::value || std::is_same<T, nsVec3I32>::value)
    {
      numOutputElements = 3;

      for (int i = 1; i < 3; ++i)
      {
        if (expectedResult.GetData()[i] != expectedResult.GetData()[i - 1])
        {
          hasDifferentOutputElements = true;
          break;
        }
      }
    }

    nsStringBuilder code;
    nsExpressionByteCode byteCode;

    code.SetFormat(formatString, sOp, aInput, bInput);
    Compile<U>(code, byteCode, bDumpASTs ? "BinaryNoConstants" : "");
    TestRes(Execute<U>(byteCode, aAsU, bAsU), expectedResultAsU, code, aValue, bValue);

    code.SetFormat(formatString, sOp, aValue, bInput);
    Compile<U>(code, byteCode, bDumpASTs ? "BinaryLeftConstant" : "");
    if constexpr ((flags & NoInstructionsCountCheck) == 0)
    {
      int leftConstantInstructions = oneConstantInstructions;
      int leftConstantRegisters = oneConstantRegisters;
      if constexpr ((flags & LeftConstantOptimization) == 0)
      {
        leftConstantInstructions += 1;
        leftConstantRegisters += 1;
      }

      if (byteCode.GetNumInstructions() != leftConstantInstructions || byteCode.GetNumTempRegisters() != leftConstantRegisters)
      {
        DumpDisassembly(byteCode, "BinaryLeftConstant", 0);
        NS_TEST_INT(byteCode.GetNumInstructions(), leftConstantInstructions);
        NS_TEST_INT(byteCode.GetNumTempRegisters(), leftConstantRegisters);
      }
    }
    TestRes(Execute<U>(byteCode, aAsU, bAsU), expectedResultAsU, code, aValue, bValue);

    code.SetFormat(formatString, sOp, aInput, bValue);
    Compile<U>(code, byteCode, bDumpASTs ? "BinaryRightConstant" : "");
    if constexpr ((flags & NoInstructionsCountCheck) == 0)
    {
      if (byteCode.GetNumInstructions() != oneConstantInstructions || byteCode.GetNumTempRegisters() != oneConstantRegisters)
      {
        DumpDisassembly(byteCode, "BinaryRightConstant", 0);
        NS_TEST_INT(byteCode.GetNumInstructions(), oneConstantInstructions);
        NS_TEST_INT(byteCode.GetNumTempRegisters(), oneConstantRegisters);
      }
    }
    TestRes(Execute<U>(byteCode, aAsU, bAsU), expectedResultAsU, code, aValue, bValue);

    code.SetFormat(formatString, sOp, aValue, bValue);
    Compile<U>(code, byteCode, bDumpASTs ? "BinaryConstant" : "");
    if (hasDifferentOutputElements == false)
    {
      int bothConstantsInstructions = 1 + numOutputElements; // MovX_C + StoreX * numOutputElements
      int bothConstantsRegisters = 1;
      if (byteCode.GetNumInstructions() != bothConstantsInstructions || byteCode.GetNumTempRegisters() != bothConstantsRegisters)
      {
        DumpDisassembly(byteCode, "BinaryConstant", 0);
        NS_TEST_INT(byteCode.GetNumInstructions(), bothConstantsInstructions);
        NS_TEST_INT(byteCode.GetNumTempRegisters(), bothConstantsRegisters);
      }
    }
    TestRes(Execute<U>(byteCode), expectedResultAsU, code, aValue, bValue);
  }

  template <typename T>
  bool CompareCode(nsStringView sTestCode, nsStringView sReferenceCode, nsExpressionByteCode& out_testByteCode, bool bDumpASTs = false)
  {
    Compile<T>(sTestCode, out_testByteCode, bDumpASTs ? "Test" : "");

    nsExpressionByteCode referenceByteCode;
    Compile<T>(sReferenceCode, referenceByteCode, bDumpASTs ? "Reference" : "");

    return CompareByteCode(out_testByteCode, referenceByteCode);
  }

  template <typename T>
  void TestInputOutput()
  {
    nsStringView testCode = "output = a + b * 2";
    nsExpressionByteCode testByteCode;
    Compile<T>(testCode, testByteCode);

    constexpr nsUInt32 uiCount = 17;
    nsHybridArray<T, uiCount> a;
    nsHybridArray<T, uiCount> b;
    nsHybridArray<T, uiCount> o;
    nsHybridArray<float, uiCount> expectedOutput;
    a.SetCountUninitialized(uiCount);
    b.SetCountUninitialized(uiCount);
    o.SetCount(uiCount);
    expectedOutput.SetCountUninitialized(uiCount);

    for (nsUInt32 i = 0; i < uiCount; ++i)
    {
      a[i] = static_cast<T>(3.0f * i);
      b[i] = static_cast<T>(1.5f * i);
      expectedOutput[i] = a[i] + b[i] * 2.0f;
    }

    nsProcessingStream inputs[] = {
      nsProcessingStream(s_sA, a.GetByteArrayPtr(), StreamDataTypeDeduction<T>::Type),
      nsProcessingStream(s_sB, b.GetByteArrayPtr(), StreamDataTypeDeduction<T>::Type),
      nsProcessingStream(s_sC, a.GetByteArrayPtr(), StreamDataTypeDeduction<T>::Type), // Dummy stream, not actually used
      nsProcessingStream(s_sD, a.GetByteArrayPtr(), StreamDataTypeDeduction<T>::Type), // Dummy stream, not actually used
    };

    nsProcessingStream outputs[] = {
      nsProcessingStream(s_sOutput, o.GetByteArrayPtr(), StreamDataTypeDeduction<T>::Type),
    };

    NS_TEST_BOOL(s_pVM->Execute(testByteCode, inputs, outputs, uiCount, nsExpression::GlobalData(), nsExpressionVM::Flags::BestPerformance).Succeeded());

    for (nsUInt32 i = 0; i < uiCount; ++i)
    {
      NS_TEST_FLOAT(static_cast<float>(o[i]), expectedOutput[i], nsMath::DefaultEpsilon<float>());
    }
  }

  static const nsEnum<nsExpression::RegisterType> s_TestFunc1InputTypes[] = {nsExpression::RegisterType::Float, nsExpression::RegisterType::Int};
  static const nsEnum<nsExpression::RegisterType> s_TestFunc2InputTypes[] = {nsExpression::RegisterType::Float, nsExpression::RegisterType::Float, nsExpression::RegisterType::Int};

  static void TestFunc1(nsExpression::Inputs inputs, nsExpression::Output output, const nsExpression::GlobalData& globalData)
  {
    const nsExpression::Register* pX = inputs[0].GetPtr();
    const nsExpression::Register* pY = inputs[1].GetPtr();
    const nsExpression::Register* pXEnd = inputs[0].GetEndPtr();
    nsExpression::Register* pOutput = output.GetPtr();

    while (pX < pXEnd)
    {
      pOutput->f = pX->f.CompMul(pY->i.ToFloat());

      ++pX;
      ++pY;
      ++pOutput;
    }
  }

  static void TestFunc2(nsExpression::Inputs inputs, nsExpression::Output output, const nsExpression::GlobalData& globalData)
  {
    const nsExpression::Register* pX = inputs[0].GetPtr();
    const nsExpression::Register* pY = inputs[1].GetPtr();
    const nsExpression::Register* pXEnd = inputs[0].GetEndPtr();
    nsExpression::Register* pOutput = output.GetPtr();

    if (inputs.GetCount() >= 3)
    {
      const nsExpression::Register* pZ = inputs[2].GetPtr();

      while (pX < pXEnd)
      {
        pOutput->f = pX->f.CompMul(pY->f) * 2.0f + pZ->i.ToFloat();

        ++pX;
        ++pY;
        ++pZ;
        ++pOutput;
      }
    }
    else
    {
      while (pX < pXEnd)
      {
        pOutput->f = pX->f.CompMul(pY->f) * 2.0f;

        ++pX;
        ++pY;
        ++pOutput;
      }
    }
  }

  nsExpressionFunction s_TestFunc1 = {
    {nsMakeHashedString("TestFunc"), nsMakeArrayPtr(s_TestFunc1InputTypes), 2, nsExpression::RegisterType::Float},
    &TestFunc1,
  };

  nsExpressionFunction s_TestFunc2 = {
    {nsMakeHashedString("TestFunc"), nsMakeArrayPtr(s_TestFunc2InputTypes), 3, nsExpression::RegisterType::Float},
    &TestFunc2,
  };

} // namespace

NS_CREATE_SIMPLE_TEST(CodeUtils, Expression)
{
  s_uiNumByteCodeComparisons = 0;

  nsStringBuilder outputPath = nsTestFramework::GetInstance()->GetAbsOutputPath();
  NS_TEST_BOOL(nsFileSystem::AddDataDirectory(outputPath.GetData(), "test", "output", nsFileSystem::AllowWrites) == NS_SUCCESS);

  s_pParser = NS_DEFAULT_NEW(nsExpressionParser);
  s_pCompiler = NS_DEFAULT_NEW(nsExpressionCompiler);
  s_pVM = NS_DEFAULT_NEW(nsExpressionVM);
  NS_SCOPE_EXIT(s_pParser = nullptr; s_pCompiler = nullptr; s_pVM = nullptr;);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Unary instructions")
  {
    // Negate
    NS_TEST_INT(TestInstruction("output = -a", 2), -2);
    NS_TEST_FLOAT(TestInstruction("output = -a", 2.5f), -2.5f, nsMath::DefaultEpsilon<float>());
    NS_TEST_INT(TestConstant<int>("output = -2"), -2);
    NS_TEST_FLOAT(TestConstant<float>("output = -2.5"), -2.5f, nsMath::DefaultEpsilon<float>());

    // Absolute
    NS_TEST_INT(TestInstruction("output = abs(a)", -2), 2);
    NS_TEST_FLOAT(TestInstruction("output = abs(a)", -2.5f), 2.5f, nsMath::DefaultEpsilon<float>());
    NS_TEST_INT(TestConstant<int>("output = abs(-2)"), 2);
    NS_TEST_FLOAT(TestConstant<float>("output = abs(-2.5)"), 2.5f, nsMath::DefaultEpsilon<float>());

    // Saturate
    NS_TEST_INT(TestInstruction("output = saturate(a)", -1), 0);
    NS_TEST_INT(TestInstruction("output = saturate(a)", 2), 1);
    NS_TEST_FLOAT(TestInstruction("output = saturate(a)", -1.5f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = saturate(a)", 2.5f), 1.0f, nsMath::DefaultEpsilon<float>());

    NS_TEST_INT(TestConstant<int>("output = saturate(-1)"), 0);
    NS_TEST_INT(TestConstant<int>("output = saturate(2)"), 1);
    NS_TEST_FLOAT(TestConstant<float>("output = saturate(-1.5)"), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = saturate(2.5)"), 1.0f, nsMath::DefaultEpsilon<float>());

    // Sqrt
    NS_TEST_FLOAT(TestInstruction("output = sqrt(a)", 25.0f), 5.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = sqrt(a)", 2.0f), nsMath::Sqrt(2.0f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = sqrt(25)"), 5.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = sqrt(2)"), nsMath::Sqrt(2.0f), nsMath::DefaultEpsilon<float>());

    // Exp
    NS_TEST_FLOAT(TestInstruction("output = exp(a)", 0.0f), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = exp(a)", 2.0f), nsMath::Exp(2.0f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = exp(0.0)"), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = exp(2.0)"), nsMath::Exp(2.0f), nsMath::DefaultEpsilon<float>());

    // Ln
    NS_TEST_FLOAT(TestInstruction("output = ln(a)", 1.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = ln(a)", 2.0f), nsMath::Ln(2.0f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = ln(1.0)"), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = ln(2.0)"), nsMath::Ln(2.0f), nsMath::DefaultEpsilon<float>());

    // Log2
    NS_TEST_INT(TestInstruction("output = log2(a)", 1), 0);
    NS_TEST_INT(TestInstruction("output = log2(a)", 8), 3);
    NS_TEST_FLOAT(TestInstruction("output = log2(a)", 1.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = log2(a)", 4.0f), 2.0f, nsMath::DefaultEpsilon<float>());

    NS_TEST_INT(TestConstant<int>("output = log2(1)"), 0);
    NS_TEST_INT(TestConstant<int>("output = log2(16)"), 4);
    NS_TEST_FLOAT(TestConstant<float>("output = log2(1.0)"), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = log2(32.0)"), 5.0f, nsMath::DefaultEpsilon<float>());

    // Log10
    NS_TEST_FLOAT(TestInstruction("output = log10(a)", 10.0f), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = log10(a)", 1000.0f), 3.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = log10(10.0)"), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = log10(100.0)"), 2.0f, nsMath::DefaultEpsilon<float>());

    // Pow2
    NS_TEST_INT(TestInstruction("output = pow2(a)", 0), 1);
    NS_TEST_INT(TestInstruction("output = pow2(a)", 3), 8);
    NS_TEST_FLOAT(TestInstruction("output = pow2(a)", 4.0f), 16.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = pow2(a)", 6.0f), 64.0f, nsMath::DefaultEpsilon<float>());

    NS_TEST_INT(TestConstant<int>("output = pow2(0)"), 1);
    NS_TEST_INT(TestConstant<int>("output = pow2(3)"), 8);
    NS_TEST_FLOAT(TestConstant<float>("output = pow2(3.0)"), 8.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = pow2(5.0)"), 32.0f, nsMath::DefaultEpsilon<float>());

    // Sin
    NS_TEST_FLOAT(TestInstruction("output = sin(a)", nsAngle::MakeFromDegree(90.0f).GetRadian()), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = sin(a)", nsAngle::MakeFromDegree(45.0f).GetRadian()), nsMath::Sin(nsAngle::MakeFromDegree(45.0f)), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = sin(PI / 2)"), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = sin(PI / 4)"), nsMath::Sin(nsAngle::MakeFromDegree(45.0f)), nsMath::DefaultEpsilon<float>());

    // Cos
    NS_TEST_FLOAT(TestInstruction("output = cos(a)", 0.0f), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = cos(a)", nsAngle::MakeFromDegree(45.0f).GetRadian()), nsMath::Cos(nsAngle::MakeFromDegree(45.0f)), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = cos(0)"), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = cos(PI / 4)"), nsMath::Cos(nsAngle::MakeFromDegree(45.0f)), nsMath::DefaultEpsilon<float>());

    // Tan
    NS_TEST_FLOAT(TestInstruction("output = tan(a)", 0.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = tan(a)", nsAngle::MakeFromDegree(45.0f).GetRadian()), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = tan(0)"), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = tan(PI / 4)"), 1.0f, nsMath::DefaultEpsilon<float>());

    // ASin
    NS_TEST_FLOAT(TestInstruction("output = asin(a)", 1.0f), nsAngle::MakeFromDegree(90.0f).GetRadian(), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = asin(a)", nsMath::Sin(nsAngle::MakeFromDegree(45.0f))), nsAngle::MakeFromDegree(45.0f).GetRadian(), nsMath::LargeEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = asin(1)"), nsAngle::MakeFromDegree(90.0f).GetRadian(), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = asin(sin(PI / 4))"), nsAngle::MakeFromDegree(45.0f).GetRadian(), nsMath::LargeEpsilon<float>());

    // ACos
    NS_TEST_FLOAT(TestInstruction("output = acos(a)", 1.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = acos(a)", nsMath::Cos(nsAngle::MakeFromDegree(45.0f))), nsAngle::MakeFromDegree(45.0f).GetRadian(), nsMath::LargeEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = acos(1)"), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = acos(cos(PI / 4))"), nsAngle::MakeFromDegree(45.0f).GetRadian(), nsMath::LargeEpsilon<float>());

    // ATan
    NS_TEST_FLOAT(TestInstruction("output = atan(a)", 0.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = atan(a)", 1.0f), nsAngle::MakeFromDegree(45.0f).GetRadian(), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = atan(0)"), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = atan(1)"), nsAngle::MakeFromDegree(45.0f).GetRadian(), nsMath::DefaultEpsilon<float>());

    // RadToDeg
    NS_TEST_FLOAT(TestInstruction("output = radToDeg(a)", nsAngle::MakeFromDegree(135.0f).GetRadian()), 135.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = rad_to_deg(a)", nsAngle::MakeFromDegree(180.0f).GetRadian()), 180.0f, nsMath::LargeEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = radToDeg(PI / 2)"), 90.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = rad_to_deg(PI/4)"), 45.0f, nsMath::DefaultEpsilon<float>());

    // DegToRad
    NS_TEST_FLOAT(TestInstruction("output = degToRad(a)", 135.0f), nsAngle::MakeFromDegree(135.0f).GetRadian(), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = deg_to_rad(a)", 180.0f), nsAngle::MakeFromDegree(180.0f).GetRadian(), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = degToRad(90.0)"), nsAngle::MakeFromDegree(90.0f).GetRadian(), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = deg_to_rad(45)"), nsAngle::MakeFromDegree(45.0f).GetRadian(), nsMath::DefaultEpsilon<float>());

    // Round
    NS_TEST_FLOAT(TestInstruction("output = round(a)", 12.34f), 12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = round(a)", -12.34f), -12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = round(a)", 12.54f), 13, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = round(a)", -12.54f), -13, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = round(4.3)"), 4, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = round(4.51)"), 5, nsMath::DefaultEpsilon<float>());

    // Floor
    NS_TEST_FLOAT(TestInstruction("output = floor(a)", 12.34f), 12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = floor(a)", -12.34f), -13, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = floor(a)", 12.54f), 12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = floor(a)", -12.54f), -13, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = floor(4.3)"), 4, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = floor(4.51)"), 4, nsMath::DefaultEpsilon<float>());

    // Ceil
    NS_TEST_FLOAT(TestInstruction("output = ceil(a)", 12.34f), 13, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = ceil(a)", -12.34f), -12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = ceil(a)", 12.54f), 13, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = ceil(a)", -12.54f), -12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = ceil(4.3)"), 5, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = ceil(4.51)"), 5, nsMath::DefaultEpsilon<float>());

    // Trunc
    NS_TEST_FLOAT(TestInstruction("output = trunc(a)", 12.34f), 12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = trunc(a)", -12.34f), -12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = trunc(a)", 12.54f), 12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = trunc(a)", -12.54f), -12, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = trunc(4.3)"), 4, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = trunc(4.51)"), 4, nsMath::DefaultEpsilon<float>());

    // Frac
    NS_TEST_FLOAT(TestInstruction("output = frac(a)", 12.34f), 0.34f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = frac(a)", -12.34f), -0.34f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = frac(a)", 12.54f), 0.54f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = frac(a)", -12.54f), -0.54f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = frac(4.3)"), 0.3f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = frac(4.51)"), 0.51f, nsMath::DefaultEpsilon<float>());

    // Length
    NS_TEST_VEC3(TestInstruction<nsVec3>("output = length(a)", nsVec3(0, 4, 3)), nsVec3(5), nsMath::DefaultEpsilon<float>());
    NS_TEST_VEC3(TestInstruction<nsVec3>("output = length(a)", nsVec3(-3, 4, 0)), nsVec3(5), nsMath::DefaultEpsilon<float>());

    // Normalize
    NS_TEST_VEC3(TestInstruction<nsVec3>("output = normalize(a)", nsVec3(1, 4, 3)), nsVec3(1, 4, 3).GetNormalized(), nsMath::DefaultEpsilon<float>());
    NS_TEST_VEC3(TestInstruction<nsVec3>("output = normalize(a)", nsVec3(-3, 7, 22)), nsVec3(-3, 7, 22).GetNormalized(), nsMath::DefaultEpsilon<float>());

    // Length and normalize optimization
    {
      nsStringView testCode = "var x = length(a); var na = normalize(a); output = b * x + na";
      nsStringView referenceCode = "var x = length(a); var na = a / x; output = b * x + na";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<nsVec3>(testCode, referenceCode, testByteCode));

      nsVec3 a = nsVec3(0, 4, 3);
      nsVec3 b = nsVec3(1, 0, 0);
      nsVec3 res = b * a.GetLength() + a.GetNormalized();
      NS_TEST_VEC3(Execute(testByteCode, a, b), res, nsMath::DefaultEpsilon<float>());
    }

    // BitwiseNot
    NS_TEST_INT(TestInstruction("output = ~a", 1), ~1);
    NS_TEST_INT(TestInstruction("output = ~a", 8), ~8);
    NS_TEST_INT(TestConstant<int>("output = ~1"), ~1);
    NS_TEST_INT(TestConstant<int>("output = ~17"), ~17);

    // LogicalNot
    NS_TEST_INT(TestInstruction("output = !(a == 1)", 1), 0);
    NS_TEST_INT(TestInstruction("output = !(a == 1)", 8), 1);
    NS_TEST_INT(TestConstant<int>("output = !(1 == 1)"), 0);
    NS_TEST_INT(TestConstant<int>("output = !(8 == 1)"), 1);

    // All
    NS_TEST_VEC3(TestInstruction("var t = (a == b); output = all(t)", nsVec3(1, 2, 3), nsVec3(1, 2, 3)), nsVec3(1), nsMath::DefaultEpsilon<float>());
    NS_TEST_VEC3(TestInstruction("var t = (a == b); output = all(t)", nsVec3(1, 2, 3), nsVec3(1, 2, 4)), nsVec3(0), nsMath::DefaultEpsilon<float>());

    // Any
    NS_TEST_VEC3(TestInstruction("var t = (a == b); output = any(t)", nsVec3(1, 2, 3), nsVec3(4, 5, 3)), nsVec3(1), nsMath::DefaultEpsilon<float>());
    NS_TEST_VEC3(TestInstruction("var t = (a == b); output = any(t)", nsVec3(1, 2, 3), nsVec3(4, 5, 6)), nsVec3(0), nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Binary instructions")
  {
    // Add
    TestBinaryInstruction<int, int, LeftConstantOptimization>("+", 3, 5, 8);
    TestBinaryInstruction<float, float, LeftConstantOptimization>("+", 3.5f, 5.3f, 8.8f);

    // Subtract
    TestBinaryInstruction<int, int, 0>("-", 9, 5, 4);
    TestBinaryInstruction<float, float, 0>("-", 9.5f, 5.3f, 4.2f);

    // Multiply
    TestBinaryInstruction<int, int, LeftConstantOptimization>("*", 3, 5, 15);
    TestBinaryInstruction<float, float, LeftConstantOptimization>("*", 3.5f, 5.3f, 18.55f);

    // Divide
    TestBinaryInstruction<int, int, 0>("/", 11, 5, 2);
    TestBinaryInstruction<int, int, NoInstructionsCountCheck>("/", -11, 4, -2); // divide by power of 2 optimization
    TestBinaryInstruction<int, int, 0>("/", 11, -4, -2);                        // divide by power of 2 optimization only works for positive divisors
    TestBinaryInstruction<float, float, 0>("/", 12.6f, 3.0f, 4.2f);

    // Modulo
    TestBinaryInstruction<int, int, NoInstructionsCountCheck>("%", 13, 5, 3);
    TestBinaryInstruction<int, int, NoInstructionsCountCheck>("%", -13, 5, -3);
    TestBinaryInstruction<int, int, NoInstructionsCountCheck>("%", 13, 4, 1);
    TestBinaryInstruction<int, int, NoInstructionsCountCheck>("%", -13, 4, -1);
    TestBinaryInstruction<float, float, NoInstructionsCountCheck>("%", 13.5, 5.0, 3.5);
    TestBinaryInstruction<float, float, NoInstructionsCountCheck>("mod(", -13.5, 5.0, -3.5);

    // Log
    TestBinaryInstruction<float, float, NoInstructionsCountCheck>("log(", 2.0f, 1024.0f, 10.0f);
    TestBinaryInstruction<float, float, NoInstructionsCountCheck>("log(", 7.1f, 81.62f, nsMath::Log(7.1f, 81.62f));

    // Pow
    TestBinaryInstruction<int, int, NoInstructionsCountCheck>("pow(", 2, 5, 32);
    TestBinaryInstruction<int, int, NoInstructionsCountCheck>("pow(", 3, 3, 27);

    // Pow is replaced by multiplication for constant exponents up until 16.
    // Test all of them to ensure the multiplication tables are correct.
    for (int i = 0; i <= 16; ++i)
    {
      nsStringBuilder testCode;
      testCode.SetFormat("output = pow(a, {})", i);

      nsExpressionByteCode testByteCode;
      Compile<int>(testCode, testByteCode);
      NS_TEST_INT(Execute(testByteCode, 3), nsMath::Pow(3, i));
    }

    {
      nsStringView testCode = "output = pow(a, 7)";
      nsStringView referenceCode = "var a2 = a * a; var a3 = a2 * a; var a6 = a3 * a3; output = a6 * a";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<int>(testCode, referenceCode, testByteCode));
      NS_TEST_INT(Execute(testByteCode, 3), 2187);
    }

    TestBinaryInstruction<float, float, NoInstructionsCountCheck>("pow(", 2.0, 5.0, 32.0);
    TestBinaryInstruction<float, float, NoInstructionsCountCheck>("pow(", 3.0f, 7.9f, nsMath::Pow(3.0f, 7.9f));

    {
      nsStringView testCode = "output = pow(a, 15.0)";
      nsStringView referenceCode = "var a2 = a * a; var a3 = a2 * a; var a6 = a3 * a3; var a12 = a6 * a6; output = a12 * a3";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<float>(testCode, referenceCode, testByteCode));
      NS_TEST_FLOAT(Execute(testByteCode, 2.1f), nsMath::Pow(2.1f, 15.0f), nsMath::DefaultEpsilon<float>());
    }

    // Min
    TestBinaryInstruction<int, int, LeftConstantOptimization>("min(", 11, 5, 5);
    TestBinaryInstruction<float, float, LeftConstantOptimization>("min(", 12.6f, 3.0f, 3.0f);

    // Max
    TestBinaryInstruction<int, int, LeftConstantOptimization>("max(", 11, 5, 11);
    TestBinaryInstruction<float, float, LeftConstantOptimization>("max(", 12.6f, 3.0f, 12.6f);

    // Dot
    TestBinaryInstruction<nsVec3, nsVec3, NoInstructionsCountCheck>("dot(", nsVec3(1, -2, 3), nsVec3(-5, -6, 7), nsVec3(28));
    TestBinaryInstruction<nsVec3I32, nsVec3I32, NoInstructionsCountCheck>("dot(", nsVec3I32(1, -2, 3), nsVec3I32(-5, -6, 7), nsVec3I32(28));

    // Cross
    TestBinaryInstruction<nsVec3, nsVec3, NoInstructionsCountCheck>("cross(", nsVec3(1, 0, 0), nsVec3(0, 1, 0), nsVec3(0, 0, 1));
    TestBinaryInstruction<nsVec3, nsVec3, NoInstructionsCountCheck>("cross(", nsVec3(0, 1, 0), nsVec3(0, 0, 1), nsVec3(1, 0, 0));
    TestBinaryInstruction<nsVec3, nsVec3, NoInstructionsCountCheck>("cross(", nsVec3(0, 0, 1), nsVec3(1, 0, 0), nsVec3(0, 1, 0));

    // Reflect
    TestBinaryInstruction<nsVec3, nsVec3, NoInstructionsCountCheck>("reflect(", nsVec3(1, 2, -1), nsVec3(0, 0, 1), nsVec3(1, 2, 1));

    // BitshiftLeft
    TestBinaryInstruction<int, int, 0>("<<", 11, 5, 11 << 5);

    // BitshiftRight
    TestBinaryInstruction<int, int, 0>(">>", 0xABCD, 8, 0xAB);

    // BitwiseAnd
    TestBinaryInstruction<int, int, LeftConstantOptimization>("&", 0xFFCD, 0xABFF, 0xABCD);

    // BitwiseXor
    TestBinaryInstruction<int, int, LeftConstantOptimization>("^", 0xFFCD, 0xABFF, 0xFFCD ^ 0xABFF);

    // BitwiseOr
    TestBinaryInstruction<int, int, LeftConstantOptimization>("|", 0x00CD, 0xAB00, 0xABCD);

    // Equal
    TestBinaryInstruction<bool, int, LeftConstantOptimization>("==", 11, 5, 0);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>("==", 12.6f, 3.0f, 0.0f);
    TestBinaryInstruction<bool, bool, LeftConstantOptimization>("==", true, false, false);

    // NotEqual
    TestBinaryInstruction<bool, int, LeftConstantOptimization>("!=", 11, 5, 1);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>("!=", 12.6f, 3.0f, 1.0f);
    TestBinaryInstruction<bool, bool, LeftConstantOptimization>("!=", true, false, true);

    // Less
    TestBinaryInstruction<bool, int, LeftConstantOptimization>("<", 11, 5, 0);
    TestBinaryInstruction<bool, int, LeftConstantOptimization>("<", 11, 11, 0);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>("<", 12.6f, 3.0f, 0.0f);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>("<", 12.6f, 12.6f, 0.0f);

    // LessEqual
    TestBinaryInstruction<bool, int, LeftConstantOptimization>("<=", 11, 5, 0);
    TestBinaryInstruction<bool, int, LeftConstantOptimization>("<=", 11, 11, 1);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>("<=", 12.6f, 3.0f, 0.0f);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>("<=", 12.6f, 12.6f, 1.0f);

    // Greater
    TestBinaryInstruction<bool, int, LeftConstantOptimization>(">", 11, 5, 1);
    TestBinaryInstruction<bool, int, LeftConstantOptimization>(">", 11, 11, 0);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>(">", 12.6f, 3.0f, 1.0f);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>(">", 12.6f, 12.6f, 0.0f);

    // GreaterEqual
    TestBinaryInstruction<bool, int, LeftConstantOptimization>(">=", 11, 5, 1);
    TestBinaryInstruction<bool, int, LeftConstantOptimization>(">=", 11, 11, 1);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>(">=", 12.6f, 3.0f, 1.0f);
    TestBinaryInstruction<bool, float, LeftConstantOptimization>(">=", 12.6f, 12.6f, 1.0f);

    // LogicalAnd
    TestBinaryInstruction<bool, bool, LeftConstantOptimization | NoInstructionsCountCheck>("&&", true, false, false);
    TestBinaryInstruction<bool, bool, LeftConstantOptimization>("&&", true, true, true);

    // LogicalOr
    TestBinaryInstruction<bool, bool, LeftConstantOptimization | NoInstructionsCountCheck>("||", true, false, true);
    TestBinaryInstruction<bool, bool, LeftConstantOptimization>("||", false, false, false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Ternary instructions")
  {
    // Clamp
    NS_TEST_INT(TestInstruction("output = clamp(a, b, c)", -1, 0, 10), 0);
    NS_TEST_INT(TestInstruction("output = clamp(a, b, c)", 2, 0, 10), 2);
    NS_TEST_FLOAT(TestInstruction("output = clamp(a, b, c)", -1.5f, 0.0f, 1.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = clamp(a, b, c)", 2.5f, 0.0f, 1.0f), 1.0f, nsMath::DefaultEpsilon<float>());

    NS_TEST_INT(TestConstant<int>("output = clamp(-1, 0, 10)"), 0);
    NS_TEST_INT(TestConstant<int>("output = clamp(2, 0, 10)"), 2);
    NS_TEST_FLOAT(TestConstant<float>("output = clamp(-1.5, 0, 2)"), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = clamp(2.5, 0, 2)"), 2.0f, nsMath::DefaultEpsilon<float>());

    // Select
    NS_TEST_INT(TestInstruction("output = (a == 1) ? b : c", 1, 2, 3), 2);
    NS_TEST_INT(TestInstruction("output = a != 1 ? b : c", 1, 2, 3), 3);
    NS_TEST_FLOAT(TestInstruction("output = (a == 1) ? b : c", 1.0f, 2.4f, 3.5f), 2.4f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = a != 1 ? b : c", 1.0f, 2.4f, 3.5f), 3.5f, nsMath::DefaultEpsilon<float>());
    NS_TEST_INT(TestInstruction("output = (a == 1) ? (b > 2) : (c > 2)", 1, 2, 3), 0);
    NS_TEST_INT(TestInstruction("output = a != 1 ? b > 2 : c > 2", 1, 2, 3), 1);

    NS_TEST_INT(TestConstant<int>("output = (1 == 1) ? 2 : 3"), 2);
    NS_TEST_INT(TestConstant<int>("output = 1 != 1 ? 2 : 3"), 3);
    NS_TEST_FLOAT(TestConstant<float>("output = (1.0 == 1.0) ? 2.4 : 3.5"), 2.4f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = 1.0 != 1.0 ? 2.4 : 3.5"), 3.5f, nsMath::DefaultEpsilon<float>());
    NS_TEST_INT(TestConstant<int>("output = (1 == 1) ? false : true"), 0);
    NS_TEST_INT(TestConstant<int>("output = 1 != 1 ? false : true"), 1);

    // Lerp
    NS_TEST_FLOAT(TestInstruction("output = lerp(a, b, c)", 1.0f, 5.0f, 0.75f), 4.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = lerp(a, b, c)", -1.0f, -11.0f, 0.1f), -2.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = lerp(1, 5, 0.75)"), 4.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = lerp(-1, -11, 0.1)"), -2.0f, nsMath::DefaultEpsilon<float>());

    // SmoothStep
    NS_TEST_FLOAT(TestInstruction("output = smoothstep(a, b, c)", 0.0f, 0.0f, 1.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smoothstep(a, b, c)", 0.2f, 0.0f, 1.0f), nsMath::SmoothStep(0.2f, 0.0f, 1.0f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smoothstep(a, b, c)", 0.5f, 0.0f, 1.0f), 0.5f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smoothstep(a, b, c)", 0.2f, 0.2f, 0.8f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smoothstep(a, b, c)", 0.4f, 0.2f, 0.8f), nsMath::SmoothStep(0.4f, 0.2f, 0.8f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = smoothstep(0.2, 0, 1)"), nsMath::SmoothStep(0.2f, 0.0f, 1.0f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = smoothstep(0.4, 0.2, 0.8)"), nsMath::SmoothStep(0.4f, 0.2f, 0.8f), nsMath::DefaultEpsilon<float>());

    // SmootherStep
    NS_TEST_FLOAT(TestInstruction("output = smootherstep(a, b, c)", 0.0f, 0.0f, 1.0f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smootherstep(a, b, c)", 0.2f, 0.0f, 1.0f), nsMath::SmootherStep(0.2f, 0.0f, 1.0f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smootherstep(a, b, c)", 0.5f, 0.0f, 1.0f), 0.5f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smootherstep(a, b, c)", 0.2f, 0.2f, 0.8f), 0.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestInstruction("output = smootherstep(a, b, c)", 0.4f, 0.2f, 0.8f), nsMath::SmootherStep(0.4f, 0.2f, 0.8f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = smootherstep(0.2, 0, 1)"), nsMath::SmootherStep(0.2f, 0.0f, 1.0f), nsMath::DefaultEpsilon<float>());
    NS_TEST_FLOAT(TestConstant<float>("output = smootherstep(0.4, 0.2, 0.8)"), nsMath::SmootherStep(0.4f, 0.2f, 0.8f), nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Local variables")
  {
    nsExpressionByteCode referenceByteCode;
    {
      nsStringView code = "output = (a + b) * 2";
      Compile<float>(code, referenceByteCode);
    }

    nsExpressionByteCode testByteCode;

    nsStringView code = "var e = a + b; output = e * 2";
    Compile<float>(code, testByteCode);
    NS_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    code = "var e = a + b; e = e * 2; output = e";
    Compile<float>(code, testByteCode);
    NS_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    code = "var e = a + b; e *= 2; output = e";
    Compile<float>(code, testByteCode);
    NS_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    code = "var e = a + b; var f = e; e = 2; output = f * e";
    Compile<float>(code, testByteCode);
    NS_TEST_BOOL(CompareByteCode(testByteCode, referenceByteCode));

    NS_TEST_FLOAT(Execute(testByteCode, 2.0f, 3.0f), 10.0f, nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assignment")
  {
    {
      nsStringView testCode = "output = 40; output += 2";
      nsStringView referenceCode = "output = 42";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<float>(testCode, referenceCode, testByteCode));

      NS_TEST_FLOAT(Execute<float>(testByteCode), 42.0f, nsMath::DefaultEpsilon<float>());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Integer arithmetic")
  {
    nsExpressionByteCode testByteCode;

    nsStringView code = "output = ((a & 0xFF) << 8) | (b & 0xFFFF >> 8)";
    Compile<int>(code, testByteCode);

    const int a = 0xABABABAB;
    const int b = 0xCDCDCDCD;
    NS_TEST_INT(Execute(testByteCode, a, b), 0xABCD);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constant folding")
  {
    nsStringView testCode = "var x = abs(-7) + saturate(2) + 2\n"
                            "var v = (sqrt(25) - 4) * 5\n"
                            "var m = min(300, 1000) / max(1, 3);"
                            "var r = m - x * 5 - v - clamp(13, 1, 3);\n"
                            "output = r";

    nsStringView referenceCode = "output = 42";

    {
      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<float>(testCode, referenceCode, testByteCode));

      NS_TEST_FLOAT(Execute<float>(testByteCode), 42.0f, nsMath::DefaultEpsilon<float>());
    }

    {
      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<int>(testCode, referenceCode, testByteCode));

      NS_TEST_INT(Execute<int>(testByteCode), 42);
    }

    testCode = "";
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constant instructions")
  {
    // There are special instructions in the vm which take the constant as the first operand in place and
    // don't require an extra mov for the constant.
    // This test checks whether the compiler transforms operations with constants as second operands to the preferred form.

    nsStringView testCode = "output = (2 + a) + (-1 + b) + (2 * c) + (d / 5) + min(1, c) + max(2, d)";

    {
      nsStringView referenceCode = "output = (a + 2) + (b + -1) + (c * 2) + (d * 0.2) + min(c, 1) + max(d, 2)";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<float>(testCode, referenceCode, testByteCode));
      NS_TEST_INT(testByteCode.GetNumInstructions(), 16);
      NS_TEST_INT(testByteCode.GetNumTempRegisters(), 4);
      NS_TEST_FLOAT(Execute(testByteCode, 1.0f, 2.0f, 3.0f, 40.f), 59.0f, nsMath::DefaultEpsilon<float>());
    }

    {
      nsStringView referenceCode = "output = (a + 2) + (b + -1) + (c * 2) + (d / 5) + min(c, 1) + max(d, 2)";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<int>(testCode, referenceCode, testByteCode));
      NS_TEST_INT(testByteCode.GetNumInstructions(), 16);
      NS_TEST_INT(testByteCode.GetNumTempRegisters(), 4);
      NS_TEST_INT(Execute(testByteCode, 1, 2, 3, 40), 59);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Integer and float conversions")
  {
    nsStringView testCode = "var x = 7; var y = 0.6\n"
                            "var e = a * x * b * y\n"
                            "int i = c * 2; i *= i; e += i\n"
                            "output = e";

    nsStringView referenceCode = "int i = (int(c) * 2); output = int((float(a * 7 * b) * 0.6) + float(i * i))";

    nsExpressionByteCode testByteCode;
    NS_TEST_BOOL(CompareCode<int>(testCode, referenceCode, testByteCode));
    NS_TEST_INT(Execute(testByteCode, 1, 2, 3), 44);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Bool conversions")
  {
    nsStringView testCode = "var x = true\n"
                            "bool y = a\n"
                            "output = x == y";

    {
      nsStringView referenceCode = "bool r = true == (a != 0); output = r ? 1 : 0";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<int>(testCode, referenceCode, testByteCode));
      NS_TEST_INT(Execute(testByteCode, 14), 1);
    }

    {
      nsStringView referenceCode = "bool r = true == (a != 0); output = r ? 1.0 : 0.0";

      nsExpressionByteCode testByteCode;
      NS_TEST_BOOL(CompareCode<float>(testCode, referenceCode, testByteCode));
      NS_TEST_FLOAT(Execute(testByteCode, 15.0f), 1.0f, nsMath::DefaultEpsilon<float>());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Load Inputs/Store Outputs")
  {
    TestInputOutput<float>();
    TestInputOutput<nsFloat16>();

    TestInputOutput<int>();
    TestInputOutput<nsInt16>();
    TestInputOutput<nsInt8>();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Function overloads")
  {
    s_pParser->RegisterFunction(s_TestFunc1.m_Desc);
    s_pParser->RegisterFunction(s_TestFunc2.m_Desc);

    s_pVM->RegisterFunction(s_TestFunc1);
    s_pVM->RegisterFunction(s_TestFunc2);

    {
      // take TestFunc1 overload for all ints
      nsStringView testCode = "output = TestFunc(1, 2, 3)";
      nsExpressionByteCode testByteCode;
      Compile<int>(testCode, testByteCode);
      NS_TEST_INT(Execute<int>(testByteCode), 2);
    }

    {
      // take TestFunc1 overload for float, int
      nsStringView testCode = "output = TestFunc(1.0, 2, 3)";
      nsExpressionByteCode testByteCode;
      Compile<int>(testCode, testByteCode);
      NS_TEST_INT(Execute<int>(testByteCode), 2);
    }

    {
      // take TestFunc2 overload for int, float
      nsStringView testCode = "output = TestFunc(1, 2.0, 3)";
      nsExpressionByteCode testByteCode;
      Compile<int>(testCode, testByteCode);
      NS_TEST_INT(Execute<int>(testByteCode), 7);
    }

    {
      // take TestFunc2 overload for all float
      nsStringView testCode = "output = TestFunc(1.0, 2.0, 3)";
      nsExpressionByteCode testByteCode;
      Compile<int>(testCode, testByteCode);
      NS_TEST_INT(Execute<int>(testByteCode), 7);
    }

    {
      // take TestFunc1 overload when only two params are given
      nsStringView testCode = "output = TestFunc(1.0, 2.0)";
      nsExpressionByteCode testByteCode;
      Compile<int>(testCode, testByteCode);
      NS_TEST_INT(Execute<int>(testByteCode), 2);
    }

    s_pParser->UnregisterFunction(s_TestFunc1.m_Desc);
    s_pParser->UnregisterFunction(s_TestFunc2.m_Desc);

    s_pVM->UnregisterFunction(s_TestFunc1);
    s_pVM->UnregisterFunction(s_TestFunc2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Common subexpression elimination")
  {
    nsStringView testCode = "var x1 = a * max(b, c)\n"
                            "var x2 = max(c, b) * a\n"
                            "var y1 = a * pow(2, 3)\n"
                            "var y2 = 8 * a\n"
                            "output = x1 + x2 + y1 + y2";

    nsStringView referenceCode = "var x = a * max(b, c); var y = a * 8; output = x + x + y + y";

    nsExpressionByteCode testByteCode;
    NS_TEST_BOOL(CompareCode<int>(testCode, referenceCode, testByteCode));
    NS_TEST_INT(Execute(testByteCode, 2, 4, 8), 64);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Vector constructors")
  {
    {
      nsStringView testCode = "var x = vec3(1, 2, 3)\n"
                              "var y = vec4(x, 4)\n"
                              "vec3 z = vec2(1, 2)\n"
                              "var w = vec4()\n"
                              "output = vec4(x) + y + vec4(z) + w";

      nsExpressionByteCode testByteCode;
      Compile<nsVec3>(testCode, testByteCode);
      NS_TEST_VEC3(Execute<nsVec3>(testByteCode), nsVec3(3, 6, 6), nsMath::DefaultEpsilon<float>());
    }

    {
      nsStringView testCode = "var x = vec4(a.xy, (vec2(6, 8) - vec2(3, 4)).xy)\n"
                              "var y = vec4(1, vec2(2, 3), 4)\n"
                              "var z = vec4(1, vec3(2, 3, 4))\n"
                              "var w = vec4(1, 2, a.zw)\n"
                              "var one = vec4(1)\n"
                              "output = vec4(x) + y + vec4(z) + w + one";

      nsExpressionByteCode testByteCode;
      Compile<nsVec3>(testCode, testByteCode);
      NS_TEST_VEC3(Execute(testByteCode, nsVec3(1, 2, 3)), nsVec3(5, 9, 13), nsMath::DefaultEpsilon<float>());
    }

    {
      nsStringView testCode = "var x = vec4(1, 2, 3, 4)\n"
                              "var y = x.z\n"
                              "x.yz = 7\n"
                              "x.xz = vec2(2, 7)\n"
                              "output = x * y";

      nsExpressionByteCode testByteCode;
      Compile<nsVec3>(testCode, testByteCode);
      NS_TEST_VEC3(Execute<nsVec3>(testByteCode), nsVec3(6, 21, 21), nsMath::DefaultEpsilon<float>());
    }

    {
      nsStringView testCode = "var x = 1\n"
                              "x.z = 7.5\n"
                              "output = x";

      nsExpressionByteCode testByteCode;
      Compile<nsVec3>(testCode, testByteCode);
      NS_TEST_VEC3(Execute<nsVec3>(testByteCode), nsVec3(1, 0, 7), nsMath::DefaultEpsilon<float>());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Vector instructions")
  {
    // The VM does only support scalar data types.
    // This test checks whether the compiler transforms everything correctly to scalar operation.

    nsStringView testCode = "output = a * vec3(1, 2, 3) + sqrt(b)";

    nsStringView referenceCode = "output.x = a.x + sqrt(b.x)\n"
                                 "output.y = a.y * 2 + sqrt(b.y)\n"
                                 "output.z = a.z * 3 + sqrt(b.z)";

    nsExpressionByteCode testByteCode;
    NS_TEST_BOOL(CompareCode<nsVec3>(testCode, referenceCode, testByteCode));
    NS_TEST_VEC3(Execute(testByteCode, nsVec3(1, 3, 5), nsVec3(4, 9, 16)), nsVec3(3, 9, 19), nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Vector swizzle")
  {
    nsStringView testCode = "var n = vec4(1, 2, 3, 4)\n"
                            "var m = vec4(5, 6, 7, 8)\n"
                            "var p = n.xxyy + m.zzww * m.abgr + n.w\n"
                            "output = p";

    // vec3(1, 1, 2) + vec3(7, 7, 8) * vec3(8, 7, 6) + 4
    // output.x = 1 + 7 * 8 + 4 = 61
    // output.y = 1 + 7 * 7 + 4 = 54
    // output.z = 2 + 8 * 6 + 4 = 54

    nsExpressionByteCode testByteCode;
    Compile<nsVec3>(testCode, testByteCode);
    NS_TEST_VEC3(Execute<nsVec3>(testByteCode), nsVec3(61, 54, 54), nsMath::DefaultEpsilon<float>());
  }
}
