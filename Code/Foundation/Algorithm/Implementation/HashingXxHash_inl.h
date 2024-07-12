#pragma once

namespace nsInternal
{
  constexpr nsUInt32 PRIME32_1 = 0x9E3779B1U;
  constexpr nsUInt32 PRIME32_2 = 0x85EBCA77U;
  constexpr nsUInt32 PRIME32_3 = 0xC2B2AE3DU;
  constexpr nsUInt32 PRIME32_4 = 0x27D4EB2FU;
  constexpr nsUInt32 PRIME32_5 = 0x165667B1U;

  constexpr nsUInt64 PRIME64_1 = 0x9E3779B185EBCA87ULL;
  constexpr nsUInt64 PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
  constexpr nsUInt64 PRIME64_3 = 0x165667B19E3779F9ULL;
  constexpr nsUInt64 PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
  constexpr nsUInt64 PRIME64_5 = 0x27D4EB2F165667C5ULL;

  constexpr nsUInt32 nsRotLeft(nsUInt32 value, nsUInt32 uiAmount)
  {
    return (value << uiAmount) | (value >> (32 - uiAmount));
  }
  constexpr nsUInt64 nsRotLeft(nsUInt64 value, nsUInt64 uiAmount)
  {
    return (value << uiAmount) | (value >> (64 - uiAmount));
  }

  template <size_t N>
  constexpr nsUInt32 CompileTimeXxHash32(const char (&str)[N], nsUInt32 uiSeed)
  {
    // Note: N will contain the trailing 0 of a string literal. This needs to be ignored.
    constexpr nsUInt32 length = static_cast<nsUInt32>(N - 1);
    if constexpr (length == 0)
    {
      return 46947589u;
    }
    else
    {
      nsUInt32 acc = 0;
      nsUInt32 index = 0;
      // Perform simple initialization if N < 16
      if constexpr (length < 16)
      {
        acc = uiSeed + PRIME32_5;
      }
      else
      {
        nsUInt32 accs[4] = {uiSeed + PRIME32_1 + PRIME32_2, uiSeed + PRIME32_2, uiSeed, uiSeed - PRIME32_1};
        for (; length - index >= 16; index += 16)
        {
          for (int i = 0; i < 4; i++)
          {
            nsUInt32 laneN = (static_cast<nsUInt32>(str[index + i * 4 + 0]) << 0) | (static_cast<nsUInt32>(str[index + i * 4 + 1]) << 8) |
                             (static_cast<nsUInt32>(str[index + i * 4 + 2]) << 16) | (static_cast<nsUInt32>(str[index + i * 4 + 3]) << 24);
            accs[i] = accs[i] + (laneN * PRIME32_2);
            accs[i] = nsRotLeft(accs[i], 13);
            accs[i] = accs[i] * PRIME32_1;
          }
        }
        acc = nsRotLeft(accs[0], 1) + nsRotLeft(accs[1], 7) + nsRotLeft(accs[2], 12) + nsRotLeft(accs[3], 18);
      }

      // Step 4
      acc = acc + length;

      // Step 5
      for (; length - index >= 4; index += 4)
      {
        nsUInt32 lane = (static_cast<nsUInt32>(str[index + 0]) << 0) | (static_cast<nsUInt32>(str[index + 1]) << 8) |
                        (static_cast<nsUInt32>(str[index + 2]) << 16) | (static_cast<nsUInt32>(str[index + 3]) << 24);
        acc = acc + lane * PRIME32_3;
        acc = nsRotLeft(acc, 17) * PRIME32_4;
      }

      for (; length - index >= 1; index++)
      {
        nsUInt32 lane = static_cast<nsUInt32>(str[index]);
        acc = acc + lane * PRIME32_5;
        acc = nsRotLeft(acc, 11) * PRIME32_1;
      }

      // Step 6
      acc = acc ^ (acc >> 15);
      acc = acc * PRIME32_2;
      acc = acc ^ (acc >> 13);
      acc = acc * PRIME32_3;
      acc = acc ^ (acc >> 16);

      return acc;
    }
  }

  template <size_t N>
  constexpr nsUInt64 CompileTimeXxHash64(const char (&str)[N], nsUInt64 uiSeed)
  {
    // Note: N will contain the trailing 0 of a string literal. This needs to be ignored.
    constexpr nsUInt32 length = static_cast<nsUInt32>(N - 1);
    if constexpr (length == 0)
    {
      return 17241709254077376921llu;
    }
    else
    {
      nsUInt64 acc = 0;
      nsUInt32 index = 0;

      // Step 1
      if constexpr (length < 32)
      {
        // simple initialization
        acc = uiSeed + PRIME64_5;
      }
      else
      {
        nsUInt64 accs[] = {uiSeed + PRIME64_1 + PRIME64_2, uiSeed + PRIME64_2, uiSeed + 0, uiSeed - PRIME64_1};
        // Step 2
        for (; length - index >= 32; index += 32)
        {
          for (int i = 0; i < 4; i++)
          {
            nsUInt64 laneN = (static_cast<nsUInt64>(str[index + i * 8 + 0]) << 0) | (static_cast<nsUInt64>(str[index + i * 8 + 1]) << 8) |
                             (static_cast<nsUInt64>(str[index + i * 8 + 2]) << 16) | (static_cast<nsUInt64>(str[index + i * 8 + 3]) << 24) |
                             (static_cast<nsUInt64>(str[index + i * 8 + 4]) << 32) | (static_cast<nsUInt64>(str[index + i * 8 + 5]) << 40) |
                             (static_cast<nsUInt64>(str[index + i * 8 + 6]) << 48) | (static_cast<nsUInt64>(str[index + i * 8 + 7]) << 56);
            accs[i] = accs[i] + (laneN * PRIME64_2);
            accs[i] = nsRotLeft(accs[i], 31ULL);
            accs[i] = accs[i] * PRIME64_1;
          }
        }

        // Step 3
        acc = nsRotLeft(accs[0], 1ULL) + nsRotLeft(accs[1], 7ULL) + nsRotLeft(accs[2], 12ULL) + nsRotLeft(accs[3], 18ULL);
        for (int i = 0; i < 4; i++)
        {
          acc = (acc ^ (nsRotLeft(accs[i] * PRIME64_2, 31ULL) * PRIME64_1)) * PRIME64_1 + PRIME64_4;
        }
      }
      // Step 4
      acc += length;

      // Step 5
      for (; length - index >= 8; index += 8)
      {
        nsUInt64 lane = (static_cast<nsUInt64>(str[index + 0]) << 0) | (static_cast<nsUInt64>(str[index + 1]) << 8) |
                        (static_cast<nsUInt64>(str[index + 2]) << 16) | (static_cast<nsUInt64>(str[index + 3]) << 24) |
                        (static_cast<nsUInt64>(str[index + 4]) << 32) | (static_cast<nsUInt64>(str[index + 5]) << 40) |
                        (static_cast<nsUInt64>(str[index + 6]) << 48) | (static_cast<nsUInt64>(str[index + 7]) << 56);
        acc = acc ^ (nsRotLeft(lane * PRIME64_2, 31ULL) * PRIME64_1);
        acc = nsRotLeft(acc, 27ULL) * PRIME64_1;
        acc += PRIME64_4;
      }

      for (; length - index >= 4; index += 4)
      {
        nsUInt64 lane = (static_cast<nsUInt64>(str[index + 0]) << 0) | (static_cast<nsUInt64>(str[index + 1]) << 8) |
                        (static_cast<nsUInt64>(str[index + 2]) << 16) | (static_cast<nsUInt64>(str[index + 3]) << 24);
        acc = acc ^ (lane * PRIME64_1);
        acc = nsRotLeft(acc, 23ULL) * PRIME64_2;
        acc += PRIME64_3;
      }

      for (; length - index >= 1; index++)
      {
        nsUInt64 lane = static_cast<nsUInt64>(str[index]);
        acc = acc ^ (lane * PRIME64_5);
        acc = nsRotLeft(acc, 11ULL) * PRIME64_1;
      }

      // Step 6
      acc = acc ^ (acc >> 33);
      acc = acc * PRIME64_2;
      acc = acc ^ (acc >> 29);
      acc = acc * PRIME64_3;
      acc = acc ^ (acc >> 32);

      return acc;
    }
  }
} // namespace nsInternal

template <size_t N>
constexpr NS_ALWAYS_INLINE nsUInt32 nsHashingUtils::xxHash32String(const char (&str)[N], nsUInt32 uiSeed)
{
  return nsInternal::CompileTimeXxHash32(str, uiSeed);
}

template <size_t N>
constexpr NS_ALWAYS_INLINE nsUInt64 nsHashingUtils::xxHash64String(const char (&str)[N], nsUInt64 uiSeed)
{
  return nsInternal::CompileTimeXxHash64(str, uiSeed);
}

NS_ALWAYS_INLINE nsUInt32 nsHashingUtils::xxHash32String(nsStringView sStr, nsUInt32 uiSeed)
{
  return xxHash32(sStr.GetStartPointer(), sStr.GetElementCount(), uiSeed);
}

NS_ALWAYS_INLINE nsUInt64 nsHashingUtils::xxHash64String(nsStringView sStr, nsUInt64 uiSeed)
{
  return xxHash64(sStr.GetStartPointer(), sStr.GetElementCount(), uiSeed);
}
