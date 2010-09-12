#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
LIST( APPEND coolfluid-client_moc_classes
  NCore.hpp
  ClientNetworkComm.hpp
  CNode.hpp
  CommitDetailsDialog.hpp
  ConfirmCommitDialog.hpp
  ConnectionDialog.hpp
  GraphicalBool.hpp
  GraphicalDouble.hpp
  GraphicalInt.hpp
  GraphicalOption.hpp
  GraphicalString.hpp
  GraphicalUrl.hpp
  GraphicalUrlArray.hpp
  GraphicalValue.hpp
  LoggingList.hpp
  MainWindow.hpp
  NArray.hpp
  NElements.hpp
  NGroup.hpp
  NLink.hpp
  NLog.hpp
  NMesh.hpp
  NMeshReader.hpp
  NMethod.hpp
  NRegion.hpp
  NTable.hpp
  NTree.hpp
  OptionPanel.hpp
  NRemoteBrowser.hpp
  NRemoteSave.hpp
  SelectFileDialog.hpp
  StatusModel.hpp
  StatusPanel.hpp
  SelectPathDialog.hpp
  TreeNode.hpp
  TreeView.hpp
  TypeAndNameDialog.hpp)

# Create the moc files
QT4_WRAP_CPP(coolfluid-client_moc_files ${coolfluid-client_moc_classes})

# Add the project files (including the moc files)
LIST( APPEND coolfluid-client_files
  AboutCFDialog.cpp
  AboutCFDialog.hpp
  ClientNetworkComm.cpp
  ClientNetworkComm.hpp
  ClientRoot.cpp
  ClientRoot.hpp
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
  FilesListItem.cpp
  FilesListItem.hpp
  GraphicalBool.hpp
  GraphicalBool.cpp
  GraphicalDouble.hpp
  GraphicalDouble.cpp
  GraphicalInt.hpp
  GraphicalInt.cpp
  GraphicalOption.cpp
  GraphicalOption.hpp
  GraphicalString.hpp
  GraphicalString.cpp
  GraphicalUrl.cpp
  GraphicalUrl.hpp
  GraphicalUrlArray.cpp
  GraphicalUrlArray.hpp
  GraphicalValue.cpp
  GraphicalValue.hpp
  LoggingList.cpp
  LoggingList.hpp
  main.cpp
  MainWindow.cpp
  MainWindow.hpp
  MenuActionInfo.cpp
  MenuActionInfo.hpp
  NArray.cpp
  NArray.hpp
  NBrowser.cpp
  NBrowser.hpp
  NCore.cpp
  NCore.hpp
  NElements.cpp
  NElements.hpp
  NGroup.cpp
  NGroup.hpp
  NLink.cpp
  NLink.hpp
  NLog.cpp
  NLog.hpp
  NMesh.cpp
  NMesh.hpp
  NMeshReader.cpp
  NMeshReader.hpp
  NMethod.cpp
  NMethod.hpp
  NRegion.cpp
  NRegion.hpp
  NRoot.cpp
  NRoot.hpp
  NTable.cpp
  NTable.hpp
  NTree.cpp
  NTree.hpp
  OptionPanel.cpp
  OptionPanel.hpp
  OptionType.cpp
  OptionType.hpp
  NRemoteBrowser.cpp
  NRemoteBrowser.hpp
  NRemoteOpen.cpp
  NRemoteOpen.hpp
  NRemoteSave.cpp
  NRemoteSave.hpp
  SelectFileDialog.cpp
  SelectFileDialog.hpp
  SelectPathDialog.cpp
  SelectPathDialog.hpp
  StatusModel.cpp
  StatusModel.hpp
  StatusPanel.cpp
  StatusPanel.hpp
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

coolfluid_add_application(coolfluid-client)

ADD_SUBDIRECTORY( uTests )
