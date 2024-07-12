#pragma once

nsTag::nsTag()


  = default;

bool nsTag::operator==(const nsTag& rhs) const
{
  return m_sTagString == rhs.m_sTagString;
}

bool nsTag::operator!=(const nsTag& rhs) const
{
  return m_sTagString != rhs.m_sTagString;
}

bool nsTag::operator<(const nsTag& rhs) const
{
  return m_sTagString < rhs.m_sTagString;
}

const nsString& nsTag::GetTagString() const
{
  return m_sTagString.GetString();
}

bool nsTag::IsValid() const
{
  return m_uiBlockIndex != 0xFFFFFFFEu;
}
