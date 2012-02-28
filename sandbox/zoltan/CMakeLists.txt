if( ZOLTAN_FOUND )

	coolfluid_find_orphan_files()

	list( APPEND sandbox-zoltan_files graph_partitioning.cpp )

	list( APPEND sandbox-zoltan_includedirs ${ZOLTAN_INCLUDE_DIR} ${ZOLTAN_EXTRA_INCLUDE_DIR} )
	list( APPEND sandbox-zoltan_libs ${ZOLTAN_LIBRARIES} ${ZOLTAN_EXTRA_LIBRARIES})
	list( APPEND sandbox-zoltan_cflibs coolfluid_common)

	set( sandbox-zoltan_sandbox_app TRUE )
	set( sandbox-zoltan_condition ${CF_HAVE_ZOLTAN} )

	coolfluid_add_application( sandbox-zoltan )

endif()