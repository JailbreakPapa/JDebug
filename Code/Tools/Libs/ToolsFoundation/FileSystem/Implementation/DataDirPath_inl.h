

inline nsDataDirPath::nsDataDirPath() = default;

inline nsDataDirPath::nsDataDirPath(nsStringView sAbsPath, nsArrayPtr<nsString> dataDirRoots, nsUInt32 uiLastKnownDataDirIndex /*= 0*/)
{
  NS_ASSERT_DEBUG(!sAbsPath.EndsWith_NoCase("/"), "");
  m_sAbsolutePath = sAbsPath;
  UpdateDataDirInfos(dataDirRoots, uiLastKnownDataDirIndex);
}

inline nsDataDirPath::nsDataDirPath(const nsStringBuilder& sAbsPath, nsArrayPtr<nsString> dataDirRoots, nsUInt32 uiLastKnownDataDirIndex /*= 0*/)
{
  NS_ASSERT_DEBUG(!sAbsPath.EndsWith_NoCase("/"), "");
  m_sAbsolutePath = sAbsPath;
  UpdateDataDirInfos(dataDirRoots, uiLastKnownDataDirIndex);
}

inline nsDataDirPath::nsDataDirPath(nsString&& sAbsPath, nsArrayPtr<nsString> dataDirRoots, nsUInt32 uiLastKnownDataDirIndex /*= 0*/)
{
  NS_ASSERT_DEBUG(!sAbsPath.EndsWith_NoCase("/"), "");
  m_sAbsolutePath = std::move(sAbsPath);
  UpdateDataDirInfos(dataDirRoots, uiLastKnownDataDirIndex);
}

inline nsDataDirPath::operator nsStringView() const
{
  return m_sAbsolutePath;
}

inline bool nsDataDirPath::operator==(nsStringView rhs) const
{
  return m_sAbsolutePath == rhs;
}

inline bool nsDataDirPath::operator!=(nsStringView rhs) const
{
  return m_sAbsolutePath != rhs;
}

inline bool nsDataDirPath::IsValid() const
{
  return m_uiDataDirParent != 0;
}

inline void nsDataDirPath::Clear()
{
  m_sAbsolutePath.Clear();
  m_uiDataDirParent = 0;
  m_uiDataDirLength = 0;
  m_uiDataDirIndex = 0;
}

inline const nsString& nsDataDirPath::GetAbsolutePath() const
{
  return m_sAbsolutePath;
}

inline nsStringView nsDataDirPath::GetDataDirParentRelativePath() const
{
  NS_ASSERT_DEBUG(IsValid(), "Path is not in a data directory, only GetAbsolutePath is allowed to be called.");
  const nsUInt32 uiOffset = m_uiDataDirParent + 1;
  return nsStringView(m_sAbsolutePath.GetData() + uiOffset, m_sAbsolutePath.GetElementCount() - uiOffset);
}

inline nsStringView nsDataDirPath::GetDataDirRelativePath() const
{
  NS_ASSERT_DEBUG(IsValid(), "Path is not in a data directory, only GetAbsolutePath is allowed to be called.");
  const nsUInt32 uiOffset = nsMath::Min(m_sAbsolutePath.GetElementCount(), m_uiDataDirParent + m_uiDataDirLength + 1u);
  return nsStringView(m_sAbsolutePath.GetData() + uiOffset, m_sAbsolutePath.GetElementCount() - uiOffset);
}

inline nsStringView nsDataDirPath::GetDataDir() const
{
  NS_ASSERT_DEBUG(IsValid(), "Path is not in a data directory, only GetAbsolutePath is allowed to be called.");
  return nsStringView(m_sAbsolutePath.GetData(), m_uiDataDirParent + m_uiDataDirLength);
}

inline nsUInt8 nsDataDirPath::GetDataDirIndex() const
{
  NS_ASSERT_DEBUG(IsValid(), "Path is not in a data directory, only GetAbsolutePath is allowed to be called.");
  return m_uiDataDirIndex;
}

inline nsStreamWriter& nsDataDirPath::Write(nsStreamWriter& inout_stream) const
{
  inout_stream << m_sAbsolutePath;
  inout_stream << m_uiDataDirParent;
  inout_stream << m_uiDataDirLength;
  inout_stream << m_uiDataDirIndex;
  return inout_stream;
}

inline nsStreamReader& nsDataDirPath::Read(nsStreamReader& inout_stream)
{
  inout_stream >> m_sAbsolutePath;
  inout_stream >> m_uiDataDirParent;
  inout_stream >> m_uiDataDirLength;
  inout_stream >> m_uiDataDirIndex;
  return inout_stream;
}

bool nsCompareDataDirPath::Less(nsStringView lhs, nsStringView rhs)
{
  int res = lhs.Compare_NoCase(rhs);
  if (res == 0)
  {
    return lhs.Compare(rhs) < 0;
  }

  return res < 0;
}

bool nsCompareDataDirPath::Equal(nsStringView lhs, nsStringView rhs)
{
  return lhs.IsEqual(rhs);
}

inline nsStreamWriter& operator<<(nsStreamWriter& inout_stream, const nsDataDirPath& value)
{
  return value.Write(inout_stream);
}

inline nsStreamReader& operator>>(nsStreamReader& inout_stream, nsDataDirPath& out_value)
{
  return out_value.Read(inout_stream);
}
