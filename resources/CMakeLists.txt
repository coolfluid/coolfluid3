set( CF_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Dir for resources" )

list( APPEND resources_files
   hextet.neu
   quadtriag.neu
   rotation-qd.neu
   rotation-tg.neu
   )

add_custom_target( resources SOURCES ${resources_files} )
