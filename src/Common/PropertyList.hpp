#ifndef CF_Common_PropertyList_hpp
#define CF_Common_PropertyList_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/any.hpp>

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Class that represents a list of properties.
/// Properties can be of any type and are internally stored as boost::any.
/// A property name is associated to each property, and both are stored in a map,
/// for quick search by name.
/// @author Tiago Quintino
class Common_API PropertyList : public boost::noncopyable,
                                public std::map< std::string, boost::any > {

public: // types

  /// storage type
  typedef std::map< std::string, boost::any > StorageType;
  /// typedef for the iterator to inner storage
  typedef StorageType::iterator iterator;
  /// typedef for the iterator to inner storage
  typedef StorageType::const_iterator const_iterator;

public: // functions

  /// Access a property and cast it to the specified Type
  /// Property must already exist
  /// @see PropertyList::check
  /// @param prop_name the property name
  template < typename Type >
      Type value ( const std::string& prop_name )
  {
    cf_assert ( check(prop_name) );
    return boost::any_cast<Type>( operator[](prop_name) );
  }

  /// check that a property with the name exists
  /// @param prop_name the property name
  bool check ( const std::string& prop_name ) const
  {
    return find(prop_name) != this->end();
  }

}; // end of class PropertyList

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PropertyList_hpp
