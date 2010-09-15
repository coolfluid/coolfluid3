// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <QString>

#include "GUI/Network/HostInfos.hpp"

using namespace CF::GUI::Network;

HostInfos::HostInfos(const QString & hostname, int nbSlots, int maxSlots)
{
  m_hostname = hostname;
  m_nbSlots = nbSlots;
  m_maxSlots = maxSlots;
}
