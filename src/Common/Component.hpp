#ifndef CF_Common_Component_HH
#define CF_Common_Component_HH

////////////////////////////////////////////////////////////////////////////////

#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Common/SafePtr.hpp"
#include "Common/DynamicObject.hpp"
#include "Common/CPath.hpp"

#include "Common/xmlParser.h"  // move to stand-alone function

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Base class for defining CF components
  /// @author Tiago Quintino
  /// @todo add ownership of (sub) components
  /// @todo add dumping of the (sub)tree to a string
  /// @todo add registration into CTree
  class Common_API Component :
      public boost::enable_shared_from_this<Component>,
      public DynamicObject {

  public:

    /// type for names of components
    typedef std::string CName;

  public:

    /// Contructor
    /// @param name of the component
    /// @param parent path where this component will be placed
    Component ( const CName& name, const CPath& parent_path = CPath() );

    /// Virtual destructor
    virtual ~Component();

    /// Get the componment throught the links to the actual components
    virtual Common::SafePtr<Component>  get ();

    /// checks if this component is in fact a link to another component
    bool is_link () const { return m_is_link; }

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
    void add_component ( boost::shared_ptr<Component> subcomp );

    /// lists the sub components and puts them on the xml_tree
    void xml_tree ( XMLNode xml );

  private:

    /// type for storing the sub components
    typedef std::map < CName , boost::shared_ptr<Component> > CompStorage_t;

  protected:

    /// component name (stored as path to ensure validity)
    CPath m_name;
    /// component current path
    CPath m_path;
    /// list of children
    CompStorage_t m_components;
    /// pointer to the root of this tree
    boost::weak_ptr<Component> m_root;
    /// pointer to the parent component
    boost::weak_ptr<Component> m_parent;
    /// is this a link component
    bool m_is_link;

  };

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Component_HH
