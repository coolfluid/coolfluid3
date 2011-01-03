list( APPEND tools_files
     install-deps.pl
     search-source.sh
     replace-source.sh
     test-mpi-scalability.py
     cmake-win32.bat
     port-to-k3.pl
   )

add_custom_target( tools SOURCES ${tools_files} )
