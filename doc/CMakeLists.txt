if( CF_ENABLE_DOCS )

FIND_PACKAGE(Doxygen)

if( DOXYGEN_FOUND )

  set( DOXYFILE doxygen.config CACHE INTERNAL "Doxygen filename" )

  # click+jump in Emacs and Visual Studio (for ${DOXYFILE}) (jw)
  IF    (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv)")
    set(DOXY_WARN_FORMAT "\"$file($line) : $text \"")
  ELSE  (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv)")
    set(DOXY_WARN_FORMAT "\"$file:$line: $text \"")
  endif(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv)")
  
  # we need latex for doxygen because of the formulas
  FIND_PACKAGE(LATEX)
  IF    (NOT LATEX_COMPILER)
    MESSAGE(STATUS "latex command LATEX_COMPILER not found but usually required. You will probably get warnings and user inetraction on doxy run.")
  endif(NOT LATEX_COMPILER)
  IF    (NOT MAKEINDEX_COMPILER)
    MESSAGE(STATUS "makeindex command MAKEINDEX_COMPILER not found but usually required.")
  endif(NOT MAKEINDEX_COMPILER)
  IF    (NOT DVIPS_CONVERTER)
    MESSAGE(STATUS "dvips command DVIPS_CONVERTER not found but usually required.")
  endif(NOT DVIPS_CONVERTER)
  
  IF   (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in")
    MESSAGE(STATUS "configured ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in --> ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in
      ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
      @ONLY )
    # use (configured) ${DOXYFILE} from (out of place) BUILD tree:
    set(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}")
  ELSE (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in")
    # use static hand-edited ${DOXYFILE} from SOURCE tree:
    set(DOXY_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}")
    IF   (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}")
      MESSAGE(STATUS "WARNING: using existing ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE} instead of configuring from ${DOXYFILE}.in file.")
    ELSE (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}")
      IF   (EXISTS "${CMAKE_MODULE_PATH}/${DOXYFILE}.in")
        # using template ${DOXYFILE}.in
        MESSAGE(STATUS "configured ${CMAKE_CMAKE_MODULE_PATH}/${DOXYFILE}.in --> ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}")
        configure_file(${CMAKE_MODULE_PATH}/${DOXYFILE}.in
          ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}
          @ONLY )
        set(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}")
      ELSE (EXISTS "${CMAKE_MODULE_PATH}/${DOXYFILE}.in")
        # failed completely...
        MESSAGE(SEND_ERROR "Please create ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in (or ${DOXYFILE} as fallback)")
      endif(EXISTS "${CMAKE_MODULE_PATH}/${DOXYFILE}.in")

    endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}")
  endif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in")
  
  ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})
  
  # create a windows help .chm file using hhc.exe
  # HTMLHelp DLL must be in path!
  # fallback: use hhw.exe interactively
  IF    (WIN32)
    FIND_PACKAGE(HTMLHelp)
    IF   (HTML_HELP_COMPILER)      
      set(TMP "${CMAKE_CURRENT_BINARY_DIR}\\Doc\\html\\index.hhp")
     string(REGEX REPLACE "[/]" "\\\\" HHP_FILE ${TMP} )
      # MESSAGE(SEND_ERROR "DBG  HHP_FILE=${HHP_FILE}")
      ADD_CUSTOM_TARGET(winhelp ${HTML_HELP_COMPILER} ${HHP_FILE})
      ADD_DEPENDENCIES (winhelp doc)
     
      if(NOT TARGET_DOC_SKIP_INSTALL)
      # install windows help?
      # determine useful name for output file 
      # should be project and version unique to allow installing 
      # multiple projects into one global directory      
      IF   (EXISTS "${PROJECT_BINARY_DIR}/Doc/html/index.chm")
        IF   (PROJECT_NAME)
          set(OUT "${PROJECT_NAME}")
        ELSE (PROJECT_NAME)
          set(OUT "Documentation") # default
        endif(PROJECT_NAME)
        IF   (${PROJECT_NAME}_VERSION_MAJOR)
          set(OUT "${OUT}-${${PROJECT_NAME}_VERSION_MAJOR}")
          IF   (${PROJECT_NAME}_VERSION_MINOR)
            set(OUT  "${OUT}.${${PROJECT_NAME}_VERSION_MINOR}")
            IF   (${PROJECT_NAME}_VERSION_PATCH)
              set(OUT "${OUT}.${${PROJECT_NAME}_VERSION_PATCH}")      
            endif(${PROJECT_NAME}_VERSION_PATCH)
          endif(${PROJECT_NAME}_VERSION_MINOR)
        endif(${PROJECT_NAME}_VERSION_MAJOR)
        # keep suffix
        set(OUT  "${OUT}.chm")
        
        #MESSAGE("DBG ${PROJECT_BINARY_DIR}/Doc/html/index.chm \n${OUT}")
        # create target used by install and package commands 
        INSTALL(FILES "${PROJECT_BINARY_DIR}/Doc/html/index.chm"
          DESTINATION "doc"
          RENAME "${OUT}"
        )
      endif(EXISTS "${PROJECT_BINARY_DIR}/Doc/html/index.chm")
      endif(NOT TARGET_DOC_SKIP_INSTALL)

    endif(HTML_HELP_COMPILER)
    # MESSAGE(SEND_ERROR "HTML_HELP_COMPILER=${HTML_HELP_COMPILER}")
  endif(WIN32) 
endif(DOXYGEN_FOUND)

endif( )