#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
list( APPEND coolfluid_client_core_moc_classes
CNode.hpp
ClientNetworkComm.hpp
NJournalBrowser.hpp
NCore.hpp
NJournal.hpp
NLink.hpp
NLog.hpp
NTree.hpp
PropertyModel.hpp
StatusModel.hpp
TreeNode.hpp
)

# Create the moc files
QT4_WRAP_CPP(coolfluid_client_core_moc_files ${coolfluid_client_core_moc_classes})

# Add the project files (including the moc files)
list( APPEND coolfluid_client_core_files
CNode.cpp
CNode.hpp
ClientNetworkComm.cpp
ClientNetworkComm.hpp
ClientRoot.cpp
ClientRoot.hpp
CommitDetails.cpp
CommitDetails.hpp
CommitDetailsItem.cpp
CommitDetailsItem.hpp
NJournalBrowser.cpp
NJournalBrowser.hpp
LibClientCore.cpp
LibClientCore.hpp
NBrowser.cpp
NBrowser.hpp
NCore.cpp
NCore.hpp
NGeneric.hpp
NGeneric.cpp
NJournal.cpp
NJournal.hpp
NLink.cpp
NLink.hpp
NLog.cpp
NLog.hpp
NRoot.cpp
NRoot.hpp
NTree.cpp
NTree.hpp
PropertyModel.cpp
PropertyModel.hpp
SignalNode.cpp
SignalNode.hpp
StatusModel.cpp
StatusModel.hpp
TSshInformation.hpp
TreeNode.cpp
TreeNode.hpp
)

list( APPEND coolfluid_client_core_cflibs coolfluid_network )

# Use Qt Libraries
list( APPEND coolfluid_client_core_libs ${QT_LIBRARIES})

coolfluid_add_library(coolfluid_client_core)
