add_subdirectory(rapidxml)
add_subdirectory(fparser)
add_subdirectory(Eigen)

# boost/numeric/quadrature library, not in official boost release
# but in the boost-vault at https://github.com/boost-vault
add_subdirectory(boost)  

if( QT4_FOUND AND CF3_ENABLE_GUI )
  add_subdirectory(qwt)
endif()
