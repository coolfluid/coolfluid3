// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_XML_SignalOptions_hpp
#define CF_Common_XML_SignalOptions_hpp

//////////////////////////////////////////////////////////////////////////////

#include "Common/OptionList.hpp"

#include "Common/XML/SignalFrame.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
namespace XML {

//////////////////////////////////////////////////////////////////////////////

class Common_API SignalOptions : public Common::OptionList
{
public:

  SignalOptions( SignalFrame & frame );

  SignalFrame create_frame( const std::string & name = std::string(),
                            const URI & sender = URI(),
                            const URI & receiver = URI() ) const;

  void add_to_map( Map & m_map ) const;

  template<typename TYPE>
  TYPE value( const std::string & name ) const;

  template<typename TYPE>
  std::vector<TYPE> array( const std::string & name ) const;

  void flush();

private:

  Map m_map;

}; // SignalOptionList

/// Abstracts the use of XML when adding options to a signal frame.

/// This class creates an "options" map (if it does not exist yet) to the
/// signal frame given to the constructor and provides methods to manipulate
/// this map. The frame may then be modified by this class.

/// @note Methods that modify the options return a reference to the object to
/// allow nested operations.

/// @warning An instance of this class keeps a pointer to the signal frame
/// it was built with. Be careful when deleting a frame that all signal
/// options linked to it will not be used anymore.

/// @author Quentin Gasper.

//class Common_API SignalOptions
//{

//public:

//  /// Constructor.

//  /// Searches for an "option" map. If it does not exist, it is created.
//  /// @param frame The signal frame to work with. May be modified. Must be valid.
//  SignalOptions( SignalFrame & frame );

//  /// Adds an option.

//  /// @param name The option name. Cannot be empty.
//  /// @param value The option value.
//  /// @param descr The option description. May be empty.
//  /// @param restr_values List of restricted values. Can be empty. If not
//  /// empty, the @c value must present in it.
//  /// @param delimiter The string used to delimit the values for the list
//  /// of restricted values. Cannot be empty.
//  /// @return Returns a reference to this object to allow nested operations.
//  /// @throw BadValue if the name is empty.
//  /// @throw ValueExists if an option with this name already exixts.
//  template<typename TYPE>
//  SignalOptions & add ( const std::string & name, const TYPE & value,
//                        const std::string & descr = std::string(),
//                        const std::vector<TYPE> & restr_values =  std::vector<TYPE>(),
//                        const std::string & restr_values_delim = " ; ");

//  /// Adds an array.

//  /// @param name The array name. Cannot be empty.
//  /// @param value The array value.
//  /// @param delimiter The string used to delimit the values. Cannot be empty.
//  /// @param descr The option description. May be empty.
//  /// @param restr_values List of restricted values. Can be empty.
//  /// The delimiter is the same as for the array.
//  /// @return Returns a reference to this object to allow nested operations.
//  /// @throw BadValue if the name or the delimiter is empty.
//  /// @throw ValueExists if an option with this name already exixts.
//  template<typename TYPE>
//  SignalOptions & add ( const std::string & name,
//                        const std::vector<TYPE> & value,
//                        const std::string & delimiter = " ; ",
//                        const std::string & descr = std::string(),
//                        const std::vector<TYPE> & restr_values =  std::vector<TYPE>() );

//  /// Adds an URI option.

//  /// @param name The option name. Cannot be empty.
//  /// @param value The option value.
//  /// @param descr The option description. May be empty.
//  /// @param sup_schemes List of allowed schemes. Can be empty (all schemes allowed).
//  /// @param restr_values List of restricted values. Can be empty. If not
//  /// empty, the @c value must present in it.
//  /// @param delimiter The string used to delimit the values for the list
//  /// of restricted values. Cannot be empty.
//  /// @return Returns a reference to this object to allow nested operations.
//  /// @throw BadValue if the name is empty.
//  /// @throw ValueExists if an option with this name already exixts.
//  SignalOptions & add ( const std::string & name, const URI & value,
//                        const std::string & descr = std::string(),
//                        const std::vector<URI::Scheme::Type> & sup_schemes = std::vector<URI::Scheme::Type>(),
//                        const std::vector<URI> & restr_values =  std::vector<URI>(),
//                        const std::string & restr_values_delim = " ; ");

//  /// Adds an array.

//  /// @param name The array name. Cannot be empty.
//  /// @param value The array value.
//  /// @param delimiter The string used to delimit the values. Cannot be empty.
//  /// @param descr The option description. May be empty.
//  /// @param sup_schemes List of allowed schemes. Can be empty (all schemes allowed).
//  /// @param restr_values List of restricted values. Can be empty.
//  /// The delimiter is the same as for the array.
//  /// @return Returns a reference to this object to allow nested operations.
//  /// @throw BadValue if the name or the delimiter is empty.
//  /// @throw ValueExists if an option with this name already exixts.
//  SignalOptions & add ( const std::string & name,
//                        const std::vector<URI> & value,
//                        const std::string & delimiter = " ; ",
//                        const std::string & descr = std::string(),
//                        const std::vector<URI::Scheme::Type> & sup_schemes = std::vector<URI::Scheme::Type>(),
//                        const std::vector<URI> & restr_values =  std::vector<URI>() );

//  template<typename TYPE>
//  TYPE option( const std::string & name ) const;

//  template<typename TYPE>
//  std::vector<TYPE> array( const std::string & name ) const;

//  /// Removes an option.

//  /// If the name is empty or the option does not exist, nothing is done.
//  /// @param name The name of the option to remove.
//  /// @return Returns a reference to this object to allow nested operations.
//  SignalOptions & remove ( const std::string & name );

//  /// Checks if an option exists.

//  /// @param name The option name.
//  /// @return Returns @c true if the option exists. Otherwise, returns @c false.
//  bool exists ( const std::string & name ) const;

//private: // data

//  /// The managed map.
//  Map map;

//}; // SignalOptions

//////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF

//////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_XML_SignalOptions_hpp
