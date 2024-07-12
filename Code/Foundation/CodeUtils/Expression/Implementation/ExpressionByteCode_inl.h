
NS_ALWAYS_INLINE const nsExpressionByteCode::StorageType* nsExpressionByteCode::GetByteCodeStart() const
{
  return m_pByteCode;
}

NS_ALWAYS_INLINE const nsExpressionByteCode::StorageType* nsExpressionByteCode::GetByteCodeEnd() const
{
  return m_pByteCode + m_uiByteCodeCount;
}

NS_ALWAYS_INLINE nsArrayPtr<const nsExpressionByteCode::StorageType> nsExpressionByteCode::GetByteCode() const
{
  return nsMakeArrayPtr(m_pByteCode, m_uiByteCodeCount);
}

NS_ALWAYS_INLINE nsUInt32 nsExpressionByteCode::GetNumInstructions() const
{
  return m_uiNumInstructions;
}

NS_ALWAYS_INLINE nsUInt32 nsExpressionByteCode::GetNumTempRegisters() const
{
  return m_uiNumTempRegisters;
}

NS_ALWAYS_INLINE nsArrayPtr<const nsExpression::StreamDesc> nsExpressionByteCode::GetInputs() const
{
  return nsMakeArrayPtr(m_pInputs, m_uiNumInputs);
}

NS_ALWAYS_INLINE nsArrayPtr<const nsExpression::StreamDesc> nsExpressionByteCode::GetOutputs() const
{
  return nsMakeArrayPtr(m_pOutputs, m_uiNumOutputs);
}

NS_ALWAYS_INLINE nsArrayPtr<const nsExpression::FunctionDesc> nsExpressionByteCode::GetFunctions() const
{
  return nsMakeArrayPtr(m_pFunctions, m_uiNumFunctions);
}

// static
NS_ALWAYS_INLINE nsExpressionByteCode::OpCode::Enum nsExpressionByteCode::GetOpCode(const StorageType*& ref_pByteCode)
{
  nsUInt32 uiOpCode = *ref_pByteCode;
  ++ref_pByteCode;
  return static_cast<OpCode::Enum>((uiOpCode >= 0 && uiOpCode < OpCode::Count) ? uiOpCode : 0);
}

// static
NS_ALWAYS_INLINE nsUInt32 nsExpressionByteCode::GetRegisterIndex(const StorageType*& ref_pByteCode)
{
  nsUInt32 uiIndex = *ref_pByteCode;
  ++ref_pByteCode;
  return uiIndex;
}

// static
NS_ALWAYS_INLINE nsExpression::Register nsExpressionByteCode::GetConstant(const StorageType*& ref_pByteCode)
{
  nsExpression::Register r;
  r.i = nsSimdVec4i(*ref_pByteCode);
  ++ref_pByteCode;
  return r;
}

// static
NS_ALWAYS_INLINE nsUInt32 nsExpressionByteCode::GetFunctionIndex(const StorageType*& ref_pByteCode)
{
  nsUInt32 uiIndex = *ref_pByteCode;
  ++ref_pByteCode;
  return uiIndex;
}

// static
NS_ALWAYS_INLINE nsUInt32 nsExpressionByteCode::GetFunctionArgCount(const StorageType*& ref_pByteCode)
{
  nsUInt32 uiArgCount = *ref_pByteCode;
  ++ref_pByteCode;
  return uiArgCount;
}
