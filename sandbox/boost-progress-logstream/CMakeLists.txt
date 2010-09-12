coolfluid_find_orphan_files()

list( APPEND boost-progress-logstream_files
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
list( APPEND boost-progress-logstream_libs Common ${BOOST_LIBRARIES} )
coolfluid_add_application( boost-progress-logstream )

