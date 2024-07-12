#pragma once

#include <Foundation/Communication/RemoteInterface.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

/// \brief An implementation for nsRemoteInterface built on top of Enet
class NS_FOUNDATION_DLL nsRemoteInterfaceEnet : public nsRemoteInterface
{
public:
  ~nsRemoteInterfaceEnet();

  /// \brief Allocates a new instance with the given allocator
  static nsInternal::NewInstance<nsRemoteInterfaceEnet> Make(nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator());

  /// \brief The port through which the connection was started
  nsUInt16 GetPort() const { return m_uiPort; }

private:
  nsRemoteInterfaceEnet();
  friend class nsRemoteInterfaceEnetImpl;

protected:
  nsUInt16 m_uiPort = 0;
};

#endif
