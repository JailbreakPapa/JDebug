#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Containers/Blob.h>

class nsStreamWriter;
class nsStreamReader;

class NS_FOUNDATION_DLL nsExpressionByteCode
{
public:
  struct OpCode
  {
    enum Enum
    {
      Nop,

      FirstUnary,

      AbsF_R,
      AbsI_R,
      SqrtF_R,

      ExpF_R,
      LnF_R,
      Log2F_R,
      Log2I_R,
      Log10F_R,
      Pow2F_R,

      SinF_R,
      CosF_R,
      TanF_R,

      ASinF_R,
      ACosF_R,
      ATanF_R,

      RoundF_R,
      FloorF_R,
      CeilF_R,
      TruncF_R,

      NotI_R,
      NotB_R,

      IToF_R,
      FToI_R,

      LastUnary,

      FirstBinary,

      AddF_RR,
      AddI_RR,

      SubF_RR,
      SubI_RR,

      MulF_RR,
      MulI_RR,

      DivF_RR,
      DivI_RR,

      MinF_RR,
      MinI_RR,

      MaxF_RR,
      MaxI_RR,

      ShlI_RR,
      ShrI_RR,
      AndI_RR,
      XorI_RR,
      OrI_RR,

      EqF_RR,
      EqI_RR,
      EqB_RR,

      NEqF_RR,
      NEqI_RR,
      NEqB_RR,

      LtF_RR,
      LtI_RR,

      LEqF_RR,
      LEqI_RR,

      GtF_RR,
      GtI_RR,

      GEqF_RR,
      GEqI_RR,

      AndB_RR,
      OrB_RR,

      LastBinary,

      FirstBinaryWithConstant,

      AddF_RC,
      AddI_RC,

      SubF_RC,
      SubI_RC,

      MulF_RC,
      MulI_RC,

      DivF_RC,
      DivI_RC,

      MinF_RC,
      MinI_RC,

      MaxF_RC,
      MaxI_RC,

      ShlI_RC,
      ShrI_RC,
      AndI_RC,
      XorI_RC,
      OrI_RC,

      EqF_RC,
      EqI_RC,
      EqB_RC,

      NEqF_RC,
      NEqI_RC,
      NEqB_RC,

      LtF_RC,
      LtI_RC,

      LEqF_RC,
      LEqI_RC,

      GtF_RC,
      GtI_RC,

      GEqF_RC,
      GEqI_RC,

      AndB_RC,
      OrB_RC,

      LastBinaryWithConstant,

      FirstTernary,

      SelF_RRR,
      SelI_RRR,
      SelB_RRR,

      LastTernary,

      FirstSpecial,

      MovX_R,
      MovX_C,
      LoadF,
      LoadI,
      StoreF,
      StoreI,

      Call,

      LastSpecial,

      Count
    };

    static const char* GetName(Enum code);
  };

  using StorageType = nsUInt32;

  nsExpressionByteCode();
  nsExpressionByteCode(const nsExpressionByteCode& other);
  ~nsExpressionByteCode();

  void operator=(const nsExpressionByteCode& other);

  bool operator==(const nsExpressionByteCode& other) const;
  bool operator!=(const nsExpressionByteCode& other) const { return !(*this == other); }

  void Clear();
  bool IsEmpty() const { return m_uiByteCodeCount == 0; }

  const StorageType* GetByteCodeStart() const;
  const StorageType* GetByteCodeEnd() const;
  nsArrayPtr<const StorageType> GetByteCode() const;

  nsUInt32 GetNumInstructions() const;
  nsUInt32 GetNumTempRegisters() const;
  nsArrayPtr<const nsExpression::StreamDesc> GetInputs() const;
  nsArrayPtr<const nsExpression::StreamDesc> GetOutputs() const;
  nsArrayPtr<const nsExpression::FunctionDesc> GetFunctions() const;

  static OpCode::Enum GetOpCode(const StorageType*& ref_pByteCode);
  static nsUInt32 GetRegisterIndex(const StorageType*& ref_pByteCode);
  static nsExpression::Register GetConstant(const StorageType*& ref_pByteCode);
  static nsUInt32 GetFunctionIndex(const StorageType*& ref_pByteCode);
  static nsUInt32 GetFunctionArgCount(const StorageType*& ref_pByteCode);

  void Disassemble(nsStringBuilder& out_sDisassembly) const;

  nsResult Save(nsStreamWriter& inout_stream) const;
  nsResult Load(nsStreamReader& inout_stream, nsByteArrayPtr externalMemory = nsByteArrayPtr());

  nsConstByteBlobPtr GetDataBlob() const { return m_Data.GetByteBlobPtr(); }

private:
  friend class nsExpressionCompiler;

  void Init(nsArrayPtr<const StorageType> byteCode, nsArrayPtr<const nsExpression::StreamDesc> inputs, nsArrayPtr<const nsExpression::StreamDesc> outputs, nsArrayPtr<const nsExpression::FunctionDesc> functions, nsUInt32 uiNumTempRegisters, nsUInt32 uiNumInstructions);

  nsBlob m_Data;

  nsExpression::StreamDesc* m_pInputs = nullptr;
  nsExpression::StreamDesc* m_pOutputs = nullptr;
  nsExpression::FunctionDesc* m_pFunctions = nullptr;
  StorageType* m_pByteCode = nullptr;

  nsUInt32 m_uiByteCodeCount = 0;
  nsUInt16 m_uiNumInputs = 0;
  nsUInt16 m_uiNumOutputs = 0;
  nsUInt16 m_uiNumFunctions = 0;

  nsUInt16 m_uiNumTempRegisters = 0;
  nsUInt32 m_uiNumInstructions = 0;
};

#if NS_ENABLED(NS_PLATFORM_64BIT)
static_assert(sizeof(nsExpressionByteCode) == 64);
#endif

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsExpressionByteCode);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsExpressionByteCode);

#include <Foundation/CodeUtils/Expression/Implementation/ExpressionByteCode_inl.h>
