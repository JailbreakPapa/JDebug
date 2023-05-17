#pragma once

/// \brief Selection specifying file lock behavior on open
struct wdFileShareMode
{
  enum Enum
  {
    Default,     ///< Results in 'Exclusive' when requesting write access and 'SharedReads' when requesting read access. See wdFileOpenMode::Enum.
    Exclusive,   ///< No other process is allowed to access the file for reading or writing, while it is open
    SharedReads, ///< Other processes may read the file concurrently
  };
};

/// \brief For file seek operations this enum defines from which relative position the seek position is described.
struct wdFileSeekMode
{
  enum Enum
  {
    FromStart,   ///< The seek position is relative to the file's beginning
    FromEnd,     ///< The seek position is relative to the file's end
    FromCurrent, ///< The seek position is relative to the file's current seek position
  };
};
