#ifndef CF_Mesh_CRegion_hpp
#define CF_Mesh_CRegion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {
  
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

  /// Contructor
  /// @param name of the component
  CRegion ( const CName& name );

  /// Virtual destructor
  virtual ~CRegion();

  /// Get the class name
  static std::string getClassName () { return "CRegion"; }

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
  
  /// a shortcut command to avoid boilerplate code
  /// @param [in] etype_name create a region with connectivity table and element info
  CRegion::Ptr create_leaf_region (const std::string& etype_name );
  
  /// copy a CArray entry from a given CArray into a given row
  /// templated with row vector type
  /// @param row      out   the row
  /// @param iElem    in    the rowindex in the connectivity table
  /// @param iNode    in    the columnindex in the connectivity table
  /// @param cArray   in    the CArray which data will be copied from
  template<typename vectorType>
  void set_row(vectorType& row, const Uint iElem, const Uint iNode, CArray& cArray)
  { 
    const Uint nbCols = cArray.get_array().shape()[1];
    for (Uint j=0; j<nbCols; ++j) 
    {
      const Uint row_in_array = m_connTable->get_table()[iElem][iNode];
      row[j] = cArray.get_array()[row_in_array][j];
    }
  }
  
  /// @return a mutable row from the connectivity table, i.e. the node indices of a single element
  CTable::Row get_row(const Uint element) {
    return (*m_connTable)[element];
  }

  /// @return a row from the connectivity table, i.e. the node indices of a single element
  CTable::ConstRow get_row(const Uint element) const {
    return (*m_connTable)[element];
  }

  /// @return the type of the elements in this region
  /// Precondition: Region must have elements
  const ElementType& elements_type() const
  {
    cf_assert(elements_count());
    return *(m_elementType->get_elementType());
  }

  /// @return the number of elements stored in this region, excluding any subregions
  Uint elements_count() const
  {
    return m_connTable.get() ? m_connTable->get_table().size() : 0;
  }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private:
  
  std::vector< CRegion::Ptr > m_subregions;
  boost::shared_ptr<CTable> m_connTable;
  
  boost::shared_ptr<CElements> m_elementType;

}; // CRegion

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_hpp
