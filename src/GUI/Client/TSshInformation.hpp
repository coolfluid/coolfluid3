#ifndef CF_GUI_Client_TSshInformation_h
#define CF_GUI_Client_TSshInformation_h

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Reprensents information needed to connect to the server and
  /// launch it (if needed) through an SSH connection.

  /// @author Quentin Gasper.

  struct TSshInformation
  {
    public :

    /// @brief Remote machine hostname.
    QString m_hostname;

    /// @brief Username to use to authenticate to the remote machine.
    QString username;

    /// @brief Socket port number.
    quint16 port;

    /// @brief If @c true, the user requests to launch a new server
    /// m_instance.
    bool launchServer;

    /// @brief Constructor.

    /// Provided for convinience.

    /// @param hostname Remote machine hostname.
    /// @param port Socket port number.
    /// @param launchServer If @c true, the user requests to
    /// launch a new server m_instance.
    /// @param username Username to use to authenticate to the remote
    /// machine.
    TSshInformation(const QString & m_hostname = QString("hostname"),
    quint16 port = 62784,
    bool launchServer = false,
    const QString & username = QString())
    {
      this->m_hostname = m_hostname;
      this->username = username;
      this->port = port;
      this->launchServer = launchServer;
    }
  }; // struct TSshInformation

////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_TSshInformation_h
