#ifndef CF_Mesh_CElements_hpp
#define CF_Mesh_CElements_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {

  class GeoShape;

////////////////////////////////////////////////////////////////////////////////

/// CElements component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CElements : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CElements> Ptr;
  typedef boost::shared_ptr<CElements const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CElements ( const CName& name );
  
  /// Initialize the CElements using the given type
  void initialize(const std::string& element_type_name, CArray& data);
  
  /// Virtual destructor
  virtual ~CElements();

  /// Get the class name
  static std::string type_name () { return "CElements"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  /// set the element type
  void set_element_type(const std::string& etype_name);

  /// return the elementType
  const ElementType& element_type() const { return *m_element_type; }
  
  /// return the number of elements
  Uint elements_count() const { return connectivity_table().size(); }
  
  /// create a CTable component and add it to the list of subcomponents
  /// @param name of the region
  CTable& create_connectivity_table ( const CName& name = "connectivity_table");
  
  /// Mutable access to the connectivity table
  CTable& connectivity_table();
  
  /// Const access to the connectivity table
  const CTable& connectivity_table() const;
    
  /// Mutable access to the coordinates
  virtual CArray& coordinates();
  
  /// Const access to the coordinates
  virtual const CArray& coordinates() const;
  
  void add_field_elements_link(CElements& field_elements);
  
  CElements& get_field_elements(const CName& field_name);
    
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

protected: // data
  
  boost::shared_ptr<ElementType> m_element_type;
    
};

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}
  
  bool operator()(const CElements::Ptr& component)
  { return component->element_type().dimension() == component->element_type().dimensionality(); }
  
  bool operator()(const CElements& component)
  { return component.element_type().dimension() == component.element_type().dimensionality(); }
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}
  
  bool operator()(const CElements::Ptr& component)
  { return component->element_type().dimension() == component->element_type().dimensionality() + 1; }
  
  bool operator()(const CElements& component)
  { return component.element_type().dimension() == component.element_type().dimensionality() + 1; }
};
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CElements_hpp
