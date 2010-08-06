#ifndef CF_GUI_Client_uTests_ExceptionThrowHandler_hpp
#define CF_GUI_Client_uTests_ExceptionThrowHandler_hpp

// Checks whether a code throws a specified exception.
// If the exception was not thrown or another was, QFAIL() is called
#define GUI_CHECK_THROW( instr, the_exception ) try \
{ \
  instr; \
  QFAIL("Exception " #the_exception " was never thrown."); \
} \
catch ( the_exception & e ) \
{ } \
catch ( CF::Common::Exception & e) \
{ \
  QFAIL(#the_exception " expected but another CF exception was thrown.\n" e.what()); \
} \
catch ( ... ) \
{ \
  QFAIL(#the_exception " expected but another one was thrown."); \
}

// Checks whether a code never throws any exception.
// If an exception was thrown, QFAIL() is called
#define GUI_CHECK_NO_THROW( instr ) try \
{ \
  instr; \
} \
catch ( CF::Common::Exception & e) \
{ \
  QFAIL( " expected but another CF exception was thrown.\n" e.what()); \
} \
catch ( ... ) \
{ \
  QFAIL("No exception expected but an unknown exception was thrown."); \
}

#endif // CF_GUI_Client_uTests_ExceptionThrowHandler_hpp
