#ifndef CF_Common_CGroup_hpp
#define CF_Common_CGroup_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Common/OptionT.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CGroup : public Component {

  public: // typedefs

    /// provider
    typedef Common::ConcreteProvider < CGroup,1 > PROVIDER;
    /// pointer to this type
    typedef boost::shared_ptr<CGroup> Ptr;
    typedef boost::shared_ptr<CGroup const> ConstPtr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CGroup ( const CName& name );

    /// Virtual destructor
    virtual ~CGroup();

    /// Get the class name
    static std::string type_name () { return "CGroup"; }

    /// Configuration Options
    static void defineConfigOptions ( Common::OptionList& options )
    {
      options.add< OptionT<CF::Real> >("pi", "Pi in a CGroup", 3.141592);
    }

  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CGroup_hpp
