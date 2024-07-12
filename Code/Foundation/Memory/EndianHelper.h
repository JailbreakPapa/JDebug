#pragma once

#include <Foundation/Basics.h>

/// \brief Collection of helper methods when working with endianess "problems"
struct NS_FOUNDATION_DLL nsEndianHelper
{

  /// \brief Returns true if called on a big endian system, false otherwise.
  ///
  /// \note Note that usually the compile time decisions with the defines NS_PLATFORM_LITTLE_ENDIAN, NS_PLATFORM_BIG_ENDIAN is preferred.
  static inline bool IsBigEndian()
  {
    const int i = 1;
    return (*(char*)&i) == 0;
  }

  /// \brief Returns true if called on a little endian system, false otherwise.
  ///
  /// \note Note that usually the compile time decisions with the defines NS_PLATFORM_LITTLE_ENDIAN, NS_PLATFORM_BIG_ENDIAN is preferred.
  static inline bool IsLittleEndian() { return !IsBigEndian(); }

  /// \brief Switches endianess of the given array of words (16 bit values).
  static inline void SwitchWords(nsUInt16* pWords, nsUInt32 uiCount) // [tested]
  {
    for (nsUInt32 i = 0; i < uiCount; i++)
      pWords[i] = Switch(pWords[i]);
  }

  /// \brief Switches endianess of the given array of double words (32 bit values).
  static inline void SwitchDWords(nsUInt32* pDWords, nsUInt32 uiCount) // [tested]
  {
    for (nsUInt32 i = 0; i < uiCount; i++)
      pDWords[i] = Switch(pDWords[i]);
  }

  /// \brief Switches endianess of the given array of quad words (64 bit values).
  static inline void SwitchQWords(nsUInt64* pQWords, nsUInt32 uiCount) // [tested]
  {
    for (nsUInt32 i = 0; i < uiCount; i++)
      pQWords[i] = Switch(pQWords[i]);
  }

  /// \brief Returns a single switched word (16 bit value).
  static NS_ALWAYS_INLINE nsUInt16 Switch(nsUInt16 uiWord) // [tested]
  {
    return (((uiWord & 0xFF) << 8) | ((uiWord >> 8) & 0xFF));
  }

  /// \brief Returns a single switched double word (32 bit value).
  static NS_ALWAYS_INLINE nsUInt32 Switch(nsUInt32 uiDWord) // [tested]
  {
    return (((uiDWord & 0xFF) << 24) | (((uiDWord >> 8) & 0xFF) << 16) | (((uiDWord >> 16) & 0xFF) << 8) | ((uiDWord >> 24) & 0xFF));
  }

  /// \brief Returns a single switched quad word (64 bit value).
  static NS_ALWAYS_INLINE nsUInt64 Switch(nsUInt64 uiQWord) // [tested]
  {
    return (((uiQWord & 0xFF) << 56) | ((uiQWord & 0xFF00) << 40) | ((uiQWord & 0xFF0000) << 24) | ((uiQWord & 0xFF000000) << 8) |
            ((uiQWord & 0xFF00000000) >> 8) | ((uiQWord & 0xFF0000000000) >> 24) | ((uiQWord & 0xFF000000000000) >> 40) |
            ((uiQWord & 0xFF00000000000000) >> 56));
  }

  /// \brief Switches a value in place (template accepts pointers for 2, 4 & 8 byte data types)
  template <typename T>
  static void SwitchInPlace(T* pValue) // [tested]
  {
    NS_CHECK_AT_COMPILETIME_MSG(
      (sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8), "Switch in place only works for type equivalents of nsUInt16, nsUInt32, nsUInt64!");

    if (sizeof(T) == 2)
    {
      struct TAnd16BitUnion
      {
        union
        {
          nsUInt16 BitValue;
          T TValue;
        };
      };

      TAnd16BitUnion Temp;
      Temp.TValue = *pValue;
      Temp.BitValue = Switch(Temp.BitValue);

      *pValue = Temp.TValue;
    }
    else if (sizeof(T) == 4)
    {
      struct TAnd32BitUnion
      {
        union
        {
          nsUInt32 BitValue;
          T TValue;
        };
      };

      TAnd32BitUnion Temp;
      Temp.TValue = *pValue;
      Temp.BitValue = Switch(Temp.BitValue);

      *pValue = Temp.TValue;
    }
    else if (sizeof(T) == 8)
    {
      struct TAnd64BitUnion
      {
        union
        {
          nsUInt64 BitValue;
          T TValue;
        };
      };

      TAnd64BitUnion Temp;
      Temp.TValue = *pValue;
      Temp.BitValue = Switch(Temp.BitValue);

      *pValue = Temp.TValue;
    }
  }

#if NS_ENABLED(NS_PLATFORM_LITTLE_ENDIAN)

  static NS_ALWAYS_INLINE void LittleEndianToNative(nsUInt16* /*pWords*/, nsUInt32 /*uiCount*/)
  {
  }

  static NS_ALWAYS_INLINE void NativeToLittleEndian(nsUInt16* /*pWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void LittleEndianToNative(nsUInt32* /*pDWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void NativeToLittleEndian(nsUInt32* /*pDWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void LittleEndianToNative(nsUInt64* /*pQWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void NativeToLittleEndian(nsUInt64* /*pQWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void BigEndianToNative(nsUInt16* pWords, nsUInt32 uiCount) { SwitchWords(pWords, uiCount); }

  static NS_ALWAYS_INLINE void NativeToBigEndian(nsUInt16* pWords, nsUInt32 uiCount) { SwitchWords(pWords, uiCount); }

  static NS_ALWAYS_INLINE void BigEndianToNative(nsUInt32* pDWords, nsUInt32 uiCount) { SwitchDWords(pDWords, uiCount); }

  static NS_ALWAYS_INLINE void NativeToBigEndian(nsUInt32* pDWords, nsUInt32 uiCount) { SwitchDWords(pDWords, uiCount); }

  static NS_ALWAYS_INLINE void BigEndianToNative(nsUInt64* pQWords, nsUInt32 uiCount) { SwitchQWords(pQWords, uiCount); }

  static NS_ALWAYS_INLINE void NativeToBigEndian(nsUInt64* pQWords, nsUInt32 uiCount) { SwitchQWords(pQWords, uiCount); }

#elif NS_ENABLED(NS_PLATFORM_BIG_ENDIAN)

  static NS_ALWAYS_INLINE void LittleEndianToNative(nsUInt16* pWords, nsUInt32 uiCount)
  {
    SwitchWords(pWords, uiCount);
  }

  static NS_ALWAYS_INLINE void NativeToLittleEndian(nsUInt16* pWords, nsUInt32 uiCount) { SwitchWords(pWords, uiCount); }

  static NS_ALWAYS_INLINE void LittleEndianToNative(nsUInt32* pDWords, nsUInt32 uiCount) { SwitchDWords(pDWords, uiCount); }

  static NS_ALWAYS_INLINE void NativeToLittleEndian(nsUInt32* pDWords, nsUInt32 uiCount) { SwitchDWords(pDWords, uiCount); }

  static NS_ALWAYS_INLINE void LittleEndianToNative(nsUInt64* pQWords, nsUInt32 uiCount) { SwitchQWords(pQWords, uiCount); }

  static NS_ALWAYS_INLINE void NativeToLittleEndian(nsUInt64* pQWords, nsUInt32 uiCount) { SwitchQWords(pQWords, uiCount); }

  static NS_ALWAYS_INLINE void BigEndianToNative(nsUInt16* /*pWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void NativeToBigEndian(nsUInt16* /*pWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void BigEndianToNative(nsUInt32* /*pWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void NativeToBigEndian(nsUInt32* /*pWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void BigEndianToNative(nsUInt64* /*pWords*/, nsUInt32 /*uiCount*/) {}

  static NS_ALWAYS_INLINE void NativeToBigEndian(nsUInt64* /*pWords*/, nsUInt32 /*uiCount*/) {}

#endif


  /// \brief Switches a given struct according to the layout described in the szFormat parameter
  ///
  /// The format string may contain the characters:
  ///  - c, b for a member of 1 byte
  ///  - w, s for a member of 2 bytes (word, nsUInt16)
  ///  - d for a member of 4 bytes (DWORD, nsUInt32)
  ///  - q for a member of 8 bytes (DWORD, nsUInt64)
  static void SwitchStruct(void* pDataPointer, const char* szFormat);

  /// \brief Templated helper method for SwitchStruct
  template <typename T>
  static void SwitchStruct(T* pDataPointer, const char* szFormat) // [tested]
  {
    SwitchStruct(static_cast<void*>(pDataPointer), szFormat);
  }

  /// \brief Switches a given set of struct according to the layout described in the szFormat parameter
  ///
  /// The format string may contain the characters:
  ///  - c, b for a member of 1 byte
  ///  - w, s for a member of 2 bytes (word, nsUInt16)
  ///  - d for a member of 4 bytes (DWORD, nsUInt32)
  ///  - q for a member of 8 bytes (DWORD, nsUInt64)
  static void SwitchStructs(void* pDataPointer, const char* szFormat, nsUInt32 uiStride, nsUInt32 uiCount); // [tested]

  /// \brief Templated helper method for SwitchStructs
  template <typename T>
  static void SwitchStructs(T* pDataPointer, const char* szFormat, nsUInt32 uiCount) // [tested]
  {
    SwitchStructs(static_cast<void*>(pDataPointer), szFormat, sizeof(T), uiCount);
  }
};
