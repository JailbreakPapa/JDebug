#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Enum.h>

// C-style strings
// No read equivalent for C-style strings (but can be read as wdString & wdStringBuilder instances)

wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const char* szValue)
{
  wdStringView szView(szValue);
  inout_stream.WriteString(szView).AssertSuccess();

  return inout_stream;
}

wdStreamWriter& operator<<(wdStreamWriter& inout_stream, wdStringView sValue)
{
  inout_stream.WriteString(sValue).AssertSuccess();

  return inout_stream;
}

// wdStringBuilder

wdStreamWriter& operator<<(wdStreamWriter& inout_stream, const wdStringBuilder& sValue)
{
  inout_stream.WriteString(sValue.GetView()).AssertSuccess();
  return inout_stream;
}

wdStreamReader& operator>>(wdStreamReader& inout_stream, wdStringBuilder& out_sValue)
{
  inout_stream.ReadString(out_sValue).AssertSuccess();
  return inout_stream;
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_StreamOperations);
