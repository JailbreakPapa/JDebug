#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Variant.h>

/// \brief This class holds a simple map that maps strings (keys) to strings (values), which represent certain stats.
///
/// This can be used by a game to store (and continuously update) information about the internal game state. Other tools can then
/// display this information in a convenient manner. For example the stats can be shown on screen. The data is also transmitted through
/// wdTelemetry, and the wdInspector tool will display the information.
class WD_FOUNDATION_DLL wdStats
{
public:
  typedef wdMap<wdString, wdVariant> MapType;

  /// \brief Removes the stat with the given name.
  ///
  /// This will also send a 'remove' message through wdTelemetry, such that external tools can remove it from their list.
  static void RemoveStat(const char* szStatName);

  /// \brief Sets the value of the given stat, adds it if it did not exist before.
  ///
  /// szStatName may contain slashes (but not backslashes) to define groups and subgroups, which can be used by tools such as wdInspector
  /// to display the stats in a hierarchical way.
  /// This function will also send the name and value of the stat through wdTelemetry, such that tools like wdInspector will show the
  /// changed value.
  static void SetStat(const char* szStatName, const wdVariant& value);

  /// \brief Returns the value of the given stat. Returns an invalid wdVariant, if the stat did not exist before.
  static const wdVariant& GetStat(const char* szStatName) { return s_Stats[szStatName]; }

  /// \brief Returns the entire map of stats, can be used to display them.
  static const MapType& GetAllStats() { return s_Stats; }

  /// \brief The event data that is broadcast whenever a stat is changed.
  struct StatsEventData
  {
    /// \brief Which type of event this is.
    enum EventType
    {
      Add,   ///< A variable has been set for the first time.
      Set,   ///< A variable has been changed.
      Remove ///< A variable that existed has been removed.
    };

    EventType m_EventType;
    const char* m_szStatName;
    wdVariant m_NewStatValue;
  };

  typedef wdEvent<const StatsEventData&, wdMutex> wdEventStats;

  /// \brief Adds an event handler that is called every time a stat is changed.
  static void AddEventHandler(wdEventStats::Handler handler) { s_StatsEvents.AddEventHandler(handler); }

  /// \brief Removes a previously added event handler.
  static void RemoveEventHandler(wdEventStats::Handler handler) { s_StatsEvents.RemoveEventHandler(handler); }

private:
  static wdMutex s_Mutex;
  static MapType s_Stats;
  static wdEventStats s_StatsEvents;
};
