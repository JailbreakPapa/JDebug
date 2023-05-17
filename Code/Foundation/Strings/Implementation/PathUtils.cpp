#include <Foundation/FoundationPCH.h>

#include <Foundation/Strings/Implementation/StringIterator.h>
#include <Foundation/Strings/StringBuilder.h>

const char* wdPathUtils::FindPreviousSeparator(const char* szPathStart, const char* szStartSearchAt)
{
  if (wdStringUtils::IsNullOrEmpty(szPathStart))
    return nullptr;

  while (szStartSearchAt > szPathStart)
  {
    wdUnicodeUtils::MoveToPriorUtf8(szStartSearchAt);

    if (IsPathSeparator(*szStartSearchAt))
      return szStartSearchAt;
  }

  return nullptr;
}

bool wdPathUtils::HasAnyExtension(wdStringView sPath)
{
  const char* szDot = wdStringUtils::FindLastSubString(sPath.GetStartPointer(), ".", nullptr, sPath.GetEndPointer());

  if (szDot == nullptr)
    return false;

  // find the last separator in the string
  const char* szSeparator = FindPreviousSeparator(sPath.GetStartPointer(), sPath.GetEndPointer());

  return (szSeparator < szDot);
}

bool wdPathUtils::HasExtension(wdStringView sPath, wdStringView sExtension)
{
  if (wdStringUtils::StartsWith(sExtension.GetStartPointer(), ".", sExtension.GetEndPointer()))
    return wdStringUtils::EndsWith_NoCase(sPath.GetStartPointer(), sExtension.GetStartPointer(), sPath.GetEndPointer(), sExtension.GetEndPointer());

  wdStringBuilder sExt;
  sExt.Append(".", sExtension);

  return wdStringUtils::EndsWith_NoCase(sPath.GetStartPointer(), sExt.GetData(), sPath.GetEndPointer());
}

wdStringView wdPathUtils::GetFileExtension(wdStringView sPath)
{
  const char* szDot = wdStringUtils::FindLastSubString(sPath.GetStartPointer(), ".", nullptr, sPath.GetEndPointer());

  if (szDot == nullptr)
    return wdStringView(nullptr);

  // find the last separator in the string
  const char* szSeparator = FindPreviousSeparator(sPath.GetStartPointer(), sPath.GetEndPointer());

  if (szSeparator > szDot)
    return wdStringView(nullptr);

  return wdStringView(szDot + 1, sPath.GetEndPointer());
}

wdStringView wdPathUtils::GetFileNameAndExtension(wdStringView sPath)
{
  const char* szSeparator = FindPreviousSeparator(sPath.GetStartPointer(), sPath.GetEndPointer());

  if (szSeparator == nullptr)
    return sPath;

  return wdStringView(szSeparator + 1, sPath.GetEndPointer());
}

wdStringView wdPathUtils::GetFileName(wdStringView sPath)
{
  const char* szSeparator = FindPreviousSeparator(sPath.GetStartPointer(), sPath.GetEndPointer());

  const char* szDot = wdStringUtils::FindLastSubString(sPath.GetStartPointer(), ".", sPath.GetEndPointer());

  if (szDot < szSeparator) // includes (szDot == nullptr), szSeparator will never be nullptr here -> no extension
  {
    return wdStringView(szSeparator + 1, sPath.GetEndPointer());
  }

  if (szSeparator == nullptr)
  {
    if (szDot == nullptr) // no folder, no extension -> the entire thing is just a name
      return sPath;

    return wdStringView(sPath.GetStartPointer(), szDot); // no folder, but an extension -> remove the extension
  }

  // now: there is a separator AND an extension

  return wdStringView(szSeparator + 1, szDot);
}

wdStringView wdPathUtils::GetFileDirectory(wdStringView sPath)
{
  auto it = rbegin(sPath);

  // if it already ends in a path separator, do not return a different directory
  if (IsPathSeparator(it.GetCharacter()))
    return sPath;

  // find the last separator in the string
  const char* szSeparator = FindPreviousSeparator(sPath.GetStartPointer(), sPath.GetEndPointer());

  // no path separator -> root dir -> return the empty path
  if (szSeparator == nullptr)
    return wdStringView(nullptr);

  return wdStringView(sPath.GetStartPointer(), szSeparator + 1);
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
const char wdPathUtils::OsSpecificPathSeparator = '\\';
#elif WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
const char wdPathUtils::OsSpecificPathSeparator = '/';
#elif WD_ENABLED(WD_PLATFORM_OSX)
const char wdPathUtils::OsSpecificPathSeparator = '/';
#else
#  error "Unknown platform."
#endif

bool wdPathUtils::IsAbsolutePath(wdStringView sPath)
{
  if (sPath.GetElementCount() < 2)
    return false;

  const char* szPath = sPath.GetStartPointer();

  // szPath[0] will not be \0 -> so we can access szPath[1] without problems

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
  /// if it is an absolute path, character 0 must be ASCII (A - Z)
  /// checks for local paths, i.e. 'C:\stuff' and UNC paths, i.e. '\\server\stuff'
  /// not sure if we should handle '//' identical to '\\' (currently we do)
  return ((szPath[1] == ':') || (IsPathSeparator(szPath[0]) && IsPathSeparator(szPath[1])));
#elif WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
  return (szPath[0] == '/');
#elif WD_ENABLED(WD_PLATFORM_OSX)
  return (szPath[0] == '/');
#else
#  error "Unknown platform."
#endif
}

bool wdPathUtils::IsRelativePath(wdStringView sPath)
{
  if (sPath.IsEmpty())
    return true;

  // if it starts with a separator, it is not a relative path, ever
  if (wdPathUtils::IsPathSeparator(*sPath.GetStartPointer()))
    return false;

  return !IsAbsolutePath(sPath) && !IsRootedPath(sPath);
}

bool wdPathUtils::IsRootedPath(wdStringView sPath)
{
  return !sPath.IsEmpty() && *sPath.GetStartPointer() == ':';
}

void wdPathUtils::GetRootedPathParts(wdStringView sPath, wdStringView& ref_sRoot, wdStringView& ref_sRelPath)
{
  ref_sRoot = wdStringView();
  ref_sRelPath = sPath;

  if (!IsRootedPath(sPath))
    return;

  const char* szStart = sPath.GetStartPointer();
  const char* szPathEnd = sPath.GetEndPointer();

  do
  {
    wdUnicodeUtils::MoveToNextUtf8(szStart, szPathEnd);

    if (*szStart == '\0')
      return;

  } while (IsPathSeparator(*szStart));

  const char* szEnd = szStart;
  wdUnicodeUtils::MoveToNextUtf8(szEnd, szPathEnd);

  while (*szEnd != '\0' && !IsPathSeparator(*szEnd))
    wdUnicodeUtils::MoveToNextUtf8(szEnd, szPathEnd);

  ref_sRoot = wdStringView(szStart, szEnd);
  if (*szEnd == '\0')
  {
    ref_sRelPath = wdStringView();
  }
  else
  {
    // skip path separator for the relative path
    wdUnicodeUtils::MoveToNextUtf8(szEnd, szPathEnd);
    ref_sRelPath = wdStringView(szEnd, szPathEnd);
  }
}

wdStringView wdPathUtils::GetRootedPathRootName(wdStringView sPath)
{
  wdStringView root, relPath;
  GetRootedPathParts(sPath, root, relPath);
  return root;
}

bool wdPathUtils::IsValidFilenameChar(wdUInt32 uiCharacter)
{
  /// \test Not tested yet

  // Windows: https://msdn.microsoft.com/library/windows/desktop/aa365247(v=vs.85).aspx
  // Unix: https://en.wikipedia.org/wiki/Filename#Reserved_characters_and_words
  // Details can be more complicated (there might be reserved names depending on the filesystem), but in general all platforms behave like
  // this:
  static const wdUInt32 forbiddenFilenameChars[] = {'<', '>', ':', '"', '|', '?', '*', '\\', '/', '\t', '\b', '\n', '\r', '\0'};

  for (int i = 0; i < WD_ARRAY_SIZE(forbiddenFilenameChars); ++i)
  {
    if (forbiddenFilenameChars[i] == uiCharacter)
      return false;
  }

  return true;
}

bool wdPathUtils::ContainsInvalidFilenameChars(wdStringView sPath)
{
  /// \test Not tested yet

  wdStringIterator it = sPath.GetIteratorFront();

  for (; it.IsValid(); ++it)
  {
    if (!IsValidFilenameChar(it.GetCharacter()))
      return true;
  }

  return false;
}

void wdPathUtils::MakeValidFilename(wdStringView sFilename, wdUInt32 uiReplacementCharacter, wdStringBuilder& out_sFilename)
{
  WD_ASSERT_DEBUG(IsValidFilenameChar(uiReplacementCharacter), "Given replacement character is not allowed for filenames.");

  out_sFilename.Clear();

  for (auto it = sFilename.GetIteratorFront(); it.IsValid(); ++it)
  {
    wdUInt32 currentChar = it.GetCharacter();

    if (IsValidFilenameChar(currentChar) == false)
      out_sFilename.Append(uiReplacementCharacter);
    else
      out_sFilename.Append(currentChar);
  }
}

bool wdPathUtils::IsSubPath(wdStringView sPrefixPath, wdStringView sFullPath)
{
  /// \test this is new

  wdStringBuilder tmp = sPrefixPath;
  tmp.MakeCleanPath();
  tmp.AppendPath("");

  return sFullPath.StartsWith_NoCase(tmp);
}

WD_STATICLINK_FILE(Foundation, Foundation_Strings_Implementation_PathUtils);
