#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/IterateBits.h>

namespace
{
  // declare bitflags using macro magic
  NS_DECLARE_FLAGS(nsUInt32, AutoFlags, Bit1, Bit2, Bit3, Bit4);

  // declare bitflags manually
  struct ManualFlags
  {
    using StorageType = nsUInt32;

    enum Enum
    {
      Bit1 = NS_BIT(0),
      Bit2 = NS_BIT(1),
      Bit3 = NS_BIT(2),
      Bit4 = NS_BIT(3),

      Default = Bit1 | Bit2
    };

    struct Bits
    {
      StorageType Bit1 : 1;
      StorageType Bit2 : 1;
      StorageType Bit3 : 1;
      StorageType Bit4 : 1;
    };
  };

  NS_DECLARE_FLAGS_OPERATORS(ManualFlags);
} // namespace

NS_DEFINE_AS_POD_TYPE(AutoFlags::Enum);
NS_CHECK_AT_COMPILETIME(sizeof(nsBitflags<AutoFlags>) == 4);


NS_CREATE_SIMPLE_TEST(Basics, Bitflags)
{
  NS_TEST_BOOL(AutoFlags::Count == 4);

  {
    nsBitflags<AutoFlags> flags = AutoFlags::Bit1 | AutoFlags::Bit4;

    NS_TEST_BOOL(flags.IsSet(AutoFlags::Bit4));
    NS_TEST_BOOL(flags.AreAllSet(AutoFlags::Bit1 | AutoFlags::Bit4));
    NS_TEST_BOOL(flags.IsAnySet(AutoFlags::Bit1 | AutoFlags::Bit2));
    NS_TEST_BOOL(!flags.IsAnySet(AutoFlags::Bit2 | AutoFlags::Bit3));
    NS_TEST_BOOL(flags.AreNoneSet(AutoFlags::Bit2 | AutoFlags::Bit3));
    NS_TEST_BOOL(!flags.AreNoneSet(AutoFlags::Bit2 | AutoFlags::Bit4));

    flags.Add(AutoFlags::Bit3);
    NS_TEST_BOOL(flags.IsSet(AutoFlags::Bit3));

    flags.Remove(AutoFlags::Bit1);
    NS_TEST_BOOL(!flags.IsSet(AutoFlags::Bit1));

    flags.Toggle(AutoFlags::Bit4);
    NS_TEST_BOOL(flags.AreAllSet(AutoFlags::Bit3));

    flags.AddOrRemove(AutoFlags::Bit2, true);
    flags.AddOrRemove(AutoFlags::Bit3, false);
    NS_TEST_BOOL(flags.AreAllSet(AutoFlags::Bit2));

    flags.Add(AutoFlags::Bit1);

    nsBitflags<ManualFlags> manualFlags = ManualFlags::Default;
    NS_TEST_BOOL(manualFlags.AreAllSet(ManualFlags::Bit1 | ManualFlags::Bit2));
    NS_TEST_BOOL(manualFlags.GetValue() == flags.GetValue());
    NS_TEST_BOOL(manualFlags.AreAllSet(ManualFlags::Default & ManualFlags::Bit2));

    NS_TEST_BOOL(flags.IsAnyFlagSet());
    flags.Clear();
    NS_TEST_BOOL(flags.IsNoFlagSet());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator&")
  {
    nsBitflags<AutoFlags> flags2 = AutoFlags::Bit1 & AutoFlags::Bit4;
    NS_TEST_BOOL(flags2.GetValue() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetValue")
  {
    nsBitflags<AutoFlags> flags;
    flags.SetValue(17);
    NS_TEST_BOOL(flags.GetValue() == 17);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator|=")
  {
    nsBitflags<AutoFlags> f = AutoFlags::Bit1 | AutoFlags::Bit2;
    f |= AutoFlags::Bit3;

    NS_TEST_BOOL(f.GetValue() == (AutoFlags::Bit1 | AutoFlags::Bit2 | AutoFlags::Bit3).GetValue());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator&=")
  {
    nsBitflags<AutoFlags> f = AutoFlags::Bit1 | AutoFlags::Bit2 | AutoFlags::Bit3;
    f &= AutoFlags::Bit3;

    NS_TEST_BOOL(f.GetValue() == AutoFlags::Bit3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Iterator")
  {
    {
      // Empty
      nsBitflags<AutoFlags> f;
      auto it = f.GetIterator();
      NS_TEST_BOOL(it == f.GetEndIterator());
      NS_TEST_BOOL(!it.IsValid());

      for (AutoFlags::Enum flag : f)
      {
        NS_TEST_BOOL_MSG(false, "No bit should be set");
      }
    }

    {
      // All flags
      nsBitflags<AutoFlags> f = AutoFlags::Bit1 | AutoFlags::Bit2 | AutoFlags::Bit3 | AutoFlags::Bit4;
      nsHybridArray<AutoFlags::Enum, 4> flags;
      flags.PushBack(AutoFlags::Bit1);
      flags.PushBack(AutoFlags::Bit2);
      flags.PushBack(AutoFlags::Bit3);
      flags.PushBack(AutoFlags::Bit4);

      nsUInt32 uiIndex = 0;
      // Iterator
      for (auto it = f.GetIterator(); it.IsValid(); ++it)
      {
        NS_TEST_INT(*it, flags[uiIndex]);
        NS_TEST_INT(it.Value(), flags[uiIndex]);
        NS_TEST_BOOL(it.IsValid());
        ++uiIndex;
      }
      NS_TEST_INT(uiIndex, 4);

      // Range-base for loop
      uiIndex = 0;
      for (AutoFlags::Enum flag : f)
      {
        NS_TEST_INT(flag, flags[uiIndex]);
        ++uiIndex;
      }
      NS_TEST_INT(uiIndex, 4);
    }
  }
}


//////////////////////////////////////////////////////////////////////////

namespace
{
  struct TypelessFlags1
  {
    enum Enum
    {
      Bit1 = NS_BIT(0),
      Bit2 = NS_BIT(1),
    };
  };

  struct TypelessFlags2
  {
    enum Enum
    {
      Bit3 = NS_BIT(2),
      Bit4 = NS_BIT(3),
    };
  };
} // namespace
