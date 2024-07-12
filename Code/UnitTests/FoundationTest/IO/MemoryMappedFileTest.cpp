#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/IO/OSFile.h>

#if NS_ENABLED(NS_SUPPORTS_MEMORY_MAPPED_FILE)

NS_CREATE_SIMPLE_TEST(IO, MemoryMappedFile)
{
  nsStringBuilder sOutputFile = nsTestFramework::GetInstance()->GetAbsOutputPath();
  sOutputFile.MakeCleanPath();
  sOutputFile.AppendPath("IO");
  sOutputFile.AppendPath("MemoryMappedFile.dat");

  const nsUInt32 uiFileSize = 1024 * 1024 * 16; // * 4

  // generate test data
  {
    nsOSFile file;
    if (!NS_TEST_BOOL_MSG(file.Open(sOutputFile, nsFileOpenMode::Write).Succeeded(), "File for memory mapping could not be created"))
      return;

    nsDynamicArray<nsUInt32> data;
    data.SetCountUninitialized(uiFileSize);

    for (nsUInt32 i = 0; i < uiFileSize; ++i)
    {
      data[i] = i;
    }

    file.Write(data.GetData(), data.GetCount() * sizeof(nsUInt32)).IgnoreResult();
    file.Close();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Memory map for writing")
  {
    nsMemoryMappedFile memFile;

    if (!NS_TEST_BOOL_MSG(memFile.Open(sOutputFile, nsMemoryMappedFile::Mode::ReadWrite).Succeeded(), "Memory mapping a file failed"))
      return;

    NS_TEST_BOOL(memFile.GetWritePointer() != nullptr);
    NS_TEST_INT(memFile.GetFileSize(), uiFileSize * sizeof(nsUInt32));

    nsUInt32* ptr = static_cast<nsUInt32*>(memFile.GetWritePointer());

    for (nsUInt32 i = 0; i < uiFileSize; ++i)
    {
      NS_TEST_INT(ptr[i], i);
      ptr[i] = ptr[i] + 1;
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Memory map for reading")
  {
    nsMemoryMappedFile memFile;

    if (!NS_TEST_BOOL_MSG(memFile.Open(sOutputFile, nsMemoryMappedFile::Mode::ReadOnly).Succeeded(), "Memory mapping a file failed"))
      return;

    NS_TEST_BOOL(memFile.GetReadPointer() != nullptr);
    NS_TEST_INT(memFile.GetFileSize(), uiFileSize * sizeof(nsUInt32));

    const nsUInt32* ptr = static_cast<const nsUInt32*>(memFile.GetReadPointer());

    for (nsUInt32 i = 0; i < uiFileSize; ++i)
    {
      NS_TEST_INT(ptr[i], i + 1);
    }

    // try to map it a second time
    nsMemoryMappedFile memFile2;

    if (!NS_TEST_BOOL_MSG(memFile2.Open(sOutputFile, nsMemoryMappedFile::Mode::ReadOnly).Succeeded(), "Memory mapping a file twice failed"))
      return;
  }

  nsOSFile::DeleteFile(sOutputFile).IgnoreResult();
}
#endif
