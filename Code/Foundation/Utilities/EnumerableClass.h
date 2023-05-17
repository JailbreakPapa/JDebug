#pragma once

/// \file

/// \brief Base class to add the ability to another class to enumerate all active instance of it, across DLL boundaries.
///
/// This creates a new class-type that has the static information
/// about all instances that were created from that class. Another class now only has to derive from that class and will
/// then gain the ability to count and enumerate its instances.
///
/// Usage is as follows:
///
/// If you have a class A that you want to be enumerable, add this to its header:
///
///   class WD_DLL_IMPORT_EXPORT_STUFF A : public wdEnumerable<A>
///   {
///     WD_DECLARE_ENUMERABLE_CLASS(A); // since A is declared as DLL import/export all code embedded in its body will also work properly
///     ...
///   };
///
/// Also add this somewhere in its source-file:
///
///   WD_ENUMERABLE_CLASS_IMPLEMENTATION(A);
///
/// That's it, now the class instances can be enumerated with 'GetFirstInstance' and 'GetNextInstance'
template <typename Derived, typename Base = wdNoBase>
class wdEnumerable : public Base
{
public:
  wdEnumerable()
  {
    if (Derived::s_pFirstInstance == nullptr)
      Derived::s_pFirstInstance = this;
    else
      Derived::s_pLastInstance->m_pNextInstance = this;

    Derived::s_pLastInstance = this;
    m_pNextInstance = nullptr;
    ++Derived::s_uiInstances;
  }

  virtual ~wdEnumerable()
  {
    --Derived::s_uiInstances;
    wdEnumerable* pPrev = nullptr;
    wdEnumerable* pCur = Derived::s_pFirstInstance;

    while (pCur)
    {
      if (pCur == this)
      {
        if (pPrev == nullptr)
          Derived::s_pFirstInstance = m_pNextInstance;
        else
          pPrev->m_pNextInstance = m_pNextInstance;

        if (Derived::s_pLastInstance == this)
          Derived::s_pLastInstance = pPrev;

        break;
      }

      pPrev = pCur;
      pCur = pCur->m_pNextInstance;
    }
  }

protected:
  wdEnumerable* m_pNextInstance;
};

/// \brief Insert this macro in a class that is supposed to be enumerable, and pass the class name as the parameter.
///
/// See class wdEnumerable for more details.
#define WD_DECLARE_ENUMERABLE_CLASS(self) WD_DECLARE_ENUMERABLE_CLASS_WITH_BASE(self, wdNoBase)

/// \brief Insert this macro in a class that is supposed to be enumerable, and pass the class name as the parameter.
///
/// See class wdEnumerable for more details.
#define WD_DECLARE_ENUMERABLE_CLASS_WITH_BASE(self, base)                      \
private:                                                                       \
  typedef base wdEnumerableBase;                                               \
  friend class wdEnumerable<self, base>;                                       \
  static wdEnumerable<self, base>* s_pFirstInstance;                           \
  static wdEnumerable<self, base>* s_pLastInstance;                            \
  static wdUInt32 s_uiInstances;                                               \
                                                                               \
public:                                                                        \
  static self* GetFirstInstance() { return (self*)s_pFirstInstance; }          \
  self* GetNextInstance() { return (self*)m_pNextInstance; }                   \
  const self* GetNextInstance() const { return (const self*)m_pNextInstance; } \
                                                                               \
private:

/// \brief Insert this macro in a cpp file and pass the class name of the to-be-enumerable class as the parameter.
///
/// See class wdEnumerable for more details.
#define WD_ENUMERABLE_CLASS_IMPLEMENTATION(self)                                \
  wdEnumerable<self, self::wdEnumerableBase>* self::s_pFirstInstance = nullptr; \
  wdEnumerable<self, self::wdEnumerableBase>* self::s_pLastInstance = nullptr;  \
  wdUInt32 self::s_uiInstances = 0
