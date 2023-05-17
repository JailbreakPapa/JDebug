#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/IO/Stream.h>

class wdLogInterface;

/// \brief The primitive data types that OpenDDL supports
enum class wdOpenDdlPrimitiveType
{
  Bool,
  Int8,
  Int16,
  Int32,
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  // Half, // Currently not supported
  Float,
  Double,
  String,
  // Ref, // Currently not supported
  // Type // Currently not supported
  Custom
};

/// \brief A low level parser for the OpenDDL format. It can incrementally parse the structure, individual blocks can be skipped.
///
/// The document structure is returned through virtual functions that need to be overridden.
class WD_FOUNDATION_DLL wdOpenDdlParser
{
public:
  wdOpenDdlParser();
  virtual ~wdOpenDdlParser() = default;

  /// \brief Whether an error occured during parsing that resulted in cancelation of further parsing.
  bool HadFatalParsingError() const { return m_bHadFatalParsingError; } // [tested]

protected:
  /// \brief Sets an wdLogInterface through which errors and warnings are reported.
  void SetLogInterface(wdLogInterface* pLog) { m_pLogInterface = pLog; }

  /// \brief Data is returned in larger chunks, to reduce the number of function calls. The cache size determines the maximum chunk size per primitive
  /// type.
  ///
  /// Default cache size is 4 KB. That means up to 1000 integers may be returned in one chunk (or 500 doubles).
  /// It does not help to increase the chunk size, when the input data doesn't use such large data lists.
  void SetCacheSize(wdUInt32 uiSizeInKB);

  /// \brief Configures the parser to read from the given stream. This can only be called once on a parser instance.
  void SetInputStream(wdStreamReader& stream, wdUInt32 uiFirstLineOffset = 0); // [tested]

  /// \brief Call this to parse the next piece of the document. This may trigger a callback through which data is returned.
  ///
  /// This function returns false when the end of the document has been reached, or a fatal parsing error has been reported.
  bool ContinueParsing(); // [tested]

  /// \brief Calls ContinueParsing() in a loop until that returns false.
  wdResult ParseAll(); // [tested]

  /// \brief Skips the rest of the currently open object. No OnEndObject() call will be done for this object either.
  void SkipRestOfObject();

  /// \brief Can be used to prevent parsing the rest of the document.
  void StopParsing();

  /// \brief Outputs that a parsing error was detected (via OnParsingError) and stops further parsing, if bFatal is set to true.
  void ParsingError(const char* szMessage, bool bFatal);

  wdLogInterface* m_pLogInterface;

protected:
  /// \brief Called when something unexpected is encountered in the document.
  ///
  /// The error message describes what was expected and what was encountered.
  /// If bFatal is true, the error has left the parser in an unrecoverable state and thus it will not continue parsing.
  /// In that case client code will need to clean up it's open state, as no further callbacks will be called.
  /// If bFatal is false, the document is not entirely valid, but the parser is still able to continue.
  virtual void OnParsingError(const char* szMessage, bool bFatal, wdUInt32 uiLine, wdUInt32 uiColumn) {}

  /// \brief Called when a new object is encountered.
  virtual void OnBeginObject(const char* szType, const char* szName, bool bGlobalName) = 0;

  /// \brief Called when the end of an object is encountered.
  virtual void OnEndObject() = 0;

  /// \brief Called when a new primitive object is encountered.
  virtual void OnBeginPrimitiveList(wdOpenDdlPrimitiveType type, const char* szName, bool bGlobalName) = 0;

  /// \brief Called when the end of a primitive object is encountered.
  virtual void OnEndPrimitiveList() = 0;

  /// \todo Currently not supported
  // virtual void OnBeginPrimitiveArrayList(wdOpenDdlPrimitiveType type, wdUInt32 uiGroupSize) = 0;
  // virtual void OnEndPrimitiveArrayList() = 0;

  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveBool(wdUInt32 count, const bool* pData, bool bThisIsAll) = 0;

  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveInt8(wdUInt32 count, const wdInt8* pData, bool bThisIsAll) = 0;
  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveInt16(wdUInt32 count, const wdInt16* pData, bool bThisIsAll) = 0;
  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveInt32(wdUInt32 count, const wdInt32* pData, bool bThisIsAll) = 0;
  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveInt64(wdUInt32 count, const wdInt64* pData, bool bThisIsAll) = 0;

  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveUInt8(wdUInt32 count, const wdUInt8* pData, bool bThisIsAll) = 0;
  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveUInt16(wdUInt32 count, const wdUInt16* pData, bool bThisIsAll) = 0;
  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveUInt32(wdUInt32 count, const wdUInt32* pData, bool bThisIsAll) = 0;
  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveUInt64(wdUInt32 count, const wdUInt64* pData, bool bThisIsAll) = 0;

  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveFloat(wdUInt32 count, const float* pData, bool bThisIsAll) = 0;
  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveDouble(wdUInt32 count, const double* pData, bool bThisIsAll) = 0;

  /// \brief Called when data for a primitive type is available. More than one value may be reported at a time.
  virtual void OnPrimitiveString(wdUInt32 count, const wdStringView* pData, bool bThisIsAll) = 0;

private:
  enum State
  {
    Finished,
    Idle,
    ReadingBool,
    ReadingInt8,
    ReadingInt16,
    ReadingInt32,
    ReadingInt64,
    ReadingUInt8,
    ReadingUInt16,
    ReadingUInt32,
    ReadingUInt64,
    ReadingFloat,
    ReadingDouble,
    ReadingString,
  };

  struct DdlState
  {
    DdlState()
      : m_State(Idle)
    {
    }
    DdlState(State s)
      : m_State(s)
    {
    }

    State m_State;
  };

  void ReadNextByte();
  bool ReadCharacter();
  bool ReadCharacterSkipComments();
  void SkipWhitespace();
  void ContinueIdle();
  void ReadIdentifier(wdUInt8* szString, wdUInt32& count);
  void ReadString();
  void ReadWord();
  wdUInt64 ReadDecimalLiteral();
  void PurgeCachedPrimitives(bool bThisIsAll);
  bool ContinuePrimitiveList();
  void ContinueString();
  void SkipString();
  void ContinueBool();
  void ContinueInt();
  void ContinueFloat();

  void ReadDecimalFloat();
  void ReadHexString();

  wdHybridArray<DdlState, 32> m_StateStack;
  wdStreamReader* m_pInput;
  wdDynamicArray<wdUInt8> m_Cache;

  static const wdUInt32 s_uiMaxIdentifierLength = 64;

  wdUInt8 m_uiCurByte;
  wdUInt8 m_uiNextByte;
  wdUInt32 m_uiCurLine;
  wdUInt32 m_uiCurColumn;
  bool m_bSkippingMode;
  bool m_bHadFatalParsingError;
  wdUInt8 m_szIdentifierType[s_uiMaxIdentifierLength];
  wdUInt8 m_szIdentifierName[s_uiMaxIdentifierLength];
  wdDynamicArray<wdUInt8> m_TempString;
  wdUInt32 m_uiTempStringLength;

  wdUInt32 m_uiNumCachedPrimitives;
  bool* m_pBoolCache;
  wdInt8* m_pInt8Cache;
  wdInt16* m_pInt16Cache;
  wdInt32* m_pInt32Cache;
  wdInt64* m_pInt64Cache;
  wdUInt8* m_pUInt8Cache;
  wdUInt16* m_pUInt16Cache;
  wdUInt32* m_pUInt32Cache;
  wdUInt64* m_pUInt64Cache;
  float* m_pFloatCache;
  double* m_pDoubleCache;
};
