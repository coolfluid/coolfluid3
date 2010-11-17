// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_NRemoteSave_h
#define CF_GUI_Client_UI_NRemoteSave_h

////////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/UI/NRemoteBrowser.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  class TypeAndNameDialog;
  class ClientNetworkComm;

////////////////////////////////////////////////////////////////////////////////

  /// @brief This class is a dialog that allows user to to select a remote
  /// place to save a file.

  /// This class subclasses @c NRemoteBrowser. User can create new directories,
  /// enter a name for the new file or choose to overwrite a file. This class
  /// does not allow user to call @c #setIncludeFiles, @c #setIncludeNoExtension
  /// and @c #showMultipleSelect base class methods.

  /// @author Quentin Gasper
  class ClientUI_API NRemoteSave : public NRemoteBrowser
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NRemoteSave> Ptr;
    typedef boost::shared_ptr<const NRemoteSave> ConstPtr;

    /// @brief Constructor

    /// @param parent Parent window. May be @c nullptr.
    NRemoteSave(QMainWindow * parent = nullptr);

    /// @brief Destructor

    /// Frees all allocated memory. Parent is not destroyed.
    ~NRemoteSave();

    static NRemoteSave::Ptr create(QMainWindow * parent = nullptr);

    /// @brief Gives the node tooltip.
    /// @return Returns the tooltip text.
    virtual QString toolTip() const;

  protected:

    /// @brief Checks if the selection is valid.

    /// This method overrides base class method. Three cases are possible:
    /// @li if the given name is a directory and user has entered a file name,
    /// the file name is appended to the directory path and the method returns
    /// @c #POLICY_VALID.
    /// @li if the given name is a directory but user has not entered a file
    /// name, the method returns @c #POLICY_ENTER_DIRECTORY.
    /// @li if the given name is a file: a confirmation to overwrite the is asked.
    /// If user confirms, @c #POLICY_VALID is returned; otherwise
    /// @c #POLICY_NOT_VALID is returned.
    /// @note If the given name is a file, any file name entered by user is
    /// ignored.
    /// @param name Name to check
    /// @param isDir If @c true, @c name is a directory; otherwise, it is a file.
    /// @return Returns the validation as described above.
    virtual ValidationPolicy isAcceptable(const QString & name, bool isDir);

    /// @brief Reinitializes internal data to their default value.

    /// This method overrides base class method.
    virtual void reinitValues();

    /// @brief Give the selected file.

    /// This method overrides base class method.
    /// @return Returns the selected file, or an empty string if the last call
    /// to @c #isAcceptable did not return @c #POLICY_VALID or if this method was
    /// never called.
    virtual QString selectedFile() const;

  private slots:

    /// @brief Asks user to enter a file name and select file type.

    /// File type associated extention is appended to the name if needed. If
    /// user enters a name that already exists, he has to confirm his entry.
    void btFileNameClick();

    /// @brief Asks user to enter a new directory name.

    /// If a name is entered, a request to create that directory is sent to
    /// the server.
    void btNewDirectoryClicked();

  private:

    /// @brief "Set file name" button
    QPushButton * m_btFileName;

    /// @brief "New directory" button
    QPushButton * m_btNewDirectory;

    /// @brief File name entered by user.
    QString m_fileName;

    /// @brief Selected file and its path.
    QString m_selectedFile;

    /// @brief Dialog used to ask the new file name
    TypeAndNameDialog * m_fileNameDialog;

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  }; // class NRemoteSave

  /////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Client_UI_NRemoteSave_h
