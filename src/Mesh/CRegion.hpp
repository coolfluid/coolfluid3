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
///   - mesh connectivity table, assuming same element type
///   - element type
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
  CRegion::Ptr create_region ( const CName& name );
  
  /// create a CTable component
  /// @param name of the region
  boost::shared_ptr<CTable> create_connectivityTable ( const CName& name = "table");
  
  /// create a CElements component
  /// @param name of the region
  boost::shared_ptr<CElements> create_elementType ( const CName& name = "type");
  
  CTable::ConstPtr get_connectivityTable( const CName& name = "table" ) const
  {
    if (count(filtered_range(*this,IsComponentName(name))))
      return boost::dynamic_pointer_cast<CTable const>(get_child(name));
    return CTable::ConstPtr();
  }

  CTable::Ptr get_connectivityTable( const CName& name = "table" )
  {
    if (count(filtered_range(*this,IsComponentName(name))))
      return boost::dynamic_pointer_cast<CTable>(get_child(name));
    return CTable::Ptr();
  }

  CElements::ConstPtr get_elementType( const CName& name = "type" ) const
  {
    if (count(filtered_range(*this,IsComponentName(name))))
      return boost::dynamic_pointer_cast<CElements const>(get_child(name));
    return CElements::ConstPtr();
  }

  CElements::Ptr get_elementType( const CName& name = "type" )
  {
    if (count(filtered_range(*this,IsComponentName(name))))
      return boost::dynamic_pointer_cast<CElements>(get_child(name));
    return CElements::Ptr();
  }

  /// a shortcut command to avoid boilerplate code
  /// @param [in] etype_name create a region with connectivity table and element info
  CRegion::Ptr create_element_region (const std::string& etype_name );
  
  /// copy a CArray entry from a given CArray into a given row
  /// templated with row vector type
  /// @param row      out   the row
  /// @param iElem    in    the rowindex in the connectivity table
  /// @param iNode    in    the columnindex in the connectivity table
  /// @param cArray   in    the CArray which data will be copied from
  template<typename vectorType>
  void set_row(vectorType& row, const Uint iElem, const Uint iNode, CArray& cArray)
  { 
    const Uint nbCols = cArray.array().shape()[1];
    for (Uint j=0; j<nbCols; ++j) 
    {
      const Uint row_in_array = get_connectivityTable()->table()[iElem][iNode];
      row[j] = cArray.array()[row_in_array][j];
    }
  }
  
  /// @return a mutable row from the connectivity table, i.e. the node indices of a single element
  CTable::Row get_row(const Uint element) {
    return (*get_connectivityTable())[element];
  }

  /// @return a row from the connectivity table, i.e. the node indices of a single element
  CTable::ConstRow get_row(const Uint element) const {
    return (*get_connectivityTable())[element];
  }

  /// @return the type of the elements in this region
  /// Precondition: Region must have elements
  const ElementType& elements_type() const
  {
    cf_assert(elements_count());
    CElements::ConstPtr eType = get_elementType();
    return *(eType->get_elementType());
  }

  /// @return the number of elements stored in this region, excluding any subregions
  Uint elements_count() const
  {
    CTable::ConstPtr connTable = get_connectivityTable();
    return connTable.get() ? connTable->table().size() : 0;
  }
  
  /// @return the number of elements stored in this region, excluding any subregions
  Uint recursive_elements_count() const
  {
    Uint elem_count = elements_count();
    BOOST_FOREACH(const CRegion& region, recursive_filtered_range_typed<CRegion>(*this,IsComponentTrue()))
    {
      elem_count += region.elements_count();
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
