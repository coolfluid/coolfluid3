#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
SET(coolfluid-client_moc_classes
ClientKernel.hpp
ClientNetworkComm.hpp
CloseConfirmationDialog.hpp
CloseConfirmationPanel.hpp
CommitConfirmationPanel.hpp
CommitDetailsDialog.hpp
ConfirmCommitDialog.hpp
ConnectionDialog.hpp
FilesPanel.hpp
GlobalLog.hpp
LoggingList.hpp
MainWindow.hpp
OptionPanel.hpp
RemoteFSBrowser.hpp
RemoteSaveFile.hpp
SaveConfirmationPanel.hpp
SelectFileDialog.hpp
StatusModel.hpp
StatusPanel.hpp
TreeModel.hpp
TreeView.hpp
TypeAndNameDialog.hpp)

# Create the moc files
QT4_WRAP_CPP(coolfluid-client_moc_files ${coolfluid-client_moc_classes})

# Add the project files (including the moc files)
LIST ( APPEND coolfluid-client_files
  AboutCFDialog.cpp
  AboutCFDialog.hpp
  ClientKernel.cpp
  ClientKernel.hpp
  ClientNetworkComm.cpp
  ClientNetworkComm.hpp
  CloseConfirmationDialog.cpp
  CloseConfirmationDialog.hpp
  CloseConfirmationInfos.hpp
  CloseConfirmationPanel.cpp
  CloseConfirmationPanel.hpp
  CommitConfirmationPanel.cpp
  CommitConfirmationPanel.hpp
  CommitDetails.cpp
  CommitDetails.hpp
  CommitDetailsDialog.cpp
  CommitDetailsDialog.hpp
  CommitDetailsItem.cpp
  CommitDetailsItem.hpp
  ConfirmCommitDialog.cpp
  ConfirmCommitDialog.hpp
  ConnectionDialog.cpp
  ConnectionDialog.hpp
  FilesListItem.cpp
  FilesListItem.hpp
  FilesPanel.cpp
  FilesPanel.hpp
  GlobalLog.cpp
  GlobalLog.hpp
  GraphicalOption.cpp
  GraphicalOption.hpp
  HostListPanel.cpp
  HostListPanel.hpp
  InvalidValueException.cpp
  InvalidValueException.hpp
  LibrariesPanel.cpp
  LibrariesPanel.hpp
  LoggingList.cpp
  LoggingList.hpp
  main.cpp
  MainWindow.cpp
  MainWindow.hpp
  MenuActionInfo.cpp
  MenuActionInfo.hpp
  OptionPanel.cpp
  OptionPanel.hpp
  OptionTypes.cpp
  OptionTypes.hpp
  RemoteFSBrowser.cpp
  RemoteFSBrowser.hpp
  RemoteOpenFile.cpp
  RemoteOpenFile.hpp
  RemoteSaveFile.cpp
  RemoteSaveFile.hpp
  SaveConfirmationPanel.cpp
  SaveConfirmationPanel.hpp
  SelectFileDialog.cpp
  SelectFileDialog.hpp
  ShutdownConfirmationPanel.cpp
  ShutdownConfirmationPanel.hpp
  StatusModel.cpp
  StatusModel.hpp
  StatusPanel.cpp
  StatusPanel.hpp
  TObjectProperties.hpp
  TreeItem.cpp
  TreeItem.hpp
  TreeModel.cpp
  TreeModel.hpp
  TreeView.cpp
  TreeView.hpp
  TreeViewAPI.hpp
  TSshInformation.hpp
  TypeAndNameDialog.cpp
  TypeAndNameDialog.hpp
  UnknownTypeException.cpp
  UnknownTypeException.hpp  )

# Use Qt Libraries
LIST( APPEND coolfluid-client_libs Network ${QT_LIBRARIES})

CF_ADD_APP(coolfluid-client)
