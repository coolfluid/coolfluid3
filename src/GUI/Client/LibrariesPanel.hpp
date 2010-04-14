#ifndef CF_GUI_Client_LibrariesPanel_h
#define CF_GUI_Client_LibrariesPanel_h

////////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/FilesPanel.hpp"

class QStringList;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////////

  /// @brief This class represents a graphical component that allows user to
  /// maintain a list of libraries.

  class LibrariesPanel : public FilesPanel
  {
    public:
    /// @brief Constructor

    /// @param parent Parent widget. May be @c NULL.
    LibrariesPanel(QWidget * parent = NULL);

    /// @brief Destructor

    /// Frees all alocated memory. The parent is not destroyed
    ~LibrariesPanel();

    /// @brief Gives the list of files.
    /// @return Returns the list of files.
    virtual QStringList getFilesList() const;

  }; // class LibrariesPanel

////////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_LibrariesPanel_h
