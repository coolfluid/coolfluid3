#ifndef CF_Common_Component_hpp
#define CF_Common_Component_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/enable_shared_from_this.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/DynamicObject.hpp"
#include "Common/CPath.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Common/XML.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

  /// Base class for defining CF components
  /// @author Tiago Quintino
  /// @todo remove XMLNode and use RapidXML
  /// @todo add ownership of (sub) components
  /// @todo add dumping of the (sub)tree to a string
  /// @todo add registration into CTree
  class Common_API Component :
      public boost::enable_shared_from_this<Component>,
      public ConfigObject,
      public DynamicObject {

  public: // typedef

    /// typedef of pointers to components
    typedef boost::shared_ptr<Component> Ptr;
    /// type for names of components
    typedef std::string CName;
    /// type of this class contruction provider
    typedef Common::ConcreteProvider < Component, NB_ARGS_1 > PROVIDER;
    /// type of first argument of constructor
    typedef const CName& ARG1;

  private: // typedef

    /// type for storing the sub components
    typedef std::map < CName , Component::Ptr > CompStorage_t;

  public: // functions

    /// Get the class name
    static std::string getClassName () { return "Component"; }

    /// Configuration Options
    static void defineConfigOptions ( Common::OptionList& options ) {}

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

    /// @todo this is temporary until search by typeid is in place
    virtual std::string type () const { return "undefined"; }

    /// @todo this is temporary until search by typeid is in place
    template <typename T>
    std::vector<boost::shared_ptr<T> > get_components_of_type ()
    {
      std::string type = DEMANGLED_TYPEID(T);
      std::vector<boost::shared_ptr<T> > vec;
      for(CompStorage_t::iterator it=m_components.begin(); it!=m_components.end(); ++it)
      {
        if (it->second->type() == type)
          vec.push_back(boost::dynamic_pointer_cast<T>(it->second));
      }
      return vec;
    }

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

    /// Create a (sub)component of this component automatically cast to the specified type
    template < typename TYPE >
        boost::shared_ptr<TYPE> create_component ( const CName& name );

    /// Add a (sub)component of this component
    void add_component ( Component::Ptr subcomp );

    /// Remove a (sub)component of this component
    Component::Ptr remove_component ( const CName& name );

    /// Move this component to within another one
    /// @param new_parent will be the new parent of this component
    void move_component ( Component::Ptr new_parent );

    /// Get a (sub)component of this component
    /// @param name the component
    Component::Ptr get_component ( const CName& name );

    /// Check if a (sub)component of this component exists
    /// @param name the component
    bool check_component ( const CName& name );

    /// Get a (sub)component of this component automatically cast to the specified type
    /// @param name the component
    template < typename TYPE >
        boost::shared_ptr<TYPE> get_component ( const CName& name );

    /// Return the parent component
    Component::Ptr get_parent() { return m_parent.lock(); }

    /// Looks for a component via its path
    /// (wdeconinck: relative path doesn't work if no root is available)
    /// @param path to the component
    Component::Ptr look_component ( const CPath& path );

    /// Resolves relative elements within a path to complete it.
    /// The path may be relative to this component or absolute.
    /// This is strictly a path operation so the path may not actually point anywhere
    /// @param path to a component
    /// @post path statisfies CPath::is_complete()
    /// @post path statisfies CPath::is_absolute()
    void complete_path ( CPath& path );

    /// add tag to this component
    void add_tag(const std::string& tag);

    /// @return raw tags
    const std::string& get_raw_tags() const { return m_tags; }

    /// @name SIGNALS 
    //@{
   
    /// creates a component from this component
    void create_component ( XmlNode& xml );

    /// lists the sub components and puts them on the xml_tree
    void list_tree ( XmlNode& xml );

    /// lists the options of this component
    void list_options ( XmlNode& xml );

    //@} END SIGNALS 
  
  protected: // functions

    /// Must be called in constructor of each derived class
   template <typename TYPE>
    void build_component(TYPE* meself);

  private: // helper functions

   /// tags this class with the classname
    template <typename TYPE>
        void tag_classname ();

  protected: // data

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
    /// tags merged as one string e.g. ":Component:CRoot:"
    std::string m_tags;

  }; // Component

////////////////////////////////////////////////////////////////////////////////

template <typename TYPE>
inline void Component::build_component(TYPE* meself)
{
  addConfigOptionsTo<TYPE>();
  tag_classname<TYPE>();
}

template <typename TYPE>
inline void Component::tag_classname ()
{
  add_tag(TYPE::getClassName());
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE >
inline boost::shared_ptr<TYPE> Component::get_component ( const CName& name )
{
  return boost::dynamic_pointer_cast<TYPE>( get_component(name) );
}

////////////////////////////////////////////////////////////////////////////////

template < typename TYPE >
inline boost::shared_ptr<TYPE> Component::create_component ( const CName& name )
{
  boost::shared_ptr<TYPE> new_component ( new TYPE(name), Deleter<TYPE>() );
  add_component(new_component);
  return new_component;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_Component_hpp
