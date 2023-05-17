#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/EndianHelper.h>

void wdEndianHelper::SwitchStruct(void* pDataPointer, const char* szFormat)
{
  WD_ASSERT_DEBUG(pDataPointer != nullptr, "Data necessary!");
  WD_ASSERT_DEBUG(szFormat != nullptr && strlen(szFormat) > 0, "Struct format description necessary!");

  wdUInt8* pWorkPointer = static_cast<wdUInt8*>(pDataPointer);
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
        wdUInt16* pWordElement = reinterpret_cast<wdUInt16*>(pWorkPointer);
        *pWordElement = Switch(*pWordElement);
        pWorkPointer += sizeof(wdUInt16);
      }
      break;

      case 'd':
      {
        wdUInt32* pDWordElement = reinterpret_cast<wdUInt32*>(pWorkPointer);
        *pDWordElement = Switch(*pDWordElement);
        pWorkPointer += sizeof(wdUInt32);
      }
      break;

      case 'q':
      {
        wdUInt64* pQWordElement = reinterpret_cast<wdUInt64*>(pWorkPointer);
        *pQWordElement = Switch(*pQWordElement);
        pWorkPointer += sizeof(wdUInt64);
      }
      break;
    }

    szFormat++;
    cCurrentElement = *szFormat;
  }
}

void wdEndianHelper::SwitchStructs(void* pDataPointer, const char* szFormat, wdUInt32 uiStride, wdUInt32 uiCount)
{
  WD_ASSERT_DEBUG(pDataPointer != nullptr, "Data necessary!");
  WD_ASSERT_DEBUG(szFormat != nullptr && strlen(szFormat) > 0, "Struct format description necessary!");
  WD_ASSERT_DEBUG(uiStride > 0, "Struct size necessary!");

  for (wdUInt32 i = 0; i < uiCount; i++)
  {
    SwitchStruct(pDataPointer, szFormat);
    pDataPointer = wdMemoryUtils::AddByteOffset(pDataPointer, uiStride);
  }
}

WD_STATICLINK_FILE(Foundation, Foundation_Memory_Implementation_EndianHelper);
