

const void* nsGALShaderByteCode::GetByteCode() const
{
  if (m_ByteCode.IsEmpty())
    return nullptr;

  return &m_ByteCode[0];
}

nsUInt32 nsGALShaderByteCode::GetSize() const
{
  return m_ByteCode.GetCount();
}

bool nsGALShaderByteCode::IsValid() const
{
  return !m_ByteCode.IsEmpty();
}
