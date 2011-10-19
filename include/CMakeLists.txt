add_subdirectory(rapidxml)
add_subdirectory(fparser)
add_subdirectory(Eigen)

if( QT4_FOUND AND CF3_ENABLE_GUI )
  add_subdirectory(qwt)
endif()
