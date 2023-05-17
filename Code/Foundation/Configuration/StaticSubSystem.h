#pragma once

/// \file

/// *** Example Subsystem declarations ***
///
/// WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, ExampleSubSystem)
///
///  BEGIN_SUBSYSTEM_DEPENDENCIES
///    "SomeOtherSubSystem",
///    "SomeOtherSubSystem2"
///  END_SUBSYSTEM_DEPENDENCIES
///
///  ON_CORESYSTEMS_STARTUP
///  {
///    wdExampleSubSystem::BasicStartup();
///  }
///
///  ON_CORESYSTEMS_SHUTDOWN
///  {
///    wdExampleSubSystem::BasicShutdown();
///  }
///
///  ON_HIGHLEVELSYSTEMS_STARTUP
///  {
///    wdExampleSubSystem::EngineStartup();
///  }
///
///  ON_HIGHLEVELSYSTEMS_SHUTDOWN
///  {
///    wdExampleSubSystem::EngineShutdown();
///  }
///
/// WD_END_SUBSYSTEM_DECLARATION;

/// \brief Put this in some cpp file of a subsystem to start its startup / shutdown sequence declaration.
///
/// The first parameter is the name of the group, in which the subsystem resides, the second is the name of the subsystem itself.
#define WD_BEGIN_SUBSYSTEM_DECLARATION(GroupName, SubsystemName)             \
  class GroupName##SubsystemName##SubSystem;                                 \
  class GroupName##SubsystemName##SubSystem : public wdSubSystem             \
  {                                                                          \
  public:                                                                    \
    virtual const char* GetGroupName() const override { return #GroupName; } \
                                                                             \
  public:                                                                    \
    virtual const char* GetSubSystemName() const override { return #SubsystemName; }

/// \brief Finishes a subsystem's startup / shutdown sequence declaration.
#define WD_END_SUBSYSTEM_DECLARATION \
  }                                  \
  static WD_CONCAT(s_SubSystem, WD_SOURCE_LINE)

/// \brief Defines what code is to be executed upon base startup.
///
/// Put this inside the subsystem declaration block.
#define ON_BASESYSTEMS_STARTUP \
private:                       \
  virtual void OnBaseSystemsStartup() override

/// \brief Defines what code is to be executed upon core startup.
///
/// Put this inside the subsystem declaration block.
#define ON_CORESYSTEMS_STARTUP \
private:                       \
  virtual void OnCoreSystemsStartup() override

/// \brief Defines what code is to be executed upon core shutdown.
///
/// Put this inside the subsystem declaration block.
#define ON_CORESYSTEMS_SHUTDOWN \
private:                        \
  virtual void OnCoreSystemsShutdown() override

/// \brief Defines what code is to be executed upon engine startup.
///
/// Put this inside the subsystem declaration block.
#define ON_HIGHLEVELSYSTEMS_STARTUP \
private:                            \
  virtual void OnHighLevelSystemsStartup() override

/// \brief Defines what code is to be executed upon engine shutdown.
///
/// Put this inside the subsystem declaration block.
#define ON_HIGHLEVELSYSTEMS_SHUTDOWN \
private:                             \
  virtual void OnHighLevelSystemsShutdown() override

/// \brief Begins the list of subsystems, on which the currently declared system depends on.
///
/// Must be followed by a series of strings with the names of the dependencies.
#define BEGIN_SUBSYSTEM_DEPENDENCIES                       \
public:                                                    \
  virtual const char* GetDependency(wdInt32 iDep) override \
  {                                                        \
    const char* szDeps[] = {

/// \brief Ends the list of subsystems, on which the currently declared system depends on.
#define END_SUBSYSTEM_DEPENDENCIES \
  , nullptr                        \
  }                                \
  ;                                \
  return szDeps[iDep];             \
  }

/// \brief This inserts a friend declaration into a class, such that the given group/subsystem can access private functions which it might need.
#define WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(GroupName, SubsystemName) friend class GroupName##SubsystemName##SubSystem;
