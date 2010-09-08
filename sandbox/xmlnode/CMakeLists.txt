add_definitions( -DXMLDLLENTRY )
list( APPEND xmlnode_files xmlnode.cpp xmlParser.h xmlParser.cpp )

CF_ADD_APP( xmlnode )


