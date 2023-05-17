#pragma once

WD_ALWAYS_INLINE void wdSimdMat4f::Transpose()
{
  wdMath::Swap(m_col0.m_v.y, m_col1.m_v.x);
  wdMath::Swap(m_col0.m_v.z, m_col2.m_v.x);
  wdMath::Swap(m_col0.m_v.w, m_col3.m_v.x);
  wdMath::Swap(m_col1.m_v.z, m_col2.m_v.y);
  wdMath::Swap(m_col1.m_v.w, m_col3.m_v.y);
  wdMath::Swap(m_col2.m_v.w, m_col3.m_v.z);
}
