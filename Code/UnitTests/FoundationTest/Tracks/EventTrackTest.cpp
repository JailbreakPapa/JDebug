#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Tracks/EventTrack.h>

NS_CREATE_SIMPLE_TEST_GROUP(Tracks);

NS_CREATE_SIMPLE_TEST(Tracks, EventTrack)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Empty")
  {
    nsEventTrack et;
    nsHybridArray<nsHashedString, 8> result;

    NS_TEST_BOOL(et.IsEmpty());
    et.Sample(nsTime::MakeZero(), nsTime::MakeFromSeconds(1.0), result);

    NS_TEST_BOOL(result.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sample")
  {
    nsEventTrack et;
    nsHybridArray<nsHashedString, 8> result;

    et.AddControlPoint(nsTime::MakeFromSeconds(3.0), "Event3");
    et.AddControlPoint(nsTime::MakeFromSeconds(0.0), "Event0");
    et.AddControlPoint(nsTime::MakeFromSeconds(4.0), "Event4");
    et.AddControlPoint(nsTime::MakeFromSeconds(1.0), "Event1");
    et.AddControlPoint(nsTime::MakeFromSeconds(2.0), "Event2");

    NS_TEST_BOOL(!et.IsEmpty());

    // sampling an empty range should yield no results, even if sampling an exact time where an event is
    result.Clear();
    {
      {
        et.Sample(nsTime::MakeFromSeconds(0.0), nsTime::MakeFromSeconds(0.0), result);
        NS_TEST_INT(result.GetCount(), 0);
      }

      {
        result.Clear();
        et.Sample(nsTime::MakeFromSeconds(1.0), nsTime::MakeFromSeconds(1.0), result);
        NS_TEST_INT(result.GetCount(), 0);
      }

      {
        result.Clear();
        et.Sample(nsTime::MakeFromSeconds(4.0), nsTime::MakeFromSeconds(4.0), result);
        NS_TEST_INT(result.GetCount(), 0);
      }
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(0.0), nsTime::MakeFromSeconds(1.0), result);
      NS_TEST_INT(result.GetCount(), 1);
      NS_TEST_STRING(result[0].GetString(), "Event0");
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(0.0), nsTime::MakeFromSeconds(2.0), result);
      NS_TEST_INT(result.GetCount(), 2);
      NS_TEST_STRING(result[0].GetString(), "Event0");
      NS_TEST_STRING(result[1].GetString(), "Event1");
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(0.0), nsTime::MakeFromSeconds(4.0), result);
      NS_TEST_INT(result.GetCount(), 4);
      NS_TEST_STRING(result[0].GetString(), "Event0");
      NS_TEST_STRING(result[1].GetString(), "Event1");
      NS_TEST_STRING(result[2].GetString(), "Event2");
      NS_TEST_STRING(result[3].GetString(), "Event3");
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(0.0), nsTime::MakeFromSeconds(10.0), result);
      NS_TEST_INT(result.GetCount(), 5);
      NS_TEST_STRING(result[0].GetString(), "Event0");
      NS_TEST_STRING(result[1].GetString(), "Event1");
      NS_TEST_STRING(result[2].GetString(), "Event2");
      NS_TEST_STRING(result[3].GetString(), "Event3");
      NS_TEST_STRING(result[4].GetString(), "Event4");
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(-0.1), nsTime::MakeFromSeconds(10.0), result);
      NS_TEST_INT(result.GetCount(), 5);
      NS_TEST_STRING(result[0].GetString(), "Event0");
      NS_TEST_STRING(result[1].GetString(), "Event1");
      NS_TEST_STRING(result[2].GetString(), "Event2");
      NS_TEST_STRING(result[3].GetString(), "Event3");
      NS_TEST_STRING(result[4].GetString(), "Event4");
    }

    et.Clear();
    NS_TEST_BOOL(et.IsEmpty());
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reverse Sample")
  {
    nsEventTrack et;
    nsHybridArray<nsHashedString, 8> result;

    et.AddControlPoint(nsTime::MakeFromSeconds(3.0), "Event3");
    et.AddControlPoint(nsTime::MakeFromSeconds(0.0), "Event0");
    et.AddControlPoint(nsTime::MakeFromSeconds(4.0), "Event4");
    et.AddControlPoint(nsTime::MakeFromSeconds(1.0), "Event1");
    et.AddControlPoint(nsTime::MakeFromSeconds(2.0), "Event2");

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(2.0), nsTime::MakeFromSeconds(0.0), result);
      NS_TEST_INT(result.GetCount(), 2);
      NS_TEST_STRING(result[0].GetString(), "Event2");
      NS_TEST_STRING(result[1].GetString(), "Event1");
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(4.0), nsTime::MakeFromSeconds(0.0), result);
      NS_TEST_INT(result.GetCount(), 4);
      NS_TEST_STRING(result[0].GetString(), "Event4");
      NS_TEST_STRING(result[1].GetString(), "Event3");
      NS_TEST_STRING(result[2].GetString(), "Event2");
      NS_TEST_STRING(result[3].GetString(), "Event1");
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(10.0), nsTime::MakeFromSeconds(0.0), result);
      NS_TEST_INT(result.GetCount(), 4);
      NS_TEST_STRING(result[0].GetString(), "Event4");
      NS_TEST_STRING(result[1].GetString(), "Event3");
      NS_TEST_STRING(result[2].GetString(), "Event2");
      NS_TEST_STRING(result[3].GetString(), "Event1");
    }

    {
      result.Clear();
      et.Sample(nsTime::MakeFromSeconds(10.0), nsTime::MakeFromSeconds(-0.1), result);
      NS_TEST_INT(result.GetCount(), 5);
      NS_TEST_STRING(result[0].GetString(), "Event4");
      NS_TEST_STRING(result[1].GetString(), "Event3");
      NS_TEST_STRING(result[2].GetString(), "Event2");
      NS_TEST_STRING(result[3].GetString(), "Event1");
      NS_TEST_STRING(result[4].GetString(), "Event0");
    }
  }
}
