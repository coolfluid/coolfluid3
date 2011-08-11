// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_NRemoteOpen_h
#define CF_GUI_Graphics_NRemoteOpen_h

////////////////////////////////////////////////////////////////////////////

#include "UI/Graphics/NRemoteBrowser.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {

namespace Core { class ClientNetworkComm; }

namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

  /// @brief This class is a dialog that allows user to select one or more
  /// files to open.

  /// This class subclasses @c NRemoteBrowser.

  /// @author Quentin Gasper
  class Graphics_API NRemoteOpen : public NRemoteBrowser
  {
  public:

    typedef boost::shared_ptr<NRemoteOpen> Ptr;
    typedef boost::shared_ptr<const NRemoteOpen> ConstPtr;

    /// @brief Constructor

    /// @param parent Parent window. May be @c nullptr
    NRemoteOpen(QMainWindow * parent = nullptr);

    /// @brief Destructor

    /// Frees all allocated memory. Parent is not destroyed.
    ~NRemoteOpen();

    static NRemoteOpen::Ptr create(QMainWindow * parent = nullptr);

    /// @brief Gives the node tooltip.
    /// @return Returns the tooltip text.
    virtual QString toolTip() const;

  protected:

    /// @brief Checks if the selection is valid.

    /// This method overrides base class method. If the given name is a
    /// directory, @c #POLICY_ENTER_DIRECTORY is returned; otherwise,
    /// @c #POLICY_VALID is returned.
    /// @param name Name to check
    /// @param isDir If @c true, @c name is a directory; otherwise, it is a file.
    /// @return Returns the validation as described above.
    virtual ValidationPolicy isAcceptable(const QString & name, bool isDir);

    /// @brief Checks if the selection is valid.

    /// This method overrides base class method. If at least one list item is a
    /// directory and the list item count is bigger than 1, an error message
    /// is shown and @c #POLICY_NOT_VALID. If the list contains one directory and
    /// no file, @c #POLICY_ENTER_DIRECTORY is returned. Finally, if all list
    /// m_items are files, @c #POLICY_VALID is returned
    /// @param names Name list to check
    /// @return Returns the validation as described above.
    virtual ValidationPolicy isAcceptable(const QStringList & names);

    /// @brief Give the selected file.

    /// This method overrides base class method.
    /// @return Returns the selected file, or an empty string if the last call
    /// to <code>isAcceptable(const QString &, bool)</code> did not return
    /// @c #POLICY_VALID or if this method was never called.
    virtual QString selectedFile() const;

    /// @brief Give the selected files.

    /// This method overrides base class method.
    /// @return Returns the selected files, or an empty string if the last call
    /// to <code>isAcceptable(const QStringList &)</code> did not return
    /// @c #POLICY_VALID or if this method was never called.
//    virtual QStringList getSelectedFileList() const;

    /// @brief Reinitializes internal data to their default value.

    /// This method overrides base class method.
    virtual void reinitValues();

  protected:

    /// Disables the local signals that need to.
    /// @param localSignals Map of local signals. All values are set to true
    /// by default.
    virtual void disableLocalSignals(QMap<QString, bool> & localSignals) const {}

  private:

    /// @brief Selected files list.

    /// Contains at most one item if the dialog is in single selection mode.
    QStringList m_fileList;

  }; // NRemoteOpen

  /////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Graphics_NRemoteOpen_h
