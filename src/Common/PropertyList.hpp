#ifndef CF_Common_PropertyList_hpp
#define CF_Common_PropertyList_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/any.hpp>

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

///
/// @author Tiago Quintino
class Common_API PropertyList : public boost::noncopyable {

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
      Type get ( const std::string& prop_name )
  {
    cf_assert ( check(prop_name) );
    return boost::any_cast<Type>( list[prop_name] );
  }

  /// Removes a property from the list.
  /// Does nothing if property does not exist.
  /// @param prop_name the property name
  void remove ( const std::string& prop_name )
  {
    iterator itr = list.find(prop_name);
    if (itr != list.end())
      list.erase(itr);
  }

  /// check that a property with the name exists
  /// @param prop_name the property name
  bool check ( const std::string& prop_name )
  {
    return list.find(prop_name) != list.end();
  }

public: // data

  /// storage of the metainfo entries
  std::map< std::string, boost::any > list;

}; // end of class PropertyList

////////////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PropertyList_hpp
