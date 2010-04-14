#ifndef CF_network_HostInfos_h
#define CF_network_HostInfos_h

////////////////////////////////////////////////////////////////////////////////

class QString;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Network {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Holds host information.

  /// @author Quentin Gasper

  struct HostInfos
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
