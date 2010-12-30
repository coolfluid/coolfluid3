################################################################################
# Test Client

if( QT_QTTEST_FOUND AND CF_ENABLE_GUI)

set(utest-client_moc_classes
  CommonFunctions.hpp
  NTreeTest.hpp
  NBrowserTest.hpp
  NLinkTest.hpp
  CNodeTest.hpp
)

QT4_WRAP_CPP(utest-client_moc_files ${utest-client_moc_classes})

#configure_file( tree.xml  ${CMAKE_CURRENT_BINARY_DIR}/tree.xml  @ONLY )

list( APPEND utest-client_cflibs coolfluid_network)
list( APPEND utest-client_libs ${QT_LIBRARIES})
list( APPEND utest-client_files
  utest-client.cpp
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

list( APPEND utest-client_libs coolfluid_client_core ${QT_LIBRARIES})

coolfluid_add_unittest( utest-client )

add_custom_command(TARGET utest-client
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/tree.xml ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

else()
    coolfluid_mark_not_orphan(
        CommonFunctions.hpp
        NTreeTest.hpp
        NBrowserTest.hpp
        NLinkTest.hpp
        CNodeTest.hpp
        utest-client.cpp
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
endif()
