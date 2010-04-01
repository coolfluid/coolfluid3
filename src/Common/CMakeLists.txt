LIST ( APPEND Common_files 
   CommonAPI.hh
   DummyCommon.cxx 
   DummyCommon.hh 
   ExportAPI.hh
   MyInt.cxx
   MyInt.hh)

SET  ( Common_kernellib TRUE )

CF_ADD_LIBRARY ( Common )

ADD_SUBDIRECTORY(tests)
