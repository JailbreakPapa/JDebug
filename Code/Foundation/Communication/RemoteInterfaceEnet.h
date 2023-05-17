#pragma once

#include <Foundation/Communication/RemoteInterface.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

/// \brief An implementation for wdRemoteInterface built on top of Enet
class WD_FOUNDATION_DLL wdRemoteInterfaceEnet : public wdRemoteInterface
{
public:
  ~wdRemoteInterfaceEnet();

  /// \brief Allocates a new instance with the given allocator
  static wdInternal::NewInstance<wdRemoteInterfaceEnet> Make(wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());

  /// \brief The port through which the connection was started
  wdUInt16 GetPort() const { return m_uiPort; }

private:
  wdRemoteInterfaceEnet();
  friend class wdRemoteInterfaceEnetImpl;

protected:
  wdUInt16 m_uiPort = 0;
};

#endif
