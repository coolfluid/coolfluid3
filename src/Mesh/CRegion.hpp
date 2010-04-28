#ifndef CF_Mesh_CRegion_HH
#define CF_Mesh_CRegion_HH

////////////////////////////////////////////////////////////////////////////////

# include <boost/iterator/iterator_facade.hpp>

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
    
  /// Contructor
  /// @param name of the component
  CRegion ( const CName& name );

  /// Virtual destructor
  virtual ~CRegion();

  /// Get the class name
  static std::string getClassName () { return "CRegion"; }

  // functions specific to the CRegion component

  /// create a CRegion component
  /// @param name of the region
  void create_region ( const CName& name );
  
  /// create a CTable component
  /// @param name of the region
  void create_connectivityTable ( const CName& name );
  
  /// create a CElements component
  /// @param name of the region
  void create_elementType ( const CName& name );
  
  /// copy a CArray entry from a given CArray into a given row
  /// templated with row vector type
  /// @param row      out   the row
  /// @param iElem    in    the rowindex in the connectivity table
  /// @param iNode    in    the columnindex in the connectivity table
  /// @param cArray   in    the CArray which data will be copied from
  template<typename vectorType>
  void set_row(vectorType& row, const Uint iElem, const Uint iNode, Common::SafePtr<CArray>& cArray)
  { 
    const Uint nbCols = cArray->get_array().shape()[1];
    for (Uint j=0; j<nbCols; ++j) 
    {
      const Uint row_in_array = m_connTable->get_table()[iElem][iNode];
      row[j] = cArray->get_array()[row_in_array][j];
    }
  }
  
  CArray::Row get_row(const Uint iElem, const Uint iNode, Common::SafePtr<CArray>& cArray)
  {
    const Uint row_in_array = m_connTable->get_table()[iElem][iNode];
    return cArray->get_array()[row_in_array];
  }
  
  void put_subregions(std::vector< boost::shared_ptr<CRegion> >& vec);  
  
  class Mesh_API iterator
  : public boost::iterator_facade<
  iterator
  , CRegion
  , boost::forward_traversal_tag
  >
  {
  public:
    iterator()
    : m_region()
    {}
    
    explicit iterator(std::vector<boost::shared_ptr<CRegion> >& vec, boost::shared_ptr<Component> parent);    
    
    void increment();
    
  private:
    
    friend class boost::iterator_core_access;
    
    bool equal(iterator const& other) const
    {
      return this->m_region == other.m_region;
    }
    
    CRegion& dereference() const { return *m_region; }
    
    std::vector<boost::shared_ptr<CRegion> > m_vec;
    std::vector<boost::shared_ptr<CRegion> >::iterator m_vecIt;
    boost::shared_ptr<CRegion> m_region;
    boost::shared_ptr<Component> m_parent;
  };
  
  iterator begin();
  
  iterator end();
    
private:
  
  std::vector< boost::shared_ptr<CRegion> > m_subregions;
  
  boost::shared_ptr<CTable> m_connTable;
  
  boost::shared_ptr<CElements> m_elementType;

};

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_HH
