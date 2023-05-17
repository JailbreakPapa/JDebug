#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Types/Variant.h>

/// \brief The base class for JSON writers.
///
/// Declares a common interface for writing JSON files. Also implements some utility functions built on top of the interface (AddVariable()).
class WD_FOUNDATION_DLL wdJSONWriter
{
public:
  /// \brief Modes to configure how much whitespace the JSON writer will output
  enum class WhitespaceMode
  {
    All,             ///< All whitespace is output. This is the default, it should be used for files that are read by humans.
    LessIndentation, ///< Saves some space by using less space for indentation
    NoIndentation,   ///< Saves even more space by dropping all indentation from the output. The result will be noticeably less readable.
    NewlinesOnly,    ///< All unnecessary whitespace, except for newlines, is not output.
    None,            ///< No whitespace, not even newlines, is output. This should be used when JSON is used for data exchange, but probably not read by humans.
  };

  /// \brief Modes to configure how arrays are written.
  enum class ArrayMode
  {
    InOneLine,      ///< All array items are written in a single line in the file.
    OneLinePerItem, ///< Each array item is put on a separate line.
  };

  /// \brief Constructor
  wdJSONWriter();

  /// \brief Destructor
  virtual ~wdJSONWriter();

  /// \brief Configures how much whitespace is output.
  void SetWhitespaceMode(WhitespaceMode whitespaceMode) { m_WhitespaceMode = whitespaceMode; }

  /// \brief Configures how arrays are written.
  void SetArrayMode(ArrayMode arrayMode) { m_ArrayMode = arrayMode; }

  /// \brief Shorthand for "BeginVariable(szName); WriteBool(value); EndVariable(); "
  void AddVariableBool(const char* szName, bool value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteInt32(value); EndVariable(); "
  void AddVariableInt32(const char* szName, wdInt32 value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteUInt32(value); EndVariable(); "
  void AddVariableUInt32(const char* szName, wdUInt32 value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteInt64(value); EndVariable(); "
  void AddVariableInt64(const char* szName, wdInt64 value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteUInt64(value); EndVariable(); "
  void AddVariableUInt64(const char* szName, wdUInt64 value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteFloat(value); EndVariable(); "
  void AddVariableFloat(const char* szName, float value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteDouble(value); EndVariable(); "
  void AddVariableDouble(const char* szName, double value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteString(value); EndVariable(); "
  void AddVariableString(const char* szName, wdStringView value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteNULL(value); EndVariable(); "
  void AddVariableNULL(const char* szName); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteTime(value); EndVariable(); "
  void AddVariableTime(const char* szName, wdTime value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteUuid(value); EndVariable(); "
  void AddVariableUuid(const char* szName, wdUuid value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteAngle(value); EndVariable(); "
  void AddVariableAngle(const char* szName, wdAngle value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteColor(value); EndVariable(); "
  void AddVariableColor(const char* szName, const wdColor& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteColorGamma(value); EndVariable(); "
  void AddVariableColorGamma(const char* szName, const wdColorGammaUB& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteVec2(value); EndVariable(); "
  void AddVariableVec2(const char* szName, const wdVec2& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteVec3(value); EndVariable(); "
  void AddVariableVec3(const char* szName, const wdVec3& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteVec4(value); EndVariable(); "
  void AddVariableVec4(const char* szName, const wdVec4& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteVec2I32(value); EndVariable(); "
  void AddVariableVec2I32(const char* szName, const wdVec2I32& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteVec3I32(value); EndVariable(); "
  void AddVariableVec3I32(const char* szName, const wdVec3I32& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteVec4I32(value); EndVariable(); "
  void AddVariableVec4I32(const char* szName, const wdVec4I32& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteQuat(value); EndVariable(); "
  void AddVariableQuat(const char* szName, const wdQuat& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteMat3(value); EndVariable(); "
  void AddVariableMat3(const char* szName, const wdMat3& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteMat4(value); EndVariable(); "
  void AddVariableMat4(const char* szName, const wdMat4& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteDataBuffer(value); EndVariable(); "
  void AddVariableDataBuffer(const char* szName, const wdDataBuffer& value); // [tested]

  /// \brief Shorthand for "BeginVariable(szName); WriteVariant(value); EndVariable(); "
  void AddVariableVariant(const char* szName, const wdVariant& value); // [tested]


  /// \brief Writes a bool to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteBool(bool value) = 0;

  /// \brief Writes an int32 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteInt32(wdInt32 value) = 0;

  /// \brief Writes a uint32 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteUInt32(wdUInt32 value) = 0;

  /// \brief Writes an int64 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteInt64(wdInt64 value) = 0;

  /// \brief Writes a uint64 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteUInt64(wdUInt64 value) = 0;

  /// \brief Writes a float to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteFloat(float value) = 0;

  /// \brief Writes a double to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteDouble(double value) = 0;

  /// \brief Writes a string to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteString(wdStringView value) = 0;

  /// \brief Writes the value 'null' to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteNULL() = 0;

  /// \brief Writes a time value to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  virtual void WriteTime(wdTime value) = 0;

  /// \brief Writes an wdColor to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteColor(const wdColor& value) = 0;

  /// \brief Writes an wdColorGammaUB to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteColorGamma(const wdColorGammaUB& value) = 0;

  /// \brief Writes an wdVec2 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteVec2(const wdVec2& value) = 0;

  /// \brief Writes an wdVec3 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteVec3(const wdVec3& value) = 0;

  /// \brief Writes an wdVec4 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteVec4(const wdVec4& value) = 0;

  /// \brief Writes an wdVec2I32 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteVec2I32(const wdVec2I32& value) = 0;

  /// \brief Writes an wdVec3I32 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteVec3I32(const wdVec3I32& value) = 0;

  /// \brief Writes an wdVec4I32 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteVec4I32(const wdVec4I32& value) = 0;

  /// \brief Writes an wdQuat to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteQuat(const wdQuat& value) = 0;

  /// \brief Writes an wdMat3 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteMat3(const wdMat3& value) = 0;

  /// \brief Writes an wdMat4 to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteMat4(const wdMat4& value) = 0;

  /// \brief Writes an wdUuid to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteUuid(const wdUuid& value) = 0;

  /// \brief Writes an wdAngle to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteAngle(wdAngle value) = 0; // [tested]

  /// \brief Writes an wdDataBuffer to the JSON file. Can only be called between BeginVariable() / EndVariable() or BeginArray() / EndArray().
  ///
  /// \note Standard JSON does not have a suitable type for this. A derived class might turn this into an object or output it via WriteBinaryData().
  virtual void WriteDataBuffer(const wdDataBuffer& value) = 0; // [tested]

  /// \brief The default implementation dispatches all supported types to WriteBool, WriteInt32, etc. and asserts on the more complex types.
  ///
  /// A derived class may override this function to implement support for the remaining variant types, if required.
  virtual void WriteVariant(const wdVariant& value); // [tested]

  /// \brief Outputs a chunk of memory in some JSON form that can be interpreted as binary data when reading it again.
  ///
  /// How exactly the raw data is represented in JSON is up to the derived class. \a szDataType allows to additionally output a string
  /// that identifies the type of data.
  virtual void WriteBinaryData(const char* szDataType, const void* pData, wdUInt32 uiBytes, const char* szValueString = nullptr) = 0;

  /// \brief Begins outputting a variable. \a szName is the variable name.
  ///
  /// Between BeginVariable() and EndVariable() you can call the WriteXYZ functions once to write out the variable's data.
  /// You can also call BeginArray() and BeginObject() without a variable name to output an array or object variable.
  virtual void BeginVariable(const char* szName) = 0;

  /// \brief Ends outputting a variable.
  virtual void EndVariable() = 0;

  /// \brief Begins outputting an array variable.
  ///
  /// If szName is nullptr this will create an anonymous array, which is necessary when you want to put an array as a value into another array.
  /// BeginArray() with a non-nullptr value for \a szName is identical to calling BeginVariable() first. In this case EndArray() will also
  /// end the variable definition, so no additional call to EndVariable() is required.
  virtual void BeginArray(const char* szName = nullptr) = 0;

  /// \brief Ends outputting an array variable.
  virtual void EndArray() = 0;

  /// \brief Begins outputting an object variable.
  ///
  /// If szName is nullptr this will create an anonymous object, which is necessary when you want to put an object as a value into an array.
  /// BeginObject() with a non-nullptr value for \a szName is identical to calling BeginVariable() first. In this case EndObject() will also
  /// end the variable definition, so no additional call to EndVariable() is required.
  virtual void BeginObject(const char* szName = nullptr) = 0;

  /// \brief Ends outputting an object variable.
  virtual void EndObject() = 0;

  /// \brief Indicates if an error was encountered while writing
  ///
  /// If any error was encountered at any time during writing, this will return true
  bool HadWriteError() const;

protected:
  WhitespaceMode m_WhitespaceMode = WhitespaceMode::All;
  ArrayMode m_ArrayMode = ArrayMode::InOneLine;

  /// \brief called internally when there was an error during writing
  void SetWriteErrorState();

private:
  bool m_bHadWriteError = false;
};


/// \brief Implements a standard compliant JSON writer, all numbers are output as double values.
///
/// wdStandardJSONWriter also implements WriteBinaryData() and the functions WriteVec2() etc., for which there is no standard way to implement them in
/// JSON. WriteVec2() etc. will simply redirect to WriteBinaryData(), which in turn implements the MongoDB convention of outputting binary data.
/// I.e. it will turn the data into a JSON object which contains one variable called "$type" that identifies the data type, and one variable called
/// "$binary" which contains the raw binary data Hex encoded in little endian format.
/// If you want to write a fully standard compliant JSON file, just don't output any of these types.
class WD_FOUNDATION_DLL wdStandardJSONWriter : public wdJSONWriter
{
public:
  /// \brief Constructor.
  wdStandardJSONWriter(); // [tested]

  /// \brief Destructor.
  ~wdStandardJSONWriter(); // [tested]

  /// \brief All output is written to this binary stream.
  void SetOutputStream(wdStreamWriter* pOutput); // [tested]

  /// \brief \copydoc wdJSONWriter::WriteBool()
  virtual void WriteBool(bool value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteInt32()
  virtual void WriteInt32(wdInt32 value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteUInt32()
  virtual void WriteUInt32(wdUInt32 value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteInt64()
  virtual void WriteInt64(wdInt64 value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteUInt64()
  virtual void WriteUInt64(wdUInt64 value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteFloat()
  virtual void WriteFloat(float value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteDouble()
  virtual void WriteDouble(double value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteString()
  virtual void WriteString(wdStringView value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteNULL()
  virtual void WriteNULL() override; // [tested]

  /// \brief Writes the time value as a double (i.e. redirects to WriteDouble()).
  virtual void WriteTime(wdTime value) override; // [tested]

  /// \brief Outputs the value via WriteVec4().
  virtual void WriteColor(const wdColor& value) override; // [tested]

  /// \brief Outputs the value via WriteVec4().
  virtual void WriteColorGamma(const wdColorGammaUB& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteVec2(const wdVec2& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteVec3(const wdVec3& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteVec4(const wdVec4& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteVec2I32(const wdVec2I32& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteVec3I32(const wdVec3I32& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteVec4I32(const wdVec4I32& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteQuat(const wdQuat& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteMat3(const wdMat3& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteMat4(const wdMat4& value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteUuid(const wdUuid& value) override; // [tested]

  /// \brief \copydoc wdJSONWriter::WriteFloat()
  virtual void WriteAngle(wdAngle value) override; // [tested]

  /// \brief Outputs the value via WriteBinaryData().
  virtual void WriteDataBuffer(const wdDataBuffer& value) override; // [tested]

  /// \brief Implements the MongoDB way of writing binary data. First writes a "$type" variable, then a "$binary" variable that represents the raw
  /// data (Hex encoded, little endian).
  virtual void WriteBinaryData(const char* szDataType, const void* pData, wdUInt32 uiBytes, const char* szValueString = nullptr) override; // [tested]

  /// \brief \copydoc wdJSONWriter::BeginVariable()
  virtual void BeginVariable(const char* szName) override; // [tested]

  /// \brief \copydoc wdJSONWriter::EndVariable()
  virtual void EndVariable() override; // [tested]

  /// \brief \copydoc wdJSONWriter::BeginArray()
  virtual void BeginArray(const char* szName = nullptr) override; // [tested]

  /// \brief \copydoc wdJSONWriter::EndArray()
  virtual void EndArray() override; // [tested]

  /// \brief \copydoc wdJSONWriter::BeginObject()
  virtual void BeginObject(const char* szName = nullptr) override; // [tested]

  /// \brief \copydoc wdJSONWriter::EndObject()
  virtual void EndObject() override; // [tested]

protected:
  void End();

  enum State
  {
    Invalid,
    Empty,
    Variable,
    Object,
    NamedObject,
    Array,
    NamedArray,
  };

  struct JSONState
  {
    JSONState();

    State m_State;
    bool m_bRequireComma;
    bool m_bValueWasWritten;
  };

  struct CommaWriter
  {
    CommaWriter(wdStandardJSONWriter* pWriter);
    ~CommaWriter();

    wdStandardJSONWriter* m_pWriter;
  };

  void OutputString(wdStringView s);
  void OutputEscapedString(wdStringView s);
  void OutputIndentation();

  wdInt32 m_iIndentation;
  wdStreamWriter* m_pOutput;

  wdHybridArray<JSONState, 16> m_StateStack;
};
