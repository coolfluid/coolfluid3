################################################################################
# Test Client core

set(utest-client-ui_moc_classes
  GraphicalBoolTest.hpp
  GraphicalStringTest.hpp
  GraphicalValueTest.hpp
)

QT4_WRAP_CPP(utest-client-ui_moc_files ${utest-client-ui_moc_classes})

list( APPEND utest-client-ui_libs ${QT_LIBRARIES})
list( APPEND utest-client-ui_files
  utest-client-ui.cpp
  GraphicalBoolTest.cpp
  GraphicalBoolTest.hpp
  GraphicalStringTest.cpp
  GraphicalStringTest.hpp
  GraphicalValueTest.cpp
  GraphicalValueTest.hpp
)

list( APPEND utest-client-ui_libs coolfluid_client_ui ${QT_LIBRARIES})

coolfluid_add_unit_test( utest-client-ui )
