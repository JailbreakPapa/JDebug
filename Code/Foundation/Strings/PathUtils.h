#pragma once

#include <Foundation/Strings/StringView.h>
#include <Foundation/Strings/UnicodeUtils.h>

class nsStringBuilder;

/// \brief Contains Helper functions to work with paths.
///
/// Only functions that require read-only access to a string are provided here
/// All functions that require to modify the path are provided by nsStringBuilder.
/// Many functions return nsStringView's, which will always be strict sub-strings of their input data.
/// That allows that these functions can work without any additional memory allocations.
class NS_FOUNDATION_DLL nsPathUtils
{
public:
  /// \brief The path separator used by this operating system.
  static const char OsSpecificPathSeparator;

  /// \brief Returns whether c is any known path separator.
  static bool IsPathSeparator(nsUInt32 c); // [tested]

  /// \brief Checks if a given character is allowed in a filename (not path!)
  static bool IsValidFilenameChar(nsUInt32 uiCharacter);

  /// \brief Checks every character in the string with IsValidFilenameChar()
  ///
  /// This is a basic check, only because each character passes the test, it does not guarantee that the full string is a valid path.
  static bool ContainsInvalidFilenameChars(nsStringView sPath);

  /// \brief Searches for the previous path separator before szStartSearchAt. Will return nullptr if it reaches szPathStart before finding
  /// any separator.
  static const char* FindPreviousSeparator(const char* szPathStart, const char* szStartSearchAt); // [tested]

  /// \brief Checks whether the given path has any file extension
  static bool HasAnyExtension(nsStringView sPath); // [tested]

  /// \brief Checks whether the path ends with the given file extension.
  /// szExtension may or may not start with a dot.
  /// The check is case insensitive.
  ///
  ///   HasExtension("file.txt", "txt") -> true
  ///   HasExtension("file.txt", ".txt") -> true
  ///   HasExtension("file.a.b", ".b") -> true
  ///   HasExtension("file.a.b", "a.b") -> true
  ///   HasExtension("file.a.b", ".a.b") -> true
  ///   HasExtension("file.a.b", "file.a.b") -> false
  static bool HasExtension(nsStringView sPath, nsStringView sExtension); // [tested]

  /// \brief Returns the file extension of the given path. Will be empty, if the path does not end with a proper extension. The dot (.) is not included.
  ///
  /// If bFullExtension is false, a file named "file.a.b.c" will return "c".
  /// If bFullExtension is true, a file named "file.a.b.c" will return "a.b.c".
  static nsStringView GetFileExtension(nsStringView sPath, bool bFullExtension = false); // [tested]

  /// \brief Returns the file name of a path, excluding the path and extension.
  ///
  /// If the path already ends with a path separator, the result will be empty.
  static nsStringView GetFileName(nsStringView sPath, bool bRemoveFullExtension = false); // [tested]

  /// \brief Returns the substring that represents the file name including the file extension.
  ///
  /// Returns an empty string, if sPath already ends in a path separator, or is empty itself.
  static nsStringView GetFileNameAndExtension(nsStringView sPath); // [tested]

  /// \brief Returns the directory of the given file, which is the substring up to the last path separator.
  ///
  /// If the path already ends in a path separator, and thus points to a folder, instead of a file, the unchanged path is returned.
  /// "path/to/file" -> "path/to/"
  /// "path/to/folder/" -> "path/to/folder/"
  /// "filename" -> ""
  /// "/file_at_root_level" -> "/"
  static nsStringView GetFileDirectory(nsStringView sPath); // [tested]

  /// \brief Returns true, if the given path represents an absolute path on the current OS.
  static bool IsAbsolutePath(nsStringView sPath); // [tested]

  /// \brief Returns true, if the given path represents a relative path on the current OS.
  static bool IsRelativePath(nsStringView sPath); // [tested]

  /// \brief A rooted path starts with a colon and then names a file-system data directory. Rooted paths are used as 'absolute' paths within
  /// the nsFileSystem.
  static bool IsRootedPath(nsStringView sPath); // [tested]

  /// \brief Splits the passed path into its root portion and the relative path
  ///
  /// ":MyRoot\file.txt" -> root = "MyRoot", relPath="file.txt"
  /// ":MyRoot\folder\file.txt" -> root = "MyRoot", relPath = "folder\file.txt"
  /// ":\MyRoot\folder\file.txt" -> root = "MyRoot", relPath = "folder\file.txt"
  /// ":/MyRoot\folder\file.txt" -> root = "MyRoot", relPath = "folder\file.txt"
  /// If the path is not rooted, then root will be an empty string and relPath is set to the full input path.
  static void GetRootedPathParts(nsStringView sPath, nsStringView& ref_sRoot, nsStringView& ref_sRelPath); // [tested]

  /// \brief Special case of GetRootedPathParts that returns the root of the input path and discards the relative path
  static nsStringView GetRootedPathRootName(nsStringView sPath); // [tested]

  /// \brief Creates a valid filename (not path!) using the given string by replacing all disallowed characters.
  ///
  /// Note that path separators in the given string will be replaced as well!
  /// Asserts that replacementCharacter is an allowed character.
  /// \see IsValidFilenameChar()
  static void MakeValidFilename(nsStringView sFilename, nsUInt32 uiReplacementCharacter, nsStringBuilder& out_sFilename);

  /// \brief Checks whether \a sFullPath starts with \a sPrefixPath.
  static bool IsSubPath(nsStringView sPrefixPath, nsStringView sFullPath); // [tested]
  /// \brief Checks whether \a sFullPath starts with \a sPrefixPath. Case insensitive.
  static bool IsSubPath_NoCase(nsStringView sPrefixPath, nsStringView sFullPath); // [tested]
};

#include <Foundation/Strings/Implementation/PathUtils_inl.h>
