// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_Option_hpp
#define CF_Common_Option_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/TaggedObject.hpp"
#include "Common/Property.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  //////////////////////////////////////////////////////////////////////////

  class Common_API Option :
      public Property,
      public TaggedObject
  {

  public:

    typedef boost::shared_ptr<Option>   Ptr;
    typedef boost::function< void() >   Trigger_t;
    typedef std::vector< Trigger_t >    TriggerStorage_t;

    Option(const std::string & name, const std::string & desc, boost::any def);

    virtual ~Option();

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// @returns the default value as a sd::string
    virtual std::string def_str () const = 0;

    virtual const char * tag() const = 0;

    //@} END VIRTUAL FUNCTIONS

    /// configure this option using the passed xml node
    void configure_option ( XmlNode& node );

    void attach_trigger ( Trigger_t trigger ) { m_triggers.push_back(trigger); }

    /// @returns the name of the option
    std::string name() const { return m_name; }

    /// @returns the default value of the option as a boost::any
    boost::any def() const { return m_default; }

    /// @returns the description of the option
    std::string description() const { return m_description; }

    /// @returns the default value of the option casted to TYPE
    template < typename TYPE >
        const TYPE def() const { return boost::any_cast<TYPE>(m_default); }

    /// @returns puts the default value of the option casted to TYPE on the passed parameter
    /// @param value which to assign the default option value
    template < typename TYPE >
        void put_def( TYPE& def ) const { def = boost::any_cast<TYPE>(m_default); }

    /// Link the state of this option to the passed parameter
    template < typename TYPE >
        void link_to ( TYPE* par )
    {
      cf_assert ( class_name<TYPE>() == type() );
      m_linked_params.push_back(par);
    }

    /// this option is tagged as a basic option on the GUI
    void mark_basic ();

    /// change the value of this option
    void change_value ( const boost::any& value);

  protected: // data
    /// storage of the default value of the option
    const boost::any m_default;
    /// option name
    std::string m_name;
    /// option description
    std::string m_description;
    /// list of processors that will process the option
    TriggerStorage_t m_triggers;
    /// parameters that also get updated when option is changed
    std::vector< void* > m_linked_params;

  protected: // function

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node ) = 0;

  }; // class Option

  //////////////////////////////////////////////////////////////////////////

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Option_hpp
