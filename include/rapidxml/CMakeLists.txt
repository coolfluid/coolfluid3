list( APPEND rapidxml_files
  rapidxml.hpp
  rapidxml_utils.hpp
  rapidxml_print.hpp
  rapidxml_iterators.hpp
  )

source_group( Headers FILES ${rapidxml_files} )

list( APPEND utest-rapidxml_files ${rapidxml_files} rapidxml.cpp )

# install rapidxml headers since tests dont install sources
install( FILES ${rapidxml_files}
         DESTINATION
         ${CF_INSTALL_INCLUDE_DIR}/rapidxml )

coolfluid_add_unittest( utest-rapidxml )
