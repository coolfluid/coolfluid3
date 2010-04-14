#ifndef CF_GUI_Client_CloseConfirmationInfos_h
#define CF_GUI_Client_CloseConfirmationInfos_h

////////////////////////////////////////////////////////////////////////////////

#include <QHash>

#include "GUI/Client/CommitDetails.hpp"

class QString;
class QStringList;

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////////

  /// @brief This structure handles information when user closes the main
  /// window.

  struct CloseConfirmationInfos
  {
    public:

    /// @brief Indicates whether the server application has to be shut down
    /// when client application exits.

    /// If @c true, the server application has to be shut down.
    bool shutdownServerRequested;

    /// @brief Indicates whether the currently modified m_options need to be
    /// committed before the client application exits.

    /// If @c true, the currently modified m_options will be committed.
    bool commitRequested;

    /// @brief Indicates whether the current configuration has to be saved
    /// locally.

    /// If @c true, the current simulation will be saved locally. If @c false,
    /// it will be saved remotely. This attribute is ignored of @c #filename is
    /// empty or null.
    bool saveLocally;

    /// @brief Indicates whether the current configuration should be saved
    /// locally if remotely saving failed.
    bool saveLocallyOnError;

    /// @brief Commit details
    CommitDetails commitDetails;

    /// @brief File name where the current configuration will be saved to.

    /// If empty or null, the current configuration will not be saved.
    QString filename;

    /// @brief Construction

    /// All boolean attributes are set @c false.
    CloseConfirmationInfos()
    {
      this->clear();
    }

    /// @brief Clears all data

    /// All boolean attributes are set @c false. @c #m_commitDetails and
    /// @c #filename attributes are cleared.
    void clear()
    {
      this->shutdownServerRequested = false;
      this->commitRequested = false;
      this->saveLocallyOnError = false;
      this->saveLocally = false;
      this->commitDetails.clear();
      this->filename.clear();
    }
  };

    /////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_CloseConfirmationInfos_h
