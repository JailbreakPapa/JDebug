
nsUInt32 nsHashHelperString_NoCase::Hash(nsStringView value)
{
  nsHybridArray<char, 256> temp;
  temp.SetCountUninitialized(value.GetElementCount());
  nsMemoryUtils::Copy(temp.GetData(), value.GetStartPointer(), value.GetElementCount());
  const nsUInt32 uiElemCount = nsStringUtils::ToLowerString(temp.GetData(), temp.GetData() + value.GetElementCount());

  return nsHashingUtils::StringHashTo32(nsHashingUtils::xxHash64((void*)temp.GetData(), uiElemCount));
}

bool nsHashHelperString_NoCase::Equal(nsStringView lhs, nsStringView rhs)
{
  return lhs.IsEqual_NoCase(rhs);
}
