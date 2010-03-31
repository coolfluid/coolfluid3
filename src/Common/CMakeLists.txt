LIST ( APPEND Common_files a.hh a.cxx b.h b.cpp DummyCommon.hh DummyCommon.cxx ExportAPI.hh CommonAPI.hh)

SET  ( Common_kernellib TRUE )

CF_ADD_LIBRARY ( Common )

