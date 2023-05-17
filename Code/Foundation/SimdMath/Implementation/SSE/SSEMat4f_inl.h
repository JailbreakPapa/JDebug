#pragma once

WD_ALWAYS_INLINE void wdSimdMat4f::Transpose()
{
  _MM_TRANSPOSE4_PS(m_col0.m_v, m_col1.m_v, m_col2.m_v, m_col3.m_v);
}
