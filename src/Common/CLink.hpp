#ifndef CF_Common_CLink_HH
#define CF_Common_CLink_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CLink : public Component {

  public:

    /// Contructor
    /// @param name of the component
    CLink ( const CName& name, const CPath& parent_path = CPath() );

    /// Virtual destructor
    virtual ~CLink();

    /// Get the class name
    static std::string getClassName () { return "CLink"; }

    /// get the componment throught the links to the actual components
    virtual Common::SafePtr<Component> get ();

    /// link to component
    void link_to ( boost::shared_ptr<Component> lnkto );

  private:

    /// this is a link to the component
    /// using weak_prt meand it might become invalid so we should test for expire()
    boost::weak_ptr<Component> m_link_component;

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CLink_HH
