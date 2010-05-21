#ifndef CF_Common_CLink_hpp
#define CF_Common_CLink_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component for creating links between components
  /// @author Tiago Quintino
  class Common_API CLink : public Component {

  public: //typedefs

    typedef Component_iterator<CLink> Iterator;
    typedef boost::shared_ptr<CLink> Ptr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CLink ( const CName& name, const CPath& parent_path = CPath() );

    /// Virtual destructor
    virtual ~CLink();

    /// Get the class name
    static std::string getClassName () { return "CLink"; }

    /// Configuration Options
    static void defineConfigOptions ( Common::OptionList& options ) {}

    /// get the componment throught the links to the actual components
    virtual Component::Ptr get ();

    // functions specific to the CLink component

    /// link to component
    void link_to ( Component::Ptr lnkto );

  private: // data

    /// this is a link to the component
    /// using weak_prt meand it might become invalid so we should test for expire()
    boost::weak_ptr<Component> m_link_component;

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CLink_hpp
