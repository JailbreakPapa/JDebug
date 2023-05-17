
WD_ALWAYS_INLINE const wdExpressionByteCode::StorageType* wdExpressionByteCode::GetByteCode() const
{
  return m_ByteCode.GetData();
}

WD_ALWAYS_INLINE const wdExpressionByteCode::StorageType* wdExpressionByteCode::GetByteCodeEnd() const
{
  return m_ByteCode.GetData() + m_ByteCode.GetCount();
}

WD_ALWAYS_INLINE wdUInt32 wdExpressionByteCode::GetNumInstructions() const
{
  return m_uiNumInstructions;
}

WD_ALWAYS_INLINE wdUInt32 wdExpressionByteCode::GetNumTempRegisters() const
{
  return m_uiNumTempRegisters;
}

WD_ALWAYS_INLINE wdArrayPtr<const wdExpression::StreamDesc> wdExpressionByteCode::GetInputs() const
{
  return m_Inputs;
}

WD_ALWAYS_INLINE wdArrayPtr<const wdExpression::StreamDesc> wdExpressionByteCode::GetOutputs() const
{
  return m_Outputs;
}

WD_ALWAYS_INLINE wdArrayPtr<const wdExpression::FunctionDesc> wdExpressionByteCode::GetFunctions() const
{
  return m_Functions;
}

// static
WD_ALWAYS_INLINE wdExpressionByteCode::OpCode::Enum wdExpressionByteCode::GetOpCode(const StorageType*& ref_pByteCode)
{
  wdUInt32 uiOpCode = *ref_pByteCode;
  ++ref_pByteCode;
  return static_cast<OpCode::Enum>((uiOpCode >= 0 && uiOpCode < OpCode::Count) ? uiOpCode : 0);
}

// static
WD_ALWAYS_INLINE wdUInt32 wdExpressionByteCode::GetRegisterIndex(const StorageType*& ref_pByteCode)
{
  wdUInt32 uiIndex = *ref_pByteCode;
  ++ref_pByteCode;
  return uiIndex;
}

// static
WD_ALWAYS_INLINE wdExpression::Register wdExpressionByteCode::GetConstant(const StorageType*& ref_pByteCode)
{
  wdExpression::Register r;
  r.i = wdSimdVec4i(*ref_pByteCode);
  ++ref_pByteCode;
  return r;
}

// static
WD_ALWAYS_INLINE wdUInt32 wdExpressionByteCode::GetFunctionIndex(const StorageType*& ref_pByteCode)
{
  wdUInt32 uiIndex = *ref_pByteCode;
  ++ref_pByteCode;
  return uiIndex;
}

// static
WD_ALWAYS_INLINE wdUInt32 wdExpressionByteCode::GetFunctionArgCount(const StorageType*& ref_pByteCode)
{
  wdUInt32 uiArgCount = *ref_pByteCode;
  ++ref_pByteCode;
  return uiArgCount;
}
