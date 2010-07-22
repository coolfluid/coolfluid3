#ifndef CF_Mesh_CRegion_hpp
#define CF_Mesh_CRegion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ComponentPredicates.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {
  
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
  CElements& create_elements (const std::string& element_type_name);
  
  /// @return the number of elements stored in this region, excluding any subregions
  Uint recursive_elements_count() const
  {
    Uint elem_count = 0;
    BOOST_FOREACH(const CElements& elements, recursive_range_typed<CElements>(*this))
    {
      elem_count += elements.connectivity_table().table().size();
    }
    return elem_count;
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

//  std::vector< CRegion::Ptr > m_subregions;
//  boost::shared_ptr<CTable> m_connTable;

//  boost::shared_ptr<CElements> m_elementType;

}; // CRegion

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_hpp
