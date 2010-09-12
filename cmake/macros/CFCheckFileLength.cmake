##############################################################################
# this macro separates the sources form the headers
##############################################################################
macro( coolfluid_check_file_length SOURCE_FILE )

 string(LENGTH ${SOURCE_FILE} FILENAME_LENGTH)
  if(FILENAME_LENGTH GREATER 90)
    MESSAGE( FATAL_ERROR "The filename [${SOURCE_FILE}] is too long (${FILENAME_LENGTH} chars).\nFilenames longer than 90 characters can cause portability issues with some versions of tar.")
  endif(FILENAME_LENGTH GREATER 90)

endmacro( coolfluid_check_file_length )
##############################################################################

