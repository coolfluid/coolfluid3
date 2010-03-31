# sunversion version check
FIND_PACKAGE(Subversion)

IF (Subversion_FOUND)

  Subversion_WC_INFO(${coolfluid_SOURCE_DIR} coolfluid)
  # MESSAGE("Current revision is ${coolfluid_WC_REVISION}")
  # MESSAGE("svn info : ${coolfluid_WC_INFO}")

  FIND_PROGRAM(Subversion_SVNVERSION_EXECUTABLE svnversion DOC "subversion svnversion command line client")
  MARK_AS_ADVANCED(Subversion_SVNVERSION_EXECUTABLE)

  IF(Subversion_SVNVERSION_EXECUTABLE)

  EXECUTE_PROCESS(COMMAND ${Subversion_SVNVERSION_EXECUTABLE} -n ${coolfluid_SOURCE_DIR}
      WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
      OUTPUT_VARIABLE coolfluid_svnversion
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  ELSE()

    LOG("Subversion svn command was found, but not svnversion.")
    SET(coolfluid_svnversion "NOVERSION-FOUND")

  ENDIF()

ELSE ()

  SET(coolfluid_svnversion "NOVERSION-FOUND")

ENDIF ()
