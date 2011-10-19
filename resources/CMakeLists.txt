set( CF3_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Dir for resources" )

list( APPEND resources_files
   hextet.neu
   quadtriag.neu
   rotation-tg-p1.neu
   rotation-qd-p1.neu
   rectangle-tg-p1.msh
   rectangle-tg-p2.msh
   rectangle-qd-p1.msh
   rectangle-qd-p2.msh
   rectangle-mix-p1.msh
   rectangle-mix-p2.msh
   sinusbump-tg-p1.msh
)

coolfluid_add_resources( resources_files )
