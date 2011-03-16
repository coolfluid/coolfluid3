################################################################################
# Test Client

if( QT_QTTEST_FOUND AND CF_ENABLE_GUI)

set(utest-client_moc_classes
  CommonFunctions.hpp
  CommitDetailsTest.hpp
  CNodeTest.hpp
  NBrowserTest.hpp
  NLinkTest.hpp
  NRootTest.hpp
  NTreeTest.hpp
  PropertyModelTest.hpp
  TreeNodeTest.hpp
)

QT4_WRAP_CPP(utest-client_moc_files ${utest-client_moc_classes})

#configure_file( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

list( APPEND utest-client_cflibs coolfluid_network)
list( APPEND utest-client_libs ${QT_LIBRARIES})
list( APPEND utest-client_files
  utest-client.cpp
  ExceptionThrowHandler.hpp
  CNodeTest.hpp
  CNodeTest.cpp
  CommitDetailsTest.cpp
  CommitDetailsTest.hpp
  CommonFunctions.cpp
  CommonFunctions.hpp
  MyNode.cpp
  MyNode.hpp
  NBrowserTest.cpp
  NBrowserTest.hpp
  NLinkTest.cpp
  NLinkTest.hpp
  NRootTest.cpp
  NRootTest.hpp
  NTreeTest.hpp
  NTreeTest.cpp
  PropertyModelTest.cpp
  PropertyModelTest.hpp
  TreeHandler.cpp
  TreeHandler.hpp
  TreeNodeTest.cpp
  TreeNodeTest.hpp
)

list( APPEND utest-client_libs coolfluid_client_core ${QT_LIBRARIES})

coolfluid_add_unit_test( utest-client )

add_custom_command(TARGET utest-client
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/tree.xml ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

else()
    coolfluid_mark_not_orphan(
  utest-client.cpp
  ExceptionThrowHandler.hpp
  CNodeTest.hpp
  CNodeTest.cpp
  CommitDetailsTest.cpp
  CommitDetailsTest.hpp
  CommonFunctions.cpp
  CommonFunctions.hpp
  MyNode.cpp
  MyNode.hpp
  NBrowserTest.cpp
  NBrowserTest.hpp
  NLinkTest.cpp
  NLinkTest.hpp
  NRootTest.cpp
  NRootTest.hpp
  NTreeTest.hpp
  NTreeTest.cpp
  PropertyModelTest.cpp
  PropertyModelTest.hpp
  TreeHandler.cpp
  TreeHandler.hpp
  TreeNodeTest.cpp
  TreeNodeTest.hpp

        )
endif()
