#ifndef CF_Common_VarRegistry_hpp
#define CF_Common_VarRegistry_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/NonCopyable.hpp"
#include "Common/GeneralStorage.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// This class serves as a registry for variables that get dynamically created
/// and therefore cannot be programmed into the code.
/// They are stored as pointers to void, and properly dynamically cast to the
/// correct type upon access.
/// This should be used to store variables with small memory foot print and
/// that are access seldolmly.
/// Each variable is stored with its name given as a std::string and its type,
/// encoded also into std::string.
/// @author Tiago Quintino
class Common_API VarRegistry : public Common::NonCopyable<VarRegistry> {

public: // functions

  /// Constructor
  VarRegistry();

  /// Destructor
  virtual ~VarRegistry();

  /// regists a variable with the given name
  /// @param name of the variable
  /// @param var pointer to the variable
  template < typename TYPE >
  void registVar ( const std::string& name, TYPE * var )
  {
    cf_assert ( var != CFNULL );
    m_storage.addEntry(name, var );
    m_typestr.addEntry(name, new std::string (DEMANGLED_TYPEID(TYPE)) );
  }

  /// unregists a variable with the given name
  /// @param name of the variable
  template < typename TYPE >
  TYPE * unregistVar ( const std::string& name )
  {
    TYPE * ptr = getVarPtr<TYPE>(name);
    m_storage.removeEntry(name);
    m_typestr.deleteEntry(name);
    return ptr;
  }

  /// accesses the variable
  /// @param name of the variable
  /// @param var pointer to the variable
  template < typename TYPE >
  TYPE& getVar ( const std::string& name )
  {
    TYPE * ptr = getVarPtr<TYPE>(name);
    return *ptr;
  }

  /// accesses the variable
  /// @param name of the variable
  /// @param var pointer to the variable
  template < typename TYPE >
  void setVar ( const std::string& name, const TYPE& value )
  {
    TYPE& var = this->getVar<TYPE>(name);
    var = value;
  }

private: // helper function

  /// accesses the variable
  /// @param name of the variable
  /// @param var pointer to the variable
  template < typename TYPE >
  TYPE * getVarPtr ( const std::string& name )
  {
    cf_assert_desc ( "TYPE must match the type of the variable" ,
                     *m_typestr.getEntry(name) != DEMANGLED_TYPEID(TYPE) );

    void * vptr = m_storage.getEntry(name);
    TYPE * ptr  = static_cast<TYPE*>(vptr);
    return ptr;
  }

private: // data

  /// storage of the variables
  Common::GeneralStorage<void> m_storage;
  /// storage of the variable types
  Common::GeneralStorage<std::string> m_typestr;

}; // end of class VarRegistry

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_VarRegistry_hpp
