#include <QString>

#include "GUI/Network/HostInfos.hh"

using namespace CF::GUI::Network;

HostInfos::HostInfos(const QString & hostname, int nbSlots, int maxSlots)
{
  m_hostname = hostname;
  m_nbSlots = nbSlots;
  m_maxSlots = maxSlots;
}
