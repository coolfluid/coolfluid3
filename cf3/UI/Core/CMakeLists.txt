#
# Compile Qt classes (with slots/signals)
#
# source : http://wiki.qtcentre.org/index.php?title=Compiling_Qt4_apps_with_CMake
#

# List of classes to "moc"
# Put here all the headers that declare a class able to emit or catch Qt signals.
# If a such header is missing here, linker returns a "missing vtable" error.
list( APPEND coolfluid_ui_core_moc_classes
CNode.hpp
NetworkThread.hpp
NJournalBrowser.hpp
NJournal.hpp
NLog.hpp
NRemoteFSBrowser.hpp
NRoot.hpp
NTree.hpp
PropertyModel.hpp
RemoteDispatcher.hpp
TreeNode.hpp
TreeThread.hpp
)

# Create the moc files
QT4_WRAP_CPP(coolfluid_ui_core_moc_files ${coolfluid_ui_core_moc_classes})

# Add the project files (including the moc files)
list( APPEND coolfluid_ui_core_files
CNode.cpp
CNode.hpp
CNodeBuilders.cpp
CNodeBuilders.hpp
CommitDetails.cpp
CommitDetails.hpp
CommitDetailsItem.cpp
CommitDetailsItem.hpp
NJournalBrowser.cpp
NJournalBrowser.hpp
LibCore.cpp
LibCore.hpp
NBrowser.cpp
NBrowser.hpp
NetworkQueue.cpp
NetworkQueue.hpp
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
NPlugin.cpp
NPlugin.hpp
NPlugins.cpp
NPlugins.hpp
NRemoteFSBrowser.cpp
NRemoteFSBrowser.hpp
NRoot.cpp
NRoot.hpp
NTree.cpp
NTree.hpp
PropertyModel.cpp
PropertyModel.hpp
RemoteDispatcher.cpp
RemoteDispatcher.hpp
ThreadManager.cpp
ThreadManager.hpp
TreeNode.cpp
TreeNode.hpp
TreeThread.cpp
TreeThread.hpp
)

list( APPEND coolfluid_ui_core_cflibs coolfluid_ui_common coolfluid_shell )

# Use Qt Libraries
list( APPEND coolfluid_ui_core_libs ${QT_LIBRARIES} )

coolfluid_add_library(coolfluid_ui_core)
