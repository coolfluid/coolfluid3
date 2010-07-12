#ifndef CF_Common_CMethod_hpp
#define CF_Common_CMethod_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Common/OptionT.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CMethod : public Component {

  public: // typedefs

    /// provider
    typedef Common::ConcreteProvider < CMethod,1 > PROVIDER;
    /// pointer to this type
    typedef boost::shared_ptr<CMethod> Ptr;

  public: // functions

    /// Contructor
    /// @param name of the component
    CMethod ( const CName& name );

    /// Virtual destructor
    virtual ~CMethod();

    /// Get the class name
    static std::string type_name () { return "CMethod"; }

    /// Configuration Options
    static void defineConfigOptions ( Common::OptionList& options )
    {
      options.add< OptionT<bool> >("myBoolMeth", "A boolean value in a CMethod", true);
      options.add< OptionT<int> >("fourtyTwo", "An integer value in a CMethod", 42);
      options.add< OptionT<CF::Real> >("euler", "Euler number in a CMethod", 2.71);
      options.add< OptionT<std::string> >("nameMeth", "Name of a CMethod", "The CMethod name");
    }

  private: // helper functions

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CMethod_hpp
