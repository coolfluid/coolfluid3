#ifndef CF_GUI_Client_RemoteOpenFile_h
#define CF_GUI_Client_RemoteOpenFile_h

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/RemoteFSBrowser.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

////////////////////////////////////////////////////////////////////////////////

  class ClientNetworkComm;

////////////////////////////////////////////////////////////////////////////////

  /// @brief This class is a dialog that allows user to select one or more
  /// files to open.

  /// This class subclasses @c RemoteFSBrowser.

  /// @author Quentin Gasper
  class RemoteOpenFile : public RemoteFSBrowser
  {
  public:

    typedef boost::shared_ptr<RemoteOpenFile> Ptr;
    typedef boost::shared_ptr<const RemoteOpenFile> ConstPtr;

    /// @brief Constructor

    /// @param parent Parent window. May be @c CFNULL
    RemoteOpenFile(QMainWindow * parent = CFNULL);

    /// @brief Destructor

    /// Frees all allocated memory. Parent is not destroyed.
    ~RemoteOpenFile();

    static RemoteOpenFile::Ptr create(QMainWindow * parent = CFNULL);

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
    static void regist_signals ( Component* self ) {}

  }; // namespace RemoteOpenFile

  /////////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////


#endif // CF_GUI_Client_RemoteOpenFile_h
