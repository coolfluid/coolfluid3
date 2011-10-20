// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_OptionArray_hpp
#define cf3_common_OptionArray_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Option.hpp"
#include "common/BasicExceptions.hpp"
#include "common/XML/Protocol.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

  /// Class used to get element type of an OptionArray when down casting
  /// from Option.
  /// @author Quentin Gasper
  class Common_API OptionArray : public Option
  {
  public:
    typedef boost::shared_ptr<OptionArray> Ptr;
    typedef boost::shared_ptr<OptionArray const> ConstPtr;

    OptionArray(const std::string& name, boost::any def);

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
  ///   - cf3::Uint
  ///   - cf3::Real
  ///   - cf3::common::URI
  /// @author Tiago Quintino
  template < typename TYPE >
      class Common_API OptionArrayT : public OptionArray  {

  public:

    typedef boost::shared_ptr<OptionArrayT> Ptr;
    typedef boost::shared_ptr<OptionArrayT const> ConstPtr;

    typedef std::vector<TYPE> value_type;

    typedef TYPE element_type;

    virtual std::string data_type() const { return type(); }

    OptionArrayT( const std::string& name, const value_type& def);

    virtual ~OptionArrayT() {}


    static Option::Ptr create(const std::string & name, const value_type& def)
    {
      return Option::Ptr ( new OptionArrayT(name, def) );
    }

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// Returns a C-strng representation of element type
    virtual const char * elem_type() const { return XML::Protocol::Tags::type<element_type>(); }

    /// @returns the xml tag for this option
    virtual const char * tag() const { return XML::Protocol::Tags::node_array(); }

    /// @brief Checks whether the option has a list of restricted values.
    /// @return Returns @c true if the option a such list; otherwise, returns
    /// @c false.
    bool has_restricted_list() const { return m_restricted_list.size() > value_vect().size(); }

    /// @returns the value as a sd::string
    virtual std::string value_str () const;

    /// @returns the default value as a sd::string
    virtual std::string def_str () const;

    std::vector<TYPE> value_vect() const;

    //@} END VIRTUAL FUNCTIONS

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void configure ( XML::XmlNode& node );

  protected:

    virtual void copy_to_linked_params ( const boost::any& val );

  private: // helper functions

    std::string dump_to_str ( const boost::any& c ) const;

  }; // class OptionArrayT

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_OptionArray_hpp
