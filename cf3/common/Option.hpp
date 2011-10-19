// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Option_hpp
#define cf3_common_Option_hpp

////////////////////////////////////////////////////////////////////////////

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/BasicExceptions.hpp"
#include "common/TaggedObject.hpp"
#include "common/SignalHandler.hpp"
#include "common/TypeInfo.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

namespace XML { class XmlNode; }

  //////////////////////////////////////////////////////////////////////////

  /// @brief Adds fonctionnalities to @c Property class.

  /// An option is a piece of data of which user can modify the value. An option
  /// provides:
  /// @li a default value (initial value when option is built)
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
      public boost::enable_shared_from_this<Option>,
      public TaggedObject {

  public:

    /// Typedef for shared pointers
    typedef boost::shared_ptr<Option>   Ptr;
    /// Typedef for const shared pointers
    typedef boost::shared_ptr<Option const>   ConstPtr;
    /// Typedef for trigger functions
    typedef boost::function< void() >   Trigger_t;
    /// Typedef for trigger functions storage.
    typedef std::vector< Trigger_t >    TriggerStorage_t;

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
        return boost::any_cast< TYPE >( data_to_value(m_value) );
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(data_to_value(m_value).type())+" to "+common::class_name<TYPE>());
      }
    }

    template<typename OPTION_TYPE>
    typename OPTION_TYPE::Ptr cast_to ()
    {
      typename OPTION_TYPE::Ptr ptr = boost::dynamic_pointer_cast<OPTION_TYPE>(shared_from_this());
      cf3_assert( is_not_null(ptr.get()) );
      return ptr;
    }

    template<typename OPTION_TYPE>
    typename OPTION_TYPE::ConstPtr cast_to () const
    {
      typename OPTION_TYPE::ConstPtr ptr = boost::dynamic_pointer_cast<const OPTION_TYPE>(shared_from_this());
      cf3_assert( is_not_null(ptr.get()) );
      return ptr;
    }

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @return Returns the default value as a sd::string
    virtual std::string def_str () const = 0;

    /// @return Returns the tag.
    virtual const char * tag() const = 0;

    /// @return Returns the data type
    virtual std::string data_type() const = 0;

    /// @return Returns the value cast to string.
    virtual std::string value_str () const = 0;

    /// @return Returns the value as it is stored.
    boost::any value() const { return m_value; }

    /// @returns the type of the option as a string
    virtual std::string type() const;


    //@} END VIRTUAL FUNCTIONS

    /// @brief Sets the option pretty name.
    /// @param pretty_name The option pretty name.
    /// @return Returns a reference to this object.
    Ptr pretty_name( const std::string & pretty_name );

    /// @brief Sets the option description.
    /// @param pretty_name The option description.
    /// @return Returns a reference to this object.
    Ptr description( const std::string & description );

    /// @brief Sets the option operator.
    /// The separator is used in some convertions to string to separate items,
    /// i.e. the restricted list of values or the option value if it is an array.
    /// @param pretty_name The option description.
    /// @return Returns a reference to this object.
    Ptr separator( const std::string & separator );

    /// configure this option using the passed xml node
    void configure_option ( XML::XmlNode & node );

    /// attach a function that will be triggered when an option gets configured
    /// @return this option
    Ptr attach_trigger ( Trigger_t trigger )
    {
      m_triggers.push_back(trigger);
      return shared_from_this();
    }

    /// @returns puts the value of the option casted to TYPE on the passed parameter
    /// @param value which to assign the option value
    template < typename TYPE >
    void put_value( TYPE& value ) const
    {
      try
      {
        value = boost::any_cast<TYPE>(m_value);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+common::class_name<TYPE>());
      }
    }

    /// @returns the name of the option
    std::string name() const { return m_name; }

    /// @returns the pretty name of the option
    std::string pretty_name() const { return m_pretty_name; }

    /// @returns the default value of the option as a boost::any
    boost::any def() const { return m_default; }

    /// @returns the description of the option
    std::string description() const { return m_description; }

    /// @returns the separator of the option
    std::string separator() const { return m_separator; }

    /// Assigns a new value to the option
    /// @param new_value The new value
    /// @return Returns a reference to this object.
    Option & operator=( const boost::any & new_value );

    /// @returns the default value of the option casted to TYPE
    template < typename TYPE >
        const TYPE def() const
    {
      try
      {
        return boost::any_cast<TYPE>(m_default);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_default.type())+" to "+common::class_name<TYPE>());
      }
    }

    /// @returns puts the default value of the option casted to TYPE on the passed parameter
    /// @param value which to assign the default option value
    template < typename TYPE >
        void put_def( TYPE& def ) const
    {
      try
      {
        def = boost::any_cast<TYPE>(m_default);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_default.type())+" to "+common::class_name<TYPE>());
      }
    }

    /// Link the state of this option to the passed parameter
    /// @return this option
    template < typename TYPE >
        Ptr link_to ( TYPE* par )
    {
      cf3_assert_desc (class_name<TYPE>()+"!="+data_type(), class_name<TYPE>() == data_type() );
      m_linked_params.push_back(par);
      *par = boost::any_cast<TYPE>(m_value);
      return shared_from_this();
    }

    /// this option is tagged as a basic option on the GUI
    /// @return this option
    Ptr mark_basic ();

    template < typename Option_t>
        boost::shared_ptr<Option_t> as_ptr()
    {
      return boost::dynamic_pointer_cast<Option_t>(shared_from_this());
    }

    /// change the value of this option
    virtual void change_value ( const boost::any& value);

    /// @brief Gives a reference to the restricted list.
    /// @return Returns a reference to the restricted list.
    std::vector<boost::any> & restricted_list() { return m_restricted_list; }

    /// @brief Gives a const reference to the restricted list.
    /// @return Returns a const reference to the restricted list.
    const std::vector<boost::any> & restricted_list() const { return m_restricted_list; }

    /// @brief Checks whether the option has a list of restricted values.
    /// @return Returns @c true if the option a such list; otherwise, returns
    /// @c false.
    virtual bool has_restricted_list() const = 0;

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XML::XmlNode & node ) = 0;

    /// Calls the triggers connected to this option.
    void trigger() const;

  protected: // data
    /// storage of the default value of the option
    boost::any m_default;
    /// storage of the value of the option
    boost::any m_value;
    /// option name
    std::string m_name;
    /// option pretty name
    std::string m_pretty_name;
    /// option description
    std::string m_description;
    /// list of processors that will process the option
    TriggerStorage_t m_triggers;
    /// parameters that also get updated when option is changed
    std::vector< void* > m_linked_params;
    /// Restricted list of values.
    std::vector<boost::any> m_restricted_list;
    /// Option separator.
    std::string m_separator;

  protected: // function

    /// Gives the option data, based on a value.
    virtual boost::any value_to_data ( const boost::any& value ) const { return value; }

    /// Gives the option value, based on data.
    virtual boost::any data_to_value ( const boost::any& data ) const { return data; }

    /// copy the configured update value to all linked parameters
    virtual void copy_to_linked_params ( const boost::any& val ) = 0;

  }; // class Option

////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

#endif // cf3_common_Option_hpp
