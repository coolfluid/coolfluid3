#ifndef CF_Common_Option_hpp
#define CF_Common_Option_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/any.hpp>

#include "Common/Log.hpp"
#include "Common/TaggedObject.hpp"
#include "Common/XML.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

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
  class Common_API Option :
      public TaggedObject
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

    /// @name VIRTUAL FUNCTIONS
    //@{

    /// updates the option value using the xml configuration
    /// @param node XML node with data for this option
    virtual void change_value ( XmlNode& node ) = 0;

    /// @returns the xml tag for this option
    virtual const char * tag() const = 0;

    /// @returns the value as a sd::string
    virtual std::string value_str () const = 0;

    /// @returns the default value as a sd::string
    virtual std::string def_str () const = 0;

    //@} END VIRTUAL FUNCTIONS

    /// configure this option using the passed xml node
    void configure_option ( XmlNode& node );

    void attach_processor ( Processor_t proc ) { m_processors.push_back(proc); }

    // accessor functions

    /// @returns the name of the option
    std::string name() const { return m_name; }
    /// @returns the type of the option as a string
    std::string type() const { return m_type; }
    /// @returns the description of the option
    std::string description() const { return m_description; }

    /// @returns the value of the option as a boost::any
    boost::any value() const { return m_value; }
    /// @returns the default value of the option as a boost::any
    boost::any def() const { return m_default; }

    /// @returns the value of the option casted to TYPE
    template < typename TYPE >
        TYPE value() const { return boost::any_cast< TYPE >(m_value); }
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
      m_linked_params.push_back(par);
    }

    /// this option is tagged as a basic option on the GUI
    void mark_basic ();

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
    std::vector< void* > m_linked_params;

    template< typename TYPE>
    const char * type_to_str() const;

  }; // Option

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Option_hpp
