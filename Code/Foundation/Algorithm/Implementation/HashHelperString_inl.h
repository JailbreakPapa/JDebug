
wdUInt32 wdHashHelperString_NoCase::Hash(wdStringView value)
{
  wdHybridArray<char, 256> temp;
  temp.SetCountUninitialized(value.GetElementCount());
  wdMemoryUtils::Copy(temp.GetData(), value.GetStartPointer(), value.GetElementCount());
  const wdUInt32 uiElemCount = wdStringUtils::ToLowerString(temp.GetData(), temp.GetData() + value.GetElementCount());

  return wdHashingUtils::StringHashTo32(wdHashingUtils::xxHash64((void*)temp.GetData(), uiElemCount));
}

bool wdHashHelperString_NoCase::Equal(wdStringView lhs, wdStringView rhs)
{
  return lhs.IsEqual_NoCase(rhs);
}
