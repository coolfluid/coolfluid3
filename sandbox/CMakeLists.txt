CF_FIND_ORPHAN_FILES()

###############################################################################
### test for rapidxml

LIST ( APPEND rapidxml_files rapidxml.cpp )
CF_ADD_APP ( rapidxml )

###############################################################################
### test for xmlnode

add_definitions ( -DXMLDLLENTRY )
list ( APPEND xmlnode_files xmlnode.cpp xmlParser.h xmlParser.cpp )
CF_ADD_APP ( xmlnode )

###############################################################################
### test for boost::lambda

list ( APPEND lambda_libs ${Boost_LIBRARIES} )
list ( APPEND lambda_files lambda.cpp )
CF_ADD_APP ( lambda )

###############################################################################
### test for boost::multi_array

add_definitions ( -DNDEBUG )
list ( APPEND multiarray_libs ${Boost_LIBRARIES} )
list ( APPEND multiarray_files multiarray.cpp )
CF_ADD_APP ( multiarray )
