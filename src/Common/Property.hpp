// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Property_hpp
#define CF_Common_Property_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/type_traits/add_reference.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/TypeInfo.hpp"
#include "Common/TaggedObject.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  class Option;

  /// Class defines options to be used in the ConfigObject class
  ///
  /// @author Tiago Quintino
  /// @todo
  ///   * provide function to return value and def as string?
  ///   * option of pointer to base class init from self regist
  ///   * option for pointer to Component
  ///   * vector of components
  ///   * modify DynamicObject class - signals with XML, rename SignalHandler
  ///   * option sets with own processors
  ///   * option for paths ( file and dirs )
  ///   * option for component path
  ///   * configuration sets [inlet conditions)
  ///       - use of configuration signature
  ///   * list_signals signal in Component
  ///   * signals should have tags like advanced, etc
  ///
  ///
  /// How to:
  ///   * how to define processors statically?
  ///   * how to define the validations statically??
  ///   * components inform GUI of
  ///      * their signals
  ///      * hide signals from GUI in advanced mode
  ///      * inform of XML parameters for each signal
  ///
  /// Done:
  ///   * option of vector of T
  ///   * configure values from XMLNode
  ///   * access configured values
  ///   * break into files
  ///       - ConfigObject ( ConfigObject, OptionList )
  ///       - Option
  ///       - OptionT
  class Common_API Property:
      public TaggedObject
  {
  public:

		typedef boost::shared_ptr<Property>   Ptr;
		typedef boost::shared_ptr<Property const>   ConstPtr;

    /// Constructor
    Property (boost::any value);

    /// Virtual destructor
    virtual ~Property ();

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @returns the xml tag for this option
    virtual const char * tag() const;

    /// @returns the value as a sd::string
    virtual std::string value_str () const;

    //@} END VIRTUAL FUNCTIONS

    // accessor functions

    /// @returns the type of the option as a string
    virtual std::string type() const;

    /// @returns @c true if the property is an option
    bool is_option() const { return m_is_option; }

    Option & as_option();

    const Option & as_option() const;

    /// @returns the value of the option as a boost::any
    boost::any value() const { return m_value; }

    /// @returns the value of the option casted to TYPE
    template < typename TYPE >
        const TYPE value() const
    {
      try
      {
        return boost::any_cast< TYPE >(m_value);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+Common::class_name<TYPE>());
      }
    }
    
    /// @returns Reference to the value of the option casted to TYPE
    template < typename TYPE >
    typename boost::add_reference<TYPE const>::type value_ref() const
    {
      try
      {
        return boost::any_cast< typename boost::add_reference<TYPE const>::type >(m_value);
      }
      catch(boost::bad_any_cast& e)
      {
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+Common::class_name<TYPE>());
      }
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
        throw CastingFailed( FromHere(), "Bad boost::any cast from "+class_name_from_typeinfo(m_value.type())+" to "+Common::class_name<TYPE>());
      }
    }

    /// change the value of this option
    virtual void change_value ( const boost::any& value);

    Property & operator = (const boost::any & value);

  protected:

    /// storage of the value of the option
    boost::any m_value;
    /// indicates whether a property is an option. Options are user-modifiable
    /// properties.
    bool m_is_option;

  }; // class Property

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Property_hpp
