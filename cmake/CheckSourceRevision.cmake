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
            OUTPUT_VARIABLE coolfluid_svn_revision
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

  endif()

endif() # svn check

#########################################################################################
# git support ( if cmake has it )

if( COMMAND FindGit )

  find_package(Git)

  # check git svn version if .git is present
  if( EXISTS ${coolfluid_SOURCE_DIR}/.git )
    
    # Check if this is a git repository, and get revision number through "git-svn"
    if(GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} svn info
          WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
          OUTPUT_VARIABLE GIT_svn_WC_INFO
          ERROR_VARIABLE GIT_svn_info_error
          RESULT_VARIABLE GIT_svn_info_result
          OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(${GIT_svn_info_result} EQUAL 0)
          # This means it is a git repository
          execute_process(COMMAND ${GIT_EXECUTABLE} svn find-rev HEAD^
              WORKING_DIRECTORY ${coolfluid_SOURCE_DIR}
              OUTPUT_VARIABLE coolfluid_svn_revision
              OUTPUT_STRIP_TRAILING_WHITESPACE)
        endif()

    endif()

  endif()  # git check

endif() # FindGit

#########################################################################################
# finally set version

if( NOT coolfluid_svn_revision )
  set(coolfluid_svn_revision "CF-REVISION-NOTFOUND")
endif()

