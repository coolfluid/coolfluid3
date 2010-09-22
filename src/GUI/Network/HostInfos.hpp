// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_network_HostInfos_h
#define CF_network_HostInfos_h

////////////////////////////////////////////////////////////////////////////////

#include "GUI/Network/LibNetwork.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Holds host information.

  /// @author Quentin Gasper

  struct Network_API HostInfos
  {
    public:

    /// @brief Hostname
    QString m_hostname;

    /// @brief Number of slots the host has.
    unsigned int m_nbSlots;

    /// @brief Maximum number of slot that can be allocated
    unsigned int m_maxSlots;

    /// @brief Constructor
    HostInfos(const QString & hostname = QString(), int nbSlots = 0,
              int maxSlots = 0);

  }; // struct HostInfos

////////////////////////////////////////////////////////////////////////////////

} // namespace Network
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_network_HostInfos_h
