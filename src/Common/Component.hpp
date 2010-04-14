#ifndef CF_Common_Component_HH
#define CF_Common_Component_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/CPath.hh"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Base class for defining CF components
  ///
  /// @author Tiago Quintino
  ///
  /// @todo add ownership of (sub) components
  /// @todo add dumping of the (sub)tree to a string
  /// @todo add registration into CTree
  class Common_API Component {

  public:

    /// type for names of components
    typedef std::string CName;

  public:

    /// Contructor
    /// @param name of the component
    /// @param parent path where this component will be placed
    Component ( const CName& name, const CPath& parent_path = CPath() );

    /// Virtual contructor
    virtual ~Component();

    /// Access the name of the component
    const CName& name () const { return m_name.string(); }
    /// Rename the component
    void rename ( const CName& name );

    /// Access the path of the component
    const CPath& path () const { return m_path; }
    /// Modify the path of the component
    void change_path ( const CPath& new_path );
    /// Construct the full path
    CPath full_path () const { return m_path / m_name; }

    /// Add a (sub)component of this component
    void add_component ( Component * subcomp );

  private:

    /// component name (stored as path to ensure validity)
    CPath m_name;
    /// component current path
    CPath m_path;
    /// list of children
    std::map < CName , Component* > m_components;

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Component_HH
