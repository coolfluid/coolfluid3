# sunversion version check
FIND_PACKAGE(Subversion QUIET)
SET(Subversion_WC FALSE)

IF (Subversion_FOUND)
  
  EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} info ${coolfluid_SOURCE_DIR}
    OUTPUT_VARIABLE Subversion_WC_INFO
    ERROR_VARIABLE Subversion_svn_info_error
    RESULT_VARIABLE Subversion_svn_info_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)
    
  IF(${Subversion_svn_info_result} EQUAL 0)
    SET(Subversion_WC TRUE)
  ENDIF()
  
ENDIF ()

IF(Subversion_WC)
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

    ENDIF()
ELSE()
    
    # Check if this is a git repository, and get revision number through "git-svn"
    FIND_PACKAGE(Git QUIET)
    SET(Git_WC FALSE)
    
    IF(Git_FOUND)
        EXECUTE_PROCESS(COMMAND ${Git_EXECUTABLE} svn info
          WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
          OUTPUT_VARIABLE Git_svn_WC_INFO
          ERROR_VARIABLE Git_svn_info_error
          RESULT_VARIABLE Git_svn_info_result
          OUTPUT_STRIP_TRAILING_WHITESPACE)
        IF(${Git_svn_info_result} EQUAL 0)
          # This means it is a git repository
          EXECUTE_PROCESS(COMMAND ${Git_EXECUTABLE} svn find-rev HEAD^
              WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
              OUTPUT_VARIABLE coolfluid_svnversion
              OUTPUT_STRIP_TRAILING_WHITESPACE)
        ENDIF()
    ELSE()
        SET(coolfluid_svnversion "NOVERSION-FOUND")
        IF(Subversion_FOUND && Git_FOUND)
            LOG("svn or git command was found, but this is not a subversion or git repository.")
        ENDIF()
    ENDIF()
ENDIF()    

