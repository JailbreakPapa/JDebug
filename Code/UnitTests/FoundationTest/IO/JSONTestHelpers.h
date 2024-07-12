#pragma once

#include <Foundation/IO/OSFile.h>

class StreamComparer : public nsStreamWriter
{
public:
  StreamComparer(const char* szExpectedData, bool bOnlyWriteResult = false)
  {
    m_bOnlyWriteResult = bOnlyWriteResult;
    m_szExpectedData = szExpectedData;
  }

  ~StreamComparer()
  {
    if (m_bOnlyWriteResult)
    {
      nsOSFile f;
      f.Open("C:\\Code\\JSON.txt", nsFileOpenMode::Write).IgnoreResult();
      f.Write(m_sResult.GetData(), m_sResult.GetElementCount()).IgnoreResult();
      f.Close();
    }
    else
      NS_TEST_BOOL(*m_szExpectedData == '\0');
  }

  nsResult WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
  {
    if (m_bOnlyWriteResult)
      m_sResult.Append((const char*)pWriteBuffer);
    else
    {
      const char* szWritten = (const char*)pWriteBuffer;

      NS_TEST_BOOL(nsMemoryUtils::IsEqual(szWritten, m_szExpectedData, (nsUInt32)uiBytesToWrite));
      m_szExpectedData += uiBytesToWrite;
    }

    return NS_SUCCESS;
  }

private:
  bool m_bOnlyWriteResult;
  nsStringBuilder m_sResult;
  const char* m_szExpectedData;
};


class StringStream : public nsStreamReader
{
public:
  StringStream(const void* pData)
  {
    m_pData = pData;
    m_uiLength = nsStringUtils::GetStringElementCount((const char*)pData);
  }

  virtual nsUInt64 ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
  {
    uiBytesToRead = nsMath::Min(uiBytesToRead, m_uiLength);
    m_uiLength -= uiBytesToRead;

    if (uiBytesToRead > 0)
    {
      nsMemoryUtils::Copy((nsUInt8*)pReadBuffer, (nsUInt8*)m_pData, (size_t)uiBytesToRead);
      m_pData = nsMemoryUtils::AddByteOffset(m_pData, (ptrdiff_t)uiBytesToRead);
    }

    return uiBytesToRead;
  }

private:
  const void* m_pData;
  nsUInt64 m_uiLength;
};
