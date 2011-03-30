// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Basics_hpp
#define CF_Common_Basics_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when an assertion fails but the code is configured to throw an exception rather than crash.
/// @author Tiago Quintino
struct Common_API FailedAssertion : public Common::Exception {

  /// Constructor
  FailedAssertion (const Common::CodeLocation& where, const std::string& what);

  virtual ~FailedAssertion() throw();

}; // end FailedAssertion

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when user provides a bad value input
/// @author Tiago Quintino
struct Common_API BadValue: public Common::Exception {

  /// Constructor
  BadValue( const Common::CodeLocation& where, const std::string& what);

  virtual ~BadValue() throw();

}; //  BadValue

////////////////////////////////////////////////////////////////////////////////

///  Exception thrown when a dynamic cast of a pointer fails.
/// @author Tiago Quintino
struct Common_API CastingFailed: public Common::Exception {

  /// Constructor
  CastingFailed( const Common::CodeLocation& where, const std::string& what);
  
  virtual ~CastingFailed() throw();
  

}; //  CastingFailed

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a file is wrongly formated
/// @author Tiago Quintino
struct Common_API FileFormatError: public Common::Exception {

  /// Constructor
  FileFormatError(const Common::CodeLocation& where, const std::string& what);

  virtual ~FileFormatError() throw();

}; //  FileFormatError

////////////////////////////////////////////////////////////////////////////////

/// Exception throw if an error occurs when accessing the filesystem.
/// It is preferable to using directly the boost::filesystem_error exception.
/// These boost exceptions should be intercepted and recast into this.
struct Common_API FileSystemError: public Common::Exception {

  /// Constructor
  /// @see Exception()
  FileSystemError(const Common::CodeLocation& where, const std::string& what);

  virtual ~FileSystemError() throw();

}; //  FileSystemError

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a floating point error happens
/// @author Tiago Quintino
struct Common_API FloatingPointError: public Common::Exception {

  /// Constructor
  FloatingPointError( const Common::CodeLocation& where, const std::string& what);

  virtual ~FloatingPointError() throw();

}; //  FloatingPointError

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a certain value already exists in a storage.
/// @author Andrea Lani
/// @author Tiago Quintino
struct Common_API ValueExists: public Common::Exception {

  /// Constructor
  ValueExists( const Common::CodeLocation& where, const std::string& what);

  virtual ~ValueExists() throw();

}; //  StorageExists

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a certain value is not found.
/// @author Tiago Quintino
struct Common_API ValueNotFound: public Common::Exception {

  /// Constructor
  ValueNotFound( const Common::CodeLocation& where, const std::string& what);

  virtual ~ValueNotFound() throw();

}; //  NoSuchValue

////////////////////////////////////////////////////////////////////////////////

/// This exception is thrown when convergence cannot be reached
/// @author Willem Deconinck
struct Common_API FailedToConverge : public Common::Exception {

  /// Constructor
  FailedToConverge(const Common::CodeLocation& where, const std::string& what);

  virtual ~FailedToConverge() throw();

}; // class FailedToConverge

////////////////////////////////////////////////////////////////////////////////

///  Exception throwna certain functionality is not implemented
/// @author Andrea Lani
/// @author Tiago Quintino
struct Common_API  NotImplemented: public Common::Exception {

  /// Constructor
  /// @see CF::Exception()
  NotImplemented(const Common::CodeLocation& where, const std::string& what);
  
  virtual ~NotImplemented() throw();
  
}; //  NotImplemented

////////////////////////////////////////////////////////////////////////////////

///  Exception thrown when certain functionality is not supported by
///  for instance an third party format or library
/// @author Willem Deconinck
struct Common_API  NotSupported: public Common::Exception {

  /// Constructor
  /// @see CF::Exception()
  NotSupported(const Common::CodeLocation& where, const std::string& what);
  
  virtual ~NotSupported() throw();
  
}; //  NotSupported

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when an error related occurs on the parallel communication
/// @author Tiago Quintino
struct Common_API ParallelError: public Common::Exception {

  /// Constructor
  ParallelError(const Common::CodeLocation& where, const std::string& what);
  
  virtual ~ParallelError() throw();

}; //  ParallelError

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when error occured while parsing some input
/// @author Tiago Quintino
struct Common_API ParsingFailed: public Common::Exception {

  /// Constructor
  ParsingFailed( const Common::CodeLocation& where, const std::string& what);

  virtual ~ParsingFailed() throw();

}; //  ParsingFailed

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown if an error occurs when setting up an object.
/// @author Tiago Quintino
struct Common_API SetupError : public Common::Exception {

  /// Constructor
  SetupError ( const Common::CodeLocation& where, const std::string& what);

  virtual ~SetupError() throw();

}; //  Setup

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown in any place of the code which
/// by some conceptual impossibility should not be reached.
/// Typically on a switch-case construction where one of the choices
/// should be taken and the default never reached.
/// @author Tiago Quintino
struct Common_API ShouldNotBeHere: public Common::Exception {

  /// Constructor
  ShouldNotBeHere(const Common::CodeLocation& where, const std::string& what);

  virtual ~ShouldNotBeHere() throw();

}; //  ShouldNotBeHere

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown if an error occurs when accessing a network URL.
/// @author Tiago Quintino
struct Common_API URLError : public Common::Exception {

  /// Constructor
  URLError (const Common::CodeLocation& where, const std::string& what);

  virtual ~URLError() throw();

}; //  URL

////////////////////////////////////////////////////////////////////////////////

///  Exception thrown when some XML code has not the correct information or format
/// @author Tiago Quintino
struct Common_API XmlError: public Common::Exception {

  /// Constructor
  XmlError( const Common::CodeLocation& where, const std::string& what);

  virtual ~XmlError() throw();

}; //  XmlError

////////////////////////////////////////////////////////////////////////////////

///  Exception thrown when the components are structured in an invalid way
/// @author Tiago Quintino
struct Common_API InvalidStructure: public Common::Exception {

  /// Constructor
  InvalidStructure( const Common::CodeLocation& where, const std::string& what);

  virtual ~InvalidStructure() throw();

}; //  InvalidStructure

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a function is called in a moment it should not.
/// i.e. function a() has be called before function b() but is not.
/// @author Quentin Gasper
struct Common_API IllegalCall: public Common::Exception {

  /// Constructor
  IllegalCall( const Common::CodeLocation& where, const std::string& what);

  virtual ~IllegalCall() throw();

}; //  IllegalCall

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when an allocation failed.
/// @author Tamas Banyai
struct Common_API NotEnoughMemory: public Common::Exception {

  /// Constructor
  NotEnoughMemory( const Common::CodeLocation& where, const std::string& what);

  virtual ~NotEnoughMemory() throw();

}; //  NotEnoughMemory


////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when a protocol is not respected.
/// @author Quentin Gasper
struct Common_API ProtocolError: public Common::Exception {

  /// Constructor
  ProtocolError( const Common::CodeLocation& where, const std::string& what);

  virtual ~ProtocolError() throw();

}; //  ProtocolError

////////////////////////////////////////////////////////////////////////////////

/// Exception thrown when pointer issues detected (mostly smart_ptr.expired==true or stg becomes nullptr)
/// @author Tamas Banyai
struct Common_API BadPointer: public Common::Exception {

  /// Constructor
  BadPointer( const Common::CodeLocation& where, const std::string& what);

  virtual ~BadPointer() throw();

}; //  BadPointer

////////////////////////////////////////////////////////////////////////////////

  } // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_FailedAssertion_hpp

