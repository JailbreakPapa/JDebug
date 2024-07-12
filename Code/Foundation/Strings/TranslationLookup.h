#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/UniquePtr.h>

/// \brief What a translated string is used for.
enum class nsTranslationUsage
{
  Default,
  Tooltip,
  HelpURL,

  ENUM_COUNT
};

/// \brief Base class to translate one string into another
class NS_FOUNDATION_DLL nsTranslator
{
public:
  nsTranslator();
  virtual ~nsTranslator();

  /// \brief The given string (with the given hash) shall be translated
  virtual nsStringView Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage) = 0;

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
  static nsHybridArray<nsTranslator*, 4> s_AllTranslators;
};

/// \brief Just returns the same string that is passed into it. Can be used to display the actually untranslated strings
class NS_FOUNDATION_DLL nsTranslatorPassThrough : public nsTranslator
{
public:
  virtual nsStringView Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage) override { return sString; }
};

/// \brief Can store translated strings and all translation requests will come from that storage. Returns nullptr if the requested string is
/// not known
class NS_FOUNDATION_DLL nsTranslatorStorage : public nsTranslator
{
public:
  /// \brief Stores szString as the translation for the string with the given hash
  virtual void StoreTranslation(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage);

  /// \brief Returns the translated string for uiStringHash, or nullptr, if not available
  virtual nsStringView Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage) override;

  /// \brief Clears all stored translation strings
  virtual void Reset() override;

  /// \brief Simply executes Reset() on this translator
  virtual void Reload() override;

protected:
  nsMap<nsUInt64, nsString> m_Translations[(int)nsTranslationUsage::ENUM_COUNT];
};

/// \brief Outputs a 'Missing Translation' warning the first time a string translation is requested.
/// Otherwise always returns nullptr, allowing the next translator to take over.
class NS_FOUNDATION_DLL nsTranslatorLogMissing : public nsTranslatorStorage
{
public:
  /// Can be used from external code to (temporarily) deactivate error logging (a bit hacky)
  static bool s_bActive;

  virtual nsStringView Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage) override;
};

/// \brief Loads translations from files. Each translator can have different search paths, but the files to be loaded are the same for all of them.
class NS_FOUNDATION_DLL nsTranslatorFromFiles : public nsTranslatorStorage
{
public:
  /// \brief Loads all files recursively from the specified folder as translation files.
  ///
  /// The given path must be absolute or resolvable to an absolute path.
  /// On failure, the function does nothing.
  /// This function depends on nsFileSystemIterator to be available.
  void AddTranslationFilesFromFolder(const char* szFolder);

  virtual nsStringView Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage) override;

  virtual void Reload() override;

private:
  void LoadTranslationFile(const char* szFullPath);

  nsHybridArray<nsString, 4> m_Folders;
};

/// \brief Returns the same string that is passed into it, but strips off class names and separates the text at CamelCase boundaries.
class NS_FOUNDATION_DLL nsTranslatorMakeMoreReadable : public nsTranslatorStorage
{
public:
  virtual nsStringView Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage) override;
};

/// \brief Handles looking up translations for strings.
///
/// Multiple translators can be registered to get translations from different sources.
class NS_FOUNDATION_DLL nsTranslationLookup
{
public:
  /// \brief Translators will be queried in the reverse order that they were added.
  static void AddTranslator(nsUniquePtr<nsTranslator> pTranslator);

  /// \brief Prefer to use the nsTranslate macro instead of calling this function directly. Will query all translators for a translation,
  /// until one is found.
  static nsStringView Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage);

  /// \brief Deletes all translators.
  static void Clear();

private:
  static nsHybridArray<nsUniquePtr<nsTranslator>, 16> s_Translators;
};

/// \brief Use this macro to query a translation for a string from the nsTranslationLookup system
#define nsTranslate(string) nsTranslationLookup::Translate(string, nsHashingUtils::StringHash(string), nsTranslationUsage::Default)

/// \brief Use this macro to query a translation for a tooltip string from the nsTranslationLookup system
#define nsTranslateTooltip(string) nsTranslationLookup::Translate(string, nsHashingUtils::StringHash(string), nsTranslationUsage::Tooltip)

/// \brief Use this macro to query a translation for a help URL from the nsTranslationLookup system
#define nsTranslateHelpURL(string) nsTranslationLookup::Translate(string, nsHashingUtils::StringHash(string), nsTranslationUsage::HelpURL)
