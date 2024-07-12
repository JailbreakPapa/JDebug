#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/OpenDdlParser.h>
#include <Foundation/Logging/Log.h>

/// \brief Represents a single 'object' in a DDL document, e.g. either a custom type or a primitives list.
class NS_FOUNDATION_DLL nsOpenDdlReaderElement
{
public:
  NS_DECLARE_POD_TYPE();

  /// \brief Whether this is a custom object type that typically contains sub-elements.
  NS_ALWAYS_INLINE bool IsCustomType() const { return m_PrimitiveType == nsOpenDdlPrimitiveType::Custom; } // [tested]

  /// \brief Whether this is a custom object type of the requested type.
  NS_ALWAYS_INLINE bool IsCustomType(nsStringView sTypeName) const
  {
    return m_PrimitiveType == nsOpenDdlPrimitiveType::Custom && m_sCustomType == sTypeName;
  }

  /// \brief Returns the string for the custom type name.
  NS_ALWAYS_INLINE nsStringView GetCustomType() const { return m_sCustomType; } // [tested]

  /// \brief Whether the name of the object is non-empty.
  NS_ALWAYS_INLINE bool HasName() const { return !m_sName.IsEmpty(); } // [tested]

  /// \brief Returns the name of the object.
  NS_ALWAYS_INLINE nsStringView GetName() const { return m_sName; } // [tested]

  /// \brief Returns whether the element name is a global or a local name.
  NS_ALWAYS_INLINE bool IsNameGlobal() const { return (m_uiNumChildElements & NS_BIT(31)) != 0; } // [tested]

  /// \brief How many sub-elements the object has.
  nsUInt32 GetNumChildObjects() const; // [tested]

  /// \brief If this is a custom type element, the returned pointer is to the first child element.
  NS_ALWAYS_INLINE const nsOpenDdlReaderElement* GetFirstChild() const
  {
    return reinterpret_cast<const nsOpenDdlReaderElement*>(m_pFirstChild);
  } // [tested]

  /// \brief If the parent is a custom type element, the next child after this is returned.
  NS_ALWAYS_INLINE const nsOpenDdlReaderElement* GetSibling() const { return m_pSiblingElement; } // [tested]

  /// \brief For non-custom types this returns how many primitives are stored at this element.
  nsUInt32 GetNumPrimitives() const; // [tested]

  /// \brief For non-custom types this returns the type of primitive that is stored at this element.
  NS_ALWAYS_INLINE nsOpenDdlPrimitiveType GetPrimitivesType() const { return m_PrimitiveType; } // [tested]

  /// \brief Returns true if the element stores the requested type of primitives AND has at least the desired amount of them, so that accessing the
  /// data array at certain indices is safe.
  bool HasPrimitives(nsOpenDdlPrimitiveType type, nsUInt32 uiMinNumberOfPrimitives = 1) const;

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const bool* GetPrimitivesBool() const { return reinterpret_cast<const bool*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsInt8* GetPrimitivesInt8() const { return reinterpret_cast<const nsInt8*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsInt16* GetPrimitivesInt16() const { return reinterpret_cast<const nsInt16*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsInt32* GetPrimitivesInt32() const { return reinterpret_cast<const nsInt32*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsInt64* GetPrimitivesInt64() const { return reinterpret_cast<const nsInt64*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsUInt8* GetPrimitivesUInt8() const { return reinterpret_cast<const nsUInt8*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsUInt16* GetPrimitivesUInt16() const { return reinterpret_cast<const nsUInt16*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsUInt32* GetPrimitivesUInt32() const { return reinterpret_cast<const nsUInt32*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsUInt64* GetPrimitivesUInt64() const { return reinterpret_cast<const nsUInt64*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const float* GetPrimitivesFloat() const { return reinterpret_cast<const float*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const double* GetPrimitivesDouble() const { return reinterpret_cast<const double*>(m_pFirstChild); } // [tested]

  /// \brief Returns a pointer to the primitive data cast to a specific type. Only valid if GetPrimitivesType() actually returns this type.
  NS_ALWAYS_INLINE const nsStringView* GetPrimitivesString() const { return reinterpret_cast<const nsStringView*>(m_pFirstChild); } // [tested]

  /// \brief Searches for a child with the given name. It does not matter whether the object's name is 'local' or 'global'.
  /// \a szName is case-sensitive.
  const nsOpenDdlReaderElement* FindChild(nsStringView sName) const; // [tested]

  /// \brief Searches for a child element that has the given type, name and if it is a primitives list, at least the desired number of primitives.
  const nsOpenDdlReaderElement* FindChildOfType(nsOpenDdlPrimitiveType type, nsStringView sName, nsUInt32 uiMinNumberOfPrimitives = 1) const;

  /// \brief Searches for a child element with the given type and optionally also a certain name.
  const nsOpenDdlReaderElement* FindChildOfType(nsStringView sType, nsStringView sName = nullptr) const;

private:
  friend class nsOpenDdlReader;

  nsOpenDdlPrimitiveType m_PrimitiveType = nsOpenDdlPrimitiveType::Custom;
  nsUInt32 m_uiNumChildElements = 0;
  const void* m_pFirstChild = nullptr;
  const nsOpenDdlReaderElement* m_pLastChild = nullptr;
  nsStringView m_sCustomType;
  nsStringView m_sName;
  const nsOpenDdlReaderElement* m_pSiblingElement = nullptr;
};

/// \brief An OpenDDL reader parses an entire DDL document and creates an in-memory representation of the document structure.
class NS_FOUNDATION_DLL nsOpenDdlReader : public nsOpenDdlParser
{
public:
  nsOpenDdlReader();
  ~nsOpenDdlReader();

  /// \brief Parses the given document, returns NS_FAILURE if an unrecoverable parsing error was encountered.
  ///
  /// \param stream is the input data.
  /// \param uiFirstLineOffset allows to adjust the reported line numbers in error messages, in case the given stream represents a sub-section of a
  /// larger file. \param pLog is used for outputting details about parsing errors. If nullptr is given, no details are logged. \param uiCacheSizeInKB
  /// is the internal cache size that the parser uses. If the parsed documents contain primitives lists with several thousand elements in a single
  /// list, increasing the cache size can improve performance, but typically this doesn't need to be adjusted.
  nsResult ParseDocument(nsStreamReader& inout_stream, nsUInt32 uiFirstLineOffset = 0, nsLogInterface* pLog = nsLog::GetThreadLocalLogSystem(),
    nsUInt32 uiCacheSizeInKB = 4); // [tested]

  /// \brief Every document has exactly one root element.
  const nsOpenDdlReaderElement* GetRootElement() const; // [tested]

  /// \brief Searches for an element with a global name. NULL if there is no such element.
  const nsOpenDdlReaderElement* FindElement(nsStringView sGlobalName) const; // [tested]

protected:
  virtual void OnBeginObject(nsStringView sType, nsStringView sName, bool bGlobalName) override;
  virtual void OnEndObject() override;

  virtual void OnBeginPrimitiveList(nsOpenDdlPrimitiveType type, nsStringView sName, bool bGlobalName) override;
  virtual void OnEndPrimitiveList() override;

  virtual void OnPrimitiveBool(nsUInt32 count, const bool* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveInt8(nsUInt32 count, const nsInt8* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveInt16(nsUInt32 count, const nsInt16* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveInt32(nsUInt32 count, const nsInt32* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveInt64(nsUInt32 count, const nsInt64* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveUInt8(nsUInt32 count, const nsUInt8* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveUInt16(nsUInt32 count, const nsUInt16* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveUInt32(nsUInt32 count, const nsUInt32* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveUInt64(nsUInt32 count, const nsUInt64* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveFloat(nsUInt32 count, const float* pData, bool bThisIsAll) override;
  virtual void OnPrimitiveDouble(nsUInt32 count, const double* pData, bool bThisIsAll) override;

  virtual void OnPrimitiveString(nsUInt32 count, const nsStringView* pData, bool bThisIsAll) override;

  virtual void OnParsingError(nsStringView sMessage, bool bFatal, nsUInt32 uiLine, nsUInt32 uiColumn) override;

protected:
  nsOpenDdlReaderElement* CreateElement(nsOpenDdlPrimitiveType type, nsStringView sType, nsStringView sName, bool bGlobalName);
  nsStringView CopyString(const nsStringView& string);
  void StorePrimitiveData(bool bThisIsAll, nsUInt32 bytecount, const nsUInt8* pData);

  void ClearDataChunks();
  nsUInt8* AllocateBytes(nsUInt32 uiNumBytes);

  static constexpr nsUInt32 s_uiChunkSize = 1000 * 4; // 4 KiB

  nsHybridArray<nsUInt8*, 16> m_DataChunks;
  nsUInt8* m_pCurrentChunk;
  nsUInt32 m_uiBytesInChunkLeft;

  nsDynamicArray<nsUInt8> m_TempCache;

  nsDeque<nsOpenDdlReaderElement> m_Elements;
  nsHybridArray<nsOpenDdlReaderElement*, 16> m_ObjectStack;

  nsDeque<nsString> m_Strings;

  nsMap<nsString, nsOpenDdlReaderElement*> m_GlobalNames;
};
