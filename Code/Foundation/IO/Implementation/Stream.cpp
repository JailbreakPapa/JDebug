#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Stream.h>
#include <Foundation/IO/StringDeduplicationContext.h>
#include <Foundation/Strings/String.h>

wdStreamReader::wdStreamReader() = default;
wdStreamReader::~wdStreamReader() = default;

wdResult wdStreamReader::ReadString(wdStringBuilder& ref_sBuilder)
{
  if (auto context = wdStringDeduplicationReadContext::GetContext())
  {
    ref_sBuilder = context->DeserializeString(*this);
  }
  else
  {
    wdUInt32 uiCount = 0;
    WD_SUCCEED_OR_RETURN(ReadDWordValue(&uiCount));

    if (uiCount > 0)
    {
      // We access the string builder directly here to
      // read the string efficiently with one allocation
      ref_sBuilder.m_Data.Reserve(uiCount + 1);
      ref_sBuilder.m_Data.SetCountUninitialized(uiCount);
      ReadBytes(ref_sBuilder.m_Data.GetData(), uiCount);
      ref_sBuilder.m_uiCharacterCount = uiCount;
      ref_sBuilder.AppendTerminator();
    }
    else
    {
      ref_sBuilder.Clear();
    }
  }

  return WD_SUCCESS;
}

wdResult wdStreamReader::ReadString(wdString& ref_sString)
{
  wdStringBuilder tmp;
  const wdResult res = ReadString(tmp);
  ref_sString = tmp;

  return res;
}

wdStreamWriter::wdStreamWriter() = default;
wdStreamWriter::~wdStreamWriter() = default;

wdResult wdStreamWriter::WriteString(const wdStringView sStringView)
{
  const wdUInt32 uiCount = sStringView.GetElementCount();

  if (auto context = wdStringDeduplicationWriteContext::GetContext())
  {
    context->SerializeString(sStringView, *this);
  }
  else
  {
    WD_SUCCEED_OR_RETURN(WriteDWordValue(&uiCount));
    if (uiCount > 0)
    {
      WD_SUCCEED_OR_RETURN(WriteBytes(sStringView.GetStartPointer(), uiCount));
    }
  }

  return WD_SUCCESS;
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_Stream);
