#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/JSONParser.h>
#include <Foundation/Types/Variant.h>

/// \brief This JSON reader will read an entire JSON document into a hierarchical structure of nsVariants.
///
/// The reader will parse the entire document and create a data structure of nsVariants, which can then be traversed easily.
/// Note that this class is much less efficient at reading large JSON documents, as it will dynamically allocate and copy objects around
/// quite a bit. For small to medium sized documents that might be good enough, for large files one should prefer to write a dedicated
/// class derived from nsJSONParser.
class NS_FOUNDATION_DLL nsJSONReader : public nsJSONParser
{
public:
  enum class ElementType : nsInt8
  {
    None,       ///< The JSON document is entirely empty (not even containing an empty object or array)
    Dictionary, ///< The top level element in the JSON document is an object
    Array,      ///< The top level element in the JSON document is an array
  };

  nsJSONReader();

  /// \brief Reads the entire stream and creates the internal data structure that represents the JSON document. Returns NS_FAILURE if any parsing
  /// error occurred.
  nsResult Parse(nsStreamReader& ref_input, nsUInt32 uiFirstLineOffset = 0);

  /// \brief Returns the top-level object of the JSON document.
  const nsVariantDictionary& GetTopLevelObject() const { return m_Stack.PeekBack().m_Dictionary; }

  /// \brief Returns the top-level array of the JSON document.
  const nsVariantArray& GetTopLevelArray() const { return m_Stack.PeekBack().m_Array; }

  /// \brief Returns whether the top level element is an array or an object.
  ElementType GetTopLevelElementType() const { return m_Stack.PeekBack().m_Mode; }

private:
  /// \brief This function can be overridden to skip certain variables, however the overriding function must still call this.
  virtual bool OnVariable(nsStringView sVarName) override;

  /// \brief [internal] Do not override further.
  virtual void OnReadValue(nsStringView sValue) override;

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

  virtual void OnParsingError(nsStringView sMessage, bool bFatal, nsUInt32 uiLine, nsUInt32 uiColumn) override;

protected:
  struct Element
  {
    nsString m_sName;
    ElementType m_Mode = ElementType::None;
    nsVariantArray m_Array;
    nsVariantDictionary m_Dictionary;
  };

  nsHybridArray<Element, 32> m_Stack;

  bool m_bParsingError = false;
  nsString m_sLastName;
};
