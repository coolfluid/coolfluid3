#ifndef CF_Common_CRoot_hpp
#define CF_Common_CRoot_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CRoot : public Component {

  public:

    /// Get the class name
    static boost::shared_ptr<CRoot> create ( const CName& name );

    /// Virtual destructor
    virtual ~CRoot();

    /// Get the class name
    static std::string getClassName () { return "CRoot"; }

    // functions specific to the CRoot component

    /// Access the component described by the path
    /// The path should be absolute
    boost::shared_ptr<Component> access_component ( const CPath& path );

    /// define the component path
    void define_component_path ( const CPath& path, boost::shared_ptr<Component> );

  private: // functions

    typedef std::map< std::string , boost::shared_ptr<Component> > CompStorage_t;

    /// Private constructor forces creation via the create() funtion
    /// @param name of the component
    CRoot ( const CName& name );

  private: // data

    /// map the paths to each component
    CompStorage_t  m_toc;

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CRoot_hpp
