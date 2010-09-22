################################################################################
# Test Client

if( QT_QTTEST_FOUND )

set(uTestClient_moc_classes

  CommonFunctions.hpp
  NTreeTest.hpp
  NBrowserTest.hpp
  NLinkTest.hpp
  CNodeTest.hpp
)

QT4_WRAP_CPP(uTestClient_moc_files ${uTestClient_moc_classes})

configure_file( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

list( APPEND uTestClient_cflibs Network)
list( APPEND uTestClient_libs ${QT_LIBRARIES})
list( APPEND uTestClient_files
  uTestClient.cpp
  ExceptionThrowHandler.hpp
  NTreeTest.hpp
  NTreeTest.cpp
  CommonFunctions.cpp
  CommonFunctions.hpp
  CNodeTest.hpp
  CNodeTest.cpp
  MyNode.cpp
  MyNode.hpp
  NBrowserTest.cpp
  NBrowserTest.hpp
  NLinkTest.cpp
  NLinkTest.hpp
  TreeHandler.cpp
  TreeHandler.hpp
)

list( APPEND uTestClient_libs ClientCore ${QT_LIBRARIES})

coolfluid_add_unittest( uTestClient )

endif()
