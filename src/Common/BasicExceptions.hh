#ifndef CF_Common_Basics_hh
#define CF_Common_Basics_hh

//////////////////////////////////////////////////////////////////////////////

#include "Common/Exception.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

//////////////////////////////////////////////////////////////////////////////

/// Exception thrown when an assertion fails but the code is configured to throw an exception rather than crash.
/// @author Tiago Quintino
struct Common_API FailedAssertion : public Common::Exception {

  /// Constructor
  FailedAssertion (const Common::CodeLocation& where, const std::string& what);

}; // end FailedAssertion

//////////////////////////////////////////////////////////////////////////////

/// Exception thrown when user provides a bad value input
/// @author Tiago Quintino
struct Common_API BadValue: public Common::Exception {

  /// Constructor
  BadValue( const Common::CodeLocation& where, const std::string& what);

}; // end of struct BadValue

//////////////////////////////////////////////////////////////////////////////

/// This struct represents an Exception thrown
/// when a dynamic cast of a pointer fails.
/// @author Tiago Quintino
struct CastingFailed: public Common::Exception {

  /// Constructor
  CastingFailed( const Common::CodeLocation& where, const std::string& what);

}; // end of struct CastingFailed

//////////////////////////////////////////////////////////////////////////////

/// This struct represents an Exception thrown when a certain
/// value is not found in a storage or container.
/// @author Andrea Lani
/// @author Tiago Quintino
struct FileFormatError: public Common::Exception {

  /// Constructor
  /// @param what is the value that has been requested,
  ///             but actually doesn't exist
  /// @see Exception()
  FileFormatError(const Common::CodeLocation& where, const std::string& what);

}; // end of struct FileFormatError

//////////////////////////////////////////////////////////////////////////////

/// This struct represents an Exception thrown when
/// a floating point error happens.
/// @author Tiago Quintino
struct Common_API FloatingPointError: public Common::Exception {

  /// Constructor
  FloatingPointError( const Common::CodeLocation& where, const std::string& what);
}; // end of struct FloatingPointError

//////////////////////////////////////////////////////////////////////////////

/// This struct represents an Exception thrown when a certain
/// value is not found in a storage.
/// @author Andrea Lani
/// @author Tiago Quintino
struct NoSuchStorage: public Common::Exception {

    /// Constructor
    NoSuchStorage( const Common::CodeLocation& where, const std::string& what);

}; // end of struct NoSuchStorage

  //////////////////////////////////////////////////////////////////////////////

struct Common_API NoSuchValue: public Common::Exception {

  /// Constructor
  NoSuchValue( const Common::CodeLocation& where, const std::string& what);

}; // end of struct NoSuchValue

//////////////////////////////////////////////////////////////////////////////

/// This struct represents an Exception throwna certain functionality is not implemented
/// @author Andrea Lani
/// @author Tiago Quintino
struct Common_API  NotImplemented: public Common::Exception {

  /// Constructor
  /// @see CF::Exception()
  NotImplemented(const Common::CodeLocation& where, const std::string& what);
}; // end of struct NotImplemented

//////////////////////////////////////////////////////////////////////////////

/// This exception is thrown in any place of the code which
/// by some conceptual impossibility should not be reached.
/// Typically on a switch-case construction where one of the choices
/// should be taken and the default never reached.
/// @author Tiago Quintino
struct Common_API NullPointerError: public Common::Exception {

  /// Constructor
  NullPointerError(const Common::CodeLocation& where, const std::string& what);

}; // end of struct NullPointerError

//////////////////////////////////////////////////////////////////////////////

/// This exception is thrown in any place of the code which
/// by some conceptual impossibility should not be reached.
/// Typically on a switch-case construction where one of the choices
/// should be taken and the default never reached.
/// @author Tiago Quintino
struct Common_API ParallelError: public Common::Exception {

  /// Constructor
  ParallelError(const Common::CodeLocation& where, const std::string& what);

}; // end of struct ParallelError

//////////////////////////////////////////////////////////////////////////////

/// Exception thrown when user provides a bad value input
/// @author Tiago Quintino
struct Common_API ParsingFailed: public Common::Exception {

  /// Constructor
  ParsingFailed( const Common::CodeLocation& where, const std::string& what);

}; // end of struct ParsingFailed

//////////////////////////////////////////////////////////////////////////////

/// This exception is thrown in any place of the code which
/// by some conceptual impossibility should not be reached.
/// Typically on a switch-case construction where one of the choices
/// should be taken and the default never reached.
/// @author Tiago Quintino
struct Common_API ShouldNotBeHere: public Common::Exception {

  /// Constructor
  ShouldNotBeHere(const Common::CodeLocation& where, const std::string& what);

}; // end of struct ShouldNotBeHere

//////////////////////////////////////////////////////////////////////////////

/// This struct represents an Exception thrown when a certain
/// value already exists in a storage.
/// @author Andrea Lani
/// @author Tiago Quintino
struct Common_API StorageExists: public Common::Exception {

  /// Constructor
  StorageExists( const Common::CodeLocation& where, const std::string& what);

}; // end of struct StorageExists

//////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_FailedAssertion_hh

