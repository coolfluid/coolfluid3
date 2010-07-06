CF_FIND_ORPHAN_FILES()

###############################################################################
### test for rapidxml

LIST ( APPEND logWithProgress_files
   Log.cpp
   Log.hpp
   LogLevel.hpp
   LogLevelFilter.cpp
   LogLevelFilter.hpp
   LogStampFilter.cpp
   LogStampFilter.hpp
   LogStream.cpp
   LogStream.hpp
   LogStringForwarder.cpp
   LogStringForwarder.hpp
   main.cpp )
LIST ( APPEND logWithProgress_libs Common ${BOOST_LIBRARIES} )
CF_ADD_APP ( logWithProgress )

