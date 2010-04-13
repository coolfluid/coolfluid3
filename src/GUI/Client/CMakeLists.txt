#
# Compile Qt classes (with slots/signals) 
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
SET(coolfluid-client_moc_classes 
ClientKernel.hh
ClientNetworkComm.hh
CloseConfirmationDialog.hh
CloseConfirmationPanel.hh
CommitConfirmationPanel.hh
CommitDetailsDialog.hh
ConfirmCommitDialog.hh 
ConnectionDialog.hh
FilesPanel.hh 
GlobalLog.hh
LoggingList.hh
MainWindow.hh
OptionPanel.hh
RemoteFSBrowser.hh
RemoteSaveFile.hh
SaveConfirmationPanel.hh
SelectFileDialog.hh 
StatusModel.hh
StatusPanel.hh
TreeModel.hh
TreeView.hh
TypeAndNameDialog.hh)

# Create the moc files
QT4_WRAP_CPP(coolfluid-client_moc_files ${coolfluid-client_moc_classes})

# Add the project files (including the moc files)
LIST ( APPEND coolfluid-client_files
  AboutCFDialog.cxx
  AboutCFDialog.hh
  ClientKernel.cxx
  ClientKernel.hh
  ClientNetworkComm.cxx
  ClientNetworkComm.hh
  CloseConfirmationDialog.cxx
  CloseConfirmationDialog.hh
  CloseConfirmationInfos.hh
  CloseConfirmationPanel.cxx
  CloseConfirmationPanel.hh
  CommitConfirmationPanel.cxx
  CommitConfirmationPanel.hh
  CommitDetails.cxx
  CommitDetails.hh
  CommitDetailsDialog.cxx
  CommitDetailsDialog.hh
  CommitDetailsItem.cxx
  CommitDetailsItem.hh
  ConfirmCommitDialog.cxx
  ConfirmCommitDialog.hh
  ConnectionDialog.cxx
  ConnectionDialog.hh
  FilesListItem.cxx
  FilesListItem.hh
  FilesPanel.cxx
  FilesPanel.hh
  GlobalLog.cxx
  GlobalLog.hh
  GraphicalOption.cxx
  GraphicalOption.hh
  HostListPanel.cxx
  HostListPanel.hh
  InvalidValueException.cxx
  InvalidValueException.hh
  LibrariesPanel.cxx
  LibrariesPanel.hh
  LoggingList.cxx
  LoggingList.hh
  main.cxx
  MainWindow.cxx
  MainWindow.hh
  MenuActionInfo.cxx
  MenuActionInfo.hh
  OptionPanel.cxx
  OptionPanel.hh
  OptionTypes.cxx
  OptionTypes.hh
  RemoteFSBrowser.cxx
  RemoteFSBrowser.hh
  RemoteOpenFile.cxx
  RemoteOpenFile.hh
  RemoteSaveFile.cxx
  RemoteSaveFile.hh
  SaveConfirmationPanel.cxx
  SaveConfirmationPanel.hh
  SelectFileDialog.cxx
  SelectFileDialog.hh
  ShutdownConfirmationPanel.cxx
  ShutdownConfirmationPanel.hh
  StatusModel.cxx
  StatusModel.hh
  StatusPanel.cxx
  StatusPanel.hh
  TObjectProperties.hh
  TreeItem.cxx
  TreeItem.hh
  TreeModel.cxx
  TreeModel.hh
  TreeView.cxx
  TreeView.hh
  TreeViewAPI.hh
  TSshInformation.hh
  TypeAndNameDialog.cxx
  TypeAndNameDialog.hh
  UnknownTypeException.cxx
  UnknownTypeException.hh  )

# Use Qt Libraries
LIST( APPEND coolfluid-client_libs Network ${QT_LIBRARIES})

CF_ADD_APP(coolfluid-client)
