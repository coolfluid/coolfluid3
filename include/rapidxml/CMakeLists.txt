list ( APPEND rapidxml_files 
  rapidxml.hpp
  rapidxml_utils.hpp
  rapidxml_print.hpp
  rapidxml_iterators.hpp 
  )

# install rapidxml headers
install ( FILES ${rapidxml_files}
          DESTINATION
                ${CF_INSTALL_INCLUDE_DIR}/rapidxml )


