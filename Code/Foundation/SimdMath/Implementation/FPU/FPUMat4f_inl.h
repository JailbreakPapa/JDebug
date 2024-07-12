#pragma once

NS_ALWAYS_INLINE void nsSimdMat4f::Transpose()
{
  nsMath::Swap(m_col0.m_v.y, m_col1.m_v.x);
  nsMath::Swap(m_col0.m_v.z, m_col2.m_v.x);
  nsMath::Swap(m_col0.m_v.w, m_col3.m_v.x);
  nsMath::Swap(m_col1.m_v.z, m_col2.m_v.y);
  nsMath::Swap(m_col1.m_v.w, m_col3.m_v.y);
  nsMath::Swap(m_col2.m_v.w, m_col3.m_v.z);
}
