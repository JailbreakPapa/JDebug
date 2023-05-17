#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/UniquePtr.h>

/// \brief What a translated string is used for.
enum class wdTranslationUsage
{
  Default,
  Tooltip,
  HelpURL,

  ENUM_COUNT
};

/// \brief Base class to translate one string into another
class WD_FOUNDATION_DLL wdTranslator
{
public:
  wdTranslator();
  virtual ~wdTranslator();

  /// \brief The given string (with the given hash) shall be translated
  virtual const char* Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage) = 0;

  /// \brief Called to reset internal state
  virtual void Reset();

  /// \brief May reload the known translations
  virtual void Reload();

  /// \brief Will call Reload() on all currently active translators
  static void ReloadAllTranslators();

  static void HighlightUntranslated(bool bHighlight);

  static bool GetHighlightUntranslated() { return s_bHighlightUntranslated; }

private:
  static bool s_bHighlightUntranslated;
  static wdHybridArray<wdTranslator*, 4> s_AllTranslators;
};

/// \brief Just returns the same string that is passed into it. Can be used to display the actually untranslated strings
class WD_FOUNDATION_DLL wdTranslatorPassThrough : public wdTranslator
{
public:
  virtual const char* Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage) override { return szString; }
};

/// \brief Can store translated strings and all translation requests will come from that storage. Returns nullptr if the requested string is
/// not known
class WD_FOUNDATION_DLL wdTranslatorStorage : public wdTranslator
{
public:
  /// \brief Stores szString as the translation for the string with the given hash
  virtual void StoreTranslation(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage);

  /// \brief Returns the translated string for uiStringHash, or nullptr, if not available
  virtual const char* Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage) override;

  /// \brief Clears all stored translation strings
  virtual void Reset() override;

  /// \brief Simply executes Reset() on this translator
  virtual void Reload() override;

protected:
  wdMap<wdUInt64, wdString> m_Translations[(int)wdTranslationUsage::ENUM_COUNT];
};

/// \brief Outputs a 'Missing Translation' warning the first time a string translation is requested.
/// Otherwise always returns nullptr, allowing the next translator to take over.
class WD_FOUNDATION_DLL wdTranslatorLogMissing : public wdTranslatorStorage
{
public:
  /// Can be used from external code to (temporarily) deactivate error logging (a bit hacky)
  static bool s_bActive;

  virtual const char* Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage) override;
};

/// \brief Loads translations from files. Each translator can have different search paths, but the files to be loaded are the same for all of them.
class WD_FOUNDATION_DLL wdTranslatorFromFiles : public wdTranslatorStorage
{
public:
  /// \brief Loads all files recursively from the specified folder as translation files.
  ///
  /// The given path must be absolute or resolvable to an absolute path.
  /// On failure, the function does nothing.
  /// This function depends on wdFileSystemIterator to be available.
  void AddTranslationFilesFromFolder(const char* szFolder);

  virtual const char* Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage) override;

  virtual void Reload() override;

private:
  void LoadTranslationFile(const char* szFullPath);

  wdHybridArray<wdString, 4> m_Folders;
};

/// \brief Returns the same string that is passed into it, but strips off class names and separates the text at CamelCase boundaries.
class WD_FOUNDATION_DLL wdTranslatorMakeMoreReadable : public wdTranslatorStorage
{
public:
  virtual const char* Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage) override;
};

/// \brief Handles looking up translations for strings.
///
/// Multiple translators can be registered to get translations from different sources.
class WD_FOUNDATION_DLL wdTranslationLookup
{
public:
  /// \brief Translators will be queried in the reverse order that they were added.
  static void AddTranslator(wdUniquePtr<wdTranslator> pTranslator);

  /// \brief Prefer to use the wdTranslate macro instead of calling this function directly. Will query all translators for a translation,
  /// until one is found.
  static const char* Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage);

  /// \brief Deletes all translators.
  static void Clear();

private:
  static wdHybridArray<wdUniquePtr<wdTranslator>, 16> s_Translators;
};

/// \brief Use this macro to query a translation for a string from the wdTranslationLookup system
#define wdTranslate(string) wdTranslationLookup::Translate(string, wdHashingUtils::StringHash(string), wdTranslationUsage::Default)

/// \brief Use this macro to query a translation for a tooltip string from the wdTranslationLookup system
#define wdTranslateTooltip(string) wdTranslationLookup::Translate(string, wdHashingUtils::StringHash(string), wdTranslationUsage::Tooltip)

/// \brief Use this macro to query a translation for a help URL from the wdTranslationLookup system
#define wdTranslateHelpURL(string) wdTranslationLookup::Translate(string, wdHashingUtils::StringHash(string), wdTranslationUsage::HelpURL)
