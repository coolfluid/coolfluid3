#ifndef CF_Common_DemangledTypeID_hpp
#define CF_Common_DemangledTypeID_hpp

/// @note This header should be included by including CF.hpp instead.

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Function to demangle the return of typeid()
Common_API std::string demangle (const char* type);

#define DEMANGLED_TYPEID(a) CF::Common::demangle(typeid(a).name())

////////////////////////////////////////////////////////////////////////////////

  } // namespace Common


  /// @brief Handles type information
  /// This struct allows to associate a type to a string. It is a singleton.
  /// @author Tiago Quintino
  /// @author Quentin Gasper
  struct TypeInfo : public boost::noncopyable
  {
    /// @return Returns the instance
    static TypeInfo& instance();

    /// @brief Registers a type to a string
    /// @param tname The type name
    template <typename TYPE>
     void  regist ( const std::string& tname )
    {
        portable_types[typeid(TYPE).name()] = tname ;
    }

    /// @brief Map for types and their associated string
    /// The key is the type, the value is the associated string. The type
    /// is converted to a string using @c typeid
    std::map< std::string, std::string> portable_types;

  };

  template < typename TYPE >
      std::string class_name ()
  {
    TypeInfo& ti = TypeInfo::instance();
    cf_assert( ti.portable_types.find(typeid(TYPE).name()) != ti.portable_types.end() );
    return ti.portable_types[ typeid(TYPE).name() ];
  }


} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_DemangledTypeID_hpp
