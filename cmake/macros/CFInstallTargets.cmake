macro (coolfluid_install_targets)
    IF(CF_ENABLE_STATIC)
        # Skip installation of static libraries when we build statically
        FOREACH(T ${ARGN})
            GET_TARGET_PROPERTY(pType ${T} TYPE)
            IF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
                INSTALL(TARGETS ${T}
                    RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR}
                    BUNDLE  DESTINATION ${CF_INSTALL_BIN_DIR}
                    LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR}
                    ARCHIVE DESTINATION ${CF_INSTALL_LIB_DIR}
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                                GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                                WORLD_READ             WORLD_EXECUTE
                    CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
                )
            ENDIF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
        ENDFOREACH(T)
    ELSE(CF_ENABLE_STATIC)
        INSTALL(TARGETS ${ARGN}
            RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR}
            BUNDLE  DESTINATION ${CF_INSTALL_BIN_DIR}
            LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${CF_INSTALL_LIB_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                        GROUP_READ GROUP_WRITE GROUP_EXECUTE 
                        WORLD_READ             WORLD_EXECUTE
            CONFIGURATIONS "";None;Debug;Release;RelWithDebInfo;MinSizeRel
        )
        IF(APPLE)
            FOREACH(target ${ARGN})
            coolfluid_log (" +++ preparing install for: ${ARGN}")
            
                GET_TARGET_PROPERTY(type ${target} TYPE)
                IF(${type} STREQUAL "SHARED_LIBRARY")
                    SET(filename lib${target}.dylib)
                    INSTALL(CODE 
                        "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                             COMMAND /bin/sh ${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/osxfixup -lib \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CF_INSTALL_LIB_DIR}/${filename}\"
                             OUTPUT_VARIABLE OSXOUT)
                         MESSAGE(STATUS \"\${OSXOUT}\")
                        ")
                    coolfluid_log (" shared lib ")
                ELSEIF(${type} STREQUAL "EXECUTABLE")
                    GET_TARGET_PROPERTY(filename ${target} OUTPUT_NAME)
                    IF(filename STREQUAL "filename-NOTFOUND")
                        SET(filename ${target})
                    ENDIF(filename STREQUAL "filename-NOTFOUND")
                    GET_TARGET_PROPERTY(bundle ${target} MACOSX_BUNDLE)
                    IF(${bundle} STREQUAL "ON")
                        INSTALL(CODE 
                            "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                COMMAND /bin/sh ${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/osxfixup -bundle \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CF_INSTALL_BIN_DIR}\" ${filename}
                                OUTPUT_VARIABLE OSXOUT)
                             MESSAGE(STATUS \"\${OSXOUT}\")
                            ")
                        coolfluid_log (" bundle ")
                    ELSE(${bundle} STREQUAL "ON")
                        INSTALL(CODE 
                            "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                COMMAND /bin/sh${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/osxfixup -exe \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CF_INSTALL_BIN_DIR}/${filename}\"
                                OUTPUT_VARIABLE OSXOUT)
                             MESSAGE(STATUS \"\${OSXOUT}\")
                            ")
                        coolfluid_log (" executable ")
                        
                    ENDIF(${bundle} STREQUAL "ON")
                ENDIF(${type} STREQUAL "SHARED_LIBRARY")
            ENDFOREACH(target)
        ENDIF(APPLE)
    ENDIF(CF_ENABLE_STATIC)
endmacro()