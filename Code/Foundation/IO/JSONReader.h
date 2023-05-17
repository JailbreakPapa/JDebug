#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/JSONParser.h>
#include <Foundation/Types/Variant.h>

/// \brief This JSON reader will read an entire JSON document into a hierarchical structure of wdVariants.
///
/// The reader will parse the entire document and create a data structure of wdVariants, which can then be traversed easily.
/// Note that this class is much less efficient at reading large JSON documents, as it will dynamically allocate and copy objects around
/// quite a bit. For small to medium sized documents that might be good enough, for large files one should prefer to write a dedicated
/// class derived from wdJSONParser.
class WD_FOUNDATION_DLL wdJSONReader : public wdJSONParser
{
public:
  wdJSONReader();

  /// \brief Reads the entire stream and creates the internal data structure that represents the JSON document. Returns WD_FAILURE if any parsing
  /// error occurred.
  wdResult Parse(wdStreamReader& ref_input, wdUInt32 uiFirstLineOffset = 0);

  /// \brief Returns the top-level object of the JSON document.
  const wdVariantDictionary& GetTopLevelObject() const { return m_Stack.PeekBack().m_Dictionary; }

private:
  /// \brief This function can be overridden to skip certain variables, however the overriding function must still call this.
  virtual bool OnVariable(const char* szVarName) override;

  /// \brief [internal] Do not override further.
  virtual void OnReadValue(const char* szValue) override;

  /// \brief [internal] Do not override further.
  virtual void OnReadValue(double fValue) override;

  /// \brief [internal] Do not override further.
  virtual void OnReadValue(bool bValue) override;

  /// \brief [internal] Do not override further.
  virtual void OnReadValueNULL() override;

  /// \brief [internal] Do not override further.
  virtual void OnBeginObject() override;

  /// \brief [internal] Do not override further.
  virtual void OnEndObject() override;

  /// \brief [internal] Do not override further.
  virtual void OnBeginArray() override;

  /// \brief [internal] Do not override further.
  virtual void OnEndArray() override;

  virtual void OnParsingError(const char* szMessage, bool bFatal, wdUInt32 uiLine, wdUInt32 uiColumn) override;

protected:
  enum class ElementMode : wdInt8
  {
    Array,
    Dictionary
  };

  struct Element
  {
    wdString m_sName;
    ElementMode m_Mode;
    wdVariantArray m_Array;
    wdVariantDictionary m_Dictionary;
  };

  wdHybridArray<Element, 32> m_Stack;

  bool m_bParsingError;
  wdString m_sLastName;
};
