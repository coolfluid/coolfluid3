list( APPEND utest-rapidxml_files
  rapidxml.hpp
  rapidxml_utils.hpp
  rapidxml_print.hpp
  rapidxml_iterators.hpp
  rapidxml.cpp
  )

# install rapidxml headers since tests dont install sources
install( FILES ${rapidxml_files}
         DESTINATION
         ${CF_INSTALL_INCLUDE_DIR}/rapidxml )

coolfluid_add_unittest( utest-rapidxml )
