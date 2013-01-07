list( APPEND coolfluid_tools_gnuplot_files
  Gnuplot.hpp
  Gnuplot.cpp
)

coolfluid3_add_library( TARGET  coolfluid_tools_gnuplot
                        KERNEL
                        SOURCES ${coolfluid_tools_gnuplot_files}
                        LIBS    coolfluid_common )
