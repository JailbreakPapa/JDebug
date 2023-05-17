#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/OpenDdlParser.h>
#include <Foundation/Logging/Log.h>

/// \brief Represents a single 'object' in a DDL document, e.g. either a custom type or a primitives list.
class WD_FOUNDATION_DLL wdOpenDdlReaderElement
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Whether this is a custom object type that typically contains sub-elements.
  WD_ALWAYS_INLINE bool IsCustomType() const { return m_PrimitiveType == wdOpenDdlPrimitiveType::Custom; } // [tested]

  /// \brief Whether this is a custom object type of the requested type.
  WD_ALWAYS_INLINE bool IsCustomType(const char* szTypeName) const
  {
    return m_PrimitiveType == wdOpenDdlPrimitiveType::Custom && wdStringUtils::IsEqual(m_szCustomType, szTypeName);
  }

  /// \brief Returns the string for the custom type name.
  WD_ALWAYS_INLINE const char* GetCustomType() const { return m_szCustomType; } // [tested]

  /// \brief Whether the name of the object is non-empty.
  WD_ALWAYS_INLINE bool HasName() const { return !wdStringUtils::IsNullOrEmpty(m_szName); } // [tested]

  /// \brief Returns the name of the object.
  WD_ALWAYS_INLINE const char* GetName() const { return m_szName; } // [tested]

  /// \brief Returns whether the element name is a global or a local name.
  WD_ALWAYS_INLINE bool IsNameGlobal() const { return (m_uiNumChildElements & WD_BIT(31)) != 0; } // [tested]

  /// \brief How many sub-elements the object has.
  wdUInt32 GetNumChildObjects() const; // [tested]

  /// \brief If this is a custom type element, the returned pointer is to the first child element.
  WD_ALWAYS_INLINE const wdOpenDdlReaderElement* GetFirstChild() const
  {
    return reinterpret_cast<const wdOpenDdlReaderElement*>(m_pFirstChild);
  } // [tested]

  /// \brief If the parent is a custom type element, the next child after this is returned.
  WD_ALWAYS_INLINE const wdOpenDdlReaderElement* GetSibling() const { return m_pSiblingElement; } // [tested]

  /// \brief For non-custom types this returns how many primitives are stored at this element.
  wdUInt32 GetNumPrimitives() const; // [tested]

  /// \brief For non-custom types this returns the type of primitive that is stored at this element.
  WD_ALWAYS_INLINE wdOpenDdlPrimitiveType GetPrimitivesType() const { return m_PrimitiveType; } // [tested]

  /// \brief Returns true if the element stores the requested type of primitives AND has at least the desired amount of them, so that accessing the
  /// data array at certain indices is safe.
  bool HasPrimitives(wdOpenDdlPrimitiveType type, wdUInt32 uiMinNumberOfPrimitives = 1) const;

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const bool* GetPrimitivesBool() const { return reinterpret_cast<const bool*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdInt8* GetPrimitivesInt8() const { return reinterpret_cast<const wdInt8*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdInt16* GetPrimitivesInt16() const { return reinterpret_cast<const wdInt16*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdInt32* GetPrimitivesInt32() const { return reinterpret_cast<const wdInt32*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdInt64* GetPrimitivesInt64() const { return reinterpret_cast<const wdInt64*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdUInt8* GetPrimitivesUInt8() const { return reinterpret_cast<const wdUInt8*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdUInt16* GetPrimitivesUInt16() const { return reinterpret_cast<const wdUInt16*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdUInt32* GetPrimitivesUInt32() const { return reinterpret_cast<const wdUInt32*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdUInt64* GetPrimitivesUInt64() const { return reinterpret_cast<const wdUInt64*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const float* GetPrimitivesFloat() const { return reinterpret_cast<const float*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const double* GetPrimitivesDouble() const { return reinterpret_cast<const double*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  WD_ALWAYS_INLINE const wdStringView* GetPrimitivesString() const { return reinterpret_cast<const wdStringView*>(m_pFirstChild); } // [tested]

  /// \brief Searches for a child with the given name. It does not matter whether the object's name is 'local' or 'global'.
  /// \a szName is case-sensitive.
  const wdOpenDdlReaderElement* FindChild(const char* szName) const; // [tested]

  /// \brief Searches for a child element that has the given type, name and if it is a primitives list, at least the desired number of primitives.
  const wdOpenDdlReaderElement* FindChildOfType(wdOpenDdlPrimitiveType type, const char* szName, wdUInt32 uiMinNumberOfPrimitives = 1) const;

  /// \brief Searches for a child element with the given type and optionally also a certain name.
  const wdOpenDdlReaderElement* FindChildOfType(const char* szType, const char* szName = nullptr) const;

private:
  friend class wdOpenDdlReader;

  wdOpenDdlPrimitiveType m_PrimitiveType;
  wdUInt32 m_uiNumChildElements;
  const void* m_pFirstChild;
  const wdOpenDdlReaderElement* m_pLastChild;
  const char* m_szCustomType;
  const char* m_szName;
  const wdOpenDdlReaderElement* m_pSiblingElement;
};

/// \brief An OpenDDL reader parses an entire DDL document and creates an in-memory representation of the document structure.
class WD_FOUNDATION_DLL wdOpenDdlReader : public wdOpenDdlParser
{
public:
  wdOpenDdlReader();
  ~wdOpenDdlReader();

  /// \brief Parses the given document, returns WD_FAILURE if an unrecoverable parsing error was encountered.
  ///
  /// \param stream is the input data.
  /// \param uiFirstLineOffset allows to adjust the reported line numbers in error messages, in case the given stream represents a sub-section of a
  /// larger file. \param pLog is used for outputting details about parsing errors. If nullptr is given, no details are logged. \param uiCacheSizeInKB
  /// is the internal cache size that the parser uses. If the parsed documents contain primitives lists with several thousand elements in a single
  /// list, increasing the cache size can improve performance, but typically this doesn't need to be adjusted.
  wdResult ParseDocument(wdStreamReader& inout_stream, wdUInt32 uiFirstLineOffset = 0, wdLogInterface* pLog = wdLog::GetThreadLocalLogSystem(),
    wdUInt32 uiCacheSizeInKB = 4); // [tested]

  /// \brief Every document has exactly one root element.
  const wdOpenDdlReaderElement* GetRootElement() const; // [tested]

  /// \brief Searches for an element with a global name. NULL if there is no such element.
  const wdOpenDdlReaderElement* FindElement(const char* szGlobalName) const; // [tested]

protected:
  virtual void OnBeginObject(const char* szType, const char* szName, bool bGlobalName) override;
  virtual void OnEndObject() override;

  virtual void OnBeginPrimitiveList(wdOpenDdlPrimitiveType type, const char* szName, bool bGlobalName) override;
  virtual void OnEndPrimitiveList() override;

  virtual void OnPrimitiveBool(wdUInt32 count, const bool* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveInt8(wdUInt32 count, const wdInt8* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveInt16(wdUInt32 count, const wdInt16* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveInt32(wdUInt32 count, const wdInt32* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveInt64(wdUInt32 count, const wdInt64* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveUInt8(wdUInt32 count, const wdUInt8* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveUInt16(wdUInt32 count, const wdUInt16* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveUInt32(wdUInt32 count, const wdUInt32* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveUInt64(wdUInt32 count, const wdUInt64* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveFloat(wdUInt32 count, const float* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveDouble(wdUInt32 count, const double* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveString(wdUInt32 count, const wdStringView* pData, bool bThisIsAll) override;

  virtual void OnParsingError(const char* szMessage, bool bFatal, wdUInt32 uiLine, wdUInt32 uiColumn) override;

protected:
  wdOpenDdlReaderElement* CreateElement(wdOpenDdlPrimitiveType type, const char* szType, const char* szName, bool bGlobalName);
  const char* CopyString(const wdStringView& string);
  void StorePrimitiveData(bool bThisIsAll, wdUInt32 bytecount, const wdUInt8* pData);

  void ClearDataChunks();
  wdUInt8* AllocateBytes(wdUInt32 uiNumBytes);

  static const wdUInt32 s_uiChunkSize = 1000 * 4; // 4 KiB

  wdHybridArray<wdUInt8*, 16> m_DataChunks;
  wdUInt8* m_pCurrentChunk;
  wdUInt32 m_uiBytesInChunkLeft;

  wdDynamicArray<wdUInt8> m_TempCache;

  wdDeque<wdOpenDdlReaderElement> m_Elements;
  wdHybridArray<wdOpenDdlReaderElement*, 16> m_ObjectStack;

  wdDeque<wdString> m_Strings;

  wdMap<wdString, wdOpenDdlReaderElement*> m_GlobalNames;
};
