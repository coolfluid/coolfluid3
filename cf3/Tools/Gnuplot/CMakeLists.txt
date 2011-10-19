list( APPEND coolfluid_tools_gnuplot_files
  Gnuplot.hpp
  Gnuplot.cpp
)

list( APPEND coolfluid_tools_gnuplot_cflibs   coolfluid_common )
coolfluid_add_library( coolfluid_tools_gnuplot )
