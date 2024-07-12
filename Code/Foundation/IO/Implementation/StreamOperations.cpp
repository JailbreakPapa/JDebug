#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Enum.h>

// C-style strings
// No read equivalent for C-style strings (but can be read as nsString & nsStringBuilder instances)

nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const char* szValue)
{
  nsStringView szView(szValue);
  inout_stream.WriteString(szView).AssertSuccess();

  return inout_stream;
}

nsStreamWriter& operator<<(nsStreamWriter& inout_stream, nsStringView sValue)
{
  inout_stream.WriteString(sValue).AssertSuccess();

  return inout_stream;
}

// nsStringBuilder

nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsStringBuilder& sValue)
{
  inout_stream.WriteString(sValue.GetView()).AssertSuccess();
  return inout_stream;
}

nsStreamReader& operator>>(nsStreamReader& inout_stream, nsStringBuilder& out_sValue)
{
  inout_stream.ReadString(out_sValue).AssertSuccess();
  return inout_stream;
}
