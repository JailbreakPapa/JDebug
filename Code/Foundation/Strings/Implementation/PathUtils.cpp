#include <Foundation/FoundationPCH.h>

#include <Foundation/Strings/Implementation/StringIterator.h>
#include <Foundation/Strings/StringBuilder.h>

const char* nsPathUtils::FindPreviousSeparator(const char* szPathStart, const char* szStartSearchAt)
{
  if (nsStringUtils::IsNullOrEmpty(szPathStart))
    return nullptr;

  while (szStartSearchAt > szPathStart)
  {
    nsUnicodeUtils::MoveToPriorUtf8(szStartSearchAt, szPathStart).AssertSuccess();

    if (IsPathSeparator(*szStartSearchAt))
      return szStartSearchAt;
  }

  return nullptr;
}

bool nsPathUtils::HasAnyExtension(nsStringView sPath)
{
  return !GetFileExtension(sPath, true).IsEmpty();
}

bool nsPathUtils::HasExtension(nsStringView sPath, nsStringView sExtension)
{
  sPath = GetFileNameAndExtension(sPath);
  nsStringView fullExt = GetFileExtension(sPath, true);

  if (sExtension.IsEmpty() && fullExt.IsEmpty())
    return true;

  // if there is a single dot at the start of the extension, remove it
  if (sExtension.StartsWith("."))
    sExtension.ChopAwayFirstCharacterAscii();

  if (!fullExt.EndsWith_NoCase(sExtension))
    return false;

  // remove the checked extension
  sPath = nsStringView(sPath.GetStartPointer(), sPath.GetEndPointer() - sExtension.GetElementCount());

  // checked extension didn't start with a dot -> make sure there is one at the end of sPath
  if (!sPath.EndsWith("."))
    return false;

  // now make sure the rest isn't just the dot
  return sPath.GetElementCount() > 1;
}

nsStringView nsPathUtils::GetFileExtension(nsStringView sPath, bool bFullExtension)
{
  // get rid of any path before the filename
  sPath = GetFileNameAndExtension(sPath);

  // ignore all dots that the file name may start with (".", "..", ".file", "..file", etc)
  // filename may be empty afterwards, which means no dot will be found -> no extension
  while (sPath.StartsWith("."))
    sPath.ChopAwayFirstCharacterAscii();

  const char* szDot;

  if (bFullExtension)
  {
    szDot = sPath.FindSubString(".");
  }
  else
  {
    szDot = sPath.FindLastSubString(".");
  }

  // no dot at all -> no extension
  if (szDot == nullptr)
    return nsStringView();

  // dot at the very end of the string -> not an extension
  if (szDot + 1 == sPath.GetEndPointer())
    return nsStringView();

  return nsStringView(szDot + 1, sPath.GetEndPointer());
}

nsStringView nsPathUtils::GetFileNameAndExtension(nsStringView sPath)
{
  const char* szSeparator = FindPreviousSeparator(sPath.GetStartPointer(), sPath.GetEndPointer());

  if (szSeparator == nullptr)
    return sPath;

  return nsStringView(szSeparator + 1, sPath.GetEndPointer());
}

nsStringView nsPathUtils::GetFileName(nsStringView sPath, bool bRemoveFullExtension)
{
  // reduce the problem to just the filename + extension
  sPath = GetFileNameAndExtension(sPath);

  nsStringView ext = GetFileExtension(sPath, bRemoveFullExtension);

  if (ext.IsEmpty())
    return sPath;

  return nsStringView(sPath.GetStartPointer(), sPath.GetEndPointer() - ext.GetElementCount() - 1);
}

nsStringView nsPathUtils::GetFileDirectory(nsStringView sPath)
{
  auto it = rbegin(sPath);

  // if it already ends in a path separator, do not return a different directory
  if (IsPathSeparator(it.GetCharacter()))
    return sPath;

  // find the last separator in the string
  const char* szSeparator = FindPreviousSeparator(sPath.GetStartPointer(), sPath.GetEndPointer());

  // no path separator -> root dir -> return the empty path
  if (szSeparator == nullptr)
    return nsStringView(nullptr);

  return nsStringView(sPath.GetStartPointer(), szSeparator + 1);
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
const char nsPathUtils::OsSpecificPathSeparator = '\\';
#elif NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID) || NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)
const char nsPathUtils::OsSpecificPathSeparator = '/';
#elif NS_ENABLED(NS_PLATFORM_OSX)
const char nsPathUtils::OsSpecificPathSeparator = '/';
#else
#  error "Unknown platform."
#endif

bool nsPathUtils::IsAbsolutePath(nsStringView sPath)
{
  if (sPath.GetElementCount() < 2)
    return false;

  const char* szPath = sPath.GetStartPointer();

  // szPath[0] will not be \0 -> so we can access szPath[1] without problems

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  /// if it is an absolute path, character 0 must be ASCII (A - Z)
  /// checks for local paths, i.e. 'C:\stuff' and UNC paths, i.e. '\\server\stuff'
  /// not sure if we should handle '//' identical to '\\' (currently we do)
  return ((szPath[1] == ':') || (IsPathSeparator(szPath[0]) && IsPathSeparator(szPath[1])));
#elif NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID) || NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)
  return (szPath[0] == '/');
#elif NS_ENABLED(NS_PLATFORM_OSX)
  return (szPath[0] == '/');
#else
#  error "Unknown platform."
#endif
}

bool nsPathUtils::IsRelativePath(nsStringView sPath)
{
  if (sPath.IsEmpty())
    return true;

  // if it starts with a separator, it is not a relative path, ever
  if (nsPathUtils::IsPathSeparator(*sPath.GetStartPointer()))
    return false;

  return !IsAbsolutePath(sPath) && !IsRootedPath(sPath);
}

bool nsPathUtils::IsRootedPath(nsStringView sPath)
{
  return !sPath.IsEmpty() && *sPath.GetStartPointer() == ':';
}

void nsPathUtils::GetRootedPathParts(nsStringView sPath, nsStringView& ref_sRoot, nsStringView& ref_sRelPath)
{
  ref_sRoot = nsStringView();
  ref_sRelPath = sPath;

  if (!IsRootedPath(sPath))
    return;

  const char* szStart = sPath.GetStartPointer();
  const char* szPathEnd = sPath.GetEndPointer();

  do
  {
    nsUnicodeUtils::MoveToNextUtf8(szStart, szPathEnd).AssertSuccess();

    if (*szStart == '\0')
      return;

  } while (IsPathSeparator(*szStart));

  const char* szEnd = szStart;
  nsUnicodeUtils::MoveToNextUtf8(szEnd, szPathEnd).AssertSuccess();

  while (*szEnd != '\0' && !IsPathSeparator(*szEnd))
    nsUnicodeUtils::MoveToNextUtf8(szEnd, szPathEnd).AssertSuccess();

  ref_sRoot = nsStringView(szStart, szEnd);
  if (*szEnd == '\0')
  {
    ref_sRelPath = nsStringView();
  }
  else
  {
    // skip path separator for the relative path
    nsUnicodeUtils::MoveToNextUtf8(szEnd, szPathEnd).AssertSuccess();
    ref_sRelPath = nsStringView(szEnd, szPathEnd);
  }
}

nsStringView nsPathUtils::GetRootedPathRootName(nsStringView sPath)
{
  nsStringView root, relPath;
  GetRootedPathParts(sPath, root, relPath);
  return root;
}

bool nsPathUtils::IsValidFilenameChar(nsUInt32 uiCharacter)
{
  /// \test Not tested yet

  // Windows: https://msdn.microsoft.com/library/windows/desktop/aa365247(v=vs.85).aspx
  // Unix: https://en.wikipedia.org/wiki/Filename#Reserved_characters_and_words
  // Details can be more complicated (there might be reserved names depending on the filesystem), but in general all platforms behave like
  // this:
  static const nsUInt32 forbiddenFilenameChars[] = {'<', '>', ':', '"', '|', '?', '*', '\\', '/', '\t', '\b', '\n', '\r', '\0'};

  for (int i = 0; i < NS_ARRAY_SIZE(forbiddenFilenameChars); ++i)
  {
    if (forbiddenFilenameChars[i] == uiCharacter)
      return false;
  }

  return true;
}

bool nsPathUtils::ContainsInvalidFilenameChars(nsStringView sPath)
{
  /// \test Not tested yet

  nsStringIterator it = sPath.GetIteratorFront();

  for (; it.IsValid(); ++it)
  {
    if (!IsValidFilenameChar(it.GetCharacter()))
      return true;
  }

  return false;
}

void nsPathUtils::MakeValidFilename(nsStringView sFilename, nsUInt32 uiReplacementCharacter, nsStringBuilder& out_sFilename)
{
  NS_ASSERT_DEBUG(IsValidFilenameChar(uiReplacementCharacter), "Given replacement character is not allowed for filenames.");

  out_sFilename.Clear();

  for (auto it = sFilename.GetIteratorFront(); it.IsValid(); ++it)
  {
    nsUInt32 currentChar = it.GetCharacter();

    if (IsValidFilenameChar(currentChar) == false)
      out_sFilename.Append(uiReplacementCharacter);
    else
      out_sFilename.Append(currentChar);
  }
}

bool nsPathUtils::IsSubPath(nsStringView sPrefixPath, nsStringView sFullPath0)
{
  if (sPrefixPath.IsEmpty())
  {
    if (sFullPath0.IsAbsolutePath())
      return true;

    NS_REPORT_FAILURE("Prefixpath is empty and checked path is not absolute.");
    return false;
  }

  nsStringBuilder tmp = sPrefixPath;
  tmp.MakeCleanPath();
  tmp.Trim("", "/");

  nsStringBuilder sFullPath = sFullPath0;
  sFullPath.MakeCleanPath();

  if (sFullPath.StartsWith(tmp))
  {
    if (tmp.GetElementCount() == sFullPath.GetElementCount())
      return true;

    return sFullPath.GetData()[tmp.GetElementCount()] == '/';
  }

  return false;
}

bool nsPathUtils::IsSubPath_NoCase(nsStringView sPrefixPath, nsStringView sFullPath)
{
  nsStringBuilder tmp = sPrefixPath;
  tmp.MakeCleanPath();
  tmp.Trim("", "/");

  if (sFullPath.StartsWith_NoCase(tmp))
  {
    if (tmp.GetElementCount() == sFullPath.GetElementCount())
      return true;

    return sFullPath.GetStartPointer()[tmp.GetElementCount()] == '/';
  }

  return false;
}
