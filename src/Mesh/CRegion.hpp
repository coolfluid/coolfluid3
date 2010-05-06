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

class CRegion;
class Mesh_API CRegion_iterator
        : public boost::iterator_facade<CRegion_iterator, 
                                        CRegion, 
                                        boost::forward_traversal_tag> 
{
public:
        CRegion_iterator() 
        {}

private:
        friend class boost::iterator_core_access;
        friend class CRegion;

        explicit CRegion_iterator(std::vector<boost::shared_ptr<CRegion> >& vec)
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
            m_region = boost::shared_ptr<CRegion>();
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

        boost::shared_ptr<CRegion> m_region;
        std::vector<boost::shared_ptr<CRegion> > m_vec;
        std::vector<boost::shared_ptr<CRegion> >::iterator m_vecIt;
};

//////////////////////////////////////////////////////////////////////////////

/// Region component class
/// This class stores
///   - subregions (same class)
///   - mesh connectivity table, assuming same element type
///   - element type
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API CRegion : public Common::Component {

public:
    
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

  // functions specific to the CRegion component

  /// create a CRegion component
  /// @param name of the region
  void create_region ( const CName& name );
  
  /// create a CTable component
  /// @param name of the region
  void create_connectivityTable ( const CName& name = "table");
  
  /// create a CElements component
  /// @param name of the region
  void create_elementType ( const CName& name = "type");
  
  /// a shortcut command to avoid boilerplate code
  /// @param [in] etype_name create a region with connectivity table and element info
  void create_region_with_elementType ( const CName& etype_name );
  
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
  
  CArray::Row get_row(const Uint iElem, const Uint iNode, boost::shared_ptr<CArray>& cArray)
  {
    const Uint row_in_array = m_connTable->get_table()[iElem][iNode];
    return cArray->get_array()[row_in_array];
  }
  
  void put_subregions(std::vector<boost::shared_ptr<CRegion> >& vec);  
  
  Iterator begin() 
   {
      std::vector<boost::shared_ptr<CRegion> > vec;
      put_subregions(vec);
      return Iterator(vec);
   }

   Iterator end() 
   {
     std::vector<boost::shared_ptr<CRegion> > vec;
     return Iterator(vec);
   }
   
   bool has_subregions() {return m_subregions.size(); }
  
private:
  
  std::vector< boost::shared_ptr<CRegion> > m_subregions;
  
  boost::shared_ptr<CTable> m_connTable;
  
  boost::shared_ptr<CElements> m_elementType;

};

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_hpp
