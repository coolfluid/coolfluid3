
# check git version if .git is present
if( EXISTS ${coolfluid_SOURCE_DIR}/.git )

  find_package(Git QUIET) # no need to inform user about this
  coolfluid_set_package(PACKAGE Git DESCRIPTION "Git version control" QUIET)

  # Check if this is a git repository, and get revision information
  if(GIT_FOUND)

    # Get abbreviated commit sha
    execute_process( COMMAND           "${GIT_EXECUTABLE}" rev-parse --short HEAD
                     WORKING_DIRECTORY "${coolfluid_SOURCE_DIR}"
                     RESULT_VARIABLE   GIT_RESULT
                     OUTPUT_VARIABLE   coolfluid_git_revision_sha
                     ERROR_VARIABLE    GIT_ERROR
                     OUTPUT_STRIP_TRAILING_WHITESPACE )

    # Get commit date
    execute_process( COMMAND           "${GIT_EXECUTABLE}" diff-tree -s --format=%ci HEAD
                     WORKING_DIRECTORY "${coolfluid_SOURCE_DIR}"
                     RESULT_VARIABLE   GIT_RESULT
                     OUTPUT_VARIABLE   coolfluid_git_revision_date
                     ERROR_VARIABLE    GIT_ERROR
                     OUTPUT_STRIP_TRAILING_WHITESPACE )

  endif(GIT_FOUND)

endif()

#########################################################################################
# finally set version

if( NOT coolfluid_git_revision_sha )
  set(coolfluid_git_revision_sha "NOTFOUND")
  set(coolfluid_git_revision_date "NOTFOUND")
endif()

