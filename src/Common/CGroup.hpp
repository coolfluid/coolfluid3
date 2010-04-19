#ifndef CF_Common_CGroup_HH
#define CF_Common_CGroup_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Component class for tree root
  /// @author Tiago Quintino
  class Common_API CGroup : public Component {

  public:

    /// Contructor
    /// @param name of the component
    CGroup ( const CName& name, const CPath& parent_path = CPath() );

    /// Virtual destructor
    virtual ~CGroup();

    /// Get the class name
    static std::string getClassName () { return "CGroup"; }

  private:

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CGroup_HH
