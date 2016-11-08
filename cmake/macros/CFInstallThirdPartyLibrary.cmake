#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#   Kathleen Bonnell, Wed Feb  3 17:25:42 PST 2010
#   Add simplifed version for windows.
#
#   Eric Brugger, Mon Mar  1 16:25:36 PST 2010
#   I modified the test to determine if a library was a shared library for
#   AIX since on AIX shared libraries can end in ".a".
#
#   Eric Brugger, Fri Mar 12 16:53:54 PST 2010
#   I corrected a typo I made that prevented archives from being included
#   in a binary distribution when VISIT_INSTALL_THIRD_PARTY was defined.
#
#****************************************************************************/

#
# This function installs a library and any of its needed symlink variants.
#

function ( coolfluid_install_third_party_library LIBFILE )
  IF(WIN32)
        IF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
            FILE(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        ENDIF(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)

        SET(tmpLIBFILE ${LIBFILE})
        GET_FILENAME_COMPONENT(LIBREALPATH ${tmpLIBFILE} REALPATH)
        GET_FILENAME_COMPONENT(curPATH ${LIBREALPATH} PATH)
        GET_FILENAME_COMPONENT(curNAMEWE ${LIBREALPATH} NAME_WE)
        SET(curNAME "${curPATH}/${curNAMEWE}")
        SET(dllNAME "${curNAME}.dll")
        SET(libNAME "${curNAME}.lib")
        IF(EXISTS ${dllNAME})
            INSTALL(FILES ${dllNAME}
                DESTINATION ${CF3_INSTALL_BIN_DIR}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
                )
            # On Windows, we also need to copy the file to the
            # binary dir so our out of source builds can run.
            EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy
                            ${dllNAME}
                            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ThirdParty)
        ENDIF(EXISTS ${dllNAME})

        IF(EXISTS ${libNAME})
            # also install the import libraries
            INSTALL(FILES ${libNAME}
                DESTINATION ${CF3_INSTALL_LIB_DIR}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
                )
        ENDIF(EXISTS ${libNAME})

  ELSE(WIN32)

    SET(tmpLIBFILE ${LIBFILE})
    GET_FILENAME_COMPONENT(LIBEXT ${tmpLIBFILE} EXT)
    IF(NOT ${LIBEXT} STREQUAL ".a")
        SET(isSHAREDLIBRARY "YES")
    ELSE(NOT ${LIBEXT} STREQUAL ".a")
        SET(isSHAREDLIBRARY "NO")
    ENDIF(NOT ${LIBEXT} STREQUAL ".a")
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
        GET_FILENAME_COMPONENT(baseNAME ${tmpLIBFILE} NAME_WE)
        # On AIX all ".a" files are archives except the following.
        IF((${baseNAME} STREQUAL "libpython2") OR
           (${baseNAME} STREQUAL "libMesaGL") OR
           (${baseNAME} STREQUAL "libOSMesa") OR
           (${baseNAME} STREQUAL "libsz"))
            SET(isSHAREDLIBRARY "YES")
        ENDIF((${baseNAME} STREQUAL "libpython2") OR
              (${baseNAME} STREQUAL "libMesaGL") OR
              (${baseNAME} STREQUAL "libOSMesa") OR
              (${baseNAME} STREQUAL "libsz"))
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")

    IF(${isSHAREDLIBRARY} STREQUAL "YES")
        GET_FILENAME_COMPONENT(LIBREALPATH ${tmpLIBFILE} REALPATH)
       # coolfluid_log("***tmpLIBFILE=${tmpLIBFILE}, LIBREALPATH=${LIBREALPATH}")
        IF(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            # We need to install a library and its symlinks
            GET_FILENAME_COMPONENT(curPATH ${LIBREALPATH} PATH)

            set( TP_SYSTEM_PATHS_LINUX "/lib" "/lib64" "/usr/lib" "/usr/lib64" )
            list( FIND TP_SYSTEM_PATHS_LINUX ${curPATH} TP_PATH_INDEX )

            IF((TP_PATH_INDEX EQUAL -1) AND (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*") AND  (NOT ${curPATH} MATCHES "^\\/Library\\/Frameworks\\/.*"))
                GET_FILENAME_COMPONENT(curNAMEWE ${LIBREALPATH} NAME_WE)
                GET_FILENAME_COMPONENT(curEXT ${LIBREALPATH} EXT)
                STRING(REPLACE "." ";" extList ${curEXT})
                SET(curNAME "${curPATH}/${curNAMEWE}")
                # Come up with all of the possible library and symlink names
                SET(allNAMES "${curNAME}${LIBEXT}")
                SET(allNAMES ${allNAMES} "${curNAME}.a")
                FOREACH(X ${extList})
                    SET(curNAME "${curNAME}.${X}")
                    SET(allNAMES ${allNAMES} "${curNAME}")           # Linux way
                    SET(allNAMES ${allNAMES} "${curNAME}${LIBEXT}")  # Mac way
                ENDFOREACH(X)

                LIST(REMOVE_DUPLICATES allNAMES)

                # Add the names that exist to the install.
                FOREACH(curNAMEWithExt ${allNAMES})
                    IF(EXISTS ${curNAMEWithExt})
#                        coolfluid_log("** Need to install ${curNAMEWithExt}")
                        IF(IS_DIRECTORY ${curNAMEWithExt})
                            # It is a framework, install as a directory
                            INSTALL(DIRECTORY ${curNAMEWithExt}
                                DESTINATION ${CF3_INSTALL_LIB_DIR}
                                DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
                            )

                            # On Apple, we need to make the framework be executable relative
                            GET_FILENAME_COMPONENT(frameworkNameWE ${curNAMEWithExt} NAME_WE)
                            GET_FILENAME_COMPONENT(realFramework ${curNAMEWithExt}/${frameworkNameWE} REALPATH)
                            STRING(REGEX MATCH "${frameworkNameWE}[A-Za-z0-9._/-]*" frameworkMatch ${realFramework})
                            INSTALL(CODE
                                "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                    COMMAND /bin/sh ${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/osxfixup -lib \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CF3_INSTALL_LIB_DIR}/${frameworkMatch}\"
                                    OUTPUT_VARIABLE OSXOUT)
                                 MESSAGE(STATUS \"\${OSXOUT}\")
                                ")
                        ELSE(IS_DIRECTORY ${curNAMEWithExt})
                            INSTALL(FILES ${curNAMEWithExt}
                                DESTINATION ${CF3_INSTALL_LIB_DIR}
                                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                                CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
                            )

                            # On Apple, we need to make the library be executable relative.
                            IF(APPLE)
                                GET_FILENAME_COMPONENT(libName ${curNAMEWithExt} NAME)
                                INSTALL(CODE
                                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                                        COMMAND /bin/sh ${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/osxfixup -lib \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CF3_INSTALL_LIB_DIR}/${libName}\"
                                        OUTPUT_VARIABLE OSXOUT)
                                     MESSAGE(STATUS \"\${OSXOUT}\")
                                    ")
                            ENDIF(APPLE)
                        ENDIF(IS_DIRECTORY ${curNAMEWithExt})
                    ENDIF(EXISTS ${curNAMEWithExt})
                ENDFOREACH(curNAMEWithExt)
            # ENDIF((NOT ${curPATH} STREQUAL "/usr/lib") AND (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*"))
            ENDIF((TP_PATH_INDEX EQUAL -1) AND (NOT ${curPATH} MATCHES "^\\/System\\/Library\\/Frameworks\\/.*") AND  (NOT ${curPATH} MATCHES "^\\/Library\\/Frameworks\\/.*"))
        ELSE(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
            # We need to install just the library
            IF(IS_DIRECTORY ${tmpLIBFILE})
                # It is a framework, install as a directory.
                INSTALL(DIRECTORY ${tmpLIBFILE}
                    DESTINATION ${CF3_INSTALL_LIB_DIR}
                    DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
                    PATTERN "Qt*_debug" EXCLUDE # Exclude Qt*_debug libraries in framework.
                )

                # On Apple, we need to make the framework be executable relative
                GET_FILENAME_COMPONENT(frameworkNameWE ${tmpLIBFILE} NAME_WE)
                GET_FILENAME_COMPONENT(realFramework ${tmpLIBFILE}/${frameworkNameWE} REALPATH)
                STRING(REGEX MATCH "${frameworkNameWE}[A-Za-z0-9._/-]*" frameworkMatch ${realFramework})
                INSTALL(CODE
                    "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                        COMMAND /bin/sh ${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/osxfixup -lib \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CF3_INSTALL_LIB_DIR}/${frameworkMatch}\"
                        OUTPUT_VARIABLE OSXOUT)
                     MESSAGE(STATUS \"\${OSXOUT}\")
                    ")
            ELSE(IS_DIRECTORY ${tmpLIBFILE})
                # Create an install target for just the library file
                INSTALL(FILES ${tmpLIBFILE}
                    DESTINATION ${CF3_INSTALL_LIB_DIR}
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
                )

                # On Apple, we need to make the library be executable relative.
                IF(APPLE)
                    GET_FILENAME_COMPONENT(libName ${tmpLIBFILE} NAME)
                    INSTALL(CODE
                        "EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                            COMMAND /bin/sh ${coolfluid_SOURCE_DIR}/tools/MacOSX_Bundle/osxfixup -lib \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CF3_INSTALL_LIB_DIR}/${libName}\"
                            OUTPUT_VARIABLE OSXOUT)
                         MESSAGE(STATUS \"\${OSXOUT}\")
                        ")
                ENDIF(APPLE)
            ENDIF(IS_DIRECTORY ${tmpLIBFILE})
            coolfluid_log_file("**We need to install lib ${tmpLIBFILE}")
        ENDIF(NOT ${tmpLIBFILE} STREQUAL ${LIBREALPATH})
    ELSE(${isSHAREDLIBRARY} STREQUAL "YES")
        # We have a .a that we need to install to archives.
        # IF(VISIT_INSTALL_THIRD_PARTY)
#            coolfluid_log("***INSTALL ${LIBFILE} to ${CF3_INSTALL_ARCHIVE_DIR}")
            INSTALL(FILES ${tmpLIBFILE}
                DESTINATION ${CF3_INSTALL_ARCHIVE_DIR}
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
            )

            # TODO: We could install windows import libraries here...

        # ENDIF(VISIT_INSTALL_THIRD_PARTY)
    ENDIF(${isSHAREDLIBRARY} STREQUAL "YES")
  ENDIF(WIN32)
endfunction (coolfluid_install_third_party_library)

#
# This function installs a library's includes.
#

function ( coolfluid_install_third_party_include  pkg incdir)
#        IF(VISIT_INSTALL_THIRD_PARTY)
            STRING(TOLOWER ${pkg} lcpkg)
            coolfluid_log("***INSTALL ${incdir} -> ${CF3_INSTALL_INCLUDE_DIR}/${lcpkg}")
            INSTALL(DIRECTORY ${incdir}
                DESTINATION ${CF3_INSTALL_INCLUDE_DIR}/${lcpkg}
                DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                COMPONENT headers
                CONFIGURATIONS "" ;None;Debug;Release;RelWithDebInfo;MinSizeRel
                FILES_MATCHING
                PATTERN "*.h"
                PATTERN "*.hh"
                PATTERN "*.H"
                PATTERN "*.hpp"
                PATTERN "*.HPP"
                PATTERN "*.inc"
                PATTERN "libccmio" EXCLUDE
                PATTERN ".svn" EXCLUDE
            )
 #       ENDIF(VISIT_INSTALL_THIRD_PARTY)
endfunction(coolfluid_install_third_party_include)
