// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_NRemoteOpen_h
#define CF_GUI_Client_UI_NRemoteOpen_h

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/NRemoteBrowser.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {

namespace ClientCore { class ClientNetworkComm; }

namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  /// @brief This class is a dialog that allows user to select one or more
  /// files to open.

  /// This class subclasses @c NRemoteBrowser.

  /// @author Quentin Gasper
  class ClientUI_API NRemoteOpen : public NRemoteBrowser
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

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    virtual QIcon getIcon() const;

    /// @brief Gives the node tooltip.
    /// @return Returns the tooltip text.
    virtual QString getToolTip() const;

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
    virtual QString getSelectedFile() const;

    /// @brief Give the selected files.

    /// This method overrides base class method.
    /// @return Returns the selected files, or an empty string if the last call
    /// to <code>isAcceptable(const QStringList &)</code> did not return
    /// @c #POLICY_VALID or if this method was never called.
//    virtual QStringList getSelectedFileList() const;

    /// @brief Reinitializes internal data to their default value.

    /// This method overrides base class method.
    virtual void reinitValues();

  private:

    /// @brief Selected files list.

    /// Contains at most one item if the dialog is in single selection mode.
    QStringList m_fileList;

    /// regists all the signals declared in this class
    static void regist_signals ( Common::Component* self ) {}

  }; // namespace NRemoteOpen

  /////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Client_UI_NRemoteOpen_h
