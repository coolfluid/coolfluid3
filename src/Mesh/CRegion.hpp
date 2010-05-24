#ifndef CF_Mesh_CRegion_hpp
#define CF_Mesh_CRegion_hpp

////////////////////////////////////////////////////////////////////////////////

# include <boost/iterator/iterator_facade.hpp>

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

class CRegion_iterator;

/// Region component class
/// This class stores
///   - subregions (same class)
///   - mesh connectivity table, assuming same element type
///   - element type
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API CRegion : public Common::Component {

public:

  typedef boost::shared_ptr<CRegion> Ptr;

  typedef CRegion_iterator                       Iterator;

  // required for boost_foreach
  typedef Iterator                                iterator;
  typedef const iterator                          const_iterator;
    
    
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
  void set_row(vectorType& row, const Uint iElem, const Uint iNode, boost::shared_ptr<CArray>& cArray)
  { 
    const Uint nbCols = cArray->get_array().shape()[1];
    for (Uint j=0; j<nbCols; ++j) 
    {
      const Uint row_in_array = m_connTable->get_table()[iElem][iNode];
      row[j] = cArray->get_array()[row_in_array][j];
    }
  }
  
  /// Returns a mutable row view corresponding to node iNode in element iElem
  CArray::Row get_row(const Uint iElem, const Uint iNode, CArray::Ptr& cArray)
  {
    const Uint row_in_array = m_connTable->get_table()[iElem][iNode];
    return cArray->get_array()[row_in_array];
  }
  
  /// Returns a constant row view corresponding to node iNode in element iElem
  CArray::ConstRow get_row(const Uint iElem, const Uint iNode, const CArray& cArray) const
  {
    const Uint row_in_array = m_connTable->get_table()[iElem][iNode];
    return cArray.get_array()[row_in_array];
  }

  void filter_subregions(std::vector<CRegion::Ptr >& vec);
  
  /// Return the element type for this region
  /// Precondition: Region must have elements
  const ElementType& getElementType() const
  {
    cf_assert(getNbElements());
    return *(m_elementType->get_elementType());
  }

  /// Return the number of elements stored in this region, excluding any subregions
  Uint getNbElements() const
  {
    return m_connTable.get() ? m_connTable->get_table().size() : 0;
  }

  Iterator begin();

  Iterator end();

  /// @todo temporary until search by type is in place
  virtual std::string type() const{ return "CRegion"; }

  struct ConstElementNodeVector;

  /// Provide a mutable view of the nodes of a single element, offering operator[] and size() functions
  /// Copying this creates a shallow copy and modifying a copy modifies the original coordinate data
  struct ElementNodeVector
  {
    ElementNodeVector() {}
    ElementNodeVector(const Uint iElem, const Uint nbNodes, CArray& coordinates, const CTable& connectivity);
    Uint size() const;
    CArray::ConstRow operator[](const Uint idx) const;
    CArray::Row operator[](const Uint idx);
  private:
    friend class ConstElementNodeVector; // Allows construction of the const version from the mutable version
    struct Data;
    boost::shared_ptr<Data> m_data;
  };

  /// Provide a read-only view of the nodes of a single element, offering operator[] and size() functions
  /// Copying this creates a shallow copy that refers to the original coordinate data
  struct ConstElementNodeVector
  {
    ConstElementNodeVector() {}
    ConstElementNodeVector(const Uint iElem, const Uint nbNodes, const CArray& coordinates, const CTable& connectivity);
    ConstElementNodeVector(const ElementNodeVector& elementNodeVector);
    Uint size() const;
    CArray::ConstRow operator[](const Uint idx) const;
  private:
    struct Data;
    boost::shared_ptr<Data> m_data;
  };

  /// Return a mutable view of the nodes for the given element.
  ElementNodeVector getNodes(const Uint iElem, CArray& coordinates)
  {
    return ElementNodeVector(iElem, getElementType().getNbNodes(), coordinates, *m_connTable);
  }

  /// Return a read-only view of the nodes for the given element.
  ConstElementNodeVector getNodes(const Uint iElem, const CArray& coordinates) const
  {
    return ConstElementNodeVector(iElem, getElementType().getNbNodes(), coordinates, *m_connTable);
  }

private:
  
  std::vector< CRegion::Ptr > m_subregions;
  boost::shared_ptr<CTable> m_connTable;
  
  boost::shared_ptr<CElements> m_elementType;
};


//////////////////////////////////////////////////////////////////////////////


class CRegion;
class Mesh_API CRegion_iterator
        : public boost::iterator_facade<CRegion_iterator,
                                        CRegion,
                                        boost::forward_traversal_tag>
{
public:
  CRegion_iterator()
  {}

  CRegion::Ptr& get_ptr()
  {
    return m_region;
  }

private:
  friend class boost::iterator_core_access;
  friend class CRegion;

  explicit CRegion_iterator(std::vector<CRegion::Ptr >& vec)
          : m_vec(vec), m_vecIt(m_vec.begin())
  {
    if (m_vec.size()) {
      m_region = vec[0];
    }
  }

  void increment()
  {
    m_vecIt++;
    if (m_vecIt != m_vec.end()) {
      m_region = (*m_vecIt);
    }
    else {
      m_region = CRegion::Ptr();
    }
  }

  bool equal(CRegion_iterator const& other) const
  {
    return m_region == other.m_region;
  }

  CRegion& dereference() const
  {
    return *m_region;
  }

  CRegion::Ptr m_region;
  std::vector<CRegion::Ptr > m_vec;
  std::vector<CRegion::Ptr >::iterator m_vecIt;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_hpp
