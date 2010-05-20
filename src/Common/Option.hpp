#ifndef CF_Common_Option_hpp
#define CF_Common_Option_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/any.hpp>

#include <boost/property_tree/detail/rapidxml.hpp>

#include "Common/CF.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines options to be used in the ConfigObject class
  /// @author Tiago Quintino
  /// @todo
  ///   * provide function to return value and def as string
  ///   * option of pointer to base class init from self regist
  ///   * option for pointer to Component
  ///   * vector of components
  ///   * modify DynamicObject class - signals with XML
  ///   * option sets with own processors
  ///   * option for paths ( file and dirs )
  ///   * option for component path
  ///   * break into files
  ///       - Configurable ( Configurable, OptionList )
  ///       - Option ( Option, OptionT )
  ///   * configuration sets [inlet conditions)
  ///       - use of configuration signature
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
  class Option
  {
  public:

    typedef boost::shared_ptr<Option>     Ptr;
    typedef boost::function< void() >     Processor_t;
    typedef std::vector< Processor_t >    ProcStorage_t;

    /// Constructor
    Option ( const std::string& name,
             const std::string& type,
             const std::string& desc,
             boost::any def);

    /// Virtual destructor
    virtual ~Option ();

    /// updates the option value using the xml configuration
    virtual void change_value ( rapidxml::xml_node<> *node ) = 0;

    /// configure this option using the passed xml node
    void configure_option ( rapidxml::xml_node<> *node );

    void attach_processor ( Processor_t proc ) { m_processors.push_back(proc); }

    // accessor functions

    /// @returns the name of the option
    std::string name() const { return m_name; }
    /// @returns the type of the option as a string
    std::string type() const { return m_name; }
    /// @returns the description of the option
    std::string description() const { return m_description; }

    /// @returns the value of the option as a boost::any
    boost::any value() const { return m_value; }
    /// @returns the default value of the option as a boost::any
    boost::any def() const { return m_default; }

    /// @returns the value of the option casted to TYPE
    template < typename TYPE >
        TYPE value() const { return boost::any_cast<TYPE>(m_value); }
    /// @returns the default value of the option casted to TYPE
    template < typename TYPE >
        TYPE def() const { return boost::any_cast<TYPE>(m_default); }

    /// @returns puts the value of the option casted to TYPE on the passed parameter
    /// @param value which to assign the option value
    template < typename TYPE >
        void put_value( TYPE& value ) const { value = boost::any_cast<TYPE>(m_value); }

    /// @returns puts the default value of the option casted to TYPE on the passed parameter
    /// @param value which to assign the default option value
    template < typename TYPE >
        void put_def( TYPE& def ) const { def = boost::any_cast<TYPE>(m_default); }

    /// Link the state of this option to the passed parameter
    template < typename TYPE >
        void link_to ( TYPE* par )
    {
      cf_assert ( DEMANGLED_TYPEID(TYPE) == m_type );
      m_other_params.push_back(par);
    }

  protected:

    /// storage of the value of the option
    boost::any m_value;
    /// storage of the default value of the option
    const boost::any m_default;
    /// option name
    std::string m_name;
    /// option type as a string
    std::string m_type;
    /// option description
    std::string m_description;
    /// list of processors that will process the option
    ProcStorage_t m_processors;
    /// parameters that also get updated when option is changed
    std::vector< void* > m_other_params;

  }; // Option

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Option_hpp
