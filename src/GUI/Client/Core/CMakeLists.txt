#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
list( APPEND coolfluid_client_core_moc_classes
CNode.hpp
NetworkThread.hpp
NJournalBrowser.hpp
NJournal.hpp
NLog.hpp
NRoot.hpp
NTree.hpp
NPlotXY.hpp
PropertyModel.hpp
TreeNode.hpp
TreeThread.hpp
)

# Create the moc files
QT4_WRAP_CPP(coolfluid_client_core_moc_files ${coolfluid_client_core_moc_classes})

# Add the project files (including the moc files)
list( APPEND coolfluid_client_core_files
CNode.cpp
CNode.hpp
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
NetworkThread.cpp
NetworkThread.hpp
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
NPlotXY.cpp
NPlotXY.hpp
PropertyModel.cpp
PropertyModel.hpp
ThreadManager.cpp
ThreadManager.hpp
TreeNode.cpp
TreeNode.hpp
TreeThread.cpp
TreeThread.hpp
)

list( APPEND coolfluid_client_core_cflibs coolfluid_ui_common )

# Use Qt Libraries
list( APPEND coolfluid_client_core_libs ${QT_LIBRARIES})

coolfluid_add_library(coolfluid_client_core)
