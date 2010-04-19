#ifndef CF_Common_CRoot_HH
#define CF_Common_CRoot_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CRoot : public Component {

  public:

    /// Contructor
    /// @param name of the component
    CRoot ( const CName& name );

    /// Virtual destructor
    virtual ~CRoot();

    /// Get the class name
    static std::string getClassName () { return "CLink"; }

  private:

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CRoot_HH
