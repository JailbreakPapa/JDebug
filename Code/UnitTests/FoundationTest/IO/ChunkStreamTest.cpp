#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/Deque.h>
#include <Foundation/IO/ChunkStream.h>
#include <Foundation/IO/MemoryStream.h>

NS_CREATE_SIMPLE_TEST(IO, ChunkStream)
{
  nsDefaultMemoryStreamStorage StreamStorage;

  nsMemoryStreamWriter MemoryWriter(&StreamStorage);
  nsMemoryStreamReader MemoryReader(&StreamStorage);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Write Format")
  {
    nsChunkStreamWriter writer(MemoryWriter);

    writer.BeginStream(1);

    {
      writer.BeginChunk("Chunk1", 1);

      writer << (nsUInt32)4;
      writer << (float)5.6f;
      writer << (double)7.8;
      writer << "nine";
      writer << nsVec3(10, 11.2f, 13.4f);

      writer.EndChunk();
    }

    {
      writer.BeginChunk("Chunk2", 2);

      writer << "chunk 2 content";

      writer.EndChunk();
    }

    {
      writer.BeginChunk("Chunk3", 3);

      writer << "chunk 3 content";

      writer.EndChunk();
    }

    writer.EndStream();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Read Format")
  {
    nsChunkStreamReader reader(MemoryReader);

    reader.BeginStream();

    // Chunk 1
    {
      NS_TEST_BOOL(reader.GetCurrentChunk().m_bValid);
      NS_TEST_STRING(reader.GetCurrentChunk().m_sChunkName.GetData(), "Chunk1");
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkVersion, 1);
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkBytes, reader.GetCurrentChunk().m_uiUnreadChunkBytes);
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkBytes, 36);

      nsUInt32 i;
      float f;
      double d;
      nsString s;

      reader >> i;
      reader >> f;
      reader >> d;
      reader >> s;

      NS_TEST_INT(i, 4);
      NS_TEST_FLOAT(f, 5.6f, 0);
      NS_TEST_DOUBLE(d, 7.8, 0);
      NS_TEST_STRING(s.GetData(), "nine");

      NS_TEST_INT(reader.GetCurrentChunk().m_uiUnreadChunkBytes, 12);
      reader.NextChunk();
    }

    // Chunk 2
    {
      NS_TEST_BOOL(reader.GetCurrentChunk().m_bValid);
      NS_TEST_STRING(reader.GetCurrentChunk().m_sChunkName.GetData(), "Chunk2");
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkVersion, 2);
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkBytes, reader.GetCurrentChunk().m_uiUnreadChunkBytes);
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkBytes, 19);

      nsString s;

      reader >> s;

      NS_TEST_STRING(s.GetData(), "chunk 2 content");

      NS_TEST_INT(reader.GetCurrentChunk().m_uiUnreadChunkBytes, 0);
      reader.NextChunk();
    }

    // Chunk 3
    {
      NS_TEST_BOOL(reader.GetCurrentChunk().m_bValid);
      NS_TEST_STRING(reader.GetCurrentChunk().m_sChunkName.GetData(), "Chunk3");
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkVersion, 3);
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkBytes, reader.GetCurrentChunk().m_uiUnreadChunkBytes);
      NS_TEST_INT(reader.GetCurrentChunk().m_uiChunkBytes, 19);

      nsString s;

      reader >> s;

      NS_TEST_STRING(s.GetData(), "chunk 3 content");

      NS_TEST_INT(reader.GetCurrentChunk().m_uiUnreadChunkBytes, 0);
      reader.NextChunk();
    }

    NS_TEST_BOOL(!reader.GetCurrentChunk().m_bValid);

    reader.SetEndChunkFileMode(nsChunkStreamReader::EndChunkFileMode::SkipToEnd);
    reader.EndStream();

    nsUInt8 Temp[1024];
    NS_TEST_INT(MemoryReader.ReadBytes(Temp, 1024), 0); // nothing left to read
  }
}
