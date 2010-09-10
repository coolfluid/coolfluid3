#ifndef CF_Mesh_CRegion_hpp
#define CF_Mesh_CRegion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ComponentPredicates.hpp"

#include "Mesh/MeshAPI.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {
  
  class CField; 
  class CTable;
  
  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

/// Region component class
/// This class stores
///   - subregions (same class)
///   - element sets (CElements)
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API CRegion : public Common::Component {

public:

  typedef boost::shared_ptr<CRegion> Ptr;
  typedef boost::shared_ptr<CRegion const> ConstPtr;

  /// Contructor
  /// @param name of the component
  CRegion ( const CName& name );

  /// Virtual destructor
  virtual ~CRegion();

  /// Get the class name
  static std::string type_name () { return "CRegion"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CRegion component

  /// create a CRegion component
  /// @param name of the region
  CRegion& create_region ( const CName& name );
  
  /// create a CElements component, initialized to take connectivity data for the given type
  /// @param name of the region
  /// @param element_type_name type of the elements
  CElements& create_elements (const std::string& element_type_name, CArray& coordinates);
  
  /// create a coordinates component, initialized with the coordinate dimension
  /// @param name of the region
  /// @param element_type_name type of the elements  
  CArray& create_coordinates(const Uint& dim);
  
  void add_field_link(CField& field);

  /// @return the number of elements stored in this region, including any subregions
  Uint recursive_elements_count() const;

  /// @return the number of elements stored in this region, including any subregions
  template <typename Predicate>
    Uint recursive_filtered_elements_count(const Predicate& pred) const;

  Uint recursive_nodes_count() const;

  /// @return the number of elements stored in this region, including any subregions
  template <typename Predicate>
    Uint recursive_filtered_nodes_count(const Predicate& pred) const;
  
  
  CField& get_field(const CName& field_name);
  
  /// @return the subregion with given name
  const CRegion& subregion(const CName& name) const;
  
  /// @return the subregion with given name
  CRegion& subregion(const CName& name);
  
  /// @return the elements with given name
  const CElements& elements (const CName& element_type_name) const;
  
  /// @return the elements with given name
  CElements& elements (const CName& element_type_name);
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

}; // CRegion

////////////////////////////////////////////////////////////////////////////////

template <typename Predicate>
inline Uint CRegion::recursive_filtered_elements_count(const Predicate& pred) const
{
  Uint elem_count = 0;
  BOOST_FOREACH(const CElements& elements, recursive_filtered_range_typed<CElements>(*this,pred))
    elem_count += elements.elements_count();

  return elem_count;
}
  
template <typename Predicate>
inline Uint CRegion::recursive_filtered_nodes_count(const Predicate& pred) const
{
  std::set<const CArray*> coordinates_set;
  BOOST_FOREACH(const CElements& elements, recursive_filtered_range_typed<CElements>(*this,pred))
  coordinates_set.insert(&elements.coordinates());
  
  // Total number of nodes in the mesh
  Uint nb_nodes = 0;
  BOOST_FOREACH(const CArray* coordinates, coordinates_set)
    nb_nodes += coordinates->size();
  
  return nb_nodes;
}
  
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_hpp
