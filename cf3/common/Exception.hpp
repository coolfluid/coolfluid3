// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Exception_hpp
#define cf3_common_Exception_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CodeLocation.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

/// Manager of behavior of exceptions
class Common_API ExceptionManager : public boost::noncopyable {
public:

  /// Constructor
  ExceptionManager();

  /// Gets the instance of the manager
  static ExceptionManager& instance ();

  /// if exception contructor should output
  bool ExceptionOutputs;
  /// if exception contructor should dump backtrace
  bool ExceptionDumps;
  /// if exception contructor should abort execution immedietly
  bool ExceptionAborts;

}; // class ExceptionManager

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class for all Exceptions in CF
///
/// This type extends std::exception by providing an implementation for what().
/// It should optionally describe the situation of the exception in
/// more detail. Information about the exception occurence may be added on the what()
/// string or passed in separate data members defined in the derived subtypes of this class.
/// This class is abstract. Only subclasses can be instantiated.
///
/// @author Tiago Quintino
/// @author Andrea Lani
class Common_API Exception : public std::exception {
public: // functions

  /// Default copy constructor
  virtual ~Exception () throw ();

  /// Returns a verbose message with all information about this exception
  std::string full_description () const throw ();

  /// Gets the what description string which does not contain EOL's.
  const std::string& str () const throw ();

  /// @return str().c_str();
  const char* what () const throw ();

  /// Append the message to the what() description
  /// @param add the std::string to be appended
  void append (const std::string& add) throw ();

  /// @returns the Exception name
  std::string type_name () const throw () { return m_class_name; }

  std::string msg() const throw () { return m_msg; }

protected: // functions

  /// The constructor is protected to force the developers to create subclasses.
  /// @param where  The location in the code from where the exception is raised. Typically received using FromHere()
  /// @param msg    A message describing the circumstances of this exception occurence which might be the empty string.
  /// @param className  classname of derived Exception type, giving first indication of the kind of exception.
  /// @pre   msg should not contain EOL's
  /// @post  new.what() == what;
  Exception (CodeLocation where, std::string msg, std::string className) throw ();

protected: // data

  /// Similar to m_msg but stores from where the exception was thrown
  /// indicating the file, line and function if possible.
  /// This is an encapsulated data member, which uses value semantics, and
  /// not a reference or a pointer. A reference or a pointer in an
  /// exception leads immediately to a memory leak, so cannot be used.
  CodeLocation m_where;

  /// Stores the message with explanation of what happened
  std::string m_msg;

  /// The subclass name
  std::string m_class_name;

  /// Stores the full description message with explanation of what happened
  /// @post same as full_description()
  std::string m_what;

}; // end class Exception

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Exception_hpp
