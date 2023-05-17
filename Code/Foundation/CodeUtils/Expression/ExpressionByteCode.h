#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/Containers/DynamicArray.h>

class wdStreamWriter;
class wdStreamReader;

class WD_FOUNDATION_DLL wdExpressionByteCode
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

  using StorageType = wdUInt32;

  wdExpressionByteCode();
  ~wdExpressionByteCode();

  bool operator==(const wdExpressionByteCode& other) const;
  bool operator!=(const wdExpressionByteCode& other) const { return !(*this == other); }

  void Clear();
  bool IsEmpty() const { return m_ByteCode.IsEmpty(); }

  const StorageType* GetByteCode() const;
  const StorageType* GetByteCodeEnd() const;

  wdUInt32 GetNumInstructions() const;
  wdUInt32 GetNumTempRegisters() const;
  wdArrayPtr<const wdExpression::StreamDesc> GetInputs() const;
  wdArrayPtr<const wdExpression::StreamDesc> GetOutputs() const;
  wdArrayPtr<const wdExpression::FunctionDesc> GetFunctions() const;

  static OpCode::Enum GetOpCode(const StorageType*& ref_pByteCode);
  static wdUInt32 GetRegisterIndex(const StorageType*& ref_pByteCode);
  static wdExpression::Register GetConstant(const StorageType*& ref_pByteCode);
  static wdUInt32 GetFunctionIndex(const StorageType*& ref_pByteCode);
  static wdUInt32 GetFunctionArgCount(const StorageType*& ref_pByteCode);

  void Disassemble(wdStringBuilder& out_sDisassembly) const;

  void Save(wdStreamWriter& inout_stream) const;
  wdResult Load(wdStreamReader& inout_stream);

private:
  friend class wdExpressionCompiler;

  wdDynamicArray<StorageType> m_ByteCode;
  wdDynamicArray<wdExpression::StreamDesc> m_Inputs;
  wdDynamicArray<wdExpression::StreamDesc> m_Outputs;
  wdDynamicArray<wdExpression::FunctionDesc> m_Functions;

  wdUInt32 m_uiNumInstructions = 0;
  wdUInt32 m_uiNumTempRegisters = 0;
};

#include <Foundation/CodeUtils/Expression/Implementation/ExpressionByteCode_inl.h>
