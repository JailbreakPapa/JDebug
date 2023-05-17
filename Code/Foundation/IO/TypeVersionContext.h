#pragma once

#include <Foundation/Containers/HashSet.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/SerializationContext.h>

class wdStreamWriter;
class wdStreamReader;

/// \brief This class allows for writing type versions to a stream in a centralized place so that
/// each object doesn't need to write its own version manually.
///
/// To use, create an object of this type on the stack, call Begin() and use the returned
/// wdStreamWriter for subsequent serialization operations. Call AddType to add a type and its parent types to the version table.
/// Call End() once you want to finish writing the type versions.
class WD_FOUNDATION_DLL wdTypeVersionWriteContext : public wdSerializationContext<wdTypeVersionWriteContext>
{
  WD_DECLARE_SERIALIZATION_CONTEXT(wdTypeVersionWriteContext);

public:
  wdTypeVersionWriteContext();
  ~wdTypeVersionWriteContext();

  /// \brief Call this method to begin collecting type version info. You need to use the returned stream writer for subsequent serialization operations until
  /// End() is called.
  wdStreamWriter& Begin(wdStreamWriter& ref_originalStream);

  /// \brief Ends the type version collection and writes the data to the original stream.
  wdResult End();

  /// \brief Adds the given type and its parent types to the version table.
  void AddType(const wdRTTI* pRtti);

  /// \brief Manually write the version table to the given stream.
  /// Can be used instead of Begin()/End() if all necessary types are available in one place anyways.
  void WriteTypeVersions(wdStreamWriter& inout_stream) const;

  /// \brief Returns the original stream that was passed to Begin().
  wdStreamWriter& GetOriginalStream() { return *m_pOriginalStream; }

protected:
  wdStreamWriter* m_pOriginalStream = nullptr;

  wdDefaultMemoryStreamStorage m_TempStreamStorage;
  wdMemoryStreamWriter m_TempStreamWriter;

  wdHashSet<const wdRTTI*> m_KnownTypes;
};

/// \brief Use this class to restore type versions written to a stream using a wdTypeVersionWriteContext.
class WD_FOUNDATION_DLL wdTypeVersionReadContext : public wdSerializationContext<wdTypeVersionReadContext>
{
  WD_DECLARE_SERIALIZATION_CONTEXT(wdTypeVersionReadContext);

public:
  /// \brief Reads the type version table from the stream
  wdTypeVersionReadContext(wdStreamReader& inout_stream);
  ~wdTypeVersionReadContext();

  wdUInt32 GetTypeVersion(const wdRTTI* pRtti) const;

protected:
  wdHashTable<const wdRTTI*, wdUInt32> m_TypeVersions;
};
