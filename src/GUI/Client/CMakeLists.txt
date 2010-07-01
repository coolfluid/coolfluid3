#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
SET(coolfluid-client_moc_classes
  AddLinkDialog.hpp
  ClientCore.hpp
  ClientNetworkComm.hpp
  CLog.hpp
  CNode.hpp
  CommitDetailsDialog.hpp
  ConfirmCommitDialog.hpp
  ConnectionDialog.hpp
  CTree.hpp
  FilesPanel.hpp
  LoggingList.hpp
  MainWindow.hpp
  NGroup.hpp
  NLink.hpp
  NMesh.hpp
  NMethod.hpp
  NRoot.hpp
  OptionPanel.hpp
  RemoteFSBrowser.hpp
  RemoteSaveFile.hpp
  SelectFileDialog.hpp
  StatusModel.hpp
  StatusPanel.hpp
  TreeView.hpp
  TypeAndNameDialog.hpp)

# Create the moc files
QT4_WRAP_CPP(coolfluid-client_moc_files ${coolfluid-client_moc_classes})

# Add the project files (including the moc files)
LIST ( APPEND coolfluid-client_files
  AboutCFDialog.cpp
  AboutCFDialog.hpp
  AddLinkDialog.cpp
  AddLinkDialog.hpp
  CBrowser.cpp
  CBrowser.hpp
  ClientCore.cpp
  ClientCore.hpp
  ClientNetworkComm.cpp
  ClientNetworkComm.hpp
  ClientRoot.cpp
  ClientRoot.hpp
  CLog.cpp
  CLog.hpp
  CNode.cpp
  CNode.hpp
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
  CTree.cpp
  CTree.hpp
  FilesListItem.cpp
  FilesListItem.hpp
  FilesPanel.cpp
  FilesPanel.hpp
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
  NGroup.cpp
  NGroup.hpp
  NLink.cpp
  NLink.hpp
  NMesh.cpp
  NMesh.hpp
  NMethod.cpp
  NMethod.hpp
  NRoot.cpp
  NRoot.hpp
  OptionPanel.cpp
  OptionPanel.hpp
  OptionType.cpp
  OptionType.hpp
  RemoteFSBrowser.cpp
  RemoteFSBrowser.hpp
  RemoteOpenFile.cpp
  RemoteOpenFile.hpp
  RemoteSaveFile.cpp
  RemoteSaveFile.hpp
  SelectFileDialog.cpp
  SelectFileDialog.hpp
  StatusModel.cpp
  StatusModel.hpp
  StatusPanel.cpp
  StatusPanel.hpp
  TObjectProperties.hpp
  TreeNode.cpp
  TreeNode.hpp
  TreeView.cpp
  TreeView.hpp
  TSshInformation.hpp
  TypeAndNameDialog.cpp
  TypeAndNameDialog.hpp
  UnknownTypeException.cpp
  UnknownTypeException.hpp  )

# Use Qt Libraries
LIST( APPEND coolfluid-client_libs Network ${QT_LIBRARIES})

CF_ADD_APP(coolfluid-client)
