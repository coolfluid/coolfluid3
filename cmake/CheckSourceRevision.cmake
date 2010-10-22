#########################################################################################
# subversion support

find_package( Subversion )

# check subversion version if .svn is present
if( EXISTS ${coolfluid_SOURCE_DIR}/.svn )

  set(Subversion_WC FALSE)

  if(Subversion_FOUND)
  
    EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} info ${coolfluid_SOURCE_DIR}
      OUTPUT_VARIABLE Subversion_WC_INFO
      ERROR_VARIABLE Subversion_svn_info_error
      RESULT_VARIABLE Subversion_svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    
    if(${Subversion_svn_info_result} EQUAL 0)
      set(Subversion_WC TRUE)
    endif()

  endif()

  if(Subversion_WC)

    Subversion_WC_INFO(${coolfluid_SOURCE_DIR} coolfluid)
    # message("Current revision is ${coolfluid_WC_REVISION}")
    # message("svn info : ${coolfluid_WC_INFO}")

    find_program(Subversion_SVNVERSION_EXECUTABLE svnversion DOC "subversion svnversion command line client")
    mark_as_advanced(Subversion_SVNVERSION_EXECUTABLE)

    if(Subversion_SVNVERSION_EXECUTABLE)
        EXECUTE_PROCESS(COMMAND ${Subversion_SVNVERSION_EXECUTABLE} -n ${coolfluid_SOURCE_DIR}
            WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
            OUTPUT_VARIABLE coolfluid_svnversion
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

  endif()

endif() # svn check

#########################################################################################
# git support

find_package(Git)

# check git svn version if .git is present
if( EXISTS ${coolfluid_SOURCE_DIR}/.git )
    
    # Check if this is a git repository, and get revision number through "git-svn"
    if(Git_FOUND)
        EXECUTE_PROCESS(COMMAND ${Git_EXECUTABLE} svn info
          WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
          OUTPUT_VARIABLE Git_svn_WC_INFO
          ERROR_VARIABLE Git_svn_info_error
          RESULT_VARIABLE Git_svn_info_result
          OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(${Git_svn_info_result} EQUAL 0)
          # This means it is a git repository
          EXECUTE_PROCESS(COMMAND ${Git_EXECUTABLE} svn find-rev HEAD^
              WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
              OUTPUT_VARIABLE coolfluid_svnversion
              OUTPUT_STRIP_TRAILING_WHITESPACE)
        endif()

coolfluid_debug_var(Git_svn_WC_INFO)
coolfluid_debug_var(Git_svn_info_result)
coolfluid_debug_var(coolfluid_svnversion)

    endif()
endif()  # git check

# finally set version
if( NOT coolfluid_svnversion )
  set(coolfluid_svnversion "NOVERSION-FOUND")
endif()
