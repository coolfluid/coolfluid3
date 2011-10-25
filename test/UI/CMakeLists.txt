  coolfluid_find_orphan_files()

  # deactivate some c++ warnings in Qt
  if( CMAKE_COMPILER_IS_GNUCC )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-sign-promo" )
  endif()

  # Add useful macros and variables
  include(${QT_USE_FILE})

  coolfluid_add_test( UTEST      utest-ui-core-cnode
                      CPP        utest-ui-core-cnode.cpp CoreApplication.hpp ExceptionThrowHandler.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  coolfluid_add_test( UTEST      utest-ui-core-commit-details
                      CPP        utest-ui-core-commit-details.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  coolfluid_add_test( UTEST      utest-ui-core-nbrowser
                      CPP        utest-ui-core-nbrowser.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  coolfluid_add_test( UTEST      utest-ui-core-nlink
                      CPP        utest-ui-core-nlink.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  coolfluid_add_test( UTEST      utest-ui-core-nroot
                      CPP        utest-ui-core-nroot.cpp CoreApplication.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  coolfluid_add_test( UTEST      utest-ui-core-ntree
                      CPP        utest-ui-core-ntree.cpp CoreApplication.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  coolfluid_add_test( UTEST      utest-ui-core-property-model
                      CPP        utest-ui-core-property-model.cpp CoreApplication.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  coolfluid_add_test( UTEST      utest-ui-core-tree-node
                      CPP        utest-ui-core-tree-node.cpp CoreApplication.hpp
                                 MyNode.cpp MyNode.hpp
                      LIBS       coolfluid_ui_core
                      CONDITION  CF3_ENABLE_GUI)

  add_custom_command( TARGET utest-ui-core-cnode
                      POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/tree.xml ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} )

  add_subdirectory( Graphics )

