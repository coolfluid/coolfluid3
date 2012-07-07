// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Option.hpp
/// @note This header gets included indirectly in common/Component.hpp
///       It should be as lean as possible!

#ifndef cf3_common_Option_hpp
#define cf3_common_Option_hpp

////////////////////////////////////////////////////////////////////////////

#include <boost/any.hpp>
#include <boost/function/function_fwd.hpp>

#include "common/BasicExceptions.hpp"
#include "common/TaggedObject.hpp"
#include "common/TypeInfo.hpp"
#include "StringConversion.hpp"

////////////////////////////////////////////////////////////////////////////


namespace cf3 {
namespace common {

namespace XML { class XmlNode; }

  //////////////////////////////////////////////////////////////////////////

  /// @brief Adds fonctionnalities to @c Property class.

  /// An option is a piece of data of which user can modify the value. An option
  /// provides:
  /// @li a value (set to a default value when the option is built)
  /// @li a description string
  /// @li basic/advanced modes
  /// @li change value using a XML node
  /// @li restricted list of values
  /// @li triggers management
  /// By default, an option is advanced. It can be marked as basic by calling
  /// @c #mark_basic() method. @n
  ///
  /// Developer can define a restricted list of values, meaning that only those
  /// values are the valid. The list is obtained with @c #restricted_list() method.
  /// Values can be added using Boost.Assign library. Note that this list always
  /// contains at least one value: the default one. @n
  ///
  /// Triggers are functions called whenever the value is modified. They can be
  /// registered with @c #attach_trigger() method. Triggers take no parameter
  /// and return nothing.@n @n
  ///
  /// About the name:@n
  /// Although the option name is stored as a string so that there is no language
  /// limitation to its format, developers are asked to respect some rules
  /// in order to make the API learning easier:
  /// @li name should be in lower case
  /// @li if the name is composed of multiple words, use undescores ('_') to
  /// separate them (no spaces).
  /// @li name should be explicit and as short as possible. Use acronyms if
  /// possible.

  /// An option has also a "pretty name". This is a human readable form of the
  /// name. It is intended to be displayed on the user interfaces. It should
  /// contain spaces between words and a capital letter at least for the first
  /// word. Note that most OS guidelines recommand this kind of strings to have
  /// a captial letter for each word and to only contain verbs if necessary.
  /// Pretty name is optional and can be empty.


  /// @author Tiago Quintino
  /// @author Quentin Gasper
  class Common_API Option :
      public TaggedObject {

  public:
    /// Typedef for trigger functions
    typedef boost::function< void() >   TriggerT;
    /// ID for triggers
    typedef Uint TriggerID;
    /// Typedef for trigger functions storage.
    typedef std::map< TriggerID, TriggerT >  TriggerStorageT;
    

    /// Constructor.
    /// @param name Option name.
    /// @param def Default value.
    Option(const std::string & name, boost::any def);

    /// Desctructor.
    virtual ~Option();

    /// @brief Casts the value to the provided TYPE
    /// @return Returns the cast value.
    /// @throw CastingFailed if the value could not be cast.
    template<typename TYPE>
    TYPE value() const
    {
      try
      {
        return boost::any_cast< TYPE >( m_value );
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+common::class_name<TYPE>()
          + " for option " + name());
      }
    }

    template<typename OPTION_TYPE>
    OPTION_TYPE& cast_to ()
    {
      return dynamic_cast<OPTION_TYPE&>(*this);
    }

    template<typename OPTION_TYPE>
    const OPTION_TYPE& cast_to () const
    {
      return dynamic_cast<const OPTION_TYPE&>(*this);
    }

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @return Returns the value cast to string.
    virtual std::string value_str () const = 0;

    /// @return A copy of the stored value
    virtual boost::any value() const { return m_value; }

    /// @returns the type of the option as a string
    virtual std::string type() const;

    /// @returns the type of a single sub-element as a string
    virtual std::string element_type() const;

    //@} END VIRTUAL FUNCTIONS

    /// @brief Sets the option pretty name.
    /// @param pretty_name The option pretty name.
    /// @return Returns a reference to this object.
    Option& pretty_name( const std::string & pretty_name );

    /// @brief Sets the option description.
    /// @param pretty_name The option description.
    /// @return Returns a reference to this object.
    Option& description( const std::string & description );

    /// @brief Sets the option operator.
    /// The separator is used in some convertions to string to separate items,
    /// i.e. the restricted list of values or the option value if it is an array.
    /// @param pretty_name The option description.
    /// @return Returns a reference to this object.
    Option& separator( const std::string & separator );

    /// configure this option using the passed xml node
    void set ( XML::XmlNode & node );

    /// attach a function that will be triggered when an option gets configured
    /// @return this option
    Option& attach_trigger ( TriggerT trigger );
    
    /// Attach a trigger, returning an ID that can be used to detach again
    TriggerID attach_trigger_tracked( TriggerT trigger );

    /// Detach the given trigger
    void detach_trigger(const TriggerID trigger_id);

    /// @returns the name of the option
    std::string name() const { return m_name; }

    /// @returns the pretty name of the option
    std::string pretty_name() const { return m_pretty_name; }

    /// @returns the description of the option
    std::string description() const { return m_description; }

    /// @returns the separator of the option
    std::string separator() const { return m_separator; }

    /// Assigns a new value to the option
    /// @param new_value The new value
    /// @return Returns a reference to this object.
    Option & operator=( const boost::any & new_value );

    /// Link the state of this option to the passed parameter
    /// @return this option
    template < typename TYPE >
    Option& link_to ( TYPE* par )
    {
      cf3_assert(typeid(TYPE) == m_value.type());
      m_linked_params.push_back(par);
      *par = boost::any_cast<TYPE>(m_value);
      return *this;
    }

    /// this option is tagged as a basic option on the GUI
    /// @return this option
    Option& mark_basic ();

    /// change the value of this option
    virtual void change_value(const boost::any& value);

    /// @brief Gives a reference to the restricted list.
    /// @return Returns a reference to the restricted list.
    std::vector<boost::any> & restricted_list() { return m_restricted_list; }

    /// @brief Gives a const reference to the restricted list.
    /// @return Returns a const reference to the restricted list.
    const std::vector<boost::any> & restricted_list() const { return m_restricted_list; }

    /// Return the restricted list as string
    virtual std::string restricted_list_str() const = 0;

    /// Set the restricted list using a vector of strings
    virtual void set_restricted_list_str(const std::vector<std::string>& list) = 0;

    /// @brief Checks whether the option has a list of restricted values.
    /// @return Returns @c true if the option a such list; otherwise, returns
    /// @c false.
    virtual bool has_restricted_list() const { return !m_restricted_list.empty(); }

    /// Calls the triggers connected to this option.
    void trigger() const;

    /// restore the default value of the option
    void restore_default() { m_value = m_default; }

  protected:
    /// storage of the value of the option
    boost::any m_value;
    /// parameters that also get updated when option is changed
    std::vector< boost::any > m_linked_params;

  private: // data
    /// storage of the default value of the option
    boost::any m_default;
    /// option name
    std::string m_name;
    /// option pretty name
    std::string m_pretty_name;
    /// option description
    std::string m_description;
    /// list of processors that will process the option
    TriggerStorageT m_triggers;
    /// Restricted list of values.
    std::vector<boost::any> m_restricted_list;
    /// Option separator.
    std::string m_separator;
    
    /// Current connection ID for the triggers
    Uint m_current_connection_id;

  private: // function

    /// copy the configured update value to all linked parameters
    virtual void copy_to_linked_params (std::vector< boost::any >& linked_params) = 0;

    /// Get the value to use from XML
    /// @param node XML node with data for this option
    virtual boost::any extract_configured_value( XML::XmlNode & node ) = 0;

    /// Concrete implementation of the value changing
    virtual void change_value_impl(const boost::any& value) = 0;

  }; // class Option

////////////////////////////////////////////////////////////////////////////////////////////

/// Helper function to convert a vector to string, skipping empty entries
template<typename T>
std::string option_vector_to_str(const std::vector<T>& vec, const std::string& delim)
{
  std::string result;
  typename std::vector<T>::const_iterator it = vec.begin();

  for( ; it != vec.end() ; ++it )
  {
    if(to_str(*it).empty())
    {
      continue;
    }
    // if it is not the first item, we add the delimiter
    if( !result.empty() )
      result += delim;

    result += to_str(*it);
  }

  return result;
}

} // common
} // cf3

#endif // cf3_common_Option_hpp
