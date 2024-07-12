#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Utilities/Progress.h>

NS_CREATE_SIMPLE_TEST(Utility, Progress)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Simple progress")
  {
    nsProgress progress;
    {
      nsProgressRange progressRange = nsProgressRange("TestProgress", 4, false, &progress);

      NS_TEST_FLOAT(progress.GetCompletion(), 0.0f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(progress.GetMainDisplayText() == "TestProgress");

      progressRange.BeginNextStep("Step1");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.0f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(progress.GetStepDisplayText() == "Step1");

      progressRange.BeginNextStep("Step2");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.25f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(progress.GetStepDisplayText() == "Step2");

      progressRange.BeginNextStep("Step3");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.5f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(progress.GetStepDisplayText() == "Step3");

      progressRange.BeginNextStep("Step4");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.75f, nsMath::DefaultEpsilon<float>());
      NS_TEST_BOOL(progress.GetStepDisplayText() == "Step4");
    }

    NS_TEST_FLOAT(progress.GetCompletion(), 1.0f, nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Weighted progress")
  {
    nsProgress progress;
    {
      nsProgressRange progressRange = nsProgressRange("TestProgress", 4, false, &progress);
      progressRange.SetStepWeighting(2, 2.0f);

      NS_TEST_FLOAT(progress.GetCompletion(), 0.0f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step0+1", 2);
      NS_TEST_FLOAT(progress.GetCompletion(), 0.2f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step2"); // this step should have twice the weight as the other steps.
      NS_TEST_FLOAT(progress.GetCompletion(), 0.4f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step3");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.8f, nsMath::DefaultEpsilon<float>());
    }

    NS_TEST_FLOAT(progress.GetCompletion(), 1.0f, nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Nested progress")
  {
    nsProgress progress;
    {
      nsProgressRange progressRange = nsProgressRange("TestProgress", 4, false, &progress);
      progressRange.SetStepWeighting(2, 2.0f);

      NS_TEST_FLOAT(progress.GetCompletion(), 0.0f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step0");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.0f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step1");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.2f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step2");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.4f, nsMath::DefaultEpsilon<float>());

      {
        nsProgressRange nestedRange = nsProgressRange("Nested", 5, false, &progress);
        nestedRange.SetStepWeighting(1, 4.0f);

        NS_TEST_FLOAT(progress.GetCompletion(), 0.4f, nsMath::DefaultEpsilon<float>());

        nestedRange.BeginNextStep("NestedStep0");
        NS_TEST_FLOAT(progress.GetCompletion(), 0.4f, nsMath::DefaultEpsilon<float>());

        nestedRange.BeginNextStep("NestedStep1");
        NS_TEST_FLOAT(progress.GetCompletion(), 0.45f, nsMath::DefaultEpsilon<float>());

        nestedRange.BeginNextStep("NestedStep2");
        NS_TEST_FLOAT(progress.GetCompletion(), 0.65f, nsMath::DefaultEpsilon<float>());

        nestedRange.BeginNextStep("NestedStep3");
        NS_TEST_FLOAT(progress.GetCompletion(), 0.7f, nsMath::DefaultEpsilon<float>());

        nestedRange.BeginNextStep("NestedStep4");
        NS_TEST_FLOAT(progress.GetCompletion(), 0.75f, nsMath::DefaultEpsilon<float>());
      }
      NS_TEST_FLOAT(progress.GetCompletion(), 0.8f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step3");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.8f, nsMath::DefaultEpsilon<float>());
    }

    NS_TEST_FLOAT(progress.GetCompletion(), 1.0f, nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Nested progress with manual completion")
  {
    nsProgress progress;
    {
      nsProgressRange progressRange = nsProgressRange("TestProgress", 3, false, &progress);
      progressRange.SetStepWeighting(1, 2.0f);

      NS_TEST_FLOAT(progress.GetCompletion(), 0.0f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step0");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.0f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step1");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.25f, nsMath::DefaultEpsilon<float>());

      {
        nsProgressRange nestedRange = nsProgressRange("Nested", false, &progress);

        NS_TEST_FLOAT(progress.GetCompletion(), 0.25f, nsMath::DefaultEpsilon<float>());

        nestedRange.SetCompletion(0.5);
        NS_TEST_FLOAT(progress.GetCompletion(), 0.5f, nsMath::DefaultEpsilon<float>());
      }
      NS_TEST_FLOAT(progress.GetCompletion(), 0.75f, nsMath::DefaultEpsilon<float>());

      progressRange.BeginNextStep("Step2");
      NS_TEST_FLOAT(progress.GetCompletion(), 0.75f, nsMath::DefaultEpsilon<float>());
    }

    NS_TEST_FLOAT(progress.GetCompletion(), 1.0f, nsMath::DefaultEpsilon<float>());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Progress Events")
  {
    nsUInt32 uiNumProgressUpdatedEvents = 0;

    nsProgress progress;
    progress.m_Events.AddEventHandler([&](const nsProgressEvent& e)
      {
      if (e.m_Type == nsProgressEvent::Type::ProgressChanged)
      {
        ++uiNumProgressUpdatedEvents;
        NS_TEST_FLOAT(e.m_pProgressbar->GetCompletion(), uiNumProgressUpdatedEvents * 0.25f, nsMath::DefaultEpsilon<float>());
      } });

    {
      nsProgressRange progressRange = nsProgressRange("TestProgress", 4, false, &progress);

      progressRange.BeginNextStep("Step1");
      progressRange.BeginNextStep("Step2");
      progressRange.BeginNextStep("Step3");
      progressRange.BeginNextStep("Step4");
    }

    NS_TEST_FLOAT(progress.GetCompletion(), 1.0f, nsMath::DefaultEpsilon<float>());
    NS_TEST_INT(uiNumProgressUpdatedEvents, 4);
  }
}
