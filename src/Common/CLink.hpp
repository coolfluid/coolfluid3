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

    typedef boost::shared_ptr<CLink> Ptr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CLink ( const CName& name );

    /// Virtual destructor
    virtual ~CLink();

    /// Get the class name
    static std::string getClassName () { return "CLink"; }

    /// Configuration Options
    static void defineConfigOptions ( Common::OptionList& options ) {}

    /// get the component through the links to the actual components
    virtual Component::Ptr get ();
    virtual Component::ConstPtr  get() const;

    // functions specific to the CLink component

    /// link to component
    void link_to ( Component::Ptr lnkto );

  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  private: // data

    /// this is a link to the component
    /// using weak_prt meand it might become invalid so we should test for expire()
    boost::weak_ptr<Component> m_link_component;

  }; // CLink

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CLink_hpp
