macro( coolfluid_get_date RESULT )

if(WIN32)
  execute_process(COMMAND "date" "/T" OUTPUT_VARIABLE ${RESULT})
  string(REGEX REPLACE "(..)/(..)/(....).*" "\\3/\\1/\\2" ${RESULT} ${${RESULT}})
elseif(UNIX)
  execute_process(COMMAND "date" "+%m/%d/%Y" OUTPUT_VARIABLE ${RESULT})
  string(REGEX REPLACE "(..)/(..)/(....).*" "\\1/\\2/\\3" ${RESULT} ${${RESULT}})
else()
  message(SEND_ERROR "date not implemented")
  set(${RESULT} 000000)
endif()

endmacro()

