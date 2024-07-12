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
/// nsTelemetry, and the nsInspector tool will display the information.
class NS_FOUNDATION_DLL nsStats
{
public:
  using MapType = nsMap<nsString, nsVariant>;

  /// \brief Removes the stat with the given name.
  ///
  /// This will also send a 'remove' message through nsTelemetry, such that external tools can remove it from their list.
  static void RemoveStat(nsStringView sStatName);

  /// \brief Sets the value of the given stat, adds it if it did not exist before.
  ///
  /// szStatName may contain slashes (but not backslashes) to define groups and subgroups, which can be used by tools such as nsInspector
  /// to display the stats in a hierarchical way.
  /// This function will also send the name and value of the stat through nsTelemetry, such that tools like nsInspector will show the
  /// changed value.
  static void SetStat(nsStringView sStatName, const nsVariant& value);

  /// \brief Returns the value of the given stat. Returns an invalid nsVariant, if the stat did not exist before.
  static const nsVariant& GetStat(nsStringView sStatName) { return s_Stats[sStatName]; }

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
    nsStringView m_sStatName;
    nsVariant m_NewStatValue;
  };

  using nsEventStats = nsEvent<const StatsEventData&, nsMutex>;

  /// \brief Adds an event handler that is called every time a stat is changed.
  static void AddEventHandler(nsEventStats::Handler handler) { s_StatsEvents.AddEventHandler(handler); }

  /// \brief Removes a previously added event handler.
  static void RemoveEventHandler(nsEventStats::Handler handler) { s_StatsEvents.RemoveEventHandler(handler); }

private:
  static nsMutex s_Mutex;
  static MapType s_Stats;
  static nsEventStats s_StatsEvents;
};
