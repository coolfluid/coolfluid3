// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_OptionArray_hpp
#define CF_Common_OptionArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>

#include "Common/Option.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/XML.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Class used to get element type of an OptionArray when down casting
  /// from Option.
  /// @author Quentin Gasper
  class Common_API OptionArray : public Option
  {
  public:
    typedef boost::shared_ptr<OptionArray> Ptr;
    typedef boost::shared_ptr<OptionArray const> ConstPtr;

    OptionArray(const std::string& name, const std::string& desc, boost::any def);

    virtual ~OptionArray();

    /// Returns a C-string representation of element type
    virtual const char * elem_type() const = 0;
  };

  /// Class defines an array of options of the same type to be used by the ConfigObject class
  /// This class supports the following types:
  ///   - bool
  ///   - int
  ///   - std::string
  ///   - boost::filesystem::path
  ///   - CF::Uint
  ///   - CF::Real
  ///   - CF::Common::URI
  /// @author Tiago Quintino
  template < typename TYPE >
      class Common_API OptionArrayT : public OptionArray  {

  public:

    typedef boost::shared_ptr<OptionArrayT> Ptr;
    typedef boost::shared_ptr<OptionArrayT const> ConstPtr;

    typedef std::vector<TYPE> value_type;

    typedef TYPE element_type;

    OptionArrayT( const std::string& name, const std::string& desc, const value_type& def);

    virtual ~OptionArrayT() {}

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// Returns a C-strng representation of element type
    virtual const char * elem_type() const { return XmlTag<element_type>::type(); }

    /// @returns the xml tag for this option
    virtual const char * tag() const { return "array"; }

    /// @returns the value as a sd::string
    virtual std::string value_str () const;

    /// @returns the default value as a sd::string
    virtual std::string def_str () const;

    std::vector<TYPE> value_vect() const;

    //@} END VIRTUAL FUNCTIONS

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XmlNode& node );

  protected:

    virtual void copy_to_linked_params ( const boost::any& val );

  private: // helper functions

    std::string dump_to_str ( const boost::any& c ) const;

  }; // class OptionArrayT

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_OptionArray_hpp
