#pragma once

#include <Foundation/Containers/HashSet.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/SerializationContext.h>

class nsStreamWriter;
class nsStreamReader;

/// \brief This class allows for writing type versions to a stream in a centralized place so that
/// each object doesn't need to write its own version manually.
///
/// To use, create an object of this type on the stack, call Begin() and use the returned
/// nsStreamWriter for subsequent serialization operations. Call AddType to add a type and its parent types to the version table.
/// Call End() once you want to finish writing the type versions.
class NS_FOUNDATION_DLL nsTypeVersionWriteContext : public nsSerializationContext<nsTypeVersionWriteContext>
{
  NS_DECLARE_SERIALIZATION_CONTEXT(nsTypeVersionWriteContext);

public:
  nsTypeVersionWriteContext();
  ~nsTypeVersionWriteContext();

  /// \brief Call this method to begin collecting type version info. You need to use the returned stream writer for subsequent serialization operations until
  /// End() is called.
  nsStreamWriter& Begin(nsStreamWriter& ref_originalStream);

  /// \brief Ends the type version collection and writes the data to the original stream.
  nsResult End();

  /// \brief Adds the given type and its parent types to the version table.
  void AddType(const nsRTTI* pRtti);

  /// \brief Manually write the version table to the given stream.
  /// Can be used instead of Begin()/End() if all necessary types are available in one place anyways.
  void WriteTypeVersions(nsStreamWriter& inout_stream) const;

  /// \brief Returns the original stream that was passed to Begin().
  nsStreamWriter& GetOriginalStream() { return *m_pOriginalStream; }

protected:
  nsStreamWriter* m_pOriginalStream = nullptr;

  nsDefaultMemoryStreamStorage m_TempStreamStorage;
  nsMemoryStreamWriter m_TempStreamWriter;

  nsHashSet<const nsRTTI*> m_KnownTypes;
};

/// \brief Use this class to restore type versions written to a stream using a nsTypeVersionWriteContext.
class NS_FOUNDATION_DLL nsTypeVersionReadContext : public nsSerializationContext<nsTypeVersionReadContext>
{
  NS_DECLARE_SERIALIZATION_CONTEXT(nsTypeVersionReadContext);

public:
  /// \brief Reads the type version table from the stream
  nsTypeVersionReadContext(nsStreamReader& inout_stream);
  ~nsTypeVersionReadContext();

  nsUInt32 GetTypeVersion(const nsRTTI* pRtti) const;

protected:
  nsHashTable<const nsRTTI*, nsUInt32> m_TypeVersions;
};
