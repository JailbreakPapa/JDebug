
#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/DataProcessing/Stream/DefaultImplementations/ZeroInitializer.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamGroup.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamIterator.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamProcessor.h>
#include <Foundation/Reflection/Reflection.h>

NS_CREATE_SIMPLE_TEST_GROUP(DataProcessing);

// Add processor

class AddOneStreamProcessor : public nsProcessingStreamProcessor
{
  NS_ADD_DYNAMIC_REFLECTION(AddOneStreamProcessor, nsProcessingStreamProcessor);

public:
  AddOneStreamProcessor()

    = default;

  void SetStreamName(nsHashedString sStreamName) { m_sStreamName = sStreamName; }

protected:
  virtual nsResult UpdateStreamBindings() override
  {
    m_pStream = m_pStreamGroup->GetStreamByName(m_sStreamName);

    return m_pStream ? NS_SUCCESS : NS_FAILURE;
  }

  virtual void InitializeElements(nsUInt64 uiStartIndex, nsUInt64 uiNumElements) override {}

  virtual void Process(nsUInt64 uiNumElements) override
  {
    nsProcessingStreamIterator<float> streamIterator(m_pStream, uiNumElements, 0);

    while (!streamIterator.HasReachedEnd())
    {
      streamIterator.Current() += 1.0f;

      streamIterator.Advance();
    }
  }

  nsHashedString m_sStreamName;
  nsProcessingStream* m_pStream = nullptr;
};

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(AddOneStreamProcessor, 1, nsRTTIDefaultAllocator<AddOneStreamProcessor>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_CREATE_SIMPLE_TEST(DataProcessing, ProcessingStream)
{
  nsProcessingStreamGroup Group;
  nsProcessingStream* pStream1 = Group.AddStream("Stream1", nsProcessingStream::DataType::Float);
  nsProcessingStream* pStream2 = Group.AddStream("Stream2", nsProcessingStream::DataType::Float3);

  NS_TEST_BOOL(pStream1 != nullptr);
  NS_TEST_BOOL(pStream2 != nullptr);

  nsProcessingStreamSpawnerZeroInitialized* pSpawner1 = NS_DEFAULT_NEW(nsProcessingStreamSpawnerZeroInitialized);
  nsProcessingStreamSpawnerZeroInitialized* pSpawner2 = NS_DEFAULT_NEW(nsProcessingStreamSpawnerZeroInitialized);

  pSpawner1->SetStreamName(pStream1->GetName());
  pSpawner2->SetStreamName(pStream2->GetName());

  Group.AddProcessor(pSpawner1);
  Group.AddProcessor(pSpawner2);

  Group.SetSize(128);

  NS_TEST_INT(Group.GetNumElements(), 128);
  NS_TEST_INT(Group.GetNumActiveElements(), 0);

  Group.InitializeElements(3);

  Group.Process();

  NS_TEST_INT(Group.GetNumActiveElements(), 3);


  {
    nsProcessingStreamIterator<float> stream1Iterator(pStream1, 3, 0);

    int iElementsVisited = 0;
    while (!stream1Iterator.HasReachedEnd())
    {
      NS_TEST_FLOAT(stream1Iterator.Current(), 0.0f, 0.0f);

      stream1Iterator.Advance();
      iElementsVisited++;
    }

    NS_TEST_INT(iElementsVisited, 3);
  }

  Group.InitializeElements(7);

  Group.Process();

  {
    nsProcessingStreamIterator<nsVec3> stream2Iterator(pStream2, Group.GetNumActiveElements(), 0);

    int iElementsVisited = 0;
    while (!stream2Iterator.HasReachedEnd())
    {
      NS_TEST_FLOAT(stream2Iterator.Current().x, 0.0f, 0.0f);
      NS_TEST_FLOAT(stream2Iterator.Current().y, 0.0f, 0.0f);
      NS_TEST_FLOAT(stream2Iterator.Current().z, 0.0f, 0.0f);

      stream2Iterator.Advance();
      iElementsVisited++;
    }

    NS_TEST_INT(iElementsVisited, 10);
  }

  NS_TEST_INT(Group.GetHighestNumActiveElements(), 10);

  Group.RemoveElement(5);
  Group.RemoveElement(7);

  Group.Process();

  NS_TEST_INT(Group.GetHighestNumActiveElements(), 10);
  NS_TEST_INT(Group.GetNumActiveElements(), 8);

  AddOneStreamProcessor* pProcessor1 = NS_DEFAULT_NEW(AddOneStreamProcessor);
  pProcessor1->SetStreamName(pStream1->GetName());

  Group.AddProcessor(pProcessor1);

  Group.Process();

  {
    nsProcessingStreamIterator<float> stream1Iterator(pStream1, Group.GetNumActiveElements(), 0);
    while (!stream1Iterator.HasReachedEnd())
    {
      NS_TEST_FLOAT(stream1Iterator.Current(), 1.0f, 0.001f);

      stream1Iterator.Advance();
    }
  }

  Group.Process();

  {
    nsProcessingStreamIterator<float> stream1Iterator(pStream1, Group.GetNumActiveElements(), 0);
    while (!stream1Iterator.HasReachedEnd())
    {
      NS_TEST_FLOAT(stream1Iterator.Current(), 2.0f, 0.001f);

      stream1Iterator.Advance();
    }
  }
}
