#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/EndianHelper.h>

void nsEndianHelper::SwitchStruct(void* pDataPointer, const char* szFormat)
{
  NS_ASSERT_DEBUG(pDataPointer != nullptr, "Data necessary!");
  NS_ASSERT_DEBUG((szFormat != nullptr) && (szFormat[0] != '\0'), "Struct format description necessary!");

  nsUInt8* pWorkPointer = static_cast<nsUInt8*>(pDataPointer);
  char cCurrentElement = *szFormat;

  while (cCurrentElement != '\0')
  {
    switch (cCurrentElement)
    {
      case 'c':
      case 'b':
        pWorkPointer++;
        break;

      case 's':
      case 'w':
      {
        nsUInt16* pWordElement = reinterpret_cast<nsUInt16*>(pWorkPointer);
        *pWordElement = Switch(*pWordElement);
        pWorkPointer += sizeof(nsUInt16);
      }
      break;

      case 'd':
      {
        nsUInt32* pDWordElement = reinterpret_cast<nsUInt32*>(pWorkPointer);
        *pDWordElement = Switch(*pDWordElement);
        pWorkPointer += sizeof(nsUInt32);
      }
      break;

      case 'q':
      {
        nsUInt64* pQWordElement = reinterpret_cast<nsUInt64*>(pWorkPointer);
        *pQWordElement = Switch(*pQWordElement);
        pWorkPointer += sizeof(nsUInt64);
      }
      break;
    }

    szFormat++;
    cCurrentElement = *szFormat;
  }
}

void nsEndianHelper::SwitchStructs(void* pDataPointer, const char* szFormat, nsUInt32 uiStride, nsUInt32 uiCount)
{
  NS_ASSERT_DEBUG(pDataPointer != nullptr, "Data necessary!");
  NS_ASSERT_DEBUG((szFormat != nullptr) && (szFormat[0] != '\0'), "Struct format description necessary!");
  NS_ASSERT_DEBUG(uiStride > 0, "Struct size necessary!");

  for (nsUInt32 i = 0; i < uiCount; i++)
  {
    SwitchStruct(pDataPointer, szFormat);
    pDataPointer = nsMemoryUtils::AddByteOffset(pDataPointer, uiStride);
  }
}
