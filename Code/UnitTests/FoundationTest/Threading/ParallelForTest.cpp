#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Threading/TaskSystem.h>

namespace
{
  static constexpr nsUInt32 s_uiNumberOfWorkers = 4;
  static constexpr nsUInt32 s_uiTaskItemSliceSize = 25;
  static constexpr nsUInt32 s_uiTotalNumberOfTaskItems = s_uiNumberOfWorkers * s_uiTaskItemSliceSize;
} // namespace

NS_CREATE_SIMPLE_TEST(Threading, ParallelFor)
{
  // set up controlled task system environment
  nsTaskSystem::SetWorkerThreadCount(::s_uiNumberOfWorkers, ::s_uiNumberOfWorkers);

  // shared variables
  nsMutex dataAccessMutex;

  nsUInt32 uiRangesEncounteredCheck = 0;
  nsUInt32 uiNumbersSum = 0;

  nsUInt32 uiNumbersCheckSum = 0;
  nsStaticArray<nsUInt32, ::s_uiTotalNumberOfTaskItems> numbers;

  nsParallelForParams parallelForParams;
  parallelForParams.m_uiBinSize = ::s_uiTaskItemSliceSize;
  parallelForParams.m_uiMaxTasksPerThread = 1;

  auto ResetSharedVariables = [&uiRangesEncounteredCheck, &uiNumbersSum, &uiNumbersCheckSum, &numbers]()
  {
    uiRangesEncounteredCheck = 0;
    uiNumbersSum = 0;

    uiNumbersCheckSum = 0;

    numbers.EnsureCount(::s_uiTotalNumberOfTaskItems);
    for (nsUInt32 i = 0; i < ::s_uiTotalNumberOfTaskItems; ++i)
    {
      numbers[i] = i + 1;
      uiNumbersCheckSum += numbers[i];
    }
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Parallel For (Indexed)")
  {
    // reset
    ResetSharedVariables();

    // test
    // sum up the slice of number assigned to us via index ranges and
    // check if the ranges described by them are as expected
    nsTaskSystem::ParallelForIndexed(
      0, ::s_uiTotalNumberOfTaskItems,
      [&dataAccessMutex, &uiRangesEncounteredCheck, &uiNumbersSum, &numbers](nsUInt32 uiStartIndex, nsUInt32 uiEndIndex)
      {
        NS_LOCK(dataAccessMutex);

        // size check
        NS_TEST_INT(uiEndIndex - uiStartIndex, ::s_uiTaskItemSliceSize);

        // note down which range this is
        uiRangesEncounteredCheck |= 1 << (uiStartIndex / ::s_uiTaskItemSliceSize);

        // sum up numbers in our slice
        for (nsUInt32 uiIndex = uiStartIndex; uiIndex < uiEndIndex; ++uiIndex)
        {
          uiNumbersSum += numbers[uiIndex];
        }
      },
      "ParallelForIndexed Test", nsTaskNesting::Never, parallelForParams);

    // check results
    NS_TEST_INT(uiRangesEncounteredCheck, 0b1111);
    NS_TEST_INT(uiNumbersSum, uiNumbersCheckSum);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Parallel For (Array)")
  {
    // reset
    ResetSharedVariables();

    // test-specific data
    nsStaticArray<nsUInt32*, ::s_uiNumberOfWorkers> startAddresses;
    for (nsUInt32 i = 0; i < ::s_uiNumberOfWorkers; ++i)
    {
      startAddresses.PushBack(numbers.GetArrayPtr().GetPtr() + (i * ::s_uiTaskItemSliceSize));
    }

    // test
    // sum up the slice of numbers assigned to us via array pointers and
    // check if the ranges described by them are as expected
    nsTaskSystem::ParallelFor<nsUInt32>(
      numbers.GetArrayPtr(),
      [&dataAccessMutex, &uiRangesEncounteredCheck, &uiNumbersSum, &startAddresses](nsArrayPtr<nsUInt32> taskItemSlice)
      {
        NS_LOCK(dataAccessMutex);

        // size check
        NS_TEST_INT(taskItemSlice.GetCount(), ::s_uiTaskItemSliceSize);

        // note down which range this is
        for (nsUInt32 index = 0; index < startAddresses.GetCount(); ++index)
        {
          if (startAddresses[index] == taskItemSlice.GetPtr())
          {
            uiRangesEncounteredCheck |= 1 << index;
          }
        }

        // sum up numbers in our slice
        for (const nsUInt32& number : taskItemSlice)
        {
          uiNumbersSum += number;
        }
      },
      "ParallelFor Array Test", parallelForParams);

    // check results
    NS_TEST_INT(15, 0b1111);
    NS_TEST_INT(uiNumbersSum, uiNumbersCheckSum);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Parallel For (Array, Single)")
  {
    // reset
    ResetSharedVariables();

    // test
    // sum up the slice of numbers by summing up the individual numbers that get handed to us
    nsTaskSystem::ParallelForSingle(
      numbers.GetArrayPtr(),
      [&dataAccessMutex, &uiNumbersSum](nsUInt32 uiNumber)
      {
        NS_LOCK(dataAccessMutex);
        uiNumbersSum += uiNumber;
      },
      "ParallelFor Array Single Test", parallelForParams);

    // check the resulting sum
    NS_TEST_INT(uiNumbersSum, uiNumbersCheckSum);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Parallel For (Array, Single, Index)")
  {
    // reset
    ResetSharedVariables();

    // test
    // sum up the slice of numbers that got assigned to us via an index range
    nsTaskSystem::ParallelForSingleIndex(
      numbers.GetArrayPtr(),
      [&dataAccessMutex, &uiNumbersSum](nsUInt32 uiIndex, nsUInt32 uiNumber)
      {
        NS_LOCK(dataAccessMutex);
        uiNumbersSum += uiNumber + (uiIndex + 1);
      },
      "ParallelFor Array Single Index Test", parallelForParams);

    // check the resulting sum
    NS_TEST_INT(uiNumbersSum, 2 * uiNumbersCheckSum);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Parallel For (Array, Single) Write")
  {
    // reset
    ResetSharedVariables();

    // test
    // modify the original array of numbers
    nsTaskSystem::ParallelForSingle(
      numbers.GetArrayPtr(),
      [&dataAccessMutex](nsUInt32& ref_uiNumber)
      {
        NS_LOCK(dataAccessMutex);
        ref_uiNumber = ref_uiNumber * 3;
      },
      "ParallelFor Array Single Write Test (Write)", parallelForParams);

    // sum up the new values to test if writing worked
    nsTaskSystem::ParallelForSingle(
      numbers.GetArrayPtr(),
      [&dataAccessMutex, &uiNumbersSum](const nsUInt32& uiNumber)
      {
        NS_LOCK(dataAccessMutex);
        uiNumbersSum += uiNumber;
      },
      "ParallelFor Array Single Write Test (Sum)", parallelForParams);

    // check the resulting sum
    NS_TEST_INT(uiNumbersSum, 3 * uiNumbersCheckSum);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Parallel For (Array, Single, Index) Write")
  {
    // reset
    ResetSharedVariables();

    // test
    // modify the original array of numbers
    nsTaskSystem::ParallelForSingleIndex(
      numbers.GetArrayPtr(),
      [&dataAccessMutex](nsUInt32, nsUInt32& ref_uiNumber)
      {
        NS_LOCK(dataAccessMutex);
        ref_uiNumber = ref_uiNumber * 4;
      },
      "ParallelFor Array Single Write Test (Write)", parallelForParams);

    // sum up the new values to test if writing worked
    nsTaskSystem::ParallelForSingle(
      numbers.GetArrayPtr(),
      [&dataAccessMutex, &uiNumbersSum](const nsUInt32& uiNumber)
      {
        NS_LOCK(dataAccessMutex);
        uiNumbersSum += uiNumber;
      },
      "ParallelFor Array Single Write Test (Sum)", parallelForParams);

    // check the resulting sum
    NS_TEST_INT(uiNumbersSum, 4 * uiNumbersCheckSum);
  }
}
