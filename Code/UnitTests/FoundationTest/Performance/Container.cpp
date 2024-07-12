#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>

#include <vector>

namespace
{
  enum constants
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    NUM_SAMPLES = 128,
    NUM_APPENDS = 1024 * 32,
    NUM_RECUSRIVE_APPENDS = 128
#else
    NUM_SAMPLES = 1024,
    NUM_APPENDS = 1024 * 64,
    NUM_RECUSRIVE_APPENDS = 256
#endif
  };

  struct SomeBigObject
  {
    NS_DECLARE_MEM_RELOCATABLE_TYPE();

    static nsUInt32 constructionCount;
    static nsUInt32 destructionCount;
    nsUInt64 i1, i2, i3, i4, i5, i6, i7, i8;

    SomeBigObject(nsUInt64 uiInit)
      : i1(uiInit)
      , i2(uiInit)
      , i3(uiInit)
      , i4(uiInit)
      , i5(uiInit)
      , i6(uiInit)
      , i7(uiInit)
      , i8(uiInit)
    {
      constructionCount++;
    }

    ~SomeBigObject() { destructionCount++; }

    SomeBigObject(const SomeBigObject& rh)
    {
      constructionCount++;
      this->i1 = rh.i1;
      this->i2 = rh.i2;
      this->i3 = rh.i3;
      this->i4 = rh.i4;
      this->i5 = rh.i5;
      this->i6 = rh.i6;
      this->i7 = rh.i7;
      this->i8 = rh.i8;
    }

    void operator=(const SomeBigObject& rh)
    {
      constructionCount++;
      this->i1 = rh.i1;
      this->i2 = rh.i2;
      this->i3 = rh.i3;
      this->i4 = rh.i4;
      this->i5 = rh.i5;
      this->i6 = rh.i6;
      this->i7 = rh.i7;
      this->i8 = rh.i8;
    }
  };

  nsUInt32 SomeBigObject::constructionCount = 0;
  nsUInt32 SomeBigObject::destructionCount = 0;
} // namespace

// Enable when needed
#define NS_PERFORMANCE_TESTS_STATE nsTestBlock::DisabledNoWarning

NS_CREATE_SIMPLE_TEST(Performance, Container)
{
  const char* TestString = "There are 10 types of people in the world. Those who understand binary and those who don't.";
  const nsUInt32 TestStringLength = (nsUInt32)strlen(TestString);

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "POD Dynamic Array Appending")
  {
    nsTime t0 = nsTime::Now();
    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      nsDynamicArray<int> a;
      for (nsUInt32 i = 0; i < NUM_APPENDS; i++)
      {
        a.PushBack(i);
      }

      for (nsUInt32 i = 0; i < NUM_APPENDS; i++)
      {
        sum += a[i];
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info("[test]POD Dynamic Array Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "POD std::vector Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      std::vector<int> a;
      for (nsUInt32 i = 0; i < NUM_APPENDS; i++)
      {
        a.push_back(i);
      }

      for (nsUInt32 i = 0; i < NUM_APPENDS; i++)
      {
        sum += a[i];
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info("[test]POD std::vector Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "nsDynamicArray<nsDynamicArray<char>> Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      nsDynamicArray<nsDynamicArray<char>> a;
      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        nsUInt32 count = a.GetCount();
        a.SetCount(count + 1);
        nsDynamicArray<char>& cur = a[count];
        for (nsUInt32 j = 0; j < TestStringLength; j++)
        {
          cur.PushBack(TestString[j]);
        }
      }

      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        sum += a[i].GetCount();
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info(
      "[test]nsDynamicArray<nsDynamicArray<char>> Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "nsDynamicArray<nsHybridArray<char, 64>> Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      nsDynamicArray<nsHybridArray<char, 64>> a;
      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        nsUInt32 count = a.GetCount();
        a.SetCount(count + 1);
        nsHybridArray<char, 64>& cur = a[count];
        for (nsUInt32 j = 0; j < TestStringLength; j++)
        {
          cur.PushBack(TestString[j]);
        }
      }

      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        sum += a[i].GetCount();
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info("[test]nsDynamicArray<nsHybridArray<char, 64>> Appending {0}ms",
      nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "std::vector<std::vector<char>> Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      std::vector<std::vector<char>> a;
      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        nsUInt32 count = (nsUInt32)a.size();
        a.resize(count + 1);
        std::vector<char>& cur = a[count];
        for (nsUInt32 j = 0; j < TestStringLength; j++)
        {
          cur.push_back(TestString[j]);
        }
      }

      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        sum += (nsUInt32)a[i].size();
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info(
      "[test]std::vector<std::vector<char>> Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "nsDynamicArray<nsString> Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      nsDynamicArray<nsString> a;
      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        nsUInt32 count = a.GetCount();
        a.SetCount(count + 1);
        nsString& cur = a[count];
        nsStringBuilder b;
        for (nsUInt32 j = 0; j < TestStringLength; j++)
        {
          b.Append(TestString[i]);
        }
        cur = std::move(b);
      }

      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        sum += a[i].GetElementCount();
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info("[test]nsDynamicArray<nsString> Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "std::vector<std::string> Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      std::vector<std::string> a;
      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        std::string cur;
        for (nsUInt32 j = 0; j < TestStringLength; j++)
        {
          cur += TestString[i];
        }
        a.push_back(std::move(cur));
      }

      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        sum += (nsUInt32)a[i].length();
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info("[test]std::vector<std::string> Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "nsDynamicArray<SomeBigObject> Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      nsDynamicArray<SomeBigObject> a;
      for (nsUInt32 i = 0; i < NUM_APPENDS; i++)
      {
        a.PushBack(SomeBigObject(i));
      }

      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        sum += (nsUInt32)a[i].i1;
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info(
      "[test]nsDynamicArray<SomeBigObject> Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(NS_PERFORMANCE_TESTS_STATE, "std::vector<SomeBigObject> Appending")
  {
    nsTime t0 = nsTime::Now();

    nsUInt32 sum = 0;
    for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
    {
      std::vector<SomeBigObject> a;
      for (nsUInt32 i = 0; i < NUM_APPENDS; i++)
      {
        a.push_back(SomeBigObject(i));
      }

      for (nsUInt32 i = 0; i < NUM_RECUSRIVE_APPENDS; i++)
      {
        sum += (nsUInt32)a[i].i1;
      }
    }

    nsTime t1 = nsTime::Now();
    nsLog::Info("[test]std::vector<SomeBigObject> Appending {0}ms", nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
  }

  NS_TEST_BLOCK(nsTestBlock::DisabledNoWarning, "nsMap<void*, nsUInt32>")
  {
    nsUInt32 sum = 0;

    for (nsUInt32 size = 1024; size < 4096 * 32; size += 1024)
    {
      nsMap<void*, nsUInt32> map;

      for (nsUInt32 i = 0; i < size; i++)
      {
        map.Insert(malloc(64), 64);
      }

      void* ptrs[1024];

      nsTime t0 = nsTime::Now();
      for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
      {
        for (nsUInt32 i = 0; i < 1024; i++)
        {
          void* mem = malloc(64);
          map.Insert(mem, 64);
          map.Remove(mem);
          ptrs[i] = mem;
        }

        for (nsUInt32 i = 0; i < 1024; i++)
          free(ptrs[i]);

        for (auto it = map.GetIterator(); it.IsValid(); ++it)
        {
          sum += it.Value();
        }
      }
      nsTime t1 = nsTime::Now();

      for (auto it = map.GetIterator(); it.IsValid(); ++it)
      {
        free(it.Key());
      }

      nsLog::Info("[test]nsMap<void*, nsUInt32> size = {0} => {1}ms", size, nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::DisabledNoWarning, "nsHashTable<void*, nsUInt32>")
  {
    nsUInt32 sum = 0;



    for (nsUInt32 size = 1024; size < 4096 * 32; size += 1024)
    {
      nsHashTable<void*, nsUInt32> map;

      for (nsUInt32 i = 0; i < size; i++)
      {
        map.Insert(malloc(64), 64);
      }

      void* ptrs[1024];

      nsTime t0 = nsTime::Now();
      for (nsUInt32 n = 0; n < NUM_SAMPLES; n++)
      {

        for (nsUInt32 i = 0; i < 1024; i++)
        {
          void* mem = malloc(64);
          map.Insert(mem, 64);
          map.Remove(mem);
          ptrs[i] = mem;
        }

        for (nsUInt32 i = 0; i < 1024; i++)
          free(ptrs[i]);

        for (auto it = map.GetIterator(); it.IsValid(); it.Next())
        {
          sum += it.Value();
        }
      }
      nsTime t1 = nsTime::Now();

      for (auto it = map.GetIterator(); it.IsValid(); it.Next())
      {
        free(it.Key());
      }

      nsLog::Info("[test]nsHashTable<void*, nsUInt32> size = {0} => {1}ms", size,
        nsArgF((t1 - t0).GetMilliseconds() / static_cast<double>(NUM_SAMPLES), 4), sum);
    }
  }
}
