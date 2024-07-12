#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/OpenDdlParser.h>

// TODO
// Write primitives in HEX (esp. float)

/// \brief The base class for OpenDDL writers.
///
/// Declares a common interface for writing OpenDDL files.
class NS_FOUNDATION_DLL nsOpenDdlWriter
{
public:
  enum class TypeStringMode
  {
    Compliant,            ///< All primitive types are written as the OpenDDL standard defines them (very verbose)
    ShortenedUnsignedInt, ///< unsigned_intX is shortened to uintX
    Shortest              ///< All primitive type names are shortened to one or two characters: i1, i2, i3, i4, u1, u2, u3, u4, b, s, f, d (int, uint, bool,
                          ///< string, float, double)
  };

  enum class FloatPrecisionMode
  {
    Readable, ///< Float values are printed as readable numbers. Precision might get lost though.
    Exact,    ///< Float values are printed as HEX, representing the exact binary data.
  };

  /// \brief Constructor
  nsOpenDdlWriter();

  virtual ~nsOpenDdlWriter() = default;

  /// \brief All output is written to this binary stream.
  void SetOutputStream(nsStreamWriter* pOutput) { m_pOutput = pOutput; } // [tested]

  /// \brief Configures how much whitespace is output.
  void SetCompactMode(bool bCompact) { m_bCompactMode = bCompact; } // [tested]

  /// \brief Configures how verbose the type strings are going to be written.
  void SetPrimitiveTypeStringMode(TypeStringMode mode) { m_TypeStringMode = mode; }

  /// \brief Configures how float values are output.
  void SetFloatPrecisionMode(FloatPrecisionMode mode) { m_FloatPrecisionMode = mode; }

  /// \brief Returns how float values are output.
  FloatPrecisionMode GetFloatPrecisionMode() const { return m_FloatPrecisionMode; }

  /// \brief Allows to set the indentation. Negative values are possible.
  /// This makes it possible to set the indentation e.g. to -2, thus the output will only have indentation after a level of 3 has been reached.
  void SetIndentation(nsInt8 iIndentation) { m_iIndentation = iIndentation; }

  /// \brief Begins outputting an object.
  void BeginObject(nsStringView sType, nsStringView sName = {}, bool bGlobalName = false, bool bSingleLine = false); // [tested]

  /// \brief Ends outputting an object.
  void EndObject(); // [tested]

  /// \brief Begins outputting a list of primitives of the given type.
  void BeginPrimitiveList(nsOpenDdlPrimitiveType type, nsStringView sName = {}, bool bGlobalName = false); // [tested]

  /// \brief Ends outputting the list of primitives.
  void EndPrimitiveList(); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteBool(const bool* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteInt8(const nsInt8* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteInt16(const nsInt16* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteInt32(const nsInt32* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteInt64(const nsInt64* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteUInt8(const nsUInt8* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteUInt16(const nsUInt16* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteUInt32(const nsUInt32* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteUInt64(const nsUInt64* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteFloat(const float* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a number of values to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteDouble(const double* pValues, nsUInt32 uiCount = 1); // [tested]

  /// \brief Writes a single string to the primitive list. Can be called multiple times between BeginPrimitiveList() / EndPrimitiveList().
  void WriteString(const nsStringView& sString); // [tested]

  /// \brief Writes a single string to the primitive list, but the value is a HEX representation of the given binary data.
  void WriteBinaryAsString(const void* pData, nsUInt32 uiBytes);


protected:
  enum State
  {
    Invalid = -5,
    Empty = -4,
    ObjectSingleLine = -3,
    ObjectMultiLine = -2,
    ObjectStart = -1,
    PrimitivesBool = 0, // same values as in nsOpenDdlPrimitiveType to enable casting
    PrimitivesInt8,
    PrimitivesInt16,
    PrimitivesInt32,
    PrimitivesInt64,
    PrimitivesUInt8,
    PrimitivesUInt16,
    PrimitivesUInt32,
    PrimitivesUInt64,
    PrimitivesFloat,
    PrimitivesDouble,
    PrimitivesString,
  };

  struct DdlState
  {
    State m_State = State::Empty;
    bool m_bPrimitivesWritten = false;
  };

  NS_ALWAYS_INLINE void OutputString(nsStringView s) { m_pOutput->WriteBytes(s.GetStartPointer(), s.GetElementCount()).IgnoreResult(); }
  NS_ALWAYS_INLINE void OutputString(nsStringView s, nsUInt32 uiElementCount) { m_pOutput->WriteBytes(s.GetStartPointer(), uiElementCount).IgnoreResult(); }
  void OutputEscapedString(const nsStringView& string);
  void OutputIndentation();
  void OutputPrimitiveTypeNameCompliant(nsOpenDdlPrimitiveType type);
  void OutputPrimitiveTypeNameShort(nsOpenDdlPrimitiveType type);
  void OutputPrimitiveTypeNameShortest(nsOpenDdlPrimitiveType type);
  void WritePrimitiveType(nsOpenDdlWriter::State exp);
  void OutputObjectName(nsStringView sName, bool bGlobalName);
  void WriteBinaryAsHex(const void* pData, nsUInt32 uiBytes);
  void OutputObjectBeginning();

  nsInt32 m_iIndentation = 0;
  bool m_bCompactMode = false;
  TypeStringMode m_TypeStringMode = TypeStringMode::ShortenedUnsignedInt;
  FloatPrecisionMode m_FloatPrecisionMode = FloatPrecisionMode::Exact;
  nsStreamWriter* m_pOutput = nullptr;
  nsStringBuilder m_sTemp;

  nsHybridArray<DdlState, 16> m_StateStack;
};
