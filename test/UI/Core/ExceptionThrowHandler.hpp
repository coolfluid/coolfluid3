// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
catch ( CF::Common::Exception ex) \
{ \
  QFAIL(QString(#the_exception " expected but another CF exception was "\
                "thrown.\n%1").arg(ex.what()).toStdString().c_str()); \
  return;\
} \
catch ( std::exception stde) \
{ \
  QFAIL(QString(#the_exception " expected but a std exception was "\
                "thrown.\n%1").arg(stde.what()).toStdString().c_str()); \
  return;\
} \
catch ( ... ) \
{ \
  QFAIL(#the_exception " expected but another one was thrown."); \
  return;\
}

// Checks whether a code never throws any exception.
// If an exception was thrown, QFAIL() is called
#define GUI_CHECK_NO_THROW( instr ) try \
{ \
  instr; \
} \
catch ( CF::Common::Exception ex) \
{ \
  QFAIL(QString("No exception expected but a CF exception was "\
                "thrown.\n%1").arg(ex.what()).toStdString().c_str()); \
  return;\
} \
catch ( std::exception stde) \
{ \
  QFAIL(QString("No exception expected but a std exception was "\
                "thrown.\n%1").arg(stde.what()).toStdString().c_str()); \
  return;\
} \
catch ( ... ) \
{ \
  QFAIL("No exception expected but an unknown exception was thrown."); \
  return;\
}

#endif // CF_GUI_Client_uTests_ExceptionThrowHandler_hpp
