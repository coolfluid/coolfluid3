################################################################################
# Test Client core

if( QT4_FOUND AND CF_ENABLE_GUI )

set(utest-client-ui_moc_classes
  GraphicalBoolTest.hpp
  GraphicalDoubleTest.hpp
  GraphicalIntTest.hpp
  GraphicalStringTest.hpp
  GraphicalUintTest.hpp
  GraphicalUriTest.hpp
  GraphicalValueTest.hpp
)

QT4_WRAP_CPP(utest-client-ui_moc_files ${utest-client-ui_moc_classes})

list( APPEND utest-client-ui_libs ${QT_LIBRARIES})
list( APPEND utest-client-ui_files
  utest-client-ui.cpp
  GraphicalBoolTest.cpp
  GraphicalBoolTest.hpp
  GraphicalDoubleTest.cpp
  GraphicalDoubleTest.hpp
  GraphicalIntTest.cpp
  GraphicalIntTest.hpp
  GraphicalStringTest.cpp
  GraphicalStringTest.hpp
  GraphicalUintTest.cpp
  GraphicalUintTest.hpp
  GraphicalUriTest.cpp
  GraphicalUriTest.hpp
  GraphicalValueTest.cpp
  GraphicalValueTest.hpp
)

list( APPEND utest-client-ui_libs coolfluid_client_ui ${QT_LIBRARIES})

coolfluid_add_unit_test( utest-client-ui )

else()

coolfluid_mark_not_orphan(
  utest-client-ui.cpp
  GraphicalBoolTest.cpp
  GraphicalBoolTest.hpp
  GraphicalDoubleTest.cpp
  GraphicalDoubleTest.hpp
  GraphicalIntTest.cpp
  GraphicalIntTest.hpp
  GraphicalStringTest.cpp
  GraphicalStringTest.hpp
  GraphicalUintTest.cpp
  GraphicalUintTest.hpp
  GraphicalUriTest.cpp
  GraphicalUriTest.hpp
  GraphicalValueTest.cpp
  GraphicalValueTest.hpp
)

endif()
