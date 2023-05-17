#pragma once

wdTag::wdTag()
  : m_uiBlockIndex(0xFFFFFFFEu)
{
}

bool wdTag::operator==(const wdTag& rhs) const
{
  return m_sTagString == rhs.m_sTagString;
}

bool wdTag::operator!=(const wdTag& rhs) const
{
  return m_sTagString != rhs.m_sTagString;
}

bool wdTag::operator<(const wdTag& rhs) const
{
  return m_sTagString < rhs.m_sTagString;
}

const wdString& wdTag::GetTagString() const
{
  return m_sTagString.GetString();
}

bool wdTag::IsValid() const
{
  return m_uiBlockIndex != 0xFFFFFFFEu;
}
