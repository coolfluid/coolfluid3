#include <boost/foreach.hpp>

#include "Common/Option.hpp"

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

  Option::Option ( const std::string& name,
                   const std::string& type,
                   const std::string& desc,
                   boost::any def) :
  m_value(def),
  m_default(def),
  m_name(name),
  m_type(type),
  m_description(desc)
  {
  }

  Option::~Option()
  {
  }

  void Option::configure_option ( XmlNode& node )
  {
    this->change_value(node); // update the value

    // call all process functors
    BOOST_FOREACH( Option::Processor_t& process, m_processors )
        process();
  }

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
