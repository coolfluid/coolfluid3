#ifndef CF_Common_Component_hpp
#define CF_Common_Component_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

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

    /// typedef of pointers to components
    typedef boost::shared_ptr<Component> Ptr;

    /// type for names of components
    typedef std::string CName;

  public:

    /// Contructor
    /// @param name of the component
    /// @param parent path where this component will be placed
    Component ( const CName& name );

    /// Virtual destructor
    virtual ~Component();

    /// Get the componment throught the links to the actual components
    virtual Component::Ptr  get ();

    /// checks if this component is in fact a link to another component
    bool is_link () const { return m_is_link; }

    /// Access the name of the component
    CName name () const { return m_name.string(); }

    /// Rename the component
    void rename ( const CName& name );

    /// Access the path of the component
    const CPath& path () const { return m_path; }

    /// Modify the parent of this component
    void change_parent ( boost::shared_ptr<Component> new_parent );

    /// Construct the full path
    CPath full_path () const { return m_path / m_name; }

    /// Add a (sub)component of this component
    void add_component ( boost::shared_ptr<Component> subcomp );

    /// Get a (sub)component of this component
    /// @param name the component
    Component::Ptr get_component ( const CName& name );

    /// Get a (sub)component of this component automatically cast to the specified type
    /// @param name the component
    template < typename TYPE >
        boost::shared_ptr<TYPE> get_component ( const CName& name );

    /// Looks for a component via its path
    /// @param path to the component
    Component::Ptr look_component ( const CPath& path );

    /// Resolves relative elements within a path to complete it.
    /// The path may be relative to this component or absolute.
    /// This is strictly a path operation so the path may not actually point anywhere
    /// @param path to a component
    /// @post path statisfies CPath::is_complete()
    /// @post path statisfies CPath::is_absolute()
    void complete_path ( CPath& path );

    /// lists the sub components and puts them on the xml_tree
    void xml_tree ( XMLNode xml );

    /// lists the options of this component
    void list_options ( XMLNode xml );
        
  private:

    /// type for storing the sub components
    typedef std::map < CName , Component::Ptr > CompStorage_t;

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

template < typename TYPE >
inline boost::shared_ptr<TYPE> Component::get_component ( const CName& name )
{
  return boost::dynamic_pointer_cast<TYPE>( m_components[name] );
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Component_hpp
